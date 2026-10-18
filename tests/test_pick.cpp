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
void run_s104_tests();
void run_s111_tests();
void test_s104_open_frame();
void test_s111_open_frame();
void test_s124_open_list();
void test_session_pick_unified();
void run_s124_tests();
void test_pc_wire_111_124();
void test_s102_deepen_multigrid();
void test_scamin_filter();
void test_display_priority_order();
void test_timeline_ui();
void test_association_faui_merge();
void test_exchange_update_robustness();
void test_exchange_cancel_reissue();
void test_exchange_edition_hardening();

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
  if (argc >= 2 && std::string(argv[1]) == "s104_open_frame") {
    test_s104_open_frame();
    if (failures) {
      std::cerr << failures << " failure(s)\n";
      return 1;
    }
    std::cout << "s104_open_frame ok\n";
    return 0;
  }
  if (argc >= 2 && std::string(argv[1]) == "s111_open_frame") {
    test_s111_open_frame();
    if (failures) {
      std::cerr << failures << " failure(s)\n";
      return 1;
    }
    std::cout << "s111_open_frame ok\n";
    return 0;
  }
  if (argc >= 2 && std::string(argv[1]) == "s124_open_list") {
    test_s124_open_list();
    if (failures) {
      std::cerr << failures << " failure(s)\n";
      return 1;
    }
    std::cout << "s124_open_list ok\n";
    return 0;
  }
  if (argc >= 2 && std::string(argv[1]) == "pc_wire_111_124") {
    test_pc_wire_111_124();
    if (failures) {
      std::cerr << failures << " failure(s)\n";
      return 1;
    }
    std::cout << "pc_wire_111_124 ok\n";
    return 0;
  }
  if (argc >= 2 && std::string(argv[1]) == "scamin_filter") {
    test_scamin_filter();
    if (failures) {
      std::cerr << failures << " failure(s)\n";
      return 1;
    }
    std::cout << "scamin_filter ok\n";
    return 0;
  }
  if (argc >= 2 && std::string(argv[1]) == "display_priority_order") {
    test_display_priority_order();
    if (failures) {
      std::cerr << failures << " failure(s)\n";
      return 1;
    }
    std::cout << "display_priority_order ok\n";
    return 0;
  }
  if (argc >= 2 && std::string(argv[1]) == "exchange_edition_hardening") {
    test_exchange_edition_hardening();  // exits non-zero on the first failed check
    return failures ? 1 : 0;
  }
  if (argc >= 2 && std::string(argv[1]) == "exchange_cancel_reissue") {
    test_exchange_cancel_reissue();  // exits non-zero on the first failed check
    return failures ? 1 : 0;
  }
  if (argc >= 2 && std::string(argv[1]) == "exchange_update_robustness") {
    test_exchange_update_robustness();  // exits non-zero on the first failed check
    return failures ? 1 : 0;
  }
  if (argc >= 2 && std::string(argv[1]) == "timeline_ui") {
    test_timeline_ui();
    if (failures) {
      std::cerr << failures << " failure(s)\n";
      return 1;
    }
    std::cout << "timeline_ui ok\n";
    return 0;
  }
  if (argc >= 2 && std::string(argv[1]) == "session_pick_unified") {
    test_session_pick_unified();
    if (failures) {
      std::cerr << failures << " failure(s)\n";
      return 1;
    }
    std::cout << "session_pick_unified ok\n";
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
  run_s104_tests();
  run_s124_tests();
  test_session_pick_unified();
  test_association_faui_merge();
  test_scamin_filter();
  test_display_priority_order();
  test_timeline_ui();
  test_exchange_update_robustness();
  test_exchange_cancel_reissue();
  test_exchange_edition_hardening();
  if (failures) {
    std::cerr << failures << " failure(s)\n";
    return 1;
  }
  std::cout << "all tests passed\n";
  return 0;
}
