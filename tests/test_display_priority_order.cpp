#include "s100/draw_order.hpp"
#include "s100/drawing_instructions.hpp"

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#define CHECK(cond)                                                                \
  do {                                                                             \
    if (!(cond)) {                                                                 \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n"; \
      std::exit(1);                                                                \
    }                                                                              \
  } while (0)

namespace {

s100::DrawingInstruction make_di(const char* id, const char* plane, int priority) {
  s100::DrawingInstruction di;
  di.command = id;
  di.kind = s100::DrawingInstruction::Kind::Point;
  di.display_plane = plane;
  di.drawing_priority = priority;
  return di;
}

std::vector<std::string> ids_of(const s100::DrawingInstructions& ios) {
  std::vector<std::string> out;
  out.reserve(ios.size());
  for (const auto& di : ios) out.push_back(di.command);
  return out;
}

bool ids_eq(const std::vector<std::string>& got, std::initializer_list<const char*> expect) {
  if (got.size() != expect.size()) return false;
  std::size_t i = 0;
  for (const char* e : expect) {
    if (got[i++] != e) return false;
  }
  return true;
}

}  // namespace

/** IHO-DISPLAY-PRIORITY / CTest display_priority_order — docs/display-priority-contract.md. */
void test_display_priority_order() {
  using s100::sort_instructions_for_draw;

  const auto P0 = make_di("P0", "OverRadar", 0);
  const auto P1 = make_di("P1", "UnderRadar", 5);
  const auto P2 = make_di("P2", "UnderRadar", 1);
  const auto P3 = make_di("P3", "OverRadar", 2);
  const auto P4 = make_di("P4", "", 3);
  const auto P5 = make_di("P5", "UnderRadar", 1);

  // nullptr → fallback ranks: Under+empty=0 before Over=1; stable on ties.
  {
    s100::DrawingInstructions ios{P0, P1, P2, P3, P4, P5};
    sort_instructions_for_draw(ios, nullptr);
    const auto ids = ids_of(ios);
    CHECK(ids_eq(ids, {"P2", "P5", "P4", "P1", "P0", "P3"}));
    std::cout << "display_priority nullptr order ok: P2,P5,P4,P1,P0,P3\n";
  }

  // Inverted pc_planes: Over=0, empty missing→fallback 0, Under=10.
  {
    const std::map<std::string, int> pc_planes{{"UnderRadar", 10}, {"OverRadar", 0}};
    s100::DrawingInstructions ios{P0, P1, P2, P3, P4, P5};
    sort_instructions_for_draw(ios, &pc_planes);
    const auto ids = ids_of(ios);
    CHECK(ids_eq(ids, {"P0", "P3", "P4", "P2", "P5", "P1"}));
    std::cout << "display_priority inverted pc_planes order ok: P0,P3,P4,P2,P5,P1\n";
  }

  // DEF parse DrawingPriority:7;DisplayPlane:OverRadar
  {
    s100::DrawingInstructions di;
    s100::PortrayalStats st;
    s100::parse_drawing_instructions(
        1, "DrawingPriority:7;DisplayPlane:OverRadar;PointInstruction:TESTSYM", di, &st);
    CHECK(di.size() == 1);
    CHECK(di[0].drawing_priority == 7);
    CHECK(di[0].display_plane == "OverRadar");
    std::cout << "display_priority DEF parse ok\n";
  }

  std::cout << "display_priority_order OK (IHO-DISPLAY-PRIORITY)\n";
}
