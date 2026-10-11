// S-100 Part 13 / Part 9a host functions for the official PC Lua framework.
//
// Every function the PC framework calls is registered per product in a HostRegistry
// (docs/portrayal-contract.md rule 7). Anything unknown — unknown IDs, unknown codes, Host*
// names nobody registered (see portrayal_engine.cpp strict _G) — raises a Lua error; nothing
// silently returns nil.
//
// Host-side quirks (catalogue files stay untouched, rule 1):
//  * Empty ATVL (S-101 "unknown value") is returned as the Part 13 unknown-value string so the
//    framework yields unknownValue instead of a broken ScaledDecimal.
//  * HostInformationTypeGetSimpleAttribute returns {} (not nil, as 13-8.2.1.15 allows) when
//    absent: PortrayalAPI.lua LookupAttributeValue applies '#' to the result.
//  * HostGetSimpleAttribute / HostGetComplexAttributeCount are not Part 13 names; they are the
//    parameter names LookupAttributeValue uses for the feature/information variants. They are
//    registered (tester list) as dispatchers over the same attribute lookup.
#include "s100/host_adapter.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <functional>

// Lua is built as C++ (third_party/CMakeLists.txt): plain headers, C++ linkage.
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "s100/dataset.hpp"
#include "s100/feature_catalogue.hpp"

namespace s100 {

// ---------------------------------------------------------------- registry

void HostRegistry::add(const std::string& name, HostFunction fn) { entries_[name] = Entry{name, fn}; }

void HostRegistry::add_alias(const std::string& alias, const std::string& target) {
  auto it = entries_.find(target);
  if (it == entries_.end()) return;  // programming error; has() in tests will catch it
  entries_[alias] = Entry{it->second.target, it->second.fn};
}

std::vector<std::string> HostRegistry::names() const {
  std::vector<std::string> v;
  for (const auto& [n, e] : entries_) v.push_back(n);
  return v;
}

bool HostRegistry::is_alias(const std::string& name) const {
  auto it = entries_.find(name);
  return it != entries_.end() && it->second.target != name;
}

std::string HostRegistry::target_of(const std::string& name) const {
  auto it = entries_.find(name);
  return it == entries_.end() ? std::string() : it->second.target;
}

HostFunction HostRegistry::function(const std::string& name) const {
  auto it = entries_.find(name);
  return it == entries_.end() ? nullptr : it->second.fn;
}

void HostAdapter::reset_run() {
  emissions.clear();
  trace_errors.clear();
  first_chance_errors = 0;
  warnings = 0;
}

namespace {

const char* kUnknownValue = "13BD40516CF742E886D5B4125DBB89742A043D0050E44B568CBB1FDDA8B464FF";

HostAdapter* self(lua_State* L) {
  return static_cast<HostAdapter*>(lua_touserdata(L, lua_upvalueindex(1)));
}

std::string arg_string(lua_State* L, int i) {
  std::size_t n = 0;
  const char* s = luaL_checklstring(L, i, &n);
  return std::string(s, n);
}

/** nil or '' -> empty (Part 13: roleCode may be nil = any role). */
std::string opt_string(lua_State* L, int i) {
  if (lua_isnoneornil(L, i)) return {};
  std::size_t n = 0;
  const char* s = luaL_checklstring(L, i, &n);
  return std::string(s, n);
}

void push_strings(lua_State* L, const std::vector<std::string>& v) {
  lua_createtable(L, static_cast<int>(v.size()), 0);
  for (std::size_t i = 0; i < v.size(); ++i) {
    lua_pushlstring(L, v[i].data(), v[i].size());
    lua_rawseti(L, -2, static_cast<int>(i + 1));
  }
}

void push_or_nil(lua_State* L, const std::string& s) {
  if (s.empty()) lua_pushnil(L);
  else lua_pushlstring(L, s.data(), s.size());
}

/** Push a framework (catalogue-side) global function, error loudly if the PC lacks it. */
void get_function(lua_State* L, const char* name) {
  lua_getglobal(L, name);
  if (!lua_isfunction(L, -1)) luaL_error(L, "host: framework function %s not defined by the PC", name);
}

template <class M>
std::vector<std::string> keys(const M& m) {
  std::vector<std::string> v;
  for (const auto& kv : m) v.push_back(kv.first);
  return v;
}

// ------------------------------------------------------- attribute lookup (Part 13 13-6.2)

std::vector<std::size_t> siblings(const std::vector<Attribute>& attrs, std::uint16_t parent,
                                  const std::string& code) {
  std::vector<std::size_t> v;
  for (std::size_t i = 0; i < attrs.size(); ++i)
    if (attrs[i].parent == parent && attrs[i].code == code && attrs[i].instruction != 2) v.push_back(i);
  std::stable_sort(v.begin(), v.end(), [&](std::size_t a, std::size_t b) { return attrs[a].index < attrs[b].index; });
  return v;
}

/** Resolve "code:idx;code:idx" to the 1-based position of the complex attribute (0 = top). */
bool resolve_path(const std::vector<Attribute>& attrs, const std::string& path, std::uint16_t& parent) {
  parent = 0;
  std::size_t a = 0;
  while (a < path.size()) {
    std::size_t b = path.find(';', a);
    if (b == std::string::npos) b = path.size();
    const std::string elem = path.substr(a, b - a);
    a = b + 1;
    if (elem.empty()) continue;
    const std::size_t c = elem.find(':');
    if (c == std::string::npos) return false;
    const std::string code = elem.substr(0, c);
    const long idx = std::strtol(elem.c_str() + c + 1, nullptr, 10);
    const auto sib = siblings(attrs, parent, code);
    if (idx < 1 || static_cast<std::size_t>(idx) > sib.size()) return false;
    parent = static_cast<std::uint16_t>(sib[static_cast<std::size_t>(idx - 1)] + 1);
  }
  return true;
}

int push_simple_values(lua_State* L, const std::vector<Attribute>& attrs, const std::string& path,
                       const std::string& code) {
  std::uint16_t parent = 0;
  if (!resolve_path(attrs, path, parent))
    return luaL_error(L, "host: attribute path '%s' not found", path.c_str());
  const auto sib = siblings(attrs, parent, code);
  lua_createtable(L, static_cast<int>(sib.size()), 0);
  int n = 0;
  for (std::size_t i : sib) {
    const std::string& v = attrs[i].value;
    if (v.empty()) lua_pushstring(L, kUnknownValue);
    else lua_pushlstring(L, v.data(), v.size());
    lua_rawseti(L, -2, ++n);
  }
  return 1;
}

int push_complex_count(lua_State* L, const std::vector<Attribute>& attrs, const std::string& path,
                       const std::string& code) {
  std::uint16_t parent = 0;
  if (!resolve_path(attrs, path, parent))
    return luaL_error(L, "host: attribute path '%s' not found", path.c_str());
  lua_pushinteger(L, static_cast<lua_Integer>(siblings(attrs, parent, code).size()));
  return 1;
}

// ------------------------------------------------------- shared: type information (13-8.2.2)

void push_item(lua_State* L, const std::string& code, const std::string& name,
               const std::string& definition, const std::vector<std::string>& alias) {
  get_function(L, "CreateItem");
  lua_pushstring(L, code.c_str());
  lua_pushstring(L, name.c_str());
  lua_pushstring(L, definition.c_str());
  lua_pushnil(L);  // remarks
  if (alias.empty()) lua_pushnil(L);
  else push_strings(L, alias);
  lua_call(L, 5, 1);
}

void push_attribute_bindings(lua_State* L, const std::vector<FcAttributeBinding>& bs) {
  lua_createtable(L, static_cast<int>(bs.size()), 0);
  int n = 0;
  for (const auto& b : bs) {
    get_function(L, "CreateAttributeBinding");
    lua_pushstring(L, b.attribute.c_str());
    lua_pushinteger(L, b.lower);
    if (b.upper < 0) lua_pushnil(L);
    else lua_pushinteger(L, b.upper);
    lua_pushboolean(L, b.sequential);
    lua_createtable(L, static_cast<int>(b.permitted_values.size()), 0);
    for (std::size_t i = 0; i < b.permitted_values.size(); ++i) {
      lua_pushinteger(L, b.permitted_values[i]);
      lua_rawseti(L, -2, static_cast<int>(i + 1));
    }
    lua_call(L, 5, 1);
    lua_rawseti(L, -2, ++n);
  }
}

void push_type_bindings(lua_State* L, const std::vector<FcTypeBinding>& bs, const char* creator) {
  lua_createtable(L, static_cast<int>(bs.size()), 0);
  int n = 0;
  for (const auto& b : bs) {
    get_function(L, creator);
    push_strings(L, b.types);
    lua_pushinteger(L, b.lower);
    if (b.upper < 0) lua_pushnil(L);
    else lua_pushinteger(L, b.upper);
    lua_pushstring(L, b.role_type.c_str());
    lua_pushstring(L, b.role.c_str());
    lua_pushstring(L, b.association.c_str());
    lua_call(L, 6, 1);
    lua_rawseti(L, -2, ++n);
  }
}

struct Common {
  static const FeatureCatalogue& fc(lua_State* L) { return self(L)->catalogue(); }

  static int GetFeatureTypeCodes(lua_State* L) { push_strings(L, keys(fc(L).feature_types)); return 1; }
  static int GetInformationTypeCodes(lua_State* L) { push_strings(L, keys(fc(L).information_types)); return 1; }
  static int GetSimpleAttributeTypeCodes(lua_State* L) { push_strings(L, keys(fc(L).simple_attributes)); return 1; }
  static int GetComplexAttributeTypeCodes(lua_State* L) { push_strings(L, keys(fc(L).complex_attributes)); return 1; }
  static int GetRoleTypeCodes(lua_State* L) { push_strings(L, keys(fc(L).roles)); return 1; }
  static int GetInformationAssociationTypeCodes(lua_State* L) { push_strings(L, keys(fc(L).information_associations)); return 1; }
  static int GetFeatureAssociationTypeCodes(lua_State* L) { push_strings(L, keys(fc(L).feature_associations)); return 1; }

  static int GetFeatureTypeInfo(lua_State* L) {
    const std::string code = arg_string(L, 1);
    const auto* t = fc(L).find(code);
    if (!t) return luaL_error(L, "host: feature type '%s' not in FC %s", code.c_str(), fc(L).version.c_str());
    get_function(L, "CreateFeatureType");
    get_function(L, "CreateObjectType");
    get_function(L, "CreateNamedType");
    push_item(L, t->code, t->name, t->definition, t->alias);
    lua_pushboolean(L, t->is_abstract);
    push_attribute_bindings(L, t->attribute_bindings);
    lua_call(L, 3, 1);  // NamedType
    push_type_bindings(L, t->information_bindings, "CreateInformationBinding");
    lua_call(L, 2, 1);  // ObjectType
    lua_pushstring(L, t->use_type.c_str());
    push_strings(L, t->primitives);
    push_type_bindings(L, t->feature_bindings, "CreateFeatureBinding");
    push_or_nil(L, t->super_type);
    push_strings(L, t->sub_types);
    lua_call(L, 6, 1);  // FeatureType
    return 1;
  }

  static int GetInformationTypeInfo(lua_State* L) {
    const std::string code = arg_string(L, 1);
    auto it = fc(L).information_types.find(code);
    if (it == fc(L).information_types.end())
      return luaL_error(L, "host: information type '%s' not in FC", code.c_str());
    const auto& t = it->second;
    get_function(L, "CreateInformationType");
    get_function(L, "CreateObjectType");
    get_function(L, "CreateNamedType");
    push_item(L, t.code, t.name, t.definition, t.alias);
    lua_pushboolean(L, t.is_abstract);
    push_attribute_bindings(L, t.attribute_bindings);
    lua_call(L, 3, 1);
    push_type_bindings(L, t.information_bindings, "CreateInformationBinding");
    lua_call(L, 2, 1);
    push_or_nil(L, t.super_type);
    push_strings(L, t.sub_types);
    lua_call(L, 3, 1);
    return 1;
  }

  static int GetSimpleAttributeTypeInfo(lua_State* L) {
    const std::string code = arg_string(L, 1);
    auto it = fc(L).simple_attributes.find(code);
    if (it == fc(L).simple_attributes.end())
      return luaL_error(L, "host: simple attribute '%s' not in FC", code.c_str());
    const auto& a = it->second;
    get_function(L, "CreateSimpleAttribute");
    push_item(L, a.code, a.name, a.definition, a.alias);
    lua_pushstring(L, a.value_type.c_str());
    push_or_nil(L, a.uom);
    push_or_nil(L, a.quantity_specification);
    lua_pushnil(L);  // attribute constraints (not used by the PC framework)
    lua_createtable(L, static_cast<int>(a.listed_values.size()), 0);
    int n = 0;
    for (const auto& lv : a.listed_values) {
      get_function(L, "CreateListedValue");
      lua_pushstring(L, lv.label.c_str());
      lua_pushstring(L, lv.definition.c_str());
      lua_pushinteger(L, lv.code);
      lua_call(L, 3, 1);
      lua_rawseti(L, -2, ++n);
    }
    lua_call(L, 6, 1);
    return 1;
  }

  static int GetComplexAttributeTypeInfo(lua_State* L) {
    const std::string code = arg_string(L, 1);
    auto it = fc(L).complex_attributes.find(code);
    if (it == fc(L).complex_attributes.end())
      return luaL_error(L, "host: complex attribute '%s' not in FC", code.c_str());
    get_function(L, "CreateComplexAttribute");
    push_item(L, it->second.code, it->second.name, it->second.definition, it->second.alias);
    push_attribute_bindings(L, it->second.sub_attribute_bindings);
    lua_call(L, 2, 1);
    return 1;
  }

  // Part 9a 9a-14.2.1
  static int PortrayalEmit(lua_State* L) {
    HostEmission e{arg_string(L, 1), opt_string(L, 2), opt_string(L, 3)};
    self(L)->emissions.push_back(std::move(e));
    lua_pushboolean(L, 1);  // continue processing
    return 1;
  }

  // Part 13 13-8.2.4.1 (optional): only error/warning accounting, otherwise a no-op.
  static int DebuggerEntry(lua_State* L) {
    const std::string action = opt_string(L, 1);
    auto* h = self(L);
    if (action == "trace" && lua_isstring(L, 2)) {
      const std::string msg = lua_tostring(L, 2);
      if (msg.rfind("Error", 0) == 0) h->trace_errors.push_back(msg);
      else if (msg.rfind("Warning", 0) == 0) ++h->warnings;
    } else if (action == "first_chance_error") {
      ++h->first_chance_errors;
    }
    return 0;
  }
};

}  // namespace

// =================================================================== S-101

struct S101HostImpl {
  static S101HostAdapter* host(lua_State* L) { return static_cast<S101HostAdapter*>(self(L)); }

  static const Feature& feature(lua_State* L, int i) {
    const std::string id = arg_string(L, i);
    const Feature* f = host(L)->feature(id);
    if (!f) luaL_error(L, "host: unknown feature ID '%s'", id.c_str());
    return *f;
  }
  static const InformationRecord& info(lua_State* L, int i) {
    const std::string id = arg_string(L, i);
    const InformationRecord* r = host(L)->information(id);
    if (!r) luaL_error(L, "host: unknown information ID '%s'", id.c_str());
    return *r;
  }

  static int GetFeatureIDs(lua_State* L) {
    const auto& fs = host(L)->ds_->features;
    lua_createtable(L, static_cast<int>(fs.size()), 0);
    for (std::size_t i = 0; i < fs.size(); ++i) {
      lua_pushstring(L, std::to_string(fs[i].id).c_str());
      lua_rawseti(L, -2, static_cast<int>(i + 1));
    }
    return 1;
  }
  static int FeatureGetCode(lua_State* L) { lua_pushstring(L, feature(L, 1).feature_type.c_str()); return 1; }
  static int InformationTypeGetCode(lua_State* L) { lua_pushstring(L, info(L, 1).information_type.c_str()); return 1; }

  static int FeatureGetSimpleAttribute(lua_State* L) {
    return push_simple_values(L, feature(L, 1).attributes, opt_string(L, 2), arg_string(L, 3));
  }
  static int FeatureGetComplexAttributeCount(lua_State* L) {
    return push_complex_count(L, feature(L, 1).attributes, opt_string(L, 2), arg_string(L, 3));
  }
  static int InformationTypeGetSimpleAttribute(lua_State* L) {
    return push_simple_values(L, info(L, 1).attributes, opt_string(L, 2), arg_string(L, 3));
  }
  static int InformationTypeGetComplexAttributeCount(lua_State* L) {
    return push_complex_count(L, info(L, 1).attributes, opt_string(L, 2), arg_string(L, 3));
  }
  static bool is_info_id(lua_State* L) {
    const char* s = luaL_checkstring(L, 1);
    return s[0] == 'I';
  }
  static int GetSimpleAttribute(lua_State* L) {
    return is_info_id(L) ? InformationTypeGetSimpleAttribute(L) : FeatureGetSimpleAttribute(L);
  }
  static int GetComplexAttributeCount(lua_State* L) {
    return is_info_id(L) ? InformationTypeGetComplexAttributeCount(L) : FeatureGetComplexAttributeCount(L);
  }

  static const char* spatial_type_name(std::uint8_t rcnm) {
    switch (rcnm) {
      case 110: return "Point";
      case 115: return "MultiPoint";
      case 120: return "Curve";
      case 125: return "CompositeCurve";
      case 130: return "Surface";
      default: return nullptr;
    }
  }

  /** CreateSpatialAssociation(spatialType, spatialID, orientation, scaleMinimum, scaleMaximum) */
  static void push_spatial_association(lua_State* L, const SpatialRef& r) {
    const char* type = spatial_type_name(r.rcnm);
    if (!type) luaL_error(L, "host: unsupported spatial record type %d", r.rcnm);
    get_function(L, "CreateSpatialAssociation");
    lua_pushstring(L, type);
    lua_pushstring(L, S101HostAdapter::spatial_id(r.rcnm, r.rcid).c_str());
    lua_pushstring(L, r.reverse ? "Reverse" : "Forward");
    auto scale = [&](std::uint32_t v) {
      if (v == 0 || v == 0xFFFFFFFFu) lua_pushnil(L);
      else lua_pushinteger(L, static_cast<lua_Integer>(v));
    };
    scale(r.scale_min);
    scale(r.scale_max);
    lua_call(L, 5, 1);
  }

  static int FeatureGetSpatialAssociations(lua_State* L) {
    const Feature& f = feature(L, 1);
    lua_createtable(L, static_cast<int>(f.spatial.size()), 0);
    int n = 0;
    for (const auto& r : f.spatial) {
      push_spatial_association(L, r);
      lua_rawseti(L, -2, ++n);
    }
    return 1;
  }

  static const SpatialRecord& spatial(lua_State* L, int i) {
    const std::string sid = arg_string(L, i);
    std::uint8_t rcnm = 0;
    std::uint32_t rcid = 0;
    if (!parse_spatial_id(sid, rcnm, rcid)) luaL_error(L, "host: malformed spatial ID '%s'", sid.c_str());
    const SpatialRecord* r = host(L)->ds_->spatial.find(rcnm, rcid);
    if (!r) luaL_error(L, "host: unknown spatial ID '%s'", sid.c_str());
    return *r;
  }

  static bool parse_spatial_id(const std::string& s, std::uint8_t& rcnm, std::uint32_t& rcid) {
    if (s.size() < 2) return false;
    switch (s[0]) {
      case 'P': rcnm = 110; break;
      case 'M': rcnm = 115; break;
      case 'C': rcnm = 120; break;
      case 'K': rcnm = 125; break;
      case 'S': rcnm = 130; break;
      default: return false;
    }
    char* end = nullptr;
    rcid = static_cast<std::uint32_t>(std::strtoul(s.c_str() + 1, &end, 10));
    return end && *end == 0;
  }

  static void push_point(lua_State* L, const Point3& p, bool with_z) {
    char x[32], y[32], z[32];
    std::snprintf(x, sizeof x, "%.10g", p.lon);
    std::snprintf(y, sizeof y, "%.10g", p.lat);
    get_function(L, "CreatePoint");
    lua_pushstring(L, x);
    lua_pushstring(L, y);
    if (with_z) {
      std::snprintf(z, sizeof z, "%.10g", p.z);
      lua_pushstring(L, z);
    } else {
      lua_pushnil(L);
    }
    lua_call(L, 3, 1);
  }

  static int GetSpatial(lua_State* L) {
    const SpatialRecord& r = spatial(L, 1);
    switch (r.rcnm) {
      case 110:
        if (r.coords.empty()) return luaL_error(L, "host: point P%u has no coordinate", r.rcid);
        push_point(L, r.coords[0], false);
        return 1;
      case 115: {
        get_function(L, "CreateMultiPoint");
        lua_createtable(L, static_cast<int>(r.coords.size()), 0);
        int n = 0;
        for (const auto& p : r.coords) {
          push_point(L, p, true);
          lua_rawseti(L, -2, ++n);
        }
        lua_call(L, 1, 1);
        return 1;
      }
      case 120: {
        if (!r.start_point || !r.end_point)
          return luaL_error(L, "host: curve C%u lacks PTAS begin/end point", r.rcid);
        get_function(L, "CreateCurve");
        push_spatial_association(L, SpatialRef{110, r.start_point, false, 0, 0, 0});
        push_spatial_association(L, SpatialRef{110, r.end_point, false, 0, 0, 0});
        lua_createtable(L, 1, 0);
        get_function(L, "CreateCurveSegment");
        lua_createtable(L, static_cast<int>(r.coords.size()), 0);
        int n = 0;
        for (const auto& p : r.coords) {
          push_point(L, p, false);
          lua_rawseti(L, -2, ++n);
        }
        lua_pushstring(L, "Loxodromic");  // S-101 SEGH INTP 4
        lua_call(L, 2, 1);
        lua_rawseti(L, -2, 1);
        lua_call(L, 3, 1);
        return 1;
      }
      case 125: {
        get_function(L, "CreateCompositeCurve");
        lua_createtable(L, static_cast<int>(r.parts.size()), 0);
        int n = 0;
        for (const auto& p : r.parts) {
          push_spatial_association(L, p);
          lua_rawseti(L, -2, ++n);
        }
        lua_call(L, 1, 1);
        return 1;
      }
      case 130: {
        const SpatialRef* ext = nullptr;
        for (const auto& p : r.parts)
          if (p.usage == 1 && !ext) ext = &p;
        if (!ext) return luaL_error(L, "host: surface S%u has no exterior ring", r.rcid);
        get_function(L, "CreateSurface");
        push_spatial_association(L, *ext);
        lua_newtable(L);
        int n = 0;
        for (const auto& p : r.parts)
          if (&p != ext) {
            push_spatial_association(L, p);
            lua_rawseti(L, -2, ++n);
          }
        lua_call(L, 2, 1);
        return 1;
      }
      default:
        return luaL_error(L, "host: unsupported spatial record type %d", r.rcnm);
    }
  }

  static int SpatialGetAssociatedFeatureIDs(lua_State* L) {
    spatial(L, 1);  // validates
    push_strings(L, host(L)->features_of_spatial(arg_string(L, 1)));
    return 1;
  }

  static bool matches(const AssociationRef& a, const std::string& assoc, const std::string& role) {
    return a.association == assoc && (role.empty() || a.role == role);
  }

  static void push_information_ids(lua_State* L, const std::vector<AssociationRef>& refs,
                                   const std::string& assoc, const std::string& role) {
    std::vector<std::string> ids;
    for (const auto& a : refs)
      if (a.rcnm == 150 && matches(a, assoc, role)) {
        if (!host(L)->info_by_rcid_.count(a.rcid))
          luaL_error(L, "host: INAS references missing information record %u", a.rcid);
        ids.push_back("I" + std::to_string(a.rcid));
      }
    push_strings(L, ids);
  }

  static int SpatialGetAssociatedInformationIDs(lua_State* L) {
    const SpatialRecord& r = spatial(L, 1);
    push_information_ids(L, r.information_associations, arg_string(L, 2), opt_string(L, 3));
    return 1;
  }

  static int FeatureGetAssociatedInformationIDs(lua_State* L) {
    const Feature& f = feature(L, 1);
    push_information_ids(L, f.information_associations, arg_string(L, 2), opt_string(L, 3));
    return 1;
  }

  /** FASC is stored on one side; the other side sees the inverse role (FC association roles). */
  static int FeatureGetAssociatedFeatureIDs(lua_State* L) {
    auto* h = host(L);
    const Feature& f = feature(L, 1);
    const std::string assoc = arg_string(L, 2);
    const std::string role = opt_string(L, 3);
    const auto& fs = h->ds_->features;
    std::vector<std::string> ids;
    auto add = [&](std::size_t idx) {
      const std::string id = std::to_string(fs[idx].id);
      if (std::find(ids.begin(), ids.end(), id) == ids.end()) ids.push_back(id);
    };
    for (const auto& a : f.feature_associations) {
      if (!matches(a, assoc, role)) continue;
      auto it = h->feature_by_rcid_.find(a.rcid);
      if (it == h->feature_by_rcid_.end())
        return luaL_error(L, "host: FASC references missing feature record %u", a.rcid);
      add(it->second);
    }
    // inverse direction
    const auto* fa = [&]() -> const FcNamedItem* {
      auto it = h->catalogue().feature_associations.find(assoc);
      return it == h->catalogue().feature_associations.end() ? nullptr : &it->second;
    }();
    for (std::size_t i = 0; i < fs.size(); ++i) {
      for (const auto& a : fs[i].feature_associations) {
        if (a.rcnm != 100 || a.rcid != f.record_id || a.association != assoc) continue;
        std::string inverse = a.role;
        if (fa && fa->roles.size() == 2) inverse = fa->roles[0] == a.role ? fa->roles[1] : fa->roles[0];
        if (role.empty() || inverse == role) add(i);
      }
    }
    push_strings(L, ids);
    return 1;
  }
};

S101HostAdapter::S101HostAdapter(const Dataset& ds, const FeatureCatalogue& fc)
    : HostAdapter("S-101", fc), ds_(&ds) {
  for (std::size_t i = 0; i < ds.features.size(); ++i) {
    feature_index_[std::to_string(ds.features[i].id)] = i;
    feature_by_rcid_[ds.features[i].record_id] = i;
  }
  for (std::size_t i = 0; i < ds.information.size(); ++i) info_by_rcid_[ds.information[i].record_id] = i;

  // The S-101 PC 2.0.0 set (docs/testing/host-functions.txt [S-101 PC 2.0.0]).
  auto& r = registry_;
  r.add("HostGetFeatureIDs", &S101HostImpl::GetFeatureIDs);
  r.add("HostFeatureGetCode", &S101HostImpl::FeatureGetCode);
  r.add("HostInformationTypeGetCode", &S101HostImpl::InformationTypeGetCode);
  r.add("HostFeatureGetSimpleAttribute", &S101HostImpl::FeatureGetSimpleAttribute);
  r.add("HostFeatureGetComplexAttributeCount", &S101HostImpl::FeatureGetComplexAttributeCount);
  r.add("HostInformationTypeGetSimpleAttribute", &S101HostImpl::InformationTypeGetSimpleAttribute);
  r.add("HostInformationTypeGetComplexAttributeCount", &S101HostImpl::InformationTypeGetComplexAttributeCount);
  r.add("HostGetSimpleAttribute", &S101HostImpl::GetSimpleAttribute);
  r.add("HostGetComplexAttributeCount", &S101HostImpl::GetComplexAttributeCount);
  r.add("HostFeatureGetSpatialAssociations", &S101HostImpl::FeatureGetSpatialAssociations);
  r.add("HostFeatureGetAssociatedFeatureIDs", &S101HostImpl::FeatureGetAssociatedFeatureIDs);
  r.add("HostFeatureGetAssociatedInformationIDs", &S101HostImpl::FeatureGetAssociatedInformationIDs);
  r.add("HostGetSpatial", &S101HostImpl::GetSpatial);
  r.add("HostSpatialGetAssociatedFeatureIDs", &S101HostImpl::SpatialGetAssociatedFeatureIDs);
  r.add("HostSpatialGetAssociatedInformationIDs", &S101HostImpl::SpatialGetAssociatedInformationIDs);
  r.add("HostGetFeatureTypeCodes", &Common::GetFeatureTypeCodes);
  r.add("HostGetInformationTypeCodes", &Common::GetInformationTypeCodes);
  r.add("HostGetSimpleAttributeTypeCodes", &Common::GetSimpleAttributeTypeCodes);
  r.add("HostGetComplexAttributeTypeCodes", &Common::GetComplexAttributeTypeCodes);
  r.add("HostGetRoleTypeCodes", &Common::GetRoleTypeCodes);
  r.add("HostGetInformationAssociationTypeCodes", &Common::GetInformationAssociationTypeCodes);
  r.add("HostGetFeatureAssociationTypeCodes", &Common::GetFeatureAssociationTypeCodes);
  r.add("HostGetFeatureTypeInfo", &Common::GetFeatureTypeInfo);
  r.add("HostGetInformationTypeInfo", &Common::GetInformationTypeInfo);
  r.add("HostGetSimpleAttributeTypeInfo", &Common::GetSimpleAttributeTypeInfo);
  r.add("HostGetComplexAttributeTypeInfo", &Common::GetComplexAttributeTypeInfo);
  r.add("HostPortrayalEmit", &Common::PortrayalEmit);
  r.add("HostDebuggerEntry", &Common::DebuggerEntry);
}

std::size_t S101HostAdapter::feature_count() const { return ds_->features.size(); }

bool S101HostAdapter::feature_id_of(const std::string& lua_id, std::uint64_t& out) const {
  auto it = feature_index_.find(lua_id);
  if (it == feature_index_.end()) return false;
  out = ds_->features[it->second].id;
  return true;
}

const Feature* S101HostAdapter::feature(const std::string& lua_id) const {
  auto it = feature_index_.find(lua_id);
  return it == feature_index_.end() ? nullptr : &ds_->features[it->second];
}

const InformationRecord* S101HostAdapter::information(const std::string& lua_id) const {
  if (lua_id.size() < 2 || lua_id[0] != 'I') return nullptr;
  char* end = nullptr;
  const auto rcid = static_cast<std::uint32_t>(std::strtoul(lua_id.c_str() + 1, &end, 10));
  if (!end || *end) return nullptr;
  auto it = info_by_rcid_.find(rcid);
  return it == info_by_rcid_.end() ? nullptr : &ds_->information[it->second];
}

std::string S101HostAdapter::spatial_id(std::uint8_t rcnm, std::uint32_t rcid) {
  const char* p = rcnm == 110 ? "P" : rcnm == 115 ? "M" : rcnm == 120 ? "C" : rcnm == 125 ? "K" : rcnm == 130 ? "S" : "?";
  return p + std::to_string(rcid);
}

void S101HostAdapter::build_spatial_index() const {
  spatial_index_built_ = true;
  // Part 13 13-8.2.1.14: directly through the feature's spatial associations, or indirectly for
  // curves referenced by composite curves or surface rings.
  for (const auto& f : ds_->features) {
    const std::string fid = std::to_string(f.id);
    std::function<void(std::uint8_t, std::uint32_t, int)> walk = [&](std::uint8_t rcnm, std::uint32_t rcid, int depth) {
      auto& v = spatial_features_[spatial_id(rcnm, rcid)];
      if (v.empty() || v.back() != fid) v.push_back(fid);
      if (depth > 16 || (rcnm != 125 && rcnm != 130)) return;
      if (const SpatialRecord* r = ds_->spatial.find(rcnm, rcid))
        for (const auto& p : r->parts) walk(p.rcnm, p.rcid, depth + 1);
    };
    for (const auto& s : f.spatial) walk(s.rcnm, s.rcid, 0);
  }
}

const std::vector<std::string>& S101HostAdapter::features_of_spatial(const std::string& sid) const {
  if (!spatial_index_built_) build_spatial_index();
  static const std::vector<std::string> none;
  auto it = spatial_features_.find(sid);
  return it == spatial_features_.end() ? none : it->second;
}

// =================================================================== S-102

struct S102HostImpl {
  static S102HostAdapter* host(lua_State* L) { return static_cast<S102HostAdapter*>(self(L)); }
  static const CoverageFeature& feature(lua_State* L, int i) {
    const std::string id = arg_string(L, i);
    const CoverageFeature* f = host(L)->feature(id);
    if (!f) luaL_error(L, "host: unknown S-102 feature ID '%s'", id.c_str());
    return *f;
  }

  static int GetFeatureIDs(lua_State* L) {
    const auto& fs = host(L)->cov_->features;
    lua_createtable(L, static_cast<int>(fs.size()), 0);
    for (std::size_t i = 0; i < fs.size(); ++i) {
      lua_pushstring(L, std::to_string(fs[i].id).c_str());
      lua_rawseti(L, -2, static_cast<int>(i + 1));
    }
    return 1;
  }
  static int FeatureGetCode(lua_State* L) { lua_pushstring(L, feature(L, 1).code.c_str()); return 1; }
  static int FeatureGetSimpleAttribute(lua_State* L) {
    return push_simple_values(L, feature(L, 1).attributes, opt_string(L, 2), arg_string(L, 3));
  }
  static int FeatureGetComplexAttributeCount(lua_State* L) {
    return push_complex_count(L, feature(L, 1).attributes, opt_string(L, 2), arg_string(L, 3));
  }
  // S-102 coverage datasets carry no information type instances: any ID is unknown.
  static int NoInformation(lua_State* L) {
    return luaL_error(L, "host: unknown information ID '%s' (S-102 coverage has no information types)",
                      luaL_checkstring(L, 1));
  }
  // Coverage features have no vector spatial records: any spatial ID is unknown.
  static int NoSpatial(lua_State* L) {
    return luaL_error(L, "host: unknown spatial ID '%s' (S-102 coverage has no vector spatials)",
                      luaL_checkstring(L, 1));
  }
  static int FeatureGetSpatialAssociations(lua_State* L) {
    feature(L, 1);
    lua_newtable(L);  // 13-8.2.1.7: empty array when the feature has no spatial associations
    return 1;
  }
  static int FeatureGetAssociations(lua_State* L) {
    feature(L, 1);
    luaL_checkstring(L, 2);
    lua_newtable(L);  // no feature/information associations in S-102 coverage data
    return 1;
  }
};

S102HostAdapter::S102HostAdapter(const CoverageDataset& cov, const FeatureCatalogue& fc)
    : HostAdapter("S-102", fc), cov_(&cov) {
  for (std::size_t i = 0; i < cov.features.size(); ++i) index_[std::to_string(cov.features[i].id)] = i;

  // The S-102 PC 3.0.0 set (docs/testing/host-functions.txt [S-102 PC 3.0.0]).
  auto& r = registry_;
  r.add("HostGetFeatureIDs", &S102HostImpl::GetFeatureIDs);
  r.add("HostFeatureGetCode", &S102HostImpl::FeatureGetCode);
  r.add_alias("HostFeatureTypeGetCode", "HostFeatureGetCode");  // S-102 PortrayalAPI.lua name variant
  r.add("HostFeatureGetSimpleAttribute", &S102HostImpl::FeatureGetSimpleAttribute);
  r.add("HostFeatureGetComplexAttributeCount", &S102HostImpl::FeatureGetComplexAttributeCount);
  r.add_alias("HostGetSimpleAttribute", "HostFeatureGetSimpleAttribute");  // only feature instances exist
  r.add_alias("HostGetComplexAttributeCount", "HostFeatureGetComplexAttributeCount");
  r.add("HostInformationTypeGetCode", &S102HostImpl::NoInformation);
  r.add("HostInformationTypeGetSimpleAttribute", &S102HostImpl::NoInformation);
  r.add("HostInformationTypeGetComplexAttributeCount", &S102HostImpl::NoInformation);
  r.add("HostFeatureGetSpatialAssociations", &S102HostImpl::FeatureGetSpatialAssociations);
  r.add("HostFeatureGetAssociatedFeatureIDs", &S102HostImpl::FeatureGetAssociations);
  r.add("HostFeatureGetAssociatedInformationIDs", &S102HostImpl::FeatureGetAssociations);
  r.add("HostGetSpatial", &S102HostImpl::NoSpatial);
  r.add("HostSpatialGetAssociatedFeatureIDs", &S102HostImpl::NoSpatial);
  r.add("HostSpatialGetAssociatedInformationIDs", &S102HostImpl::NoSpatial);
  r.add("HostGetFeatureTypeCodes", &Common::GetFeatureTypeCodes);
  r.add("HostGetInformationTypeCodes", &Common::GetInformationTypeCodes);
  r.add("HostGetSimpleAttributeTypeCodes", &Common::GetSimpleAttributeTypeCodes);
  r.add("HostGetComplexAttributeTypeCodes", &Common::GetComplexAttributeTypeCodes);
  r.add("HostGetRoleTypeCodes", &Common::GetRoleTypeCodes);
  r.add("HostGetInformationAssociationTypeCodes", &Common::GetInformationAssociationTypeCodes);
  r.add("HostGetFeatureAssociationTypeCodes", &Common::GetFeatureAssociationTypeCodes);
  r.add("HostGetFeatureTypeInfo", &Common::GetFeatureTypeInfo);
  r.add("HostGetInformationTypeInfo", &Common::GetInformationTypeInfo);
  r.add("HostGetSimpleAttributeTypeInfo", &Common::GetSimpleAttributeTypeInfo);
  r.add("HostGetComplexAttributeTypeInfo", &Common::GetComplexAttributeTypeInfo);
  r.add("HostPortrayalEmit", &Common::PortrayalEmit);
  r.add("HostDebuggerEntry", &Common::DebuggerEntry);
}

bool S102HostAdapter::feature_id_of(const std::string& lua_id, std::uint64_t& out) const {
  auto it = index_.find(lua_id);
  if (it == index_.end()) return false;
  out = cov_->features[it->second].id;
  return true;
}

const CoverageFeature* S102HostAdapter::feature(const std::string& lua_id) const {
  auto it = index_.find(lua_id);
  return it == index_.end() ? nullptr : &cov_->features[it->second];
}

}  // namespace s100
