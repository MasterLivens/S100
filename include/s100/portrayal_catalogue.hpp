#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace s100 {

/** A context parameter (mariner setting) declared by the PC with its default. */
struct PcContextParameter {
  std::string id;             // e.g. "SafetyContour"
  std::string type;           // as in the PC: Boolean | Double | Integer | String | Date
  std::string default_value;  // PC default, textual
  std::string name;
};

struct PcRuleFile {
  std::string id, file_name, rule_type;  // rule_type: TopLevelTemplate | SubTemplate
};

/** Item that points at an asset file (symbol, line style, area fill, colour profile). */
struct PcAsset {
  std::string id, file_name, file_type, file_format;
};

struct PcViewingGroup {
  std::string id, name, description;
};

/** Parsed `portrayal_catalogue.xml` (S-100 Part 9). Data only; no Lua, no Qt. */
struct PortrayalCatalogue {
  std::string product;   // productId, e.g. "S-101"
  std::string version;   // e.g. "2.0.0"
  std::filesystem::path root;  // directory holding portrayal_catalogue.xml
  std::vector<PcContextParameter> context_parameters;  // PC order
  std::vector<PcRuleFile> rules;
  std::vector<PcAsset> color_profiles, symbols, line_styles, area_fills;
  std::map<std::string, PcViewingGroup> viewing_groups;
  std::map<std::string, std::vector<std::string>> viewing_group_layers;  // layer id -> viewing groups
  std::vector<std::pair<std::string, std::vector<std::string>>> display_modes;  // mode -> layers (PC order)
  std::map<std::string, int> display_planes;  // id -> order
  std::vector<std::string> foundation_mode;   // viewing groups

  std::filesystem::path rules_dir() const { return root / "Rules"; }
  /** TopLevelTemplate rule file (main.lua), empty if none. */
  std::string top_level_rule() const;
  const PcContextParameter* find_context_parameter(const std::string& id) const;
  /** First display mode (PC order: DisplayBase, StandardDisplay, OtherInformation) whose
   *  viewing group layers contain `viewing_group`; empty if none. */
  std::string display_mode_of(const std::string& viewing_group) const;
  /** PointInstruction symbol_ref -> <root>/Symbols/<file> (PC symbol item file name, else
   *  <ref>.svg). A missing file is an error (docs/bridge-render-contract.md hook 4): returns an
   *  empty path and sets *err; never a silent drop. */
  std::filesystem::path symbol_path(const std::string& symbol_ref, std::string* err = nullptr) const;
};

/** Load from a PC directory or its portrayal_catalogue.xml. */
bool load_portrayal_catalogue(const std::filesystem::path& path, PortrayalCatalogue& out,
                              std::string* error = nullptr);

}  // namespace s100
