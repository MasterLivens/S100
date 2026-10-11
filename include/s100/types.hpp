#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace s100 {

struct Point {
  double lon{0};
  double lat{0};
};

struct Ring {
  std::vector<Point> points;
};

enum class GeometryKind { Empty, Point, LineString, Polygon };

struct Geometry {
  GeometryKind kind{GeometryKind::Empty};
  Point point{};
  std::vector<Point> line;
  std::vector<Ring> polygon;
};

enum class LayerId { LAND, DEPARE, Other };

struct Feature {
  std::uint64_t id{0};
  std::string feature_type;
  LayerId layer{LayerId::Other};
  Geometry geometry;
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
