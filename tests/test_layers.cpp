#include "s100/layer.hpp"
#include <iostream>

extern int failures; // not shared - use local checks via macros duplicated simply

#define CHECK(cond)                                                                          \
  do {                                                                                       \
    if (!(cond)) {                                                                           \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";           \
      std::exit(1);                                                                          \
    }                                                                                        \
  } while (0)

void test_layers() {
  s100::LayerVisibility v;
  CHECK(v.is_visible(s100::LayerId::LAND));
  v.set(s100::LayerId::LAND, false);
  CHECK(!v.is_visible(s100::LayerId::LAND));
  v.set(s100::LayerId::DEPARE, true);
  CHECK(v.is_visible(s100::LayerId::DEPARE));
  v.set(s100::LayerId::DEPARE, false);
  CHECK(!v.is_visible(s100::LayerId::DEPARE));

  s100::LayerSet set;
  set.register_layer(s100::LayerId::LAND, "LAND");
  set.register_layer(s100::LayerId::DEPARE, "DEPARE");
  CHECK(set.size() == 2);
}
