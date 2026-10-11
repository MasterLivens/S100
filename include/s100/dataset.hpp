#pragma once
#include "s100/coverage.hpp"
#include "s100/layer.hpp"
#include "s100/product.hpp"
#include "s100/types.hpp"
#include <filesystem>
#include <optional>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace s100 {

/** Information type record (IRID). */
struct InformationRecord {
  std::uint32_t record_id{0};
  std::string information_type;  // ITCS code, e.g. "SpatialQuality"
  std::vector<Attribute> attributes;
  std::vector<AssociationRef> information_associations;
};

/** One ISO 8211 spatial record kept as topology (not flattened), for the portrayal host. */
struct SpatialRecord {
  std::uint8_t rcnm{0};
  std::uint32_t rcid{0};
  std::vector<Point3> coords;     // point: 1 entry; multi point: n (z = depth); curve: control points
  std::uint32_t start_point{0};   // curve PTAS begin point RCID (0 = none)
  std::uint32_t end_point{0};     // curve PTAS end point RCID (0 = none)
  std::vector<SpatialRef> parts;  // composite curve components (CUCO) or surface rings (RIAS)
  std::vector<AssociationRef> information_associations;  // INAS (e.g. SpatialQuality)
};

/** Spatial records by (RCNM, RCID). */
struct SpatialStore {
  std::unordered_map<std::uint64_t, SpatialRecord> records;
  static std::uint64_t key(std::uint8_t rcnm, std::uint32_t rcid) {
    return (static_cast<std::uint64_t>(rcnm) << 32) | rcid;
  }
  const SpatialRecord* find(std::uint8_t rcnm, std::uint32_t rcid) const {
    auto it = records.find(key(rcnm, rcid));
    return it == records.end() ? nullptr : &it->second;
  }
};

struct Dataset {
  ProductId product{ProductId::Unknown};
  std::filesystem::path path;
  /** DSID DSED edition.update — required for update sequencing (exchange-set.md). */
  int edition_number{0};
  int update_number{0};
  std::string dataset_name;  // DSID DSNM (file name in the cell)
  std::vector<Feature> features;
  LayerSet layers;
  std::vector<InformationRecord> information;  // IRID records
  SpatialStore spatial;                        // topology for the portrayal host
  /** Coverage payload for S-102+ (grids are never flattened into features). */
  std::optional<CoverageDataset> coverage;
  DatasetStats stats() const;
  /** WGS84 envelope: coverage geographic bounds when present, else feature envelope. */
  Bounds bounds() const;
};

enum class LoadErrorKind { Io, Invalid, Parse };

struct LoadError {
  LoadErrorKind kind;
  std::string message;
};

/** Parsed DSID DSED / DSNM from an S-101 ISO 8211 cell or update. */
struct DatasetIdentification {
  int edition_number{0};
  int update_number{0};
  std::string dataset_name;
  bool ok{false};
};

/** Read DSID edition/update/name from a cell or update file. */
bool read_s101_dsid(const std::filesystem::path& path, DatasetIdentification& out,
                    LoadError* err = nullptr);

/** Load an S-101 base cell (.000). Fills Dataset DSID edition/update. */
bool load_s101(const std::filesystem::path& path, Dataset& out, LoadError* err = nullptr);

/** Product-aware entry for base cells / non-update products.
 *  Lone S-101 updates (.001…) are rejected (LoadErrorKind::Invalid) — use
 *  open_dataset_with_updates / ChartSession. S-102 HDF5 loads via load_s102 into coverage. */
bool open_dataset(const std::filesystem::path& path, Dataset& out, LoadError* err = nullptr);

}  // namespace s100
