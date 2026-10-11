#pragma once
#include "s100/drawing_instructions.hpp"

namespace s100 {

/** Returns false → caller must not paint this DI (docs/scamin-contract.md). */
bool visible_at_scale(const DrawingInstruction& di, long view_scale_denom,
                      bool ignore_scale_minimum);

/** Erase so only visible_at_scale==true remain; stable relative order. */
void filter_instructions_by_scale(DrawingInstructions& ios, long view_scale_denom,
                                  bool ignore_scale_minimum);

}  // namespace s100
