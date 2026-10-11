#include "s100/water_level.hpp"
#include "s100/hdf5_product.hpp"

#include <hdf5.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <sstream>
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

/** Best-effort parse of ISO-8601 UTC to approximate unix seconds (lexicographic fallback). */
double iso_to_seconds(std::string_view iso) {
  // YYYY-MM-DDTHH:MM:SS[.fff]Z or without Z
  int Y = 0, M = 0, D = 0, h = 0, m = 0, s = 0;
  std::string tmp(iso);
  if (!tmp.empty() && (tmp.back() == 'Z' || tmp.back() == 'z')) tmp.pop_back();
  if (std::sscanf(tmp.c_str(), "%d-%d-%dT%d:%d:%d", &Y, &M, &D, &h, &m, &s) >= 3) {
    // Approximate days from 1970 (no leap-second care; good enough for nearest-frame).
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
  // Fallback: hash-ish from string bytes so nearest still picks something stable.
  double acc = 0;
  for (char c : iso) acc = acc * 131.0 + static_cast<unsigned char>(c);
  return acc;
}

struct HeightTrend {
  float height;
  std::uint8_t trend;
};

bool load_group_values(hid_t group, CoverageGrid& grid, LoadError* err) {
  if (!H5Lexists(group, "values", H5P_DEFAULT)) {
    set_err(err, LoadErrorKind::Parse, "S-104 missing Group values dataset");
    return false;
  }
  hid_t ds = H5Dopen(group, "values", H5P_DEFAULT);
  if (ds < 0) {
    set_err(err, LoadErrorKind::Parse, "S-104 cannot open values");
    return false;
  }
  hid_t space = H5Dget_space(ds);
  const int nd = H5Sget_simple_extent_ndims(space);
  hsize_t dims[2] = {0, 0};
  if (nd != 2 || H5Sget_simple_extent_dims(space, dims, nullptr) < 0) {
    H5Sclose(space);
    H5Dclose(ds);
    set_err(err, LoadErrorKind::Parse, "S-104 values dataset is not a 2-D array");
    return false;
  }
  if (grid.nrows == 0 || grid.ncols == 0) {
    grid.nrows = static_cast<std::size_t>(dims[0]);
    grid.ncols = static_cast<std::size_t>(dims[1]);
  } else if (dims[0] != grid.nrows || dims[1] != grid.ncols) {
    grid.nrows = static_cast<std::size_t>(dims[0]);
    grid.ncols = static_cast<std::size_t>(dims[1]);
  }

  // Prefer FC names; also accept surfaceHeight / Height (producer variation).
  hid_t file_type = H5Dget_type(ds);
  const int nmembers = H5Tget_nmembers(file_type);
  std::string height_name = "waterLevelHeight";
  for (int i = 0; i < nmembers; ++i) {
    char* mn = H5Tget_member_name(file_type, static_cast<unsigned>(i));
    if (!mn) continue;
    if (std::strcmp(mn, "waterLevelHeight") == 0 || std::strcmp(mn, "surfaceHeight") == 0 ||
        std::strcmp(mn, "Height") == 0) {
      height_name = mn;
      H5free_memory(mn);
      break;
    }
    H5free_memory(mn);
  }
  H5Tclose(file_type);

  hid_t mem_type = H5Tcreate(H5T_COMPOUND, sizeof(HeightTrend));
  H5Tinsert(mem_type, height_name.c_str(), HOFFSET(HeightTrend, height), H5T_NATIVE_FLOAT);
  // Trend optional — insert a dummy member only if we can; otherwise read height alone.
  // Use a flexible approach: try compound with height+trend; on failure read height field only.
  H5Tinsert(mem_type, "waterLevelTrend", HOFFSET(HeightTrend, trend), H5T_NATIVE_UINT8);

  const std::size_t n = grid.nrows * grid.ncols;
  std::vector<HeightTrend> buf(n);
  herr_t rd = H5Dread(ds, mem_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf.data());
  if (rd < 0) {
    H5Tclose(mem_type);
    // Fallback: height-only compound
    hid_t height_only = H5Tcreate(H5T_COMPOUND, sizeof(float));
    H5Tinsert(height_only, height_name.c_str(), 0, H5T_NATIVE_FLOAT);
    std::vector<float> hbuf(n);
    rd = H5Dread(ds, height_only, H5S_ALL, H5S_ALL, H5P_DEFAULT, hbuf.data());
    H5Tclose(height_only);
    if (rd < 0) {
      H5Sclose(space);
      H5Dclose(ds);
      set_err(err, LoadErrorKind::Parse, "S-104 H5Dread(values) failed");
      return false;
    }
    for (std::size_t i = 0; i < n; ++i) {
      buf[i].height = hbuf[i];
      buf[i].trend = 0;
    }
  } else {
    H5Tclose(mem_type);
  }
  H5Sclose(space);
  H5Dclose(ds);

  grid.fill_depth = 1.0e6f;
  grid.fill_uncertainty = 1.0e6f;
  grid.depth.resize(n);
  grid.uncertainty.clear();
  for (std::size_t i = 0; i < n; ++i) grid.depth[i] = buf[i].height;
  return true;
}

std::string band_token_for_level(float level) {
  // Reuse S-101/S-102 colour tokens so Day/Dusk palette remap works without S-104 PC.
  if (level < 0.0f) return "DEPVS";
  if (level < 2.0f) return "DEPMS";
  if (level < 4.0f) return "DEPMD";
  return "DEPDW";
}

}  // namespace

Bounds WaterLevelDataset::bounds() const {
  Bounds b;
  bool any = false;
  for (const auto& fr : frames) {
    if (!fr.grid.bounds_valid) continue;
    if (!any) {
      b.west = fr.grid.west;
      b.east = fr.grid.east;
      b.south = fr.grid.south;
      b.north = fr.grid.north;
      any = true;
    } else {
      b.west = std::min(b.west, fr.grid.west);
      b.east = std::max(b.east, fr.grid.east);
      b.south = std::min(b.south, fr.grid.south);
      b.north = std::max(b.north, fr.grid.north);
    }
  }
  b.valid = any;
  return b;
}

bool WaterLevelDataset::set_active_frame(std::size_t index) {
  if (index >= frames.size()) return false;
  active_frame = index;
  return true;
}

bool WaterLevelDataset::set_active_time(std::string_view iso_utc) {
  if (frames.empty()) return false;
  const double target = iso_to_seconds(iso_utc);
  std::size_t best = 0;
  double best_dist = std::numeric_limits<double>::infinity();
  for (std::size_t i = 0; i < frames.size(); ++i) {
    const double d = std::fabs(iso_to_seconds(frames[i].time_iso) - target);
    if (d < best_dist) {
      best_dist = d;
      best = i;
    }
  }
  active_frame = best;
  return true;
}

bool WaterLevelDataset::sample_wgs84(double lon, double lat, CoverageSample& out) const {
  out = {};
  const WaterLevelFrame* fr = active();
  if (!fr) return false;
  return fr->grid.sample_wgs84(lon, lat, out);
}

bool portray_water_level(const WaterLevelDataset& wl, DrawingInstructions& out, std::string* err) {
  out.clear();
  const WaterLevelFrame* fr = wl.active();
  if (!fr || fr->grid.ncols == 0 || fr->grid.nrows == 0) {
    if (err) *err = "S-104 portray: no active water-level frame";
    return false;
  }

  // Representative level for tokenized band (centre cell or first valid).
  float rep = 0;
  bool have = false;
  {
    const std::size_t mid_r = fr->grid.nrows / 2;
    const std::size_t mid_c = fr->grid.ncols / 2;
    CoverageSample sm;
    if (fr->grid.sample_cell(mid_r, mid_c, sm) && sm.has_depth) {
      rep = sm.depth;
      have = true;
    } else {
      for (float v : fr->grid.depth) {
        if (fr->grid.is_fill_depth(v)) continue;
        rep = v;
        have = true;
        break;
      }
    }
  }
  if (!have) {
    if (err) *err = "S-104 portray: active frame has no water-level samples";
    return false;
  }

  const std::string active_token = band_token_for_level(rep);

  DrawingInstruction di;
  di.feature_id = wl.features.empty() ? 1 : wl.features.front().id;
  di.kind = DrawingInstruction::Kind::Coverage;
  di.command = "CoverageFill";
  di.coverage_attribute = "depth";  // depth carries water level metres
  di.viewing_group = "24050";
  di.drawing_priority = 0;
  di.display_plane = "UnderRadar";

  // Full diverging band table for painting + active-frame token first so frame
  // switches change CoverageColor tokens (docs/s104-contract.md MUST).
  auto push_band = [&](const std::string& token, const std::string& label, const std::string& lo,
                       const std::string& hi, const char* interval) {
    di.coverage_state.push_back("CoverageColor:" + token + ",0.25");
    di.coverage_state.push_back("LookupEntry:" + label + "," + lo + "," + hi + "," + interval);
  };
  // Active token first with tight band around representative level (changes per frame).
  {
    const double lo = static_cast<double>(rep) - 0.05;
    const double hi = static_cast<double>(rep) + 0.05;
    push_band(active_token, "ActiveLevel", std::to_string(lo), std::to_string(hi), "geLtInterval");
  }
  push_band("DEPVS", "BelowDatum", "", "0", "ltSemiInterval");
  push_band("DEPMS", "Low", "0", "2", "geLtInterval");
  push_band("DEPMD", "Mid", "2", "4", "geLtInterval");
  push_band("DEPDW", "High", "4", "", "geSemiInterval");

  out.push_back(std::move(di));
  return true;
}

bool load_s104(const std::filesystem::path& path, WaterLevelDataset& out, LoadError* err) {
  out = WaterLevelDataset{};
  out.product = ProductId::S104;
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
  // Claimed S-104 must be readable; wrong product is a hard error when /WaterLevel present
  // or productSpecification names S-104.
  const bool claims_s104 =
      (!out.product_specification.empty() &&
       (out.product_specification.find("S-104") != std::string::npos ||
        out.product_specification.find("S104") != std::string::npos)) ||
      H5Lexists(file, "/WaterLevel", H5P_DEFAULT);

  if (!claims_s104) {
    close_file();
    set_err(err, LoadErrorKind::Parse, "HDF5 is not an S-104 water-level product: " + path.string());
    return false;
  }
  if (!H5Lexists(file, "/WaterLevel", H5P_DEFAULT)) {
    close_file();
    set_err(err, LoadErrorKind::Parse, "S-104 missing /WaterLevel group: " + path.string());
    return false;
  }

  int crs = 4326;
  read_scalar_int(file, "horizontalCRS", &crs);
  double root_w = 0, root_e = 0, root_s = 0, root_n = 0;
  const bool root_bounds = read_scalar_double(file, "westBoundLongitude", &root_w) &&
                           read_scalar_double(file, "eastBoundLongitude", &root_e) &&
                           read_scalar_double(file, "southBoundLatitude", &root_s) &&
                           read_scalar_double(file, "northBoundLatitude", &root_n);

  hid_t wl = H5Gopen(file, "/WaterLevel", H5P_DEFAULT);
  if (wl < 0) {
    close_file();
    set_err(err, LoadErrorKind::Parse, "cannot open /WaterLevel");
    return false;
  }

  // Prefer first WaterLevel.NN instance (dcf2 regular grid with Group_NNN time steps).
  std::vector<std::string> inst_names;
  H5G_info_t info{};
  H5Gget_info(wl, &info);
  for (hsize_t i = 0; i < info.nlinks; ++i) {
    char name[256];
    ssize_t len = H5Lget_name_by_idx(wl, ".", H5_INDEX_NAME, H5_ITER_INC, i, name, sizeof(name),
                                     H5P_DEFAULT);
    if (len <= 0) continue;
    if (std::strncmp(name, "WaterLevel.", 11) == 0) inst_names.emplace_back(name);
  }
  std::sort(inst_names.begin(), inst_names.end());
  if (inst_names.empty()) {
    H5Gclose(wl);
    close_file();
    set_err(err, LoadErrorKind::Parse, "S-104 has no WaterLevel.* instance");
    return false;
  }

  const std::string inst_path = "/WaterLevel/" + inst_names.front();
  hid_t inst = H5Gopen(file, inst_path.c_str(), H5P_DEFAULT);
  H5Gclose(wl);
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
    set_err(err, LoadErrorKind::Parse, "S-104 instance missing gridOrigin/gridSpacing");
    return false;
  }
  double ncols = 0, nrows = 0;
  if (!read_scalar_double(inst, "numPointsLongitudinal", &ncols) ||
      !read_scalar_double(inst, "numPointsLatitudinal", &nrows) || ncols < 1 || nrows < 1) {
    H5Gclose(inst);
    close_file();
    set_err(err, LoadErrorKind::Parse, "S-104 instance missing numPointsLongitudinal/Latitudinal");
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
    set_err(err, LoadErrorKind::Parse, "S-104 instance has no Group_* time steps");
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
    WaterLevelFrame frame;
    read_string_attr(grp, "timePoint", &frame.time_iso);
    if (frame.time_iso.empty()) frame.time_iso = gname;
    frame.grid = proto;
    if (!load_group_values(grp, frame.grid, err)) {
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
    set_err(err, LoadErrorKind::Parse, "S-104 loaded zero frames: " + path.string());
    return false;
  }
  out.active_frame = 0;
  CoverageFeature feat;
  feat.id = 1;
  feat.code = "WaterLevel";
  out.features.push_back(std::move(feat));
  return true;
}

}  // namespace s100
