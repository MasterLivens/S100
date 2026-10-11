#include "s100/s100.hpp"
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

// FC-driven layers (docs/api-contracts.md, Layers): key = FC code, no fallback layer.
void test_layers() {
  namespace fs = std::filesystem;
  const fs::path src = CMAKE_SOURCE_DIR_STR;
  s100::FeatureCatalogue fc;
  std::string fc_err;
  CHECK(s100::load_feature_catalogue(src / "catalogues/S-101/S-101_FC_2.0.0.xml", fc, &fc_err));

  s100::Dataset ds;
  s100::LoadError err;
  CHECK(s100::open_dataset(src / "testdata/s101/101AA00DS0008.000", ds, &err));
  CHECK(ds.layers.empty());  // codec stays FC-free

  s100::ClassifyResult cr;
  CHECK(s100::classify_layers(ds, fc, &cr));
  CHECK(cr.unknown_codes.empty());
  std::size_t counted = 0;
  for (const auto& f : ds.features) {
    const s100::LayerInfo* li = ds.layers.find(f.feature_type);
    CHECK(li && li->code == f.feature_type);
    CHECK(!li->use_type.empty() && !li->name.empty());
  }
  for (const auto& [code, li] : ds.layers.all()) counted += li.feature_count;
  CHECK(counted == ds.features.size());

  // Unknown code → false, listed, no fallback layer.
  s100::Dataset bad = ds;
  bad.features.front().feature_type = "NotAFeatureType";
  s100::ClassifyResult bad_r;
  CHECK(!s100::classify_layers(bad, fc, &bad_r));
  CHECK(bad_r.unknown_codes.size() == 1 && bad_r.unknown_codes[0] == "NotAFeatureType");
  CHECK(!bad.layers.find("NotAFeatureType") && !bad.layers.find("Other"));

  // Visibility: defaults geographic on / meta off; layer rule overrides use-type rule.
  s100::LayerVisibility v;
  const auto* depth = ds.layers.find("DepthArea");
  const auto* cov = ds.layers.find("DataCoverage");
  CHECK(depth && cov && cov->use_type == "meta");
  CHECK(v.is_visible(*depth));
  CHECK(!v.is_visible(*cov));
  v.set_layer("DataCoverage", true);
  CHECK(v.is_visible(*cov));
  v.set_use_type("geographic", false);
  CHECK(!v.is_visible(*depth));
  v.set_layer("DepthArea", true);
  CHECK(v.is_visible(*depth));
  std::cout << "layers ok count=" << ds.layers.size() << "\n";
}
