#pragma once
#include "s100/drawing_instructions.hpp"

#include <map>
#include <string>
#include <string_view>

namespace s100 {

/** Lower rank paints first (underneath). docs/display-priority-contract.md */
int display_plane_rank(std::string_view plane,
                       const std::map<std::string, int>* pc_planes /*nullable*/);

/** True if a must paint before b (underneath). Stable on ties. */
bool draw_order_less(const DrawingInstruction& a, const DrawingInstruction& b,
                     const std::map<std::string, int>* pc_planes);

/** stable_sort ios by draw_order_less. */
void sort_instructions_for_draw(DrawingInstructions& ios,
                                const std::map<std::string, int>* pc_planes = nullptr);

}  // namespace s100
