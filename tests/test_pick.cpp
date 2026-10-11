#include "s100/pick.hpp"
#include <cstdlib>
#include <iostream>
#include <string>

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
void test_soundings();
void test_feature_catalogue();
void test_bounds_multipoint_only();
void test_fixtures_pick_0008();
void test_aggregates_empty_geometry();
void test_chs_bounds_vs_catalog();
void test_fc_permitted_primitives();
void test_layers_all_cells();
void test_exchange_set_synthetic_and_chs();
void run_s102_tests();
void test_s102_deepen_multigrid();
void test_association_faui_merge();

int main(int argc, char** argv) {
  if (argc >= 2 && std::string(argv[1]) == "s102_deepen_multigrid") {
    test_s102_deepen_multigrid();
    if (failures) {
      std::cerr << failures << " failure(s)\n";
      return 1;
    }
    std::cout << "s102_deepen_multigrid ok\n";
    return 0;
  }
  test_pick_center();
  test_layers();
  test_load_samples();
  test_attributes();
  test_soundings();
  test_feature_catalogue();
  test_bounds_multipoint_only();
  test_fixtures_pick_0008();
  test_aggregates_empty_geometry();
  test_chs_bounds_vs_catalog();
  test_fc_permitted_primitives();
  test_layers_all_cells();
  test_exchange_set_synthetic_and_chs();
  run_s102_tests();
  test_association_faui_merge();
  if (failures) {
    std::cerr << failures << " failure(s)\n";
    return 1;
  }
  std::cout << "all tests passed\n";
  return 0;
}
