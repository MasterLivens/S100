#pragma once
// Coverage products (S-102+): grid payload + portrayal feature instances.
// Grids must not be flattened into Dataset::features (docs/architecture.md rule 4).
#include "s100/product.hpp"
#include "s100/types.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace s100 {

/** One coverage feature instance for the S-102 portrayal host (e.g. BathymetryCoverage). */
struct CoverageFeature {
  std::uint64_t id{0};
  std::string code;                   // FC feature type code
  std::vector<Attribute> attributes;  // feature-instance metadata attributes
};

/** One sample from a coverage grid. */
struct CoverageSample {
  float depth{0};
  float uncertainty{0};
  bool has_depth{false};
  bool has_uncertainty{false};
};

/**
 * Regular-grid bathymetry (S-102 dataCodingFormat = Regular Grid).
 * Values are row-major: index = row * ncols + col, matching HDF5 dims
 * (numPointsLatitudinal, numPointsLongitudinal). Grid origin / spacing are in the
 * dataset horizontal CRS (EPSG); geographic_bounds always come from root WGS84 attrs
 * (or per-instance geographic attrs for multi-grid EPSG:4326).
 */
struct CoverageGrid {
  int horizontal_crs{0};  // EPSG code (4326 = WGS84 lon/lat grid axes)
  double west{0}, south{0}, east{0}, north{0};  // geographic envelope (WGS84)
  bool bounds_valid{false};
  double origin_x{0}, origin_y{0};  // gridOriginLongitude / Latitude (native CRS)
  double spacing_x{0}, spacing_y{0};
  std::size_t ncols{0}, nrows{0};
  float fill_depth{1.0e6f};
  float fill_uncertainty{1.0e6f};
  std::vector<float> depth;
  std::vector<float> uncertainty;

  Bounds geographic_bounds() const {
    Bounds b;
    b.west = west;
    b.south = south;
    b.east = east;
    b.north = north;
    b.valid = bounds_valid;
    return b;
  }

  /** Nearest-neighbour sample from WGS84 lon/lat. EPSG:4326 uses lon/lat grid axes;
   *  EPSG:326xx/327xx (UTM) are converted internally; other projected EPSG return false. */
  bool sample_wgs84(double lon, double lat, CoverageSample& out) const;
  bool sample_cell(std::size_t row, std::size_t col, CoverageSample& out) const;
  bool is_fill_depth(float v) const;
  bool is_fill_uncertainty(float v) const;
};

/**
 * Coverage product payload (S-102+). Host portrayal uses `features`
 * (one BathymetryCoverage instance per loaded grid).
 */
struct CoverageDataset {
  ProductId product{ProductId::S102};
  std::filesystem::path path;
  std::string dataset_name;
  std::string product_specification;  // e.g. INT.IHO.S-102.3.0.0
  std::vector<CoverageGrid> grids;    // size >= 1 after successful open
  std::vector<CoverageFeature> features;

  /** Temporary shim → grids[0] (one release; prefer grids / sample_wgs84). */
  CoverageGrid& grid() { return grids.at(0); }
  const CoverageGrid& grid() const { return grids.at(0); }

  /** Among grids whose geographic envelope contains (lon,lat), pick smallest
   *  enclosing area then lowest index; else false. */
  bool sample_wgs84(double lon, double lat, CoverageSample& out) const;
};

}  // namespace s100
