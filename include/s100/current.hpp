#pragma once
// S-111 surface current: time-framed speed/direction planes (docs/s111-contract.md).
#include "s100/coverage.hpp"
#include "s100/drawing_instructions.hpp"
#include "s100/product.hpp"
#include "s100/types.hpp"
#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace s100 {

/** Emit Arrow DI every kS111ArrowStride cells on each axis (edges inclusive). */
inline constexpr std::size_t kS111ArrowStride = 2;

struct CurrentSample {
  float speed_ms{0};       // m/s, >= 0
  float direction_deg{0};  // from-direction, deg clockwise from true N, in [0, 360)
  bool valid{false};
};

struct CurrentFrame {
  std::string time_iso;    // UTC
  CoverageGrid speed;      // values = speed_ms (stored in CoverageGrid::depth)
  CoverageGrid direction;  // values = direction_deg (same shape as speed)
};

struct CurrentDataset {
  ProductId product{ProductId::S111};
  std::filesystem::path path;
  std::string dataset_name;
  std::string product_specification;
  std::vector<CurrentFrame> frames;
  std::size_t active_frame{0};
  std::vector<CoverageFeature> features;

  Bounds bounds() const;
  /** false if index >= frames.size(); no change on failure. */
  bool set_active_frame(std::size_t index);
  /** Nearest |Δt|; ties → lower index; empty frames → false. */
  bool set_active_time(std::string_view iso_utc);
  /** Active frame only; façade is speed + direction_deg (no u/v). */
  bool sample_wgs84(double lon, double lat, CurrentSample& out) const;

  const CurrentFrame* active() const {
    return active_frame < frames.size() ? &frames[active_frame] : nullptr;
  }
};

/** Emit Arrow instructions for the active frame (stride kS111ArrowStride). ≥1 Arrow. */
bool portray_current(const CurrentDataset& cur, DrawingInstructions& out,
                     std::string* err = nullptr);

}  // namespace s100
