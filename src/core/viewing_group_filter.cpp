#include "s100/viewing_group_filter.hpp"

#include <algorithm>

namespace s100 {

bool visible_by_viewing_group(const DrawingInstruction& di, const std::set<std::string>& hidden_groups) {
  if (hidden_groups.empty()) return true;
  if (!di.viewing_group.empty() && hidden_groups.count(di.viewing_group)) return false;
  for (const auto& vg : di.viewing_groups)
    if (hidden_groups.count(vg)) return false;
  return true;
}

void filter_instructions_by_viewing_group(DrawingInstructions& ios,
                                          const std::set<std::string>& hidden_groups) {
  if (hidden_groups.empty()) return;
  ios.erase(std::remove_if(ios.begin(), ios.end(),
                           [&](const DrawingInstruction& di) {
                             return !visible_by_viewing_group(di, hidden_groups);
                           }),
            ios.end());
}

}  // namespace s100
