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

// Real-data check on CHS St. Lawrence cells (testing-only licence, gitignored); skips when absent.
void test_soundings() {
  namespace fs = std::filesystem;
  if (!s100test::chs_exercised("soundings")) return;
  const fs::path base = s100test::chs_s101_dir();
  std::size_t cells = 0;
  for (const auto& e : fs::recursive_directory_iterator(base)) {
    if (e.path().extension() != ".000") continue;
    s100::Dataset ds;
    s100::LoadError err;
    CHECK(s100::open_dataset(e.path(), ds, &err));
    std::size_t soundings = 0, points = 0;
    // Reference area from the producer, not from our own decode: envelope of the
    // cell's DataCoverage features (bounds() includes soundings, so it would be circular).
    s100::Dataset coverage;
    for (const auto& f : ds.features)
      if (f.feature_type == "DataCoverage") coverage.features.push_back(f);
    CHECK(!coverage.features.empty());
    const auto b = coverage.bounds();
    CHECK(b.valid);
    for (const auto& f : ds.features) {
      if (f.feature_type != "Sounding") continue;
      ++soundings;
      CHECK(f.geometry.kind == s100::GeometryKind::MultiPoint);
      CHECK(!f.geometry.multipoint.empty());
      for (const auto& s : f.geometry.multipoint) {
        ++points;
        CHECK(s.lon >= b.west - 1e-9 && s.lon <= b.east + 1e-9);
        CHECK(s.lat >= b.south - 1e-9 && s.lat <= b.north + 1e-9);
        CHECK(s.z > -50.0 && s.z < 500.0);  // plausible river/estuary depths (m)
      }
    }
    CHECK(soundings > 0);
    std::cout << "soundings ok " << e.path().filename() << " features=" << soundings
              << " points=" << points << "\n";
    ++cells;
  }
  CHECK(cells > 0);
}

// bounds() must include MultiPoint so a soundings-only dataset still has a valid envelope.
void test_bounds_multipoint_only() {
  s100::Dataset ds;
  s100::Feature f;
  f.feature_type = "Sounding";
  f.geometry.kind = s100::GeometryKind::MultiPoint;
  f.geometry.multipoint = {{-71.2, 46.8, 5.0}, {-71.1, 46.9, 12.0}};
  ds.features.push_back(f);
  const auto b = ds.bounds();
  CHECK(b.valid);
  CHECK(b.west == -71.2 && b.east == -71.1 && b.south == 46.8 && b.north == 46.9);
}
