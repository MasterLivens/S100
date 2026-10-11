#include "s100/feature_catalogue.hpp"

#include <pugixml.hpp>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>

namespace s100 {
namespace {

// Match on local names so the S100FC: / S100Base: prefixes don't matter.
const char* local(const pugi::xml_node& n) {
  const char* name = n.name();
  const char* c = std::strchr(name, ':');
  return c ? c + 1 : name;
}

pugi::xml_node child(const pugi::xml_node& n, const char* name) {
  for (auto c : n.children())
    if (std::strcmp(local(c), name) == 0) return c;
  return {};
}

std::vector<pugi::xml_node> children(const pugi::xml_node& n, const char* name) {
  std::vector<pugi::xml_node> v;
  for (auto c : n.children())
    if (std::strcmp(local(c), name) == 0) v.push_back(c);
  return v;
}

std::string text(const pugi::xml_node& n, const char* name) {
  return child(n, name).text().as_string();
}

std::vector<std::string> texts(const pugi::xml_node& n, const char* name) {
  std::vector<std::string> v;
  for (auto c : children(n, name)) v.emplace_back(c.text().as_string());
  return v;
}

std::string ref(const pugi::xml_node& n, const char* name) {
  return child(n, name).attribute("ref").as_string();
}

void multiplicity(const pugi::xml_node& b, int& lower, int& upper) {
  const auto m = child(b, "multiplicity");
  lower = child(m, "lower").text().as_int(0);
  const auto u = child(m, "upper");
  const bool inf = u.attribute("infinite").as_bool(false) || u.attribute("xsi:nil").as_bool(false);
  upper = inf || !u ? -1 : u.text().as_int(-1);
}

FcAttributeBinding attribute_binding(const pugi::xml_node& b) {
  FcAttributeBinding ab;
  ab.attribute = ref(b, "attribute");
  ab.sequential = b.attribute("sequential").as_bool(false);
  multiplicity(b, ab.lower, ab.upper);
  for (auto v : children(child(b, "permittedValues"), "value")) ab.permitted_values.push_back(v.text().as_int());
  return ab;
}

FcTypeBinding type_binding(const pugi::xml_node& b, const char* target) {
  FcTypeBinding tb;
  multiplicity(b, tb.lower, tb.upper);
  tb.role_type = b.attribute("roleType").as_string();
  tb.role = ref(b, "role");
  tb.association = ref(b, "association");
  for (auto t : children(b, target)) tb.types.emplace_back(t.attribute("ref").as_string());
  return tb;
}

template <class T>
void item(const pugi::xml_node& n, T& t) {
  t.code = text(n, "code");
  t.name = text(n, "name");
  t.definition = text(n, "definition");
}

/** Prepend supertype bindings (recursively) so a subtype answers for inherited attributes. */
template <class Map, class Get>
void inherit(Map& m, Get bindings_of) {
  std::map<std::string, bool> done;
  std::function<void(const std::string&)> resolve = [&](const std::string& code) {
    if (done[code]) return;
    done[code] = true;
    auto& t = m.at(code);
    if (t.super_type.empty() || !m.count(t.super_type)) return;
    resolve(t.super_type);
    const auto& sup = m.at(t.super_type);
    bindings_of(t, sup);
    m[t.super_type].sub_types.push_back(code);
  };
  for (auto& [code, t] : m) resolve(code);
}

template <class V>
void prepend_missing(std::vector<V>& own, const std::vector<V>& inherited,
                     std::function<std::string(const V&)> key) {
  std::vector<V> merged;
  for (const auto& v : inherited) {
    const auto k = key(v);
    if (std::none_of(own.begin(), own.end(), [&](const V& o) { return key(o) == k; })) merged.push_back(v);
  }
  merged.insert(merged.end(), own.begin(), own.end());
  own = std::move(merged);
}

}  // namespace

bool load_feature_catalogue(const std::filesystem::path& path, FeatureCatalogue& out,
                            std::string* error) {
  pugi::xml_document doc;
  const auto res = doc.load_file(path.c_str());
  if (!res) {
    if (error) *error = "cannot parse " + path.string() + ": " + res.description();
    return false;
  }
  const auto root = doc.document_element();
  out = FeatureCatalogue{};
  out.product = text(root, "productId");
  out.version = text(root, "versionNumber");
  out.date = text(root, "versionDate");

  for (auto n : children(child(root, "S100_FC_SimpleAttributes"), "S100_FC_SimpleAttribute")) {
    FcSimpleAttribute a;
    item(n, a);
    a.alias = texts(n, "alias");
    a.value_type = text(n, "valueType");
    a.uom = text(child(n, "uom"), "name");
    a.quantity_specification = text(n, "quantitySpecification");
    for (auto lv : children(child(n, "listedValues"), "listedValue")) {
      FcListedValue v;
      v.label = text(lv, "label");
      v.definition = text(lv, "definition");
      v.code = child(lv, "code").text().as_int();
      a.listed_values.push_back(std::move(v));
    }
    if (!a.code.empty()) out.simple_attributes[a.code] = std::move(a);
  }
  for (auto n : children(child(root, "S100_FC_ComplexAttributes"), "S100_FC_ComplexAttribute")) {
    FcComplexAttribute a;
    item(n, a);
    a.alias = texts(n, "alias");
    for (auto b : children(n, "subAttributeBinding")) a.sub_attribute_bindings.push_back(attribute_binding(b));
    if (!a.code.empty()) out.complex_attributes[a.code] = std::move(a);
  }
  for (auto n : children(child(root, "S100_FC_Roles"), "S100_FC_Role")) {
    FcNamedItem r;
    item(n, r);
    if (!r.code.empty()) out.roles[r.code] = std::move(r);
  }
  auto assocs = [&](const char* group, const char* one, std::map<std::string, FcNamedItem>& m) {
    for (auto n : children(child(root, group), one)) {
      FcNamedItem a;
      item(n, a);
      a.is_abstract = n.attribute("isAbstract").as_bool(false);
      a.super_type = text(n, "superType");
      for (auto r : children(n, "role")) a.roles.emplace_back(r.attribute("ref").as_string());
      if (!a.code.empty()) m[a.code] = std::move(a);
    }
  };
  assocs("S100_FC_InformationAssociations", "S100_FC_InformationAssociation", out.information_associations);
  assocs("S100_FC_FeatureAssociations", "S100_FC_FeatureAssociation", out.feature_associations);

  for (auto n : children(child(root, "S100_FC_InformationTypes"), "S100_FC_InformationType")) {
    FcInformationType t;
    item(n, t);
    t.alias = texts(n, "alias");
    t.is_abstract = n.attribute("isAbstract").as_bool(false);
    t.super_type = text(n, "superType");
    for (auto b : children(n, "attributeBinding")) t.attribute_bindings.push_back(attribute_binding(b));
    for (auto b : children(n, "informationBinding")) t.information_bindings.push_back(type_binding(b, "informationType"));
    if (!t.code.empty()) out.information_types[t.code] = std::move(t);
  }
  for (auto n : children(child(root, "S100_FC_FeatureTypes"), "S100_FC_FeatureType")) {
    FcFeatureType ft;
    item(n, ft);
    ft.alias = texts(n, "alias");
    ft.is_abstract = n.attribute("isAbstract").as_bool(false);
    ft.use_type = text(n, "featureUseType");
    ft.super_type = text(n, "superType");
    ft.primitives = texts(n, "permittedPrimitives");
    for (auto b : children(n, "attributeBinding")) ft.attribute_bindings.push_back(attribute_binding(b));
    for (auto b : children(n, "informationBinding")) ft.information_bindings.push_back(type_binding(b, "informationType"));
    for (auto b : children(n, "featureBinding")) ft.feature_bindings.push_back(type_binding(b, "featureType"));
    if (!ft.code.empty()) out.feature_types[ft.code] = std::move(ft);
  }

  using AB = FcAttributeBinding;
  using TB = FcTypeBinding;
  const std::function<std::string(const AB&)> akey = [](const AB& b) { return b.attribute; };
  const std::function<std::string(const TB&)> tkey = [](const TB& b) {
    std::string k = b.association + "|" + b.role;
    for (const auto& t : b.types) k += "|" + t;
    return k;
  };
  inherit(out.feature_types, [&](FcFeatureType& t, const FcFeatureType& sup) {
    prepend_missing(t.attribute_bindings, sup.attribute_bindings, akey);
    prepend_missing(t.information_bindings, sup.information_bindings, tkey);
    prepend_missing(t.feature_bindings, sup.feature_bindings, tkey);
  });
  inherit(out.information_types, [&](FcInformationType& t, const FcInformationType& sup) {
    prepend_missing(t.attribute_bindings, sup.attribute_bindings, akey);
    prepend_missing(t.information_bindings, sup.information_bindings, tkey);
  });

  if (out.feature_types.empty()) {
    if (error) *error = "no S100_FC_FeatureType entries in " + path.string();
    return false;
  }
  return true;
}

}  // namespace s100
