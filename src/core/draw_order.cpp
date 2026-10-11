#include "s100/draw_order.hpp"

#include <algorithm>

namespace s100 {

int display_plane_rank(std::string_view plane,
                       const std::map<std::string, int>* pc_planes) {
  if (pc_planes) {
    const auto it = pc_planes->find(std::string(plane));
    if (it != pc_planes->end()) return it->second;
  }
  // Fallback: OverRadar → 1; empty / UnderRadar / anything else → 0.
  return plane == "OverRadar" ? 1 : 0;
}

bool draw_order_less(const DrawingInstruction& a, const DrawingInstruction& b,
                     const std::map<std::string, int>* pc_planes) {
  const int ra = display_plane_rank(a.display_plane, pc_planes);
  const int rb = display_plane_rank(b.display_plane, pc_planes);
  if (ra != rb) return ra < rb;
  if (a.drawing_priority != b.drawing_priority)
    return a.drawing_priority < b.drawing_priority;
  return false;
}

void sort_instructions_for_draw(DrawingInstructions& ios,
                                const std::map<std::string, int>* pc_planes) {
  std::stable_sort(ios.begin(), ios.end(),
                   [pc_planes](const DrawingInstruction& a, const DrawingInstruction& b) {
                     return draw_order_less(a, b, pc_planes);
                   });
}

}  // namespace s100
