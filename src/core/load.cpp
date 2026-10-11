#include <algorithm>
#include "s100/dataset.hpp"
#include "s100/hdf5_product.hpp"

namespace s100 {

bool load_iso8211_file(const std::filesystem::path& path, Dataset& out, LoadError* err);

DatasetStats Dataset::stats() const {
  return DatasetStats{features.size(), layers.size()};
}

Bounds Dataset::bounds() const {
  Bounds b;
  bool any = false;
  auto consider = [&](double lon, double lat) {
    if (!any) { b.west = b.east = lon; b.south = b.north = lat; any = true; return; }
    b.west = std::min(b.west, lon); b.east = std::max(b.east, lon);
    b.south = std::min(b.south, lat); b.north = std::max(b.north, lat);
  };
  auto consider_bounds = [&](const Bounds& o) {
    if (!o.valid) return;
    consider(o.west, o.south);
    consider(o.east, o.north);
  };
  // Feature envelope (S-101) and coverage geographic envelope (S-102) are unioned so
  // ChartSession / viewer overlays frame both products (docs/s102-contract.md).
  for (const auto& f : features) {
    const auto& g = f.geometry;
    if (g.kind == GeometryKind::Empty) continue;
    for (const auto& p : g.multipoint) consider(p.lon, p.lat);
    if (g.kind == GeometryKind::Point) consider(g.point.lon, g.point.lat);
    else if (g.kind == GeometryKind::LineString) {
      for (const auto& p : g.line) consider(p.lon, p.lat);
    } else if (g.kind == GeometryKind::Polygon) {
      for (const auto& ring : g.polygon)
        for (const auto& p : ring.points) consider(p.lon, p.lat);
    }
  }
  if (coverage && coverage->grid.bounds_valid)
    consider_bounds(coverage->grid.geographic_bounds());
  b.valid = any;
  return b;
}

bool load_s101(const std::filesystem::path& path, Dataset& out, LoadError* err) {
  if (!load_iso8211_file(path, out, err)) return false;
  out.product = ProductId::S101;
  return true;
}

bool open_dataset(const std::filesystem::path& path, Dataset& out, LoadError* err) {
  const auto ext = path.extension().string();
  // S-101 ISO 8211: base .000 only. Lone updates (.001…) must use open_dataset_with_updates.
  if (ext.size() == 4 && ext[0] == '.' &&
      ext[1] >= '0' && ext[1] <= '9' &&
      ext[2] >= '0' && ext[2] <= '9' &&
      ext[3] >= '0' && ext[3] <= '9') {
    const int n = (ext[1] - '0') * 100 + (ext[2] - '0') * 10 + (ext[3] - '0');
    if (n != 0) {
      if (err) {
        err->kind = LoadErrorKind::Invalid;
        err->message =
            "open_dataset rejects update file " + path.string() +
            "; use open_dataset_with_updates (or open_cell_series / ChartSession)";
      }
      return false;
    }
    return load_s101(path, out, err);
  }
  // HDF5: S-102 bathymetry (S-104/S-111 still Parse via load_hdf5_product).
  if (ext == ".h5" || ext == ".hdf5") {
    return load_hdf5_product(ProductId::Unknown, path, out, err);
  }
  // GML warnings — reserved (P3)
  if (ext == ".gml" || ext == ".xml") {
    if (err) {
      err->kind = LoadErrorKind::Parse;
      err->message = "GML/XML products (e.g. S-124) not implemented yet: " + path.string();
    }
    return false;
  }
  if (err) {
    err->kind = LoadErrorKind::Invalid;
    err->message = "unknown product for " + path.string();
  }
  return false;
}

}  // namespace s100
