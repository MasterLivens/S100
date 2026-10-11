#include "s100/drawing_instructions.hpp"
#include "s100/scale_filter.hpp"

#include <cstdlib>
#include <iostream>
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

s100::DrawingInstruction make_di(const char* id, long scale_minimum, long scale_maximum) {
  s100::DrawingInstruction di;
  di.command = id;
  di.kind = s100::DrawingInstruction::Kind::Point;
  di.scale_minimum = scale_minimum;
  di.scale_maximum = scale_maximum;
  return di;
}

std::vector<std::string> ids_of(const s100::DrawingInstructions& ios) {
  std::vector<std::string> out;
  out.reserve(ios.size());
  for (const auto& di : ios) out.push_back(di.command);
  return out;
}

}  // namespace

/** IHO-SCAMIN-FILTER / CTest scamin_filter — docs/scamin-contract.md oracles A–H. */
void test_scamin_filter() {
  using s100::visible_at_scale;
  using s100::filter_instructions_by_scale;

  const auto A = make_di("A", 0, 0);
  const auto B = make_di("B", 22000, 0);
  const auto C = make_di("C", 22000, 0);
  const auto D = make_di("D", 22000, 0);
  const auto E = make_di("E", 0, 10000);
  const auto F = make_di("F", 0, 10000);
  const auto G = make_di("G", 22000, 10000);
  const auto H = make_di("H", 22000, 10000);

  CHECK(visible_at_scale(A, 12000, false) == true);
  CHECK(visible_at_scale(B, 12000, false) == true);
  CHECK(visible_at_scale(C, 50000, false) == false);
  CHECK(visible_at_scale(D, 50000, true) == true);
  CHECK(visible_at_scale(E, 5000, false) == false);
  CHECK(visible_at_scale(F, 12000, false) == true);
  CHECK(visible_at_scale(G, 15000, false) == true);
  CHECK(visible_at_scale(H, 0, false) == false);
  CHECK(visible_at_scale(H, -1, false) == false);
  std::cout << "scamin oracle A–H ok\n";

  // Batch: [A,B,C] at 50000/false → [A]
  {
    s100::DrawingInstructions ios{A, B, C};
    filter_instructions_by_scale(ios, 50000, false);
    const auto ids = ids_of(ios);
    CHECK(ids.size() == 1 && ids[0] == "A");
  }
  // Same list at 50000/ignore=true → [A,B,C]
  {
    s100::DrawingInstructions ios{A, B, C};
    filter_instructions_by_scale(ios, 50000, true);
    const auto ids = ids_of(ios);
    CHECK(ids.size() == 3 && ids[0] == "A" && ids[1] == "B" && ids[2] == "C");
  }
  std::cout << "scamin batch filter ok\n";

  // DEF parse: ScaleMinimum:22000 → field 22000
  {
    s100::DrawingInstructions di;
    s100::PortrayalStats st;
    s100::parse_drawing_instructions(
        1, "ScaleMinimum:22000;PointInstruction:TESTSYM", di, &st);
    CHECK(di.size() == 1);
    CHECK(di[0].scale_minimum == 22000);
  }
  std::cout << "scamin DEF ScaleMinimum parse ok\n";
  std::cout << "scamin_filter OK (IHO-SCAMIN-FILTER)\n";
}
