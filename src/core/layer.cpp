#include "s100/layer.hpp"

namespace s100 {

void LayerSet::register_layer(LayerId id, std::string name) {
  names_[id] = std::move(name);
}

void LayerVisibility::set(LayerId id, bool on) { visible_[id] = on; }

bool LayerVisibility::is_visible(LayerId id) const {
  auto it = visible_.find(id);
  return it == visible_.end() ? true : it->second;
}

}  // namespace s100
