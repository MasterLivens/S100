#include "s100/pick.hpp"
#include <cstdlib>
#include <iostream>

static int failures = 0;

#define CHECK(cond)                                                                          \
  do {                                                                                       \
    if (!(cond)) {                                                                           \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";           \
      ++failures;                                                                            \
    }                                                                                        \
  } while (0)

void test_pick_center() {
  auto r = s100::pick_wgs84(50.f, 50.f, 100.f, 100.f, -1.0, -1.0, 1.0, 1.0);
  CHECK(s100::within_tolerance(r, s100::Point{0.0, 0.0}, 1e-5));
}

// declared in other TUs
void test_load_samples();
void test_layers();
void test_attributes();
void test_fixtures_pick_0008();
void test_aggregates_empty_geometry();

int main() {
  test_pick_center();
  test_layers();
  test_load_samples();
  test_attributes();
  test_fixtures_pick_0008();
  test_aggregates_empty_geometry();
  if (failures) {
    std::cerr << failures << " failure(s)\n";
    return 1;
  }
  std::cout << "all tests passed\n";
  return 0;
}
