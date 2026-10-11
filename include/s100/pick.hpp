#pragma once
#include "s100/types.hpp"

namespace s100 {

/** Map window pixel → WGS84. view = (west, south, east, north); origin top-left. */
PickResult pick_wgs84(float pixel_x, float pixel_y, float width, float height,
                      double west, double south, double east, double north);

bool within_tolerance(const PickResult& got, const Point& expect, double tol_deg);

}  // namespace s100
