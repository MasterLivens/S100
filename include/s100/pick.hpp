#pragma once
#include "s100/session.hpp"
#include "s100/types.hpp"
#include "s100/product.hpp"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace s100 {

/** Map screen pixel → WGS84. view = (west, south, east, north); origin top-left. */
PickResult pick_wgs84(float pixel_x, float pixel_y, float width, float height,
                      double west, double south, double east, double north);

bool within_tolerance(const PickResult& got, const Point& expect, double tol_deg);

/** One hit from chart_session_pick (docs/session-pick-contract.md). */
struct SessionPickHit {
  enum class Kind { Feature, CoverageSample, WaterLevel, Current, NavWarning };
  Kind kind{};
  std::string member_key;
  ProductId product{ProductId::Unknown};
  std::uint64_t feature_id{0};  // S-101 feature id; S-124: 0 (use attributes)
  std::string feature_type;     // S-101 FC code; S-124: NW id (also attributes["id"])
  std::string label;
  std::map<std::string, std::string> attributes;
  float depth_or_level{0};
  float speed_ms{0};
  float direction_deg{0};
  bool numeric_valid{false};  // true iff depth_or_level / speed / direction are meaningful
};

struct SessionPickResult {
  double lon{0}, lat{0};
  std::vector<SessionPickHit> hits;  // empty = no hits, still success
};

/** Geographic pick across loaded ChartSession members.
 *  true  = completed (hits may be empty — OOB / ocean)
 *  false = hard failure only; fill err. Never calls set_active_frame / set_active_time. */
bool chart_session_pick(const ChartSession& session, double lon, double lat,
                        SessionPickResult& out, std::string* err = nullptr);

}  // namespace s100
