#include "s100/dataset.hpp"
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

// Features without SPAS (aggregates such as Bridge, geometry via FASC) may be Empty;
// every other feature must carry geometry. P1 will render aggregates via FASC.
void test_aggregates_empty_geometry() {
  namespace fs = std::filesystem;
  const char* env = std::getenv("S100_TESTDATA");
  fs::path root = env ? fs::path(env) : fs::path("testdata/s101");
  for (const char* c : {"101AA00DS0003.000", "101AA00DS0006.000", "101AA00DS0008.000"}) {
    s100::Dataset ds;
    s100::LoadError err;
    CHECK(s100::open_dataset(root / c, ds, &err));
    std::size_t empty = 0;
    for (const auto& f : ds.features) {
      if (f.geometry.kind == s100::GeometryKind::Empty) {
        ++empty;
        if (f.feature_type != "Bridge") {
          std::cerr << c << " unexpected empty geometry: " << f.feature_type << " id=" << f.id << "\n";
          std::exit(1);
        }
      }
    }
    std::cout << "aggregates " << c << " empty=" << empty << " ok\n";
  }
}
