#include "s100/portrayal_catalogue.hpp"

#include <pugixml.hpp>

#include <cstring>

namespace s100 {
namespace {

const char* local(const pugi::xml_node& n) {
  const char* c = std::strchr(n.name(), ':');
  return c ? c + 1 : n.name();
}
pugi::xml_node child(const pugi::xml_node& n, const char* name) {
  for (auto c : n.children())
    if (!std::strcmp(local(c), name)) return c;
  return {};
}
std::vector<pugi::xml_node> children(const pugi::xml_node& n, const char* name) {
  std::vector<pugi::xml_node> v;
  for (auto c : n.children())
    if (!std::strcmp(local(c), name)) v.push_back(c);
  return v;
}
std::string text(const pugi::xml_node& n, const char* name) { return child(n, name).text().as_string(); }

std::vector<PcAsset> assets(const pugi::xml_node& root, const char* group, const char* one) {
  std::vector<PcAsset> v;
  for (auto n : children(child(root, group), one))
    v.push_back(PcAsset{n.attribute("id").as_string(), text(n, "fileName"), text(n, "fileType"),
                        text(n, "fileFormat")});
  return v;
}

}  // namespace

std::string PortrayalCatalogue::top_level_rule() const {
  for (const auto& r : rules)
    if (r.rule_type == "TopLevelTemplate") return r.file_name;
  return {};
}

std::filesystem::path PortrayalCatalogue::symbol_path(const std::string& ref, std::string* err) const {
  if (ref.empty()) {
    if (err) *err = "symbol: empty symbol_ref";
    return {};
  }
  std::string file = ref + ".svg";
  for (const auto& s : symbols)
    if (s.id == ref && !s.file_name.empty()) {
      file = s.file_name;
      break;
    }
  const std::filesystem::path p = root / "Symbols" / file;
  std::error_code ec;
  if (!std::filesystem::is_regular_file(p, ec)) {
    if (err) *err = "symbol '" + ref + "' missing: " + p.string();
    return {};
  }
  return p;
}

const PcContextParameter* PortrayalCatalogue::find_context_parameter(const std::string& id) const {
  for (const auto& p : context_parameters)
    if (p.id == id) return &p;
  return nullptr;
}

std::string PortrayalCatalogue::display_mode_of(const std::string& vg) const {
  for (const auto& [mode, layers] : display_modes)
    for (const auto& layer : layers) {
      auto it = viewing_group_layers.find(layer);
      if (it == viewing_group_layers.end()) continue;
      for (const auto& g : it->second)
        if (g == vg) return mode;
    }
  return {};
}

bool load_portrayal_catalogue(const std::filesystem::path& path, PortrayalCatalogue& out,
                              std::string* error) {
  const auto xml = std::filesystem::is_directory(path) ? path / "portrayal_catalogue.xml" : path;
  pugi::xml_document doc;
  const auto res = doc.load_file(xml.c_str());
  if (!res) {
    if (error) *error = "cannot parse " + xml.string() + ": " + res.description();
    return false;
  }
  const auto root = doc.document_element();
  if (std::strcmp(local(root), "portrayalCatalog") != 0) {
    if (error) *error = "not a portrayalCatalog: " + xml.string();
    return false;
  }
  out = PortrayalCatalogue{};
  out.root = xml.parent_path();
  out.product = root.attribute("productId").as_string();
  out.version = root.attribute("version").as_string();

  for (auto p : children(child(root, "context"), "parameter"))
    out.context_parameters.push_back(PcContextParameter{p.attribute("id").as_string(), text(p, "type"),
                                                        text(p, "default"),
                                                        text(child(p, "description"), "name")});
  for (auto r : children(child(root, "rules"), "ruleFile"))
    out.rules.push_back(PcRuleFile{r.attribute("id").as_string(), text(r, "fileName"), text(r, "ruleType")});
  out.color_profiles = assets(root, "colorProfiles", "colorProfile");
  out.symbols = assets(root, "symbols", "symbol");
  out.line_styles = assets(root, "lineStyles", "lineStyle");
  out.area_fills = assets(root, "areaFills", "areaFill");
  for (auto g : children(child(root, "viewingGroups"), "viewingGroup")) {
    PcViewingGroup vg{g.attribute("id").as_string(), text(child(g, "description"), "name"),
                      text(child(g, "description"), "description")};
    out.viewing_groups[vg.id] = vg;
  }
  for (auto l : children(child(root, "viewingGroupLayers"), "viewingGroupLayer")) {
    auto& v = out.viewing_group_layers[l.attribute("id").as_string()];
    for (auto g : children(l, "viewingGroup")) v.emplace_back(g.text().as_string());
  }
  for (auto m : children(child(root, "displayModes"), "displayMode")) {
    std::vector<std::string> layers;
    for (auto l : children(m, "viewingGroupLayer")) layers.emplace_back(l.text().as_string());
    out.display_modes.emplace_back(m.attribute("id").as_string(), std::move(layers));
  }
  for (auto d : children(child(root, "displayPlanes"), "displayPlane"))
    out.display_planes[d.attribute("id").as_string()] = d.attribute("order").as_int();
  for (auto g : children(child(root, "foundationMode"), "viewingGroup"))
    out.foundation_mode.emplace_back(g.text().as_string());

  if (out.top_level_rule().empty()) {
    if (error) *error = "no TopLevelTemplate rule in " + xml.string();
    return false;
  }
  return true;
}

}  // namespace s100
