#pragma once
#include "s100/drawing_instructions.hpp"

#include <set>
#include <string>

namespace s100 {

/** True if `di` is enabled with `hidden_groups` switched off. S-100 Part 9 9-11.1.3: a drawing
 *  instruction with several viewing groups is disabled when ANY assigned group is disabled
 *  (e.g. a TextInstruction 'ViewingGroup:11,12210' hides with its feature group 12210).
 *  Checks `viewing_group` and every entry of `viewing_groups`. */
bool visible_by_viewing_group(const DrawingInstruction& di, const std::set<std::string>& hidden_groups);

/** Erase every DI that is not visible_by_viewing_group; stable order of the rest.
 *  Pipeline: VG filter -> SCAMIN filter -> sort -> paint (docs/bridge-render-contract.md hook 6). */
void filter_instructions_by_viewing_group(DrawingInstructions& ios,
                                          const std::set<std::string>& hidden_groups);

}  // namespace s100
