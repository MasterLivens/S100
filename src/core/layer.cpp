#include "s100/layer.hpp"

#include <algorithm>
#include <set>

#include "s100/dataset.hpp"
#include "s100/feature_catalogue.hpp"

namespace s100 {

const LayerInfo* LayerSet::find(const std::string& code) const {
  auto it = layers_.find(code);
  return it == layers_.end() ? nullptr : &it->second;
}

LayerVisibility::LayerVisibility() {
  by_use_type_["geographic"] = true;
  by_use_type_["meta"] = false;
  by_use_type_["cartographic"] = true;
}

void LayerVisibility::set_layer(const std::string& code, bool on) { by_layer_[code] = on; }
void LayerVisibility::set_use_type(const std::string& use_type, bool on) {
  by_use_type_[use_type] = on;
}
void LayerVisibility::clear_layer_rule(const std::string& code) { by_layer_.erase(code); }

bool LayerVisibility::is_visible(const LayerInfo& layer) const {
  if (auto it = by_layer_.find(layer.code); it != by_layer_.end()) return it->second;
  if (auto it = by_use_type_.find(layer.use_type); it != by_use_type_.end()) return it->second;
  return true;
}

bool classify_layers(Dataset& ds, const FeatureCatalogue& fc, ClassifyResult* r) {
  ds.layers.clear();
  std::set<std::string> unknown;
  for (const auto& f : ds.features) {
    const FcFeatureType* ft = fc.find(f.feature_type);
    if (!ft) {
      unknown.insert(f.feature_type);
      continue;
    }
    LayerInfo& li = ds.layers.upsert(ft->code);
    if (li.code.empty()) {
      li.code = ft->code;
      li.name = ft->name;
      li.use_type = ft->use_type;
    }
    ++li.feature_count;
  }
  if (r) r->unknown_codes.assign(unknown.begin(), unknown.end());
  return unknown.empty();
}

}  // namespace s100
