#include "s100/pick.hpp"
#include <cmath>

namespace s100 {

PickResult pick_wgs84(float pixel_x, float pixel_y, float width, float height,
                      double west, double south, double east, double north) {
  const double w = width > 0.f ? static_cast<double>(width) : 1.0;
  const double h = height > 0.f ? static_cast<double>(height) : 1.0;
  const double u = static_cast<double>(pixel_x) / w;
  const double v = static_cast<double>(pixel_y) / h;
  PickResult r;
  r.lon = west + u * (east - west);
  r.lat = north - v * (north - south);
  return r;
}

bool within_tolerance(const PickResult& got, const Point& expect, double tol_deg) {
  return std::fabs(got.lon - expect.lon) <= tol_deg &&
         std::fabs(got.lat - expect.lat) <= tol_deg;
}

}  // namespace s100
