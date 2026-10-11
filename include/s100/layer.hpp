#pragma once
#include "s100/types.hpp"
#include <map>
#include <string>

namespace s100 {

class LayerSet {
 public:
  void register_layer(LayerId id, std::string name);
  std::size_t size() const { return names_.size(); }
  bool empty() const { return names_.empty(); }
  const std::map<LayerId, std::string>& names() const { return names_; }

 private:
  std::map<LayerId, std::string> names_;
};

class LayerVisibility {
 public:
  void set(LayerId id, bool on);
  bool is_visible(LayerId id) const;

 private:
  std::map<LayerId, bool> visible_;
};

}  // namespace s100
