#pragma once
#include "s100/layer.hpp"
#include "s100/product.hpp"
#include "s100/types.hpp"
#include <filesystem>
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
  std::vector<Feature> features;
  LayerSet layers;
  std::vector<InformationRecord> information;  // IRID records
  SpatialStore spatial;                        // topology for the portrayal host
  DatasetStats stats() const;
  /** WGS84 envelope; valid=false until real spatial decode fills it. */
  Bounds bounds() const;
};

enum class LoadErrorKind { Io, Invalid, Parse };

struct LoadError {
  LoadErrorKind kind;
  std::string message;
};

/** Load an S-101 cell (.000). On failure returns empty optional and fills err. */
bool load_s101(const std::filesystem::path& path, Dataset& out, LoadError* err = nullptr);

/** Product-aware entry: dispatches by extension/header. P0: S-101 .000 only;
 *  S-102/S-104/S-111 HDF5 and S-124 GML return Parse until those loaders land. */
bool open_dataset(const std::filesystem::path& path, Dataset& out, LoadError* err = nullptr);

}  // namespace s100
