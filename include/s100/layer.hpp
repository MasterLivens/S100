#pragma once
#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace s100 {

struct Dataset;
struct FeatureCatalogue;

/** One layer per FC feature type code (see docs/api-contracts.md, Layers). */
struct LayerInfo {
  std::string code;      // FC feature type code, e.g. "DepthArea" (= layer key)
  std::string name;      // FC human name
  std::string use_type;  // geographic | meta | cartographic | theme
  std::size_t feature_count{0};
  // Filled by apply_portrayal_to_layers() after a portrayal run (PC-driven, empty before):
  std::string viewing_group;     // dominant viewing group, e.g. "13030"
  int drawing_priority{0};       // dominant drawing priority
  std::string display_plane;     // dominant display plane, e.g. "UnderRadar"
  std::string display_category;  // PC display mode: DisplayBase | StandardDisplay | OtherInformation
};

/** Per Dataset, built by classify_layers(). */
class LayerSet {
 public:
  const LayerInfo* find(const std::string& code) const;
  const std::map<std::string, LayerInfo>& all() const { return layers_; }
  std::size_t size() const { return layers_.size(); }
  bool empty() const { return layers_.empty(); }
  void clear() { layers_.clear(); }
  LayerInfo& upsert(const std::string& code) { return layers_[code]; }

 private:
  std::map<std::string, LayerInfo> layers_;
};

/** Per session / viewer — never stored in Dataset. A per-layer rule overrides the use-type rule.
 *  Defaults: geographic on, meta off, cartographic on, anything else on. */
class LayerVisibility {
 public:
  LayerVisibility();
  void set_layer(const std::string& code, bool on);
  void set_use_type(const std::string& use_type, bool on);
  void clear_layer_rule(const std::string& code);
  bool is_visible(const LayerInfo& layer) const;

 private:
  std::map<std::string, bool> by_layer_;
  std::map<std::string, bool> by_use_type_;
};

struct ClassifyResult {
  std::vector<std::string> unknown_codes;  // sorted, unique
};

/** Build ds.layers from the FC. Returns false if any feature code is missing from the FC
 *  (listed in r->unknown_codes). No fallback layer. The FC is product-level: load it once and
 *  share it across datasets. */
bool classify_layers(Dataset& ds, const FeatureCatalogue& fc, ClassifyResult* r = nullptr);

}  // namespace s100
