#include "s100/s100.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <set>

#define CHECK(cond)                                                                    \
  do {                                                                                 \
    if (!(cond)) {                                                                     \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";     \
      std::exit(1);                                                                    \
    }                                                                                  \
  } while (0)

void test_feature_catalogue() {
  namespace fs = std::filesystem;
  const fs::path fc_path = fs::path(CMAKE_SOURCE_DIR_STR) / "catalogues/S-101/S-101_FC_2.0.0.xml";
  s100::FeatureCatalogue fc;
  std::string err;
  CHECK(s100::load_feature_catalogue(fc_path, fc, &err));
  CHECK(fc.product == "S-101");
  CHECK(fc.version == "2.0.0");
  CHECK(fc.feature_types.size() == 190);

  const auto* depth = fc.find("DepthArea");
  CHECK(depth && depth->name == "Depth Area" && depth->use_type == "geographic");
  const auto* sounding = fc.find("Sounding");
  CHECK(sounding);
  CHECK(fc.find("DataCoverage") && fc.find("DataCoverage")->use_type == "meta");

  // Every feature type present in our sample cells must exist in the FC (IHO TDS is S-101 2.0.0).
  const fs::path cell = fs::path(CMAKE_SOURCE_DIR_STR) / "testdata/s101/101AA00DS0008.000";
  s100::Dataset ds;
  s100::LoadError lerr;
  CHECK(s100::open_dataset(cell, ds, &lerr));
  std::set<std::string> missing;
  for (const auto& f : ds.features)
    if (!fc.find(f.feature_type)) missing.insert(f.feature_type);
  for (const auto& m : missing) std::cerr << "not in FC: " << m << "\n";
  CHECK(missing.empty());
  std::cout << "feature catalogue ok " << fc.product << " " << fc.version << " types="
            << fc.feature_types.size() << "\n";
}
