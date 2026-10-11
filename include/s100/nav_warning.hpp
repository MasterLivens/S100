#pragma once
// S-124 navigational warnings: GML list + overlay (docs/s124-contract.md).
#include "s100/drawing_instructions.hpp"
#include "s100/product.hpp"
#include "s100/types.hpp"
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace s100 {

struct LoadError;  // dataset.hpp

struct NavWarning {
  std::string id;
  std::string title;
  std::string message;   // may be empty
  std::string time_iso;  // UTC when present
  Bounds bounds;
  std::vector<Geometry> geometries;
  // severity / status: NOT in this slice
};

struct NavWarningDataset {
  ProductId product{ProductId::S124};
  std::filesystem::path path;
  std::vector<NavWarning> warnings;

  Bounds bounds() const;  // union of warning bounds
  const NavWarning* find(std::string_view id) const;
};

/** Load S-124 GML. Empty/whitespace → Invalid; zero NW → Parse;
 *  wrong product → Invalid/Parse and leaves out empty. */
bool load_s124(const std::filesystem::path& path, NavWarningDataset& out, LoadError* err = nullptr);

/** Emit geom DI (Point/Line/Area) + Text title per warning; fill/line token CHYLW.
 *  Fixture → ≥2 geom + ≥2 Text. */
bool portray_nav_warnings(const NavWarningDataset& nw, DrawingInstructions& out,
                          std::string* err = nullptr);

}  // namespace s100
