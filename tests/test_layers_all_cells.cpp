#include "s100/s100.hpp"
#include "support/test_data_roots.hpp"
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

// Every sample cell (IHO + CHS if present) classifies with no unknown codes, every feature
// is in exactly its FC-code layer, and layer metadata equals the FC entry (not just non-empty).
void test_layers_all_cells() {
  namespace fs = std::filesystem;
  const fs::path src = S100_SOURCE_DIR;
  s100::FeatureCatalogue fc;
  std::string e;
  CHECK(s100::load_feature_catalogue(src / "catalogues/S-101/S-101_FC_2.0.0.xml", fc, &e));

  std::vector<fs::path> cells;
  for (const char* c : {"101AA00DS0003.000", "101AA00DS0006.000", "101AA00DS0008.000"})
    cells.push_back(src / "testdata/s101" / c);
  if (s100test::chs_exercised("layers_all_cells")) {
    const fs::path chs = s100test::chs_s101_dir();
    for (const auto& d : fs::recursive_directory_iterator(chs))
      if (d.path().extension() == ".000") cells.push_back(d.path());
  }

  for (const auto& cell : cells) {
    s100::Dataset ds;
    s100::LoadError err;
    CHECK(s100::open_dataset(cell, ds, &err));
    s100::ClassifyResult cr;
    if (!s100::classify_layers(ds, fc, &cr)) {
      for (const auto& u : cr.unknown_codes) std::cerr << cell.filename() << " unknown " << u << "\n";
      CHECK(false);
    }
    std::size_t sum = 0;
    for (const auto& [code, li] : ds.layers.all()) {
      const auto* ft = fc.find(code);
      CHECK(ft);
      CHECK(li.code == code && li.name == ft->name && li.use_type == ft->use_type);
      CHECK(li.feature_count > 0);  // no empty layers
      sum += li.feature_count;
    }
    CHECK(sum == ds.features.size());
    for (const auto& f : ds.features) CHECK(ds.layers.find(f.feature_type));
    std::cout << "layers " << cell.filename() << " n=" << ds.layers.size() << " ok\n";
  }
}
