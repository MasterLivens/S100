#include "s100/current.hpp"
#include "s100/hdf5_product.hpp"

#include <hdf5.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
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

double iso_to_seconds(std::string_view iso) {
  int Y = 0, M = 0, D = 0, h = 0, m = 0, s = 0;
  std::string tmp(iso);
  if (!tmp.empty() && (tmp.back() == 'Z' || tmp.back() == 'z')) tmp.pop_back();
  if (std::sscanf(tmp.c_str(), "%d-%d-%dT%d:%d:%d", &Y, &M, &D, &h, &m, &s) >= 3) {
    auto days_from_civil = [](int y, int mo, int d) -> long long {
      y -= mo <= 2;
      const int era = (y >= 0 ? y : y - 399) / 400;
      const unsigned yoe = static_cast<unsigned>(y - era * 400);
      const unsigned doy = (153 * (mo + (mo > 2 ? -3 : 9)) + 2) / 5 + d - 1;
      const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
      return static_cast<long long>(era) * 146097 + static_cast<long long>(doe) - 719468;
    };
    const long long days = days_from_civil(Y, M, D);
    return static_cast<double>(days) * 86400.0 + h * 3600.0 + m * 60.0 + s;
  }
  double acc = 0;
  for (char c : iso) acc = acc * 131.0 + static_cast<unsigned char>(c);
  return acc;
}

struct SpeedDir {
  float speed;
  float direction;
};

struct UVPair {
  float u;
  float v;
};

float normalize_dir_deg(float deg) {
  if (!std::isfinite(deg)) return 0.f;
  float d = std::fmod(deg, 360.f);
  if (d < 0.f) d += 360.f;
  return d;
}

bool load_group_values(hid_t group, CoverageGrid& speed, CoverageGrid& direction, LoadError* err) {
  if (!H5Lexists(group, "values", H5P_DEFAULT)) {
    set_err(err, LoadErrorKind::Parse, "S-111 missing Group values dataset");
    return false;
  }
  hid_t ds = H5Dopen(group, "values", H5P_DEFAULT);
  if (ds < 0) {
    set_err(err, LoadErrorKind::Parse, "S-111 cannot open values");
    return false;
  }
  hid_t space = H5Dget_space(ds);
  const int nd = H5Sget_simple_extent_ndims(space);
  hsize_t dims[2] = {0, 0};
  if (nd != 2 || H5Sget_simple_extent_dims(space, dims, nullptr) < 0) {
    H5Sclose(space);
    H5Dclose(ds);
    set_err(err, LoadErrorKind::Parse, "S-111 values dataset is not a 2-D array");
    return false;
  }
  if (speed.nrows == 0 || speed.ncols == 0) {
    speed.nrows = static_cast<std::size_t>(dims[0]);
    speed.ncols = static_cast<std::size_t>(dims[1]);
  } else if (dims[0] != speed.nrows || dims[1] != speed.ncols) {
    speed.nrows = static_cast<std::size_t>(dims[0]);
    speed.ncols = static_cast<std::size_t>(dims[1]);
  }
  direction.nrows = speed.nrows;
  direction.ncols = speed.ncols;

  hid_t file_type = H5Dget_type(ds);
  const int nmembers = H5Tget_nmembers(file_type);
  std::string speed_name = "surfaceCurrentSpeed";
  std::string dir_name = "surfaceCurrentDirection";
  bool have_speed = false, have_dir = false, have_u = false, have_v = false;
  std::string u_name = "surfaceCurrentU", v_name = "surfaceCurrentV";
  for (int i = 0; i < nmembers; ++i) {
    char* mn = H5Tget_member_name(file_type, static_cast<unsigned>(i));
    if (!mn) continue;
    if (std::strcmp(mn, "surfaceCurrentSpeed") == 0 || std::strcmp(mn, "speed") == 0) {
      speed_name = mn;
      have_speed = true;
    } else if (std::strcmp(mn, "surfaceCurrentDirection") == 0 ||
               std::strcmp(mn, "direction") == 0) {
      dir_name = mn;
      have_dir = true;
    } else if (std::strcmp(mn, "surfaceCurrentU") == 0 || std::strcmp(mn, "u") == 0) {
      u_name = mn;
      have_u = true;
    } else if (std::strcmp(mn, "surfaceCurrentV") == 0 || std::strcmp(mn, "v") == 0) {
      v_name = mn;
      have_v = true;
    }
    H5free_memory(mn);
  }
  H5Tclose(file_type);

  const std::size_t n = speed.nrows * speed.ncols;
  speed.fill_depth = 1.0e6f;
  speed.fill_uncertainty = 1.0e6f;
  direction.fill_depth = 1.0e6f;
  direction.fill_uncertainty = 1.0e6f;
  speed.depth.resize(n);
  speed.uncertainty.clear();
  direction.depth.resize(n);
  direction.uncertainty.clear();

  bool ok = false;
  if (have_speed && have_dir) {
    hid_t mem_type = H5Tcreate(H5T_COMPOUND, sizeof(SpeedDir));
    H5Tinsert(mem_type, speed_name.c_str(), HOFFSET(SpeedDir, speed), H5T_NATIVE_FLOAT);
    H5Tinsert(mem_type, dir_name.c_str(), HOFFSET(SpeedDir, direction), H5T_NATIVE_FLOAT);
    std::vector<SpeedDir> buf(n);
    const herr_t rd = H5Dread(ds, mem_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf.data());
    H5Tclose(mem_type);
    if (rd >= 0) {
      for (std::size_t i = 0; i < n; ++i) {
        speed.depth[i] = buf[i].speed;
        direction.depth[i] = normalize_dir_deg(buf[i].direction);
      }
      ok = true;
    }
  }
  if (!ok && have_u && have_v) {
    // Convert u/v → speed/direction at load (façade never exposes u/v).
    hid_t mem_type = H5Tcreate(H5T_COMPOUND, sizeof(UVPair));
    H5Tinsert(mem_type, u_name.c_str(), HOFFSET(UVPair, u), H5T_NATIVE_FLOAT);
    H5Tinsert(mem_type, v_name.c_str(), HOFFSET(UVPair, v), H5T_NATIVE_FLOAT);
    std::vector<UVPair> buf(n);
    const herr_t rd = H5Dread(ds, mem_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf.data());
    H5Tclose(mem_type);
    if (rd >= 0) {
      for (std::size_t i = 0; i < n; ++i) {
        const float u = buf[i].u;
        const float v = buf[i].v;
        speed.depth[i] = std::sqrt(u * u + v * v);
        // from-direction: atan2(u, v) gives clockwise-from-N when u=east, v=north.
        float deg = static_cast<float>(std::atan2(static_cast<double>(u), static_cast<double>(v)) *
                                       (180.0 / 3.14159265358979323846));
        direction.depth[i] = normalize_dir_deg(deg);
      }
      ok = true;
    }
  }
  H5Sclose(space);
  H5Dclose(ds);
  if (!ok) {
    set_err(err, LoadErrorKind::Parse, "S-111 H5Dread(values) failed (need speed/direction or u/v)");
    return false;
  }
  return true;
}

/** Indices 0, stride, 2*stride, … and last index (edges inclusive). */
std::vector<std::size_t> stride_indices(std::size_t n, std::size_t stride) {
  std::vector<std::size_t> out;
  if (n == 0 || stride == 0) return out;
  for (std::size_t i = 0; i < n; i += stride) out.push_back(i);
  if (out.back() != n - 1) out.push_back(n - 1);
  return out;
}

}  // namespace

Bounds CurrentDataset::bounds() const {
  Bounds b;
  bool any = false;
  for (const auto& fr : frames) {
    const CoverageGrid& g = fr.speed;
    if (!g.bounds_valid) continue;
    if (!any) {
      b.west = g.west;
      b.east = g.east;
      b.south = g.south;
      b.north = g.north;
      any = true;
    } else {
      b.west = std::min(b.west, g.west);
      b.east = std::max(b.east, g.east);
      b.south = std::min(b.south, g.south);
      b.north = std::max(b.north, g.north);
    }
  }
  b.valid = any;
  return b;
}

bool CurrentDataset::set_active_frame(std::size_t index) {
  if (index >= frames.size()) return false;
  active_frame = index;
  return true;
}

bool CurrentDataset::set_active_time(std::string_view iso_utc) {
  if (frames.empty()) return false;
  const double target = iso_to_seconds(iso_utc);
  std::size_t best = 0;
  double best_dist = std::numeric_limits<double>::infinity();
  for (std::size_t i = 0; i < frames.size(); ++i) {
    const double d = std::fabs(iso_to_seconds(frames[i].time_iso) - target);
    // Strict < so ties keep the lower index.
    if (d < best_dist) {
      best_dist = d;
      best = i;
    }
  }
  active_frame = best;
  return true;
}

bool CurrentDataset::sample_wgs84(double lon, double lat, CurrentSample& out) const {
  out = {};
  const CurrentFrame* fr = active();
  if (!fr) return false;
  CoverageSample spd, dir;
  if (!fr->speed.sample_wgs84(lon, lat, spd) || !spd.has_depth) return false;
  if (!fr->direction.sample_wgs84(lon, lat, dir) || !dir.has_depth) return false;
  out.speed_ms = spd.depth;
  out.direction_deg = normalize_dir_deg(dir.depth);
  out.valid = true;
  return true;
}


// Interim mirror of catalogues/S-111/PC_2.0.0/Rules/select_arrow.xsl (geLtInterval bands).
// Forbidden: invent S-111 Lua HostAdapter — XSLT host remains WARN (docs/pc-wire-contract.md).
std::string scarow_symbol_ref(float speed_ms) {
  if (!(speed_ms > 0.f) || !std::isfinite(speed_ms)) return {};  // zero / fill / noData → no Arrow
  if (speed_ms < 0.50f) return "SCAROW01";
  if (speed_ms < 1.00f) return "SCAROW02";
  if (speed_ms < 2.00f) return "SCAROW03";
  if (speed_ms < 3.00f) return "SCAROW04";
  if (speed_ms < 5.00f) return "SCAROW05";
  if (speed_ms < 7.00f) return "SCAROW06";
  if (speed_ms < 10.00f) return "SCAROW07";
  if (speed_ms < 13.00f) return "SCAROW08";
  return "SCAROW09";
}

bool portray_current(const CurrentDataset& cur, DrawingInstructions& out, std::string* err) {
  out.clear();
  const CurrentFrame* fr = cur.active();
  if (!fr || fr->speed.ncols == 0 || fr->speed.nrows == 0) {
    if (err) *err = "S-111 portray: no active current frame";
    return false;
  }
  const CoverageGrid& speed = fr->speed;
  const CoverageGrid& direction = fr->direction;
  if (direction.nrows != speed.nrows || direction.ncols != speed.ncols ||
      speed.depth.size() != speed.nrows * speed.ncols ||
      direction.depth.size() != direction.nrows * direction.ncols) {
    if (err) *err = "S-111 portray: speed/direction grid shape mismatch";
    return false;
  }

  const auto rows = stride_indices(speed.nrows, kS111ArrowStride);
  const auto cols = stride_indices(speed.ncols, kS111ArrowStride);
  const std::uint64_t fid = cur.features.empty() ? 1 : cur.features.front().id;

  for (std::size_t r : rows) {
    for (std::size_t c : cols) {
      const std::size_t i = r * speed.ncols + c;
      if (speed.is_fill_depth(speed.depth[i])) continue;
      const float spd = speed.depth[i];
      const std::string scarow = scarow_symbol_ref(spd);
      if (scarow.empty()) continue;  // zero / fill / noData — match PC note (select_arrow.xsl)
      const float dir = normalize_dir_deg(direction.depth[i]);

      DrawingInstruction di;
      di.feature_id = fid;
      di.kind = DrawingInstruction::Kind::Arrow;
      di.command = "Arrow";
      // PC symbol id from select_arrow.xsl band table (interim C++ mirror; no S-111 Lua).
      di.symbol_ref = scarow;
      // Optional PC colour token SCBN1…9 aligned with band index.
      di.fill_ref = "SCBN" + std::to_string(scarow.back() - '0');  // SCAROW0N → SCBNN
      di.rotation = static_cast<double>(dir);
      di.rotation_crs = "GeographicCRS";
      // scale_factor proportional to speed_ms (viewer clamps paint length).
      di.scale_factor = static_cast<double>(spd);
      di.viewing_group = "24050";
      di.drawing_priority = 0;
      di.display_plane = "OverRadar";

      Geometry g;
      g.kind = GeometryKind::Point;
      if (speed.horizontal_crs == 4326) {
        g.point.lon = speed.origin_x + static_cast<double>(c) * speed.spacing_x;
        g.point.lat = speed.origin_y + static_cast<double>(r) * speed.spacing_y;
      } else if (speed.bounds_valid && speed.ncols > 1 && speed.nrows > 1) {
        // Geographic envelope linear map when grid axes are projected.
        const double u = static_cast<double>(c) / static_cast<double>(speed.ncols - 1);
        const double v = static_cast<double>(r) / static_cast<double>(speed.nrows - 1);
        g.point.lon = speed.west + u * (speed.east - speed.west);
        g.point.lat = speed.south + v * (speed.north - speed.south);
      } else {
        g.point.lon = speed.origin_x + static_cast<double>(c) * speed.spacing_x;
        g.point.lat = speed.origin_y + static_cast<double>(r) * speed.spacing_y;
      }
      di.geometries.push_back(std::move(g));
      out.push_back(std::move(di));
    }
  }

  if (out.empty()) {
    if (err) *err = "S-111 portray: no Arrow samples on active frame";
    return false;
  }
  return true;
}

bool load_s111(const std::filesystem::path& path, CurrentDataset& out, LoadError* err) {
  out = CurrentDataset{};
  out.product = ProductId::S111;
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
  const bool claims_s111 =
      (!out.product_specification.empty() &&
       (out.product_specification.find("S-111") != std::string::npos ||
        out.product_specification.find("S111") != std::string::npos)) ||
      H5Lexists(file, "/SurfaceCurrent", H5P_DEFAULT);

  if (!claims_s111) {
    close_file();
    set_err(err, LoadErrorKind::Parse, "HDF5 is not an S-111 surface-current product: " + path.string());
    return false;
  }
  if (!H5Lexists(file, "/SurfaceCurrent", H5P_DEFAULT)) {
    close_file();
    set_err(err, LoadErrorKind::Parse, "S-111 missing /SurfaceCurrent group: " + path.string());
    return false;
  }

  int crs = 4326;
  read_scalar_int(file, "horizontalCRS", &crs);
  double root_w = 0, root_e = 0, root_s = 0, root_n = 0;
  const bool root_bounds = read_scalar_double(file, "westBoundLongitude", &root_w) &&
                           read_scalar_double(file, "eastBoundLongitude", &root_e) &&
                           read_scalar_double(file, "southBoundLatitude", &root_s) &&
                           read_scalar_double(file, "northBoundLatitude", &root_n);

  hid_t sc = H5Gopen(file, "/SurfaceCurrent", H5P_DEFAULT);
  if (sc < 0) {
    close_file();
    set_err(err, LoadErrorKind::Parse, "cannot open /SurfaceCurrent");
    return false;
  }

  std::vector<std::string> inst_names;
  H5G_info_t info{};
  H5Gget_info(sc, &info);
  for (hsize_t i = 0; i < info.nlinks; ++i) {
    char name[256];
    ssize_t len = H5Lget_name_by_idx(sc, ".", H5_INDEX_NAME, H5_ITER_INC, i, name, sizeof(name),
                                     H5P_DEFAULT);
    if (len <= 0) continue;
    if (std::strncmp(name, "SurfaceCurrent.", 15) == 0) inst_names.emplace_back(name);
  }
  std::sort(inst_names.begin(), inst_names.end());
  if (inst_names.empty()) {
    H5Gclose(sc);
    close_file();
    set_err(err, LoadErrorKind::Parse, "S-111 has no SurfaceCurrent.* instance");
    return false;
  }

  const std::string inst_path = "/SurfaceCurrent/" + inst_names.front();
  hid_t inst = H5Gopen(file, inst_path.c_str(), H5P_DEFAULT);
  H5Gclose(sc);
  if (inst < 0) {
    close_file();
    set_err(err, LoadErrorKind::Parse, "cannot open " + inst_path);
    return false;
  }

  CoverageGrid proto;
  proto.horizontal_crs = crs;
  if (!read_scalar_double(inst, "gridOriginLongitude", &proto.origin_x) ||
      !read_scalar_double(inst, "gridOriginLatitude", &proto.origin_y) ||
      !read_scalar_double(inst, "gridSpacingLongitudinal", &proto.spacing_x) ||
      !read_scalar_double(inst, "gridSpacingLatitudinal", &proto.spacing_y)) {
    H5Gclose(inst);
    close_file();
    set_err(err, LoadErrorKind::Parse, "S-111 instance missing gridOrigin/gridSpacing");
    return false;
  }
  double ncols = 0, nrows = 0;
  if (!read_scalar_double(inst, "numPointsLongitudinal", &ncols) ||
      !read_scalar_double(inst, "numPointsLatitudinal", &nrows) || ncols < 1 || nrows < 1) {
    H5Gclose(inst);
    close_file();
    set_err(err, LoadErrorKind::Parse, "S-111 instance missing numPointsLongitudinal/Latitudinal");
    return false;
  }
  proto.ncols = static_cast<std::size_t>(ncols);
  proto.nrows = static_cast<std::size_t>(nrows);

  double iw = 0, ie = 0, isouth = 0, inorth = 0;
  const bool ib = read_scalar_double(inst, "westBoundLongitude", &iw) &&
                  read_scalar_double(inst, "eastBoundLongitude", &ie) &&
                  read_scalar_double(inst, "southBoundLatitude", &isouth) &&
                  read_scalar_double(inst, "northBoundLatitude", &inorth);
  if (ib) {
    proto.west = iw;
    proto.east = ie;
    proto.south = isouth;
    proto.north = inorth;
    proto.bounds_valid = true;
  } else if (root_bounds) {
    proto.west = root_w;
    proto.east = root_e;
    proto.south = root_s;
    proto.north = root_n;
    proto.bounds_valid = true;
  } else if (crs == 4326) {
    proto.west = proto.origin_x;
    proto.south = proto.origin_y;
    proto.east = proto.origin_x + static_cast<double>(proto.ncols - 1) * proto.spacing_x;
    proto.north = proto.origin_y + static_cast<double>(proto.nrows - 1) * proto.spacing_y;
    proto.bounds_valid = true;
  }

  std::vector<std::string> group_names;
  H5G_info_t ginfo{};
  H5Gget_info(inst, &ginfo);
  for (hsize_t i = 0; i < ginfo.nlinks; ++i) {
    char name[256];
    ssize_t len = H5Lget_name_by_idx(inst, ".", H5_INDEX_NAME, H5_ITER_INC, i, name, sizeof(name),
                                     H5P_DEFAULT);
    if (len <= 0) continue;
    if (std::strncmp(name, "Group_", 6) == 0) group_names.emplace_back(name);
  }
  std::sort(group_names.begin(), group_names.end());
  if (group_names.empty()) {
    H5Gclose(inst);
    close_file();
    set_err(err, LoadErrorKind::Parse, "S-111 instance has no Group_* time steps");
    return false;
  }

  for (const std::string& gname : group_names) {
    hid_t grp = H5Gopen(inst, gname.c_str(), H5P_DEFAULT);
    if (grp < 0) {
      H5Gclose(inst);
      close_file();
      set_err(err, LoadErrorKind::Parse, "cannot open " + inst_path + "/" + gname);
      return false;
    }
    CurrentFrame frame;
    read_string_attr(grp, "timePoint", &frame.time_iso);
    if (frame.time_iso.empty()) frame.time_iso = gname;
    frame.speed = proto;
    frame.direction = proto;
    if (!load_group_values(grp, frame.speed, frame.direction, err)) {
      H5Gclose(grp);
      H5Gclose(inst);
      close_file();
      return false;
    }
    H5Gclose(grp);
    out.frames.push_back(std::move(frame));
  }
  H5Gclose(inst);
  close_file();

  if (out.frames.empty()) {
    set_err(err, LoadErrorKind::Parse, "S-111 loaded zero frames: " + path.string());
    return false;
  }
  out.active_frame = 0;
  CoverageFeature feat;
  feat.id = 1;
  feat.code = "SurfaceCurrent";
  out.features.push_back(std::move(feat));
  return true;
}

}  // namespace s100
