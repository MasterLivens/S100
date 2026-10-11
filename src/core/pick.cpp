#include "s100/pick.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace s100 {
namespace {

constexpr double kPickTolDeg = 1e-4;

double dist2(double lon0, double lat0, double lon1, double lat1) {
  const double dlon = lon0 - lon1;
  const double dlat = lat0 - lat1;
  return dlon * dlon + dlat * dlat;
}

bool point_near(double lon, double lat, double plon, double plat, double tol_deg) {
  return dist2(lon, lat, plon, plat) <= tol_deg * tol_deg;
}

/** Distance from point to segment (lon/lat degrees), squared. */
double dist2_to_segment(double lon, double lat, double ax, double ay, double bx, double by) {
  const double abx = bx - ax;
  const double aby = by - ay;
  const double apx = lon - ax;
  const double apy = lat - ay;
  const double ab2 = abx * abx + aby * aby;
  if (ab2 <= 0.0) return dist2(lon, lat, ax, ay);
  double t = (apx * abx + apy * aby) / ab2;
  if (t < 0.0) t = 0.0;
  if (t > 1.0) t = 1.0;
  return dist2(lon, lat, ax + t * abx, ay + t * aby);
}

/** Ray-cast point-in-polygon (lon/lat). Closing vertex optional. */
bool point_in_ring(double lon, double lat, const std::vector<Point>& ring) {
  if (ring.size() < 3) return false;
  std::size_t n = ring.size();
  // Drop repeated closing vertex for edge iteration consistency.
  if (n >= 2 && ring.front().lon == ring.back().lon && ring.front().lat == ring.back().lat) --n;
  if (n < 3) return false;

  bool inside = false;
  for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
    const double xi = ring[i].lon, yi = ring[i].lat;
    const double xj = ring[j].lon, yj = ring[j].lat;
    const bool intersect =
        ((yi > lat) != (yj > lat)) &&
        (lon < (xj - xi) * (lat - yi) / (yj - yi + std::numeric_limits<double>::epsilon()) + xi);
    if (intersect) inside = !inside;
  }
  return inside;
}

bool geometry_hits(const Geometry& g, double lon, double lat, double tol_deg) {
  switch (g.kind) {
    case GeometryKind::Point:
      return point_near(lon, lat, g.point.lon, g.point.lat, tol_deg);
    case GeometryKind::MultiPoint:
      for (const auto& p : g.multipoint)
        if (point_near(lon, lat, p.lon, p.lat, tol_deg)) return true;
      return false;
    case GeometryKind::LineString: {
      if (g.line.size() == 1) return point_near(lon, lat, g.line[0].lon, g.line[0].lat, tol_deg);
      const double tol2 = tol_deg * tol_deg;
      for (std::size_t i = 1; i < g.line.size(); ++i) {
        if (dist2_to_segment(lon, lat, g.line[i - 1].lon, g.line[i - 1].lat, g.line[i].lon,
                             g.line[i].lat) <= tol2)
          return true;
      }
      return false;
    }
    case GeometryKind::Polygon: {
      if (g.polygon.empty()) return false;
      // Exterior = first ring; subsequent rings are holes.
      if (!point_in_ring(lon, lat, g.polygon.front().points)) return false;
      for (std::size_t i = 1; i < g.polygon.size(); ++i)
        if (point_in_ring(lon, lat, g.polygon[i].points)) return false;
      return true;
    }
    default:
      return false;
  }
}

SessionPickHit make_feature_hit(const std::string& key, const Feature& f) {
  SessionPickHit h;
  h.kind = SessionPickHit::Kind::Feature;
  h.member_key = key;
  h.product = ProductId::S101;
  h.feature_id = f.id;
  h.feature_type = f.feature_type;
  h.label = f.feature_type;
  h.numeric_valid = false;
  return h;
}

SessionPickHit make_nw_hit(const std::string& key, const NavWarning& w) {
  SessionPickHit h;
  h.kind = SessionPickHit::Kind::NavWarning;
  h.member_key = key;
  h.product = ProductId::S124;
  h.feature_id = 0;
  h.feature_type = w.id;
  h.label = w.title;
  h.attributes["id"] = w.id;
  h.attributes["title"] = w.title;
  h.attributes["time_iso"] = w.time_iso;
  h.numeric_valid = false;
  return h;
}

}  // namespace

PickResult pick_wgs84(float pixel_x, float pixel_y, float width, float height, double west,
                      double south, double east, double north) {
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

bool chart_session_pick(const ChartSession& session, double lon, double lat, SessionPickResult& out,
                        std::string* err) {
  out = {};
  out.lon = lon;
  out.lat = lat;
  if (err) err->clear();

  std::vector<SessionPickHit> features;
  std::vector<SessionPickHit> nav_warnings;
  std::vector<SessionPickHit> coverage;
  std::vector<SessionPickHit> water_levels;
  std::vector<SessionPickHit> currents;

  for (const auto& [key, ds] : session.datasets()) {
    // 1) S-101 Features
    if (ds.product == ProductId::S101) {
      for (const auto& f : ds.features) {
        if (geometry_hits(f.geometry, lon, lat, kPickTolDeg))
          features.push_back(make_feature_hit(key, f));
      }
    }

    // 2) S-124 NavWarnings
    if (ds.nav_warnings) {
      for (const auto& w : ds.nav_warnings->warnings) {
        bool hit = false;
        for (const auto& g : w.geometries) {
          if (geometry_hits(g, lon, lat, kPickTolDeg)) {
            hit = true;
            break;
          }
        }
        if (hit) nav_warnings.push_back(make_nw_hit(key, w));
      }
    }

    // 3) S-102 CoverageSample — active grids as already defined; no frame API
    if (ds.coverage) {
      CoverageSample sm;
      if (ds.coverage->sample_wgs84(lon, lat, sm) && sm.has_depth) {
        SessionPickHit h;
        h.kind = SessionPickHit::Kind::CoverageSample;
        h.member_key = key;
        h.product = ProductId::S102;
        h.depth_or_level = sm.depth;
        h.numeric_valid = true;
        coverage.push_back(std::move(h));
      }
    }

    // 4) S-104 WaterLevel — current active_frame only (do not set_active_*)
    if (ds.water_level) {
      CoverageSample sm;
      if (ds.water_level->sample_wgs84(lon, lat, sm) && sm.has_depth) {
        SessionPickHit h;
        h.kind = SessionPickHit::Kind::WaterLevel;
        h.member_key = key;
        h.product = ProductId::S104;
        h.depth_or_level = sm.depth;
        h.numeric_valid = true;
        water_levels.push_back(std::move(h));
      }
    }

    // 5) S-111 Current — current active_frame only
    if (ds.current) {
      CurrentSample sm;
      if (ds.current->sample_wgs84(lon, lat, sm) && sm.valid) {
        SessionPickHit h;
        h.kind = SessionPickHit::Kind::Current;
        h.member_key = key;
        h.product = ProductId::S111;
        h.speed_ms = sm.speed_ms;
        h.direction_deg = sm.direction_deg;
        h.numeric_valid = true;
        currents.push_back(std::move(h));
      }
    }
  }

  std::sort(features.begin(), features.end(), [](const SessionPickHit& a, const SessionPickHit& b) {
    if (a.feature_id != b.feature_id) return a.feature_id < b.feature_id;
    return a.member_key < b.member_key;
  });
  std::sort(nav_warnings.begin(), nav_warnings.end(),
            [](const SessionPickHit& a, const SessionPickHit& b) {
              if (a.feature_type != b.feature_type) return a.feature_type < b.feature_type;
              return a.member_key < b.member_key;
            });

  out.hits.reserve(features.size() + nav_warnings.size() + coverage.size() + water_levels.size() +
                   currents.size());
  out.hits.insert(out.hits.end(), features.begin(), features.end());
  out.hits.insert(out.hits.end(), nav_warnings.begin(), nav_warnings.end());
  out.hits.insert(out.hits.end(), coverage.begin(), coverage.end());
  out.hits.insert(out.hits.end(), water_levels.begin(), water_levels.end());
  out.hits.insert(out.hits.end(), currents.begin(), currents.end());
  return true;
}

}  // namespace s100
