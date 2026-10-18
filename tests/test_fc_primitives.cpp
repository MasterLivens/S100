#include "s100/dataset.hpp"
#include "s100/feature_catalogue.hpp"
#include "support/test_data_roots.hpp"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>
#include <set>

#define CHECK(cond)                                                                \
  do {                                                                             \
    if (!(cond)) {                                                                 \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n"; \
      std::exit(1);                                                                \
    }                                                                              \
  } while (0)

namespace {
const char* primitive_of(s100::GeometryKind k) {
  switch (k) {
    case s100::GeometryKind::Point: return "point";
    case s100::GeometryKind::MultiPoint: return "pointSet";
    case s100::GeometryKind::LineString: return "curve";
    case s100::GeometryKind::Polygon: return "surface";
    default: return nullptr;
  }
}
// permittedPrimitives may be declared on an abstract super type.
std::vector<std::string> permitted(const s100::FeatureCatalogue& fc, const std::string& code) {
  for (const auto* t = fc.find(code); t; t = t->super_type.empty() ? nullptr : fc.find(t->super_type))
    if (!t->primitives.empty()) return t->primitives;
  return {};
}
}  // namespace

// Independent oracle: every decoded geometry must be a primitive the official FC permits
// for that feature type. Catches decode/topology mistakes our own output cannot.
void test_fc_permitted_primitives() {
  namespace fs = std::filesystem;
  s100::FeatureCatalogue fc;
  std::string e;
  CHECK(s100::load_feature_catalogue(fs::path(S100_SOURCE_DIR) / "catalogues/S-101/S-101_FC_2.0.0.xml", fc, &e));

  std::vector<fs::path> cells;
  for (const char* c : {"101AA00DS0003.000", "101AA00DS0006.000", "101AA00DS0008.000"})
    cells.push_back(fs::path(S100_SOURCE_DIR) / "testdata/s101" / c);
  if (s100test::chs_exercised("fc_primitives")) {
    const fs::path chs = s100test::chs_s101_dir();
    for (const auto& d : fs::recursive_directory_iterator(chs))
      if (d.path().extension() == ".000") cells.push_back(d.path());
  }

  std::map<std::string, int> bad;  // "cell type kind" -> count
  for (const auto& cell : cells) {
    s100::Dataset ds;
    s100::LoadError err;
    CHECK(s100::open_dataset(cell, ds, &err));
    for (const auto& f : ds.features) {
      const char* prim = primitive_of(f.geometry.kind);
      if (!prim) continue;  // Empty: aggregates / no-geometry types, covered elsewhere
      CHECK(fc.find(f.feature_type) != nullptr);
      auto allowed = permitted(fc, f.feature_type);
      if (std::find(allowed.begin(), allowed.end(), prim) == allowed.end())
        ++bad[cell.filename().string() + " " + f.feature_type + " " + prim];
    }
    std::cout << "fc primitives checked " << cell.filename() << "\n";
  }
  for (const auto& [k, n] : bad) std::cerr << "not permitted by FC: " << k << " x" << n << "\n";
  CHECK(bad.empty());
}
