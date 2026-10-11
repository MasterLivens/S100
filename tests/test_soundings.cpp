#include "s100/dataset.hpp"
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
  const fs::path root = "testdata/chs";
  fs::path base = fs::exists(root) ? root : fs::path(CMAKE_SOURCE_DIR_STR) / root;
  if (!fs::exists(base)) {
    std::cerr << "skip soundings: no " << base << "\n";
    return;
  }
  std::size_t cells = 0;
  for (const auto& e : fs::recursive_directory_iterator(base)) {
    if (e.path().extension() != ".000" || e.path().string().find("S-101") == std::string::npos)
      continue;
    s100::Dataset ds;
    s100::LoadError err;
    CHECK(s100::open_dataset(e.path(), ds, &err));
    std::size_t soundings = 0, points = 0;
    for (const auto& f : ds.features) {
      if (f.feature_type != "Sounding") continue;
      ++soundings;
      CHECK(f.geometry.kind == s100::GeometryKind::MultiPoint);
      CHECK(!f.geometry.multipoint.empty());
      const auto b = ds.bounds();
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
