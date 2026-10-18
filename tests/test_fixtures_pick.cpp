#include "s100/dataset.hpp"
#include "s100/pick.hpp"
#include "support/test_data_roots.hpp"
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>

#define CHECK(cond)                                                                          \
  do {                                                                                       \
    if (!(cond)) {                                                                           \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";           \
      std::exit(1);                                                                          \
    }                                                                                        \
  } while (0)

// Mirrors testdata/s101/fixtures.toml for 101AA00DS0008.000
void test_fixtures_pick_0008() {
  namespace fs = std::filesystem;
  fs::path root = s100test::fixture_root();
  fs::path cell = root / "101AA00DS0008.000";
  CHECK(fs::exists(cell));

  s100::Dataset ds;
  s100::LoadError err;
  CHECK(s100::open_dataset(cell, ds, &err));
  const auto b = ds.bounds();
  CHECK(b.valid);
  CHECK(std::fabs(b.west - 62.666667) < 1e-4);
  CHECK(std::fabs(b.south - (-32.633333)) < 1e-4);
  CHECK(std::fabs(b.east - 62.8333337) < 1e-4);
  CHECK(std::fabs(b.north - (-32.4666663)) < 1e-4);

  struct Pt {
    const char* name;
    float px, py, w, h;
    double lon, lat;
  };
  const Pt pts[] = {
      {"nw_corner", 0.f, 0.f, 1000.f, 1000.f, 62.666667, -32.4666663},
      {"se_corner", 1000.f, 1000.f, 1000.f, 1000.f, 62.8333337, -32.633333},
      {"center", 500.f, 500.f, 1000.f, 1000.f, 62.75000035, -32.54999965},
  };
  for (const auto& p : pts) {
    auto got = s100::pick_wgs84(p.px, p.py, p.w, p.h, b.west, b.south, b.east, b.north);
    CHECK(s100::within_tolerance(got, s100::Point{p.lon, p.lat}, 1e-5));
    std::cout << "fixture " << p.name << " ok\n";
  }
}
