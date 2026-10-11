#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace s100 {

struct Point {
  double lon{0};
  double lat{0};
};

/** 3-D point: WGS84 lon/lat plus Z. For S-101 soundings Z is depth in metres (positive down). */
struct Point3 {
  double lon{0};
  double lat{0};
  double z{0};
};

struct Ring {
  std::vector<Point> points;
};

enum class GeometryKind { Empty, Point, LineString, Polygon, MultiPoint };

struct Geometry {
  GeometryKind kind{GeometryKind::Empty};
  Point point{};
  std::vector<Point> line;
  std::vector<Ring> polygon;
  std::vector<Point3> multipoint;  // MultiPoint (e.g. Sounding) with Z
};


/** One S-101 attribute instance (ATTR field). Complex attributes have an empty value
 *  and their sub-attributes point back to them through `parent` (1-based index, 0 = top level). */
struct Attribute {
  std::string code;            // camelCase code from ATCS, e.g. "depthRangeMinimumValue"
  std::string value;           // raw ATVL text; empty for complex attributes
  std::uint16_t index{0};      // ATIX: instance number among same-code siblings
  std::uint16_t parent{0};     // PAIX: 1-based position of the parent attribute, 0 = top level
  std::uint8_t instruction{1}; // ATIN: 1 insert (update files may use 2 delete / 3 modify)
};

struct Feature {
  std::uint64_t id{0};
  std::string feature_type;  // FC feature type code; also the layer key (classify_layers)
  Geometry geometry;
  std::vector<Attribute> attributes;

  /** First top-level attribute value for `code`, or nullptr. */
  const std::string* attribute(const std::string& code) const {
    for (const auto& a : attributes)
      if (a.parent == 0 && a.code == code) return &a.value;
    return nullptr;
  }
};

struct DatasetStats {
  std::size_t feature_count{0};
  std::size_t layer_count{0};
};

struct PickResult {
  double lon{0};
  double lat{0};
};

struct Bounds {
  double west{0};
  double south{0};
  double east{0};
  double north{0};
  bool valid{false};
};

}  // namespace s100
