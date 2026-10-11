#include "s100/dataset.hpp"
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>

#define CHECK(cond)                                                                \
  do {                                                                             \
    if (!(cond)) {                                                                 \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n"; \
      std::exit(1);                                                                \
    }                                                                              \
  } while (0)

// Independent oracle: decoded bounds must match the producer's CATALOG.XML bounding
// boxes (CHS real data, Quebec / St. Lawrence). Catches lat/lon swaps that synthetic
// IHO cells cannot. Data is test-only licence and git-ignored: skip when absent.
void test_chs_bounds_vs_catalog() {
  namespace fs = std::filesystem;
  const char* env = std::getenv("S100_CHS_ROOT");
  fs::path root = env ? fs::path(env) : fs::path(S100_SOURCE_DIR "/testdata/chs/COMBINED/S100_ROOT");
  if (!fs::exists(root)) {
    std::cerr << "skip chs: no " << root << "\n";
    return;
  }
  struct Cell { const char* rel; double w, s, e, n; };
  const Cell cells[] = {
      {"S-101/DATASET_FILES/CA00/101CA00P468N0712W/2/101CA00P468N0712W.000",
       -71.2, 46.8, -71.1278611, 46.8855778},
      {"S-101/DATASET_FILES/CA00/101CA00T460N0720W/6/101CA00T460N0720W.000",
       -72.0, 46.4333583, -71.0, 46.9947604},
  };
  for (const auto& c : cells) {
    s100::Dataset ds;
    s100::LoadError err;
    CHECK(s100::open_dataset(root / c.rel, ds, &err));
    const auto b = ds.bounds();
    CHECK(b.valid);
    CHECK(std::fabs(b.west - c.w) < 1e-6);
    CHECK(std::fabs(b.south - c.s) < 1e-6);
    CHECK(std::fabs(b.east - c.e) < 1e-6);
    CHECK(std::fabs(b.north - c.n) < 1e-6);
    std::cout << "chs bounds " << fs::path(c.rel).filename() << " ok\n";
  }
}
