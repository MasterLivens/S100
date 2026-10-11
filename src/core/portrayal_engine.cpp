#include "s100/portrayal_engine.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <set>

// Lua is built as C++ (third_party/CMakeLists.txt): plain headers, C++ linkage.
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "s100/dataset.hpp"
#include "s100/feature_catalogue.hpp"

namespace s100 {

// ------------------------------------------------------------------ context

PortrayalContext PortrayalContext::defaults(const PortrayalCatalogue& pc) {
  PortrayalContext c;
  for (const auto& p : pc.context_parameters) c.values[p.id] = p.default_value;
  return c;
}

bool PortrayalContext::set(const PortrayalCatalogue& pc, const std::string& id, const std::string& value,
                           std::string* err) {
  if (!pc.find_context_parameter(id)) {
    if (err) *err = "context parameter '" + id + "' not declared by PC " + pc.product + " " + pc.version;
    return false;
  }
  values[id] = value;
  return true;
}

std::string PortrayalContext::hash() const {
  std::uint64_t h = 1469598103934665603ull;  // FNV-1a 64
  for (const auto& [k, v] : values) {
    for (char c : k + "=" + v + ";") {
      h ^= static_cast<unsigned char>(c);
      h *= 1099511628211ull;
    }
  }
  char buf[17];
  std::snprintf(buf, sizeof buf, "%016llx", static_cast<unsigned long long>(h));
  return buf;
}

// ------------------------------------------------------------------ engine

namespace {

struct LuaState {
  lua_State* L{luaL_newstate()};
  ~LuaState() {
    if (L) lua_close(L);
  }
};

void open_lib(lua_State* L, lua_CFunction f, const char* name) {
  lua_pushcfunction(L, f);
  lua_pushstring(L, name);
  lua_call(L, 1, 0);
}

int traceback(lua_State* L) {
  // debug library is not opened (sandbox); keep the message as is.
  if (!lua_isstring(L, 1)) lua_pushliteral(L, "(non-string Lua error)");
  return 1;
}

bool pcall(lua_State* L, int nargs, int nres, std::string* err, const char* what) {
  const int base = lua_gettop(L) - nargs;
  lua_pushcfunction(L, traceback);
  lua_insert(L, base);
  const int rc = lua_pcall(L, nargs, nres, base);
  lua_remove(L, base);
  if (rc != 0) {
    if (err) *err = std::string(what) + ": " + (lua_isstring(L, -1) ? lua_tostring(L, -1) : "?");
    lua_pop(L, 1);
    return false;
  }
  return true;
}

// Unregistered Host* globals fail loudly (contract rule 2). Other unknown globals keep normal
// Lua semantics (nil), which the official framework relies on.
const char* kStrictHost = R"lua(
local product = ...
setmetatable(_G, { __index = function(t, k)
  if type(k) == 'string' and k:sub(1, 4) == 'Host' then
    error("Host function '" .. k .. "' is not implemented by the " .. product .. " host", 2)
  end
end })
)lua";


/** Case-insensitive require searcher for PC Rules/ (Linux). Upvalue 1 = rules dir path. */
int case_insensitive_rules_searcher(lua_State* L) {
  const char* modname = luaL_checkstring(L, 1);
  const char* rules_dir = lua_tostring(L, lua_upvalueindex(1));
  if (!rules_dir || !modname) return 0;
  namespace fs = std::filesystem;
  const fs::path want = std::string(modname) + ".lua";
  const fs::path exact = fs::path(rules_dir) / want;
  if (fs::is_regular_file(exact)) {
    if (luaL_loadfile(L, exact.string().c_str()) != 0) return lua_error(L);
    return 1;
  }
  auto ieq = [](char a, char b) {
    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
  };
  std::error_code ec;
  if (!fs::is_directory(rules_dir, ec)) return 0;
  for (const auto& ent : fs::directory_iterator(rules_dir, ec)) {
    if (!ent.is_regular_file(ec)) continue;
    const std::string fn = ent.path().filename().string();
    if (fn.size() != want.string().size()) continue;
    bool match = true;
    for (std::size_t i = 0; i < fn.size(); ++i) {
      if (!ieq(fn[i], want.string()[i])) {
        match = false;
        break;
      }
    }
    if (!match) continue;
    if (luaL_loadfile(L, ent.path().string().c_str()) != 0) return lua_error(L);
    return 1;
  }
  return 0;
}

}  // namespace

bool PortrayalEngine::run(HostAdapter& host, const PortrayalContext& ctx, DrawingInstructions& out,
                          PortrayalStats* stats_out, std::string* err) const {
  PortrayalStats local;
  PortrayalStats& stats = stats_out ? *stats_out : local;
  stats = PortrayalStats{};
  host.reset_run();
  out.clear();

  LuaState state;
  lua_State* L = state.L;
  if (!L) {
    if (err) *err = "cannot create Lua state";
    return false;
  }
  open_lib(L, luaopen_base, "");
  open_lib(L, luaopen_package, LUA_LOADLIBNAME);
  open_lib(L, luaopen_table, LUA_TABLIBNAME);
  open_lib(L, luaopen_string, LUA_STRLIBNAME);
  open_lib(L, luaopen_math, LUA_MATHLIBNAME);
  // Sandbox: rules are loaded only through require from the PC Rules directory.
  for (const char* g : {"dofile", "loadfile"}) {
    lua_pushnil(L);
    lua_setglobal(L, g);
  }
  lua_getglobal(L, "package");
  const std::string path = (pc_->rules_dir() / "?.lua").string();
  lua_pushstring(L, path.c_str());
  lua_setfield(L, -2, "path");
  lua_pushstring(L, "");
  lua_setfield(L, -2, "cpath");
  lua_pushnil(L);
  lua_setfield(L, -2, "loadlib");
  // Linux case sensitivity: PC XML/FC codes may be Navwarn* while on-disk Rules are NAVWARN*.
  // Official trees stay read-only — resolve require() case-insensitively under Rules/.
  {
    lua_getfield(L, -1, "loaders");  // Lua 5.1
    if (!lua_istable(L, -1)) {
      lua_pop(L, 1);
      lua_getfield(L, -1, "searchers");  // Lua 5.2+
    }
    if (lua_istable(L, -1)) {
      const int n = static_cast<int>(lua_objlen(L, -1));
      for (int i = n; i >= 1; --i) {
        lua_rawgeti(L, -1, i);
        lua_rawseti(L, -2, i + 1);
      }
      lua_pushstring(L, pc_->rules_dir().string().c_str());
      lua_pushcclosure(L, case_insensitive_rules_searcher, 1);
      lua_rawseti(L, -2, 1);
    }
    lua_pop(L, 1);  // loaders/searchers
  }
  lua_pop(L, 1);  // package

  // Host functions from the product registry; each gets the adapter as upvalue.
  for (const auto& name : host.registry().names()) {
    lua_pushlightuserdata(L, &host);
    lua_pushcclosure(L, host.registry().function(name), 1);
    lua_setglobal(L, name.c_str());
  }
  if (luaL_loadstring(L, kStrictHost) != 0) {
    if (err) *err = "strict host chunk";
    return false;
  }
  lua_pushstring(L, host.product().c_str());
  if (!pcall(L, 1, 0, err, "strict host")) return false;

  // Top level template (main.lua), unmodified.
  const auto main_file = (pc_->rules_dir() / pc_->top_level_rule()).string();
  if (luaL_loadfile(L, main_file.c_str()) != 0) {
    if (err) *err = std::string("load ") + main_file + ": " + lua_tostring(L, -1);
    return false;
  }
  if (!pcall(L, 0, 0, err, "main.lua")) return false;

  // Part 9a 9a-14.1.2/3: context parameters with PC defaults, then session values.
  lua_newtable(L);
  int n = 0;
  for (const auto& p : pc_->context_parameters) {
    lua_getglobal(L, "PortrayalCreateContextParameter");
    lua_pushstring(L, p.id.c_str());
    lua_pushstring(L, p.type.c_str());
    lua_pushstring(L, p.default_value.c_str());
    if (!pcall(L, 3, 1, err, "PortrayalCreateContextParameter")) return false;
    lua_rawseti(L, -2, ++n);
  }
  lua_getglobal(L, "PortrayalInitializeContextParameters");
  lua_insert(L, -2);
  if (!pcall(L, 1, 0, err, "PortrayalInitializeContextParameters")) return false;
  for (const auto& p : pc_->context_parameters) {
    auto it = ctx.values.find(p.id);
    if (it == ctx.values.end() || it->second == p.default_value) continue;
    lua_getglobal(L, "PortrayalSetContextParameter");
    lua_pushstring(L, p.id.c_str());
    lua_pushstring(L, it->second.c_str());
    if (!pcall(L, 2, 0, err, "PortrayalSetContextParameter")) return false;
  }
  for (const auto& [id, v] : ctx.values)
    if (!pc_->find_context_parameter(id)) {
      if (err) *err = "context parameter '" + id + "' not declared by the PC";
      return false;
    }

  // Part 9a 9a-14.1.1: all features (nil = every feature ID).
  stats.features = host.feature_count();
  lua_getglobal(L, "PortrayalMain");
  lua_pushnil(L);
  if (!pcall(L, 1, 1, err, "PortrayalMain")) {
    ++stats.lua_errors;
    return false;
  }
  const bool ok = lua_toboolean(L, -1) != 0;
  lua_pop(L, 1);

  stats.lua_errors += host.trace_errors.size();
  for (const auto& e : host.trace_errors)
    if (stats.errors.size() < 100) stats.errors.push_back(e);
  stats.first_chance_errors = host.first_chance_errors;
  stats.warnings = host.warnings;
  stats.emitted_features = host.emissions.size();

  std::set<std::uint64_t> with_instructions;
  for (const auto& e : host.emissions) {
    std::uint64_t fid = 0;
    if (!host.feature_id_of(e.feature_id, fid)) {
      if (err) *err = "HostPortrayalEmit for unknown feature ID " + e.feature_id;
      return false;
    }
    const std::size_t before = out.size();
    parse_drawing_instructions(fid, e.instructions, out, &stats);
    if (out.size() > before) with_instructions.insert(fid);
  }
  if (const auto* s1 = dynamic_cast<const S101HostAdapter*>(&host)) {
    for (const auto& f : s1->dataset().features)
      if (!with_instructions.count(f.id)) stats.features_without_instructions.push_back(f.id);
  }
  if (!ok) {
    if (err) *err = "PortrayalMain returned false";
    return false;
  }
  return true;
}


// ------------------------------------------------------------------ SpatialReference → Geometry

namespace {

bool parse_spatial_ref_token(const std::string& token, std::uint8_t& rcnm, std::uint32_t& rcid,
                             bool& reverse) {
  reverse = false;
  std::string id = token;
  static const char kRev[] = ",reverse";
  if (id.size() > sizeof(kRev) - 1 &&
      id.compare(id.size() - (sizeof(kRev) - 1), sizeof(kRev) - 1, kRev) == 0) {
    reverse = true;
    id.resize(id.size() - (sizeof(kRev) - 1));
  }
  if (id.size() < 2) return false;
  switch (id[0]) {
    case 'P': rcnm = 110; break;
    case 'M': rcnm = 115; break;
    case 'C': rcnm = 120; break;
    case 'K': rcnm = 125; break;
    case 'S': rcnm = 130; break;
    default: return false;
  }
  char* end = nullptr;
  rcid = static_cast<std::uint32_t>(std::strtoul(id.c_str() + 1, &end, 10));
  return end && *end == '\0';
}

std::vector<Point> resolve_curve_points(const SpatialStore& store, std::uint8_t rcnm,
                                        std::uint32_t rcid, bool reverse, int depth) {
  std::vector<Point> pts;
  if (depth > 16) return pts;
  if (rcnm == 120) {
    const SpatialRecord* r = store.find(120, rcid);
    if (!r) return pts;
    pts.reserve(r->coords.size());
    for (const auto& c : r->coords) pts.push_back(Point{c.lon, c.lat});
  } else if (rcnm == 125) {
    const SpatialRecord* r = store.find(125, rcid);
    if (!r) return pts;
    for (const auto& part : r->parts) {
      auto part_pts = resolve_curve_points(store, part.rcnm, part.rcid, part.reverse, depth + 1);
      if (part_pts.empty()) continue;
      if (!pts.empty() && pts.back().lon == part_pts.front().lon &&
          pts.back().lat == part_pts.front().lat)
        part_pts.erase(part_pts.begin());
      pts.insert(pts.end(), part_pts.begin(), part_pts.end());
    }
  }
  if (reverse) std::reverse(pts.begin(), pts.end());
  return pts;
}

bool resolve_one_spatial(const SpatialStore& store, std::uint8_t rcnm, std::uint32_t rcid,
                         bool reverse, Geometry& out) {
  out = Geometry{};
  switch (rcnm) {
    case 110: {
      const SpatialRecord* r = store.find(110, rcid);
      if (!r || r->coords.empty()) return false;
      out.kind = GeometryKind::Point;
      out.point = Point{r->coords[0].lon, r->coords[0].lat};
      return true;
    }
    case 115: {
      const SpatialRecord* r = store.find(115, rcid);
      if (!r || r->coords.empty()) return false;
      out.kind = GeometryKind::MultiPoint;
      out.multipoint = r->coords;
      return true;
    }
    case 120:
    case 125: {
      auto pts = resolve_curve_points(store, rcnm, rcid, reverse, 0);
      if (pts.size() < 2) return false;
      out.kind = GeometryKind::LineString;
      out.line = std::move(pts);
      return true;
    }
    case 130: {
      const SpatialRecord* r = store.find(130, rcid);
      if (!r) return false;
      std::vector<Ring> exterior, interior;
      for (const auto& part : r->parts) {
        // XOR orientation: surface-part reverse combined with SpatialReference reverse.
        const bool rev = part.reverse ^ reverse;
        Ring ring;
        ring.points = resolve_curve_points(store, part.rcnm, part.rcid, rev, 0);
        if (ring.points.size() < 3) continue;
        (part.usage == 2 ? interior : exterior).push_back(std::move(ring));
      }
      if (exterior.empty()) return false;
      out.kind = GeometryKind::Polygon;
      out.polygon = std::move(exterior);
      for (auto& ring : interior) out.polygon.push_back(std::move(ring));
      return true;
    }
    default:
      return false;
  }
}

}  // namespace

void resolve_spatial_references(const Dataset& ds, DrawingInstructions& instructions) {
  for (auto& di : instructions) {
    if (di.spatial_refs.empty()) continue;
    di.geometries.clear();
    di.geometries.reserve(di.spatial_refs.size());
    for (const auto& token : di.spatial_refs) {
      std::uint8_t rcnm = 0;
      std::uint32_t rcid = 0;
      bool reverse = false;
      if (!parse_spatial_ref_token(token, rcnm, rcid, reverse)) continue;
      Geometry g;
      if (!resolve_one_spatial(ds.spatial, rcnm, rcid, reverse, g)) continue;
      di.geometries.push_back(std::move(g));
    }
  }
}

bool portray(const Dataset& ds, const FeatureCatalogue& fc, const PortrayalCatalogue& pc,
             const PortrayalContext& ctx, DrawingInstructions& out, std::string* err,
             PortrayalStats* stats) {
  S101HostAdapter host(ds, fc);
  if (!PortrayalEngine(pc).run(host, ctx, out, stats, err)) return false;
  resolve_spatial_references(ds, out);
  return true;
}

bool portray(const CoverageDataset& cov, const FeatureCatalogue& fc, const PortrayalCatalogue& pc,
             const PortrayalContext& ctx, DrawingInstructions& out, std::string* err,
             PortrayalStats* stats) {
  S102HostAdapter host(cov, fc);
  return PortrayalEngine(pc).run(host, ctx, out, stats, err);
}

// ------------------------------------------------------------------ layers

void apply_portrayal_to_layers(Dataset& ds, const DrawingInstructions& di, const PortrayalCatalogue& pc) {
  std::map<std::uint64_t, const DrawingInstruction*> first;
  for (const auto& d : di) first.emplace(d.feature_id, &d);
  struct Tally {
    std::map<std::string, int> vg, plane;
    std::map<int, int> prio;
  };
  std::map<std::string, Tally> tally;
  for (const auto& f : ds.features) {
    auto it = first.find(f.id);
    if (it == first.end()) continue;
    auto& t = tally[f.feature_type];
    ++t.vg[it->second->viewing_group];
    ++t.plane[it->second->display_plane];
    ++t.prio[it->second->drawing_priority];
  }
  auto best = [](const auto& m) {
    auto b = m.begin();
    for (auto i = m.begin(); i != m.end(); ++i)
      if (i->second > b->second) b = i;
    return b->first;
  };
  for (const auto& [code, t] : tally) {
    if (!ds.layers.find(code)) continue;  // layers come from classify_layers only (no new keys)
    LayerInfo& li = ds.layers.upsert(code);
    li.viewing_group = best(t.vg);
    li.display_plane = best(t.plane);
    li.drawing_priority = best(t.prio);
    li.display_category = pc.display_mode_of(li.viewing_group);
  }
}

// ------------------------------------------------------------------ session

std::string PortrayalSession::cache_key(const Dataset& ds) const {
  return ds.path.string() + "|" + pc_->product + " " + pc_->version + "|" + ctx_.hash();
}

const DrawingInstructions* PortrayalSession::portray(Dataset& ds, PortrayalStats* stats, std::string* err) {
  const std::string key = cache_key(ds);
  auto it = cache_.find(key);
  if (it == cache_.end()) {
    Entry e;
    if (!s100::portray(ds, *fc_, *pc_, ctx_, e.instructions, err, &e.stats)) return nullptr;
    ++runs_;
    it = cache_.emplace(key, std::move(e)).first;
  }
  apply_portrayal_to_layers(ds, it->second.instructions, *pc_);
  if (stats) *stats = it->second.stats;
  return &it->second.instructions;
}

}  // namespace s100
