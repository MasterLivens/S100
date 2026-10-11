#include "s100/scale_filter.hpp"

#include <algorithm>

namespace s100 {

bool visible_at_scale(const DrawingInstruction& di, long view_scale_denom,
                      bool ignore_scale_minimum) {
  // 1. Invalid view scale → never visible.
  if (view_scale_denom <= 0) return false;
  // 2. SCAMIN: hide when zoomed out past scale_minimum (unless ignored).
  if (!ignore_scale_minimum && di.scale_minimum > 0 && view_scale_denom > di.scale_minimum)
    return false;
  // 3. SCAMAX: hide when zoomed in past scale_maximum.
  if (di.scale_maximum > 0 && view_scale_denom < di.scale_maximum) return false;
  // 4. Else visible (0 min/max = none).
  return true;
}

void filter_instructions_by_scale(DrawingInstructions& ios, long view_scale_denom,
                                  bool ignore_scale_minimum) {
  ios.erase(std::remove_if(ios.begin(), ios.end(),
                           [&](const DrawingInstruction& di) {
                             return !visible_at_scale(di, view_scale_denom, ignore_scale_minimum);
                           }),
            ios.end());
}

}  // namespace s100
