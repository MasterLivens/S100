#include "s100/dataset.hpp"
#include "support/test_data_roots.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>

#define CHECK(cond)                                                                    \
  do {                                                                                 \
    if (!(cond)) {                                                                     \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";     \
      std::exit(1);                                                                    \
    }                                                                                  \
  } while (0)

void test_attributes() {
  namespace fs = std::filesystem;
  const fs::path cell = s100test::fixture_root() / "101AA00DS0008.000";
  if (!fs::exists(cell)) {
    std::cerr << "skip attributes: no " << cell << "\n";
    return;
  }
  s100::Dataset ds;
  s100::LoadError err;
  CHECK(s100::open_dataset(cell, ds, &err));
  std::size_t with_attrs = 0, unnamed = 0;
  bool depth_area_has_range = false;
  for (const auto& f : ds.features) {
    if (!f.attributes.empty()) ++with_attrs;
    for (const auto& a : f.attributes) {
      if (a.code.rfind("attr", 0) == 0) ++unnamed;  // code missing from ATCS
      if (a.parent != 0) CHECK(a.parent <= f.attributes.size());
    }
    if (f.feature_type == "DepthArea" && f.attribute("depthRangeMinimumValue"))
      depth_area_has_range = true;
  }
  CHECK(with_attrs > 0);
  CHECK(unnamed == 0);
  CHECK(depth_area_has_range);
  std::cout << "attributes ok features_with_attrs=" << with_attrs << "\n";
}
