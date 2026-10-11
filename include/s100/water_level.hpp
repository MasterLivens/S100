#pragma once
// S-104 water level: time-framed CoverageGrid planes (docs/s104-contract.md).
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

struct WaterLevelFrame {
  std::string time_iso;  // UTC
  CoverageGrid grid;     // one WL plane (values-as-level in CoverageGrid::depth)
};

struct WaterLevelDataset {
  ProductId product{ProductId::S104};
  std::filesystem::path path;
  std::string dataset_name;
  std::string product_specification;
  std::vector<WaterLevelFrame> frames;  // size >= 1 after successful open
  std::size_t active_frame{0};
  std::vector<CoverageFeature> features;

  Bounds bounds() const;
  bool set_active_frame(std::size_t index);
  /** Nearest frame by time; always picks one if frames non-empty. */
  bool set_active_time(std::string_view iso_utc);
  /** Active frame only; CoverageSample::depth carries water level (metres). */
  bool sample_wgs84(double lon, double lat, CoverageSample& out) const;

  const WaterLevelFrame* active() const {
    return active_frame < frames.size() ? &frames[active_frame] : nullptr;
  }
};

/** Emit ≥1 CoverageFill instruction for the active frame (minimal tokenized surface;
 *  official S-104 PC deferred — docs/s104-contract.md WARN). */
bool portray_water_level(const WaterLevelDataset& wl, DrawingInstructions& out,
                         std::string* err = nullptr);

}  // namespace s100
