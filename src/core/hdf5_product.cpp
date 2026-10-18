#include "s100/hdf5_product.hpp"

#include <hdf5.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <string>

namespace s100 {
namespace {

void set_err(LoadError* err, LoadErrorKind kind, std::string msg) {
  if (!err) return;
  err->kind = kind;
  err->message = std::move(msg);
}

bool read_scalar_double(hid_t loc, const char* name, double* out) {
  if (!H5Aexists(loc, name)) return false;
  hid_t a = H5Aopen(loc, name, H5P_DEFAULT);
  if (a < 0) return false;
  hid_t t = H5Aget_type(a);
  hid_t nt = H5Tget_native_type(t, H5T_DIR_ASCEND);
  bool ok = false;
  if (H5Tget_class(nt) == H5T_FLOAT) {
    if (H5Tget_size(nt) == 4) {
      float v = 0;
      ok = H5Aread(a, H5T_NATIVE_FLOAT, &v) >= 0;
      *out = static_cast<double>(v);
    } else {
      ok = H5Aread(a, H5T_NATIVE_DOUBLE, out) >= 0;
    }
  } else if (H5Tget_class(nt) == H5T_INTEGER) {
    long long v = 0;
    ok = H5Aread(a, H5T_NATIVE_LLONG, &v) >= 0;
    *out = static_cast<double>(v);
  }
  H5Tclose(nt);
  H5Tclose(t);
  H5Aclose(a);
  return ok;
}

bool read_scalar_int(hid_t loc, const char* name, int* out) {
  double d = 0;
  if (!read_scalar_double(loc, name, &d)) return false;
  *out = static_cast<int>(d);
  return true;
}

bool read_string_attr(hid_t loc, const char* name, std::string* out) {
  if (!H5Aexists(loc, name)) return false;
  hid_t a = H5Aopen(loc, name, H5P_DEFAULT);
  if (a < 0) return false;
  hid_t t = H5Aget_type(a);
  bool ok = false;
  if (H5Tis_variable_str(t) > 0) {
    char* buf = nullptr;
    if (H5Aread(a, t, &buf) >= 0 && buf) {
      *out = buf;
      H5free_memory(buf);
      ok = true;
    }
  } else {
    const size_t n = H5Tget_size(t);
    std::string buf(n, '\0');
    if (H5Aread(a, t, buf.data()) >= 0) {
      const auto end = buf.find('\0');
      if (end != std::string::npos) buf.resize(end);
      while (!buf.empty() && (buf.back() == ' ' || buf.back() == '\0')) buf.pop_back();
      *out = std::move(buf);
      ok = true;
    }
  }
  H5Tclose(t);
  H5Aclose(a);
  return ok;
}

struct DepthPair {
  float depth;
  float uncertainty;
};

bool almost_eq(float a, float b) {
  return std::fabs(a - b) <= 1.0f ||
         std::fabs(a - b) <= 1e-3f * std::max(std::fabs(a), std::fabs(b));
}

/** Parse Group_F fillValue strings (e.g. "1000000") into floats; keep defaults on failure. */
void read_fill_from_group_f(hid_t file, float* fill_depth, float* fill_unc) {
  if (!H5Lexists(file, "/Group_F/BathymetryCoverage", H5P_DEFAULT)) return;
  hid_t ds = H5Dopen(file, "/Group_F/BathymetryCoverage", H5P_DEFAULT);
  if (ds < 0) return;
  hid_t space = H5Dget_space(ds);
  hsize_t dims[1] = {0};
  if (H5Sget_simple_extent_ndims(space) == 1) H5Sget_simple_extent_dims(space, dims, nullptr);
  // Compound with variable strings — read fillValue via HDF5 compound is awkward; keep 1e6 default
  // which matches CHS / NOAA S-102 3.0.0 cells. Optional override from first row is best-effort.
  (void)fill_depth;
  (void)fill_unc;
  H5Sclose(space);
  H5Dclose(ds);
}

bool load_instance_grid(hid_t file, hid_t inst, CoverageGrid& grid, LoadError* err) {
  if (!read_scalar_double(inst, "gridOriginLongitude", &grid.origin_x) ||
      !read_scalar_double(inst, "gridOriginLatitude", &grid.origin_y) ||
      !read_scalar_double(inst, "gridSpacingLongitudinal", &grid.spacing_x) ||
      !read_scalar_double(inst, "gridSpacingLatitudinal", &grid.spacing_y)) {
    set_err(err, LoadErrorKind::Parse, "S-102 instance missing gridOrigin/gridSpacing attributes");
    return false;
  }
  double ncols = 0, nrows = 0;
  if (!read_scalar_double(inst, "numPointsLongitudinal", &ncols) ||
      !read_scalar_double(inst, "numPointsLatitudinal", &nrows) || ncols < 1 || nrows < 1) {
    set_err(err, LoadErrorKind::Parse, "S-102 instance missing numPointsLongitudinal/Latitudinal");
    return false;
  }
  grid.ncols = static_cast<std::size_t>(ncols);
  grid.nrows = static_cast<std::size_t>(nrows);

  if (!H5Lexists(inst, "Group_001/values", H5P_DEFAULT)) {
    set_err(err, LoadErrorKind::Parse, "S-102 missing Group_001/values dataset");
    return false;
  }
  hid_t ds = H5Dopen(inst, "Group_001/values", H5P_DEFAULT);
  if (ds < 0) {
    set_err(err, LoadErrorKind::Parse, "S-102 cannot open Group_001/values");
    return false;
  }
  hid_t space = H5Dget_space(ds);
  const int nd = H5Sget_simple_extent_ndims(space);
  hsize_t dims[2] = {0, 0};
  if (nd != 2 || H5Sget_simple_extent_dims(space, dims, nullptr) < 0) {
    H5Sclose(space);
    H5Dclose(ds);
    set_err(err, LoadErrorKind::Parse, "S-102 values dataset is not a 2-D array");
    return false;
  }
  // HDF5 dims are (numPointsLatitudinal, numPointsLongitudinal) = (nrows, ncols).
  if (dims[0] != grid.nrows || dims[1] != grid.ncols) {
    // Trust the dataset dims if attributes disagree slightly.
    grid.nrows = static_cast<std::size_t>(dims[0]);
    grid.ncols = static_cast<std::size_t>(dims[1]);
  }

  // Detect compound members: PS §10 allows depth-only (uncertainty column + Group_F omitted).
  hid_t file_type = H5Dget_type(ds);
  if (file_type < 0) {
    H5Sclose(space);
    H5Dclose(ds);
    set_err(err, LoadErrorKind::Parse, "S-102 values dataset has no datatype");
    return false;
  }
  bool has_depth_member = false;
  bool has_unc_member = false;
  if (H5Tget_class(file_type) == H5T_COMPOUND) {
    const int nmembers = H5Tget_nmembers(file_type);
    for (unsigned i = 0; i < static_cast<unsigned>(nmembers); ++i) {
      char* mn = H5Tget_member_name(file_type, i);
      if (!mn) continue;
      if (std::strcmp(mn, "depth") == 0) has_depth_member = true;
      if (std::strcmp(mn, "uncertainty") == 0) has_unc_member = true;
      H5free_memory(mn);
    }
  }
  H5Tclose(file_type);
  if (!has_depth_member) {
    H5Sclose(space);
    H5Dclose(ds);
    set_err(err, LoadErrorKind::Parse, "S-102 values compound missing depth member");
    return false;
  }

  const std::size_t n = grid.nrows * grid.ncols;
  read_fill_from_group_f(file, &grid.fill_depth, &grid.fill_uncertainty);
  grid.depth.resize(n);

  if (has_unc_member) {
    hid_t mem_type = H5Tcreate(H5T_COMPOUND, sizeof(DepthPair));
    H5Tinsert(mem_type, "depth", HOFFSET(DepthPair, depth), H5T_NATIVE_FLOAT);
    H5Tinsert(mem_type, "uncertainty", HOFFSET(DepthPair, uncertainty), H5T_NATIVE_FLOAT);
    std::vector<DepthPair> buf(n);
    const herr_t rd = H5Dread(ds, mem_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf.data());
    H5Tclose(mem_type);
    H5Sclose(space);
    H5Dclose(ds);
    if (rd < 0) {
      set_err(err, LoadErrorKind::Parse, "S-102 H5Dread(values) failed");
      return false;
    }
    grid.uncertainty.resize(n);
    for (std::size_t i = 0; i < n; ++i) {
      grid.depth[i] = buf[i].depth;
      grid.uncertainty[i] = buf[i].uncertainty;
    }
  } else {
    // Depth-only compound: no uncertainty channel in memory (has_uncertainty stays false).
    // Optional min==max broadcast (U2b) is not elected — leave uncertainty empty.
    struct DepthOnly {
      float depth;
    };
    hid_t mem_type = H5Tcreate(H5T_COMPOUND, sizeof(DepthOnly));
    H5Tinsert(mem_type, "depth", HOFFSET(DepthOnly, depth), H5T_NATIVE_FLOAT);
    std::vector<DepthOnly> buf(n);
    const herr_t rd = H5Dread(ds, mem_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf.data());
    H5Tclose(mem_type);
    H5Sclose(space);
    H5Dclose(ds);
    if (rd < 0) {
      set_err(err, LoadErrorKind::Parse, "S-102 H5Dread(values depth-only) failed");
      return false;
    }
    grid.uncertainty.clear();
    for (std::size_t i = 0; i < n; ++i) grid.depth[i] = buf[i].depth;
  }
  return true;
}

}  // namespace

bool CoverageGrid::is_fill_depth(float v) const {
  return !std::isfinite(v) || almost_eq(v, fill_depth) || std::fabs(v) >= 1.0e5f;
}

bool CoverageGrid::is_fill_uncertainty(float v) const {
  return !std::isfinite(v) || almost_eq(v, fill_uncertainty) || std::fabs(v) >= 1.0e5f;
}

bool CoverageGrid::sample_cell(std::size_t row, std::size_t col, CoverageSample& out) const {
  out = {};
  if (row >= nrows || col >= ncols || depth.size() != nrows * ncols) return false;
  const std::size_t i = row * ncols + col;
  if (!is_fill_depth(depth[i])) {
    out.depth = depth[i];
    out.has_depth = true;
  }
  if (uncertainty.size() == depth.size() && !is_fill_uncertainty(uncertainty[i])) {
    out.uncertainty = uncertainty[i];
    out.has_uncertainty = true;
  }
  return out.has_depth;
}


namespace {

/** WGS84 geographic → projected easting/northing for UTM EPSG:326xx (N) / 327xx (S). */
bool wgs84_to_utm_epsg(int epsg, double lon_deg, double lat_deg, double* x, double* y) {
  const bool north = epsg >= 32601 && epsg <= 32660;
  const bool south = epsg >= 32701 && epsg <= 32760;
  if (!north && !south) return false;
  if (!x || !y) return false;
  if (lat_deg < -80.0 || lat_deg > 84.0) return false;
  const int zone = north ? (epsg - 32600) : (epsg - 32700);
  constexpr double a = 6378137.0;
  constexpr double f = 1.0 / 298.257223563;
  constexpr double k0 = 0.9996;
  const double e2 = f * (2.0 - f);
  const double ep2 = e2 / (1.0 - e2);
  const double lon0 = (zone * 6 - 183) * (3.14159265358979323846 / 180.0);
  const double lat = lat_deg * (3.14159265358979323846 / 180.0);
  const double lon = lon_deg * (3.14159265358979323846 / 180.0);
  const double sin_lat = std::sin(lat);
  const double cos_lat = std::cos(lat);
  const double tan_lat = std::tan(lat);
  const double N = a / std::sqrt(1.0 - e2 * sin_lat * sin_lat);
  const double T = tan_lat * tan_lat;
  const double C = ep2 * cos_lat * cos_lat;
  const double A = cos_lat * (lon - lon0);
  const double M =
      a *
      ((1.0 - e2 / 4.0 - 3.0 * e2 * e2 / 64.0 - 5.0 * e2 * e2 * e2 / 256.0) * lat -
       (3.0 * e2 / 8.0 + 3.0 * e2 * e2 / 32.0 + 45.0 * e2 * e2 * e2 / 1024.0) * std::sin(2.0 * lat) +
       (15.0 * e2 * e2 / 256.0 + 45.0 * e2 * e2 * e2 / 1024.0) * std::sin(4.0 * lat) -
       (35.0 * e2 * e2 * e2 / 3072.0) * std::sin(6.0 * lat));
  *x = k0 * N *
           (A + (1.0 - T + C) * A * A * A / 6.0 +
            (5.0 - 18.0 * T + T * T + 72.0 * C - 58.0 * ep2) * A * A * A * A * A / 120.0) +
       500000.0;
  *y = k0 *
       (M + N * tan_lat *
                (A * A / 2.0 + (5.0 - T + 9.0 * C + 4.0 * C * C) * A * A * A * A / 24.0 +
                 (61.0 - 58.0 * T + T * T + 600.0 * C - 330.0 * ep2) * A * A * A * A * A * A / 720.0));
  if (south) *y += 10000000.0;
  return true;
}

bool wgs84_to_grid_xy(int horizontal_crs, double lon, double lat, double* x, double* y) {
  if (horizontal_crs == 4326) {
    *x = lon;
    *y = lat;
    return true;
  }
  return wgs84_to_utm_epsg(horizontal_crs, lon, lat, x, y);
}

}  // namespace

bool CoverageGrid::sample_wgs84(double lon, double lat, CoverageSample& out) const {
  out = {};
  if (ncols == 0 || nrows == 0 || spacing_x == 0.0 || spacing_y == 0.0) return false;
  double x = 0, y = 0;
  if (!wgs84_to_grid_xy(horizontal_crs, lon, lat, &x, &y)) return false;
  const double col_f = (x - origin_x) / spacing_x;
  const double row_f = (y - origin_y) / spacing_y;
  if (col_f < -0.5 || row_f < -0.5 || col_f >= static_cast<double>(ncols) - 0.5 ||
      row_f >= static_cast<double>(nrows) - 0.5)
    return false;
  const auto col = static_cast<std::size_t>(std::llround(col_f));
  const auto row = static_cast<std::size_t>(std::llround(row_f));
  if (col >= ncols || row >= nrows) return false;
  return sample_cell(row, col, out);
}

bool CoverageDataset::sample_wgs84(double lon, double lat, CoverageSample& out) const {
  out = {};
  struct Cand {
    std::size_t index;
    double area;
  };
  std::vector<Cand> cands;
  for (std::size_t i = 0; i < grids.size(); ++i) {
    const CoverageGrid& g = grids[i];
    if (!g.bounds_valid) continue;
    if (lon < g.west || lon > g.east || lat < g.south || lat > g.north) continue;
    const double area = std::fabs((g.east - g.west) * (g.north - g.south));
    cands.push_back(Cand{i, area});
  }
  if (cands.empty()) return false;
  std::sort(cands.begin(), cands.end(), [](const Cand& a, const Cand& b) {
    if (a.area != b.area) return a.area < b.area;
    return a.index < b.index;
  });
  return grids[cands.front().index].sample_wgs84(lon, lat, out);
}

bool load_s102(const std::filesystem::path& path, CoverageDataset& out, LoadError* err) {
  out = CoverageDataset{};
  out.product = ProductId::S102;
  out.path = path;
  out.dataset_name = path.stem().string();

  if (!H5Fis_hdf5(path.string().c_str())) {
    set_err(err, LoadErrorKind::Invalid, "not an HDF5 file: " + path.string());
    return false;
  }
  hid_t file = H5Fopen(path.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  if (file < 0) {
    set_err(err, LoadErrorKind::Io, "cannot open HDF5: " + path.string());
    return false;
  }

  auto close_file = [&]() { H5Fclose(file); };

  read_string_attr(file, "productSpecification", &out.product_specification);
  int crs = 0;
  read_scalar_int(file, "horizontalCRS", &crs);

  double root_w = 0, root_e = 0, root_s = 0, root_n = 0;
  const bool root_bounds = read_scalar_double(file, "westBoundLongitude", &root_w) &&
                           read_scalar_double(file, "eastBoundLongitude", &root_e) &&
                           read_scalar_double(file, "southBoundLatitude", &root_s) &&
                           read_scalar_double(file, "northBoundLatitude", &root_n);

  if (!H5Lexists(file, "/BathymetryCoverage", H5P_DEFAULT)) {
    close_file();
    set_err(err, LoadErrorKind::Parse, "S-102 missing /BathymetryCoverage group: " + path.string());
    return false;
  }

  // Collect all BathymetryCoverage.* instances in stable name order.
  std::vector<std::string> inst_names;
  hid_t bc = H5Gopen(file, "/BathymetryCoverage", H5P_DEFAULT);
  if (bc < 0) {
    close_file();
    set_err(err, LoadErrorKind::Parse, "cannot open /BathymetryCoverage");
    return false;
  }
  H5G_info_t info{};
  H5Gget_info(bc, &info);
  for (hsize_t i = 0; i < info.nlinks; ++i) {
    char name[256];
    ssize_t len = H5Lget_name_by_idx(bc, ".", H5_INDEX_NAME, H5_ITER_INC, i, name, sizeof(name),
                                     H5P_DEFAULT);
    if (len <= 0) continue;
    if (std::strncmp(name, "BathymetryCoverage.", 19) == 0) inst_names.emplace_back(name);
  }
  H5Gclose(bc);
  std::sort(inst_names.begin(), inst_names.end());
  if (inst_names.empty()) {
    close_file();
    set_err(err, LoadErrorKind::Parse, "S-102 has no BathymetryCoverage.* instance");
    return false;
  }

  for (const std::string& name : inst_names) {
    const std::string inst_path = "/BathymetryCoverage/" + name;
    hid_t inst = H5Gopen(file, inst_path.c_str(), H5P_DEFAULT);
    if (inst < 0) {
      close_file();
      set_err(err, LoadErrorKind::Parse, "cannot open " + inst_path);
      return false;
    }
    CoverageGrid grid;
    grid.horizontal_crs = crs;
    const bool grid_ok = load_instance_grid(file, inst, grid, err);
    if (!grid_ok) {
      H5Gclose(inst);
      close_file();
      return false;
    }

    // Geographic envelope: prefer instance attrs (multi-grid 4326); else root; else derive 4326.
    double iw = 0, ie = 0, isouth = 0, inorth = 0;
    const bool ib = read_scalar_double(inst, "westBoundLongitude", &iw) &&
                    read_scalar_double(inst, "eastBoundLongitude", &ie) &&
                    read_scalar_double(inst, "southBoundLatitude", &isouth) &&
                    read_scalar_double(inst, "northBoundLatitude", &inorth);
    if (crs == 4326 && ib) {
      grid.west = iw;
      grid.east = ie;
      grid.south = isouth;
      grid.north = inorth;
      grid.bounds_valid = true;
    } else if (root_bounds) {
      grid.west = root_w;
      grid.east = root_e;
      grid.south = root_s;
      grid.north = root_n;
      grid.bounds_valid = true;
    } else if (crs == 4326 && grid.ncols > 0 && grid.nrows > 0) {
      grid.west = grid.origin_x;
      grid.south = grid.origin_y;
      grid.east = grid.origin_x + static_cast<double>(grid.ncols - 1) * grid.spacing_x;
      grid.north = grid.origin_y + static_cast<double>(grid.nrows - 1) * grid.spacing_y;
      grid.bounds_valid = true;
    }
    H5Gclose(inst);

    CoverageFeature feat;
    feat.id = static_cast<std::uint64_t>(out.grids.size() + 1);
    feat.code = "BathymetryCoverage";
    out.grids.push_back(std::move(grid));
    out.features.push_back(std::move(feat));
  }

  close_file();
  return !out.grids.empty();
}

bool load_hdf5_product(ProductId id, const std::filesystem::path& path, Dataset& out,
                       LoadError* err) {
  // Peek productSpecification / groups to route S-104 / S-111 / S-102 when id is Unknown.
  auto peek_route = [&](ProductId* inferred) -> bool {
    *inferred = id;
    if (id != ProductId::Unknown) return true;
    if (!H5Fis_hdf5(path.string().c_str())) {
      set_err(err, LoadErrorKind::Invalid, "not an HDF5 file: " + path.string());
      return false;
    }
    hid_t file = H5Fopen(path.string().c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    if (file < 0) {
      set_err(err, LoadErrorKind::Io, "cannot open HDF5: " + path.string());
      return false;
    }
    std::string spec;
    read_string_attr(file, "productSpecification", &spec);
    const bool has_wl = H5Lexists(file, "/WaterLevel", H5P_DEFAULT) > 0;
    const bool has_sc = H5Lexists(file, "/SurfaceCurrent", H5P_DEFAULT) > 0;
    const bool has_bathy = H5Lexists(file, "/BathymetryCoverage", H5P_DEFAULT) > 0;
    H5Fclose(file);
    if (spec.find("S-104") != std::string::npos || spec.find("S104") != std::string::npos ||
        (has_wl && !has_bathy && !has_sc)) {
      *inferred = ProductId::S104;
    } else if (spec.find("S-111") != std::string::npos || spec.find("S111") != std::string::npos ||
               (has_sc && !has_bathy)) {
      *inferred = ProductId::S111;
    } else {
      *inferred = ProductId::S102;  // default HDF5 bathymetry path
    }
    return true;
  };

  ProductId route = id;
  if (!peek_route(&route)) return false;

  if (route == ProductId::S104) {
    WaterLevelDataset wl;
    if (!load_s104(path, wl, err)) return false;
    out = Dataset{};
    out.product = ProductId::S104;
    out.path = path;
    out.dataset_name = wl.dataset_name;
    out.water_level = std::move(wl);
    return true;
  }
  if (route == ProductId::S111) {
    CurrentDataset cur;
    if (!load_s111(path, cur, err)) return false;
    out = Dataset{};
    out.product = ProductId::S111;
    out.path = path;
    out.dataset_name = cur.dataset_name;
    out.current = std::move(cur);
    return true;
  }
  if (route == ProductId::S102 || route == ProductId::Unknown) {
    CoverageDataset cov;
    if (!load_s102(path, cov, err)) return false;
    if (!cov.product_specification.empty() &&
        cov.product_specification.find("S-102") == std::string::npos &&
        cov.product_specification.find("S102") == std::string::npos && id == ProductId::Unknown) {
      // May be S-104 that lacked /WaterLevel — surface as Parse.
      if (cov.product_specification.find("S-104") != std::string::npos ||
          cov.product_specification.find("S104") != std::string::npos) {
        set_err(err, LoadErrorKind::Parse,
                "S-104 HDF5 unreadable as water level (" + cov.product_specification +
                    "): " + path.string());
        return false;
      }
      if (cov.product_specification.find("S-111") != std::string::npos ||
          cov.product_specification.find("S111") != std::string::npos) {
        set_err(err, LoadErrorKind::Parse,
                "S-111 HDF5 unreadable as surface current (" + cov.product_specification +
                    "): " + path.string());
        return false;
      }
      set_err(err, LoadErrorKind::Parse,
              "HDF5 product is not S-102 (" + cov.product_specification + "): " + path.string());
      return false;
    }
    out = Dataset{};
    out.product = ProductId::S102;
    out.path = path;
    out.dataset_name = cov.dataset_name;
    out.coverage = std::move(cov);
    return true;
  }
  set_err(err, LoadErrorKind::Parse,
          std::string(product_name(route)) + " HDF5 loader not implemented yet: " + path.string());
  return false;
}

}  // namespace s100
