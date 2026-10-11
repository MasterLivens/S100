#include "s100/iso8211.hpp"
#include "s100/dataset.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace s100 {
namespace {

constexpr std::uint8_t kFieldTerm = 0x1e;

struct Leader {
  std::uint32_t rec_len{0};
  std::uint32_t base_addr{0};
  char id{'?'};
  int len_sz{0};
  int pos_sz{0};
  int tag_sz{0};
};

std::uint32_t read_exact_digits(const std::uint8_t* p, int n) {
  std::uint32_t v = 0;
  for (int i = 0; i < n; ++i) {
    if (p[i] < '0' || p[i] > '9') return 0;
    v = v * 10u + static_cast<std::uint32_t>(p[i] - '0');
  }
  return v;
}

bool parse_leader(const std::uint8_t* p, std::size_t n, Leader& L) {
  if (n < 24) return false;
  auto dig = [&](int i) { return p[i] >= '0' && p[i] <= '9'; };
  for (int i = 0; i < 5; ++i)
    if (!dig(i)) return false;
  for (int i = 12; i < 17; ++i)
    if (!dig(i)) return false;
  if (!dig(20) || !dig(21) || !dig(23)) return false;
  // Record length / base address are fixed-width; do not use atoi (next byte may be a digit).
  L.rec_len = read_exact_digits(p, 5);
  L.base_addr = read_exact_digits(p + 12, 5);
  L.id = static_cast<char>(p[6]);
  L.len_sz = p[20] - '0';
  L.pos_sz = p[21] - '0';
  L.tag_sz = p[23] - '0';
  return L.rec_len >= 24 && L.len_sz > 0 && L.pos_sz > 0 && L.tag_sz > 0;
}

std::int32_t read_i32(const std::uint8_t* p) {
  std::int32_t v;
  std::memcpy(&v, p, 4);
  return v;
}

std::uint32_t read_u32(const std::uint8_t* p) {
  std::uint32_t v;
  std::memcpy(&v, p, 4);
  return v;
}

std::uint16_t read_u16(const std::uint8_t* p) {
  std::uint16_t v;
  std::memcpy(&v, p, 2);
  return v;
}

struct Dssi {
  double cmfx{1e7};
  double cmfy{1e7};
  bool ok{false};
};

Dssi parse_dssi(const std::vector<std::uint8_t>& b) {
  Dssi d;
  // Layout observed on IHO TDS: 24-byte origin (3×float64 or zeros) + 3×int32 CMF + counts.
  if (b.size() < 36) return d;
  std::int32_t cmfx = read_i32(b.data() + 24);
  std::int32_t cmfy = read_i32(b.data() + 28);
  if (cmfx != 0) d.cmfx = static_cast<double>(cmfx);
  if (cmfy != 0) d.cmfy = static_cast<double>(cmfy);
  d.ok = true;
  return d;
}

std::vector<Point> decode_c2(const std::vector<std::uint8_t>& blob, double cmfx, double cmfy) {
  std::vector<Point> pts;
  for (std::size_t i = 0; i + 8 <= blob.size(); i += 8) {
    const auto x = read_i32(blob.data() + i);
    const auto y = read_i32(blob.data() + i + 4);
    pts.push_back(Point{static_cast<double>(x) / cmfx, static_cast<double>(y) / cmfy});
  }
  return pts;
}

/** FTCS: name\x1f + u16 code, repeating. */
std::unordered_map<std::uint16_t, std::string> parse_ftcs(const std::vector<std::uint8_t>& b) {
  std::unordered_map<std::uint16_t, std::string> m;
  std::size_t i = 0;
  while (i < b.size()) {
    std::size_t start = i;
    while (i < b.size() && b[i] != 0x1f) ++i;
    if (i >= b.size()) break;
    std::string name(reinterpret_cast<const char*>(b.data() + start), i - start);
    ++i;  // skip UT
    if (i + 2 > b.size()) break;
    std::uint16_t code = read_u16(b.data() + i);
    i += 2;
    m[code] = std::move(name);
  }
  return m;
}

/** ATTR repeating: NATC(u16) ATIX(u16) PAIX(u16) ATIN(u8) ATVL(text, UT-terminated). */
std::vector<Attribute> parse_attributes(const std::vector<std::uint8_t>& b,
                                        const std::unordered_map<std::uint16_t, std::string>& atcs) {
  std::vector<Attribute> out;
  std::size_t i = 0;
  while (i + 7 <= b.size()) {
    Attribute a;
    const std::uint16_t natc = read_u16(b.data() + i);
    a.index = read_u16(b.data() + i + 2);
    a.parent = read_u16(b.data() + i + 4);
    a.instruction = b[i + 6];
    i += 7;
    const std::size_t start = i;
    while (i < b.size() && b[i] != 0x1f) ++i;
    a.value.assign(reinterpret_cast<const char*>(b.data() + start), i - start);
    if (i < b.size()) ++i;  // skip UT
    auto it = atcs.find(natc);
    a.code = it != atcs.end() ? it->second : ("attr" + std::to_string(natc));
    out.push_back(std::move(a));
  }
  return out;
}

LayerId layer_for_feature_type(const std::string& name) {
  if (name == "LandArea" || name == "LandRegion" || name == "ShorelineConstruction" ||
      name == "Dam" || name == "Dyke" || name == "Causeway") {
    return LayerId::LAND;
  }
  if (name == "DepthArea" || name == "DepthContour" || name == "QualityOfBathymetricData" ||
      name == "SoundingDatum" || name == "SeaAreaNamedWaterArea") {
    return LayerId::DEPARE;
  }
  return LayerId::Other;
}

struct SpatialKey {
  std::uint8_t rcnm{0};
  std::uint32_t rcid{0};
  bool operator==(const SpatialKey& o) const { return rcnm == o.rcnm && rcid == o.rcid; }
};

struct SpatialKeyHash {
  std::size_t operator()(const SpatialKey& k) const {
    return (static_cast<std::size_t>(k.rcnm) << 32) ^ static_cast<std::size_t>(k.rcid);
  }
};

struct SpatialGeom {
  SpatialKey key;
  Geometry geometry;
};

SpatialKey key_from_name_id(const std::vector<std::uint8_t>& b) {
  SpatialKey k;
  if (b.size() < 5) return k;
  k.rcnm = b[0];
  k.rcid = read_u32(b.data() + 1);
  return k;
}

std::uint16_t nftc_from_frid(const std::vector<std::uint8_t>& b) {
  // FRID: RCNM(u8) + RCID(u32) + NFTC(u16) + ...
  if (b.size() >= 7) return read_u16(b.data() + 5);
  return 0;
}

SpatialKey spas_target_key(const std::vector<std::uint8_t>& b) {
  return key_from_name_id(b);
}

}  // namespace

const Iso8211Field* Iso8211Record::find(const char* tag) const {
  for (const auto& f : fields) {
    if (f.tag == tag) return &f;
  }
  return nullptr;
}

bool parse_iso8211_records(const std::vector<std::uint8_t>& bytes,
                           std::vector<Iso8211Record>& out, LoadError* err) {
  out.clear();
  std::size_t off = 0;
  while (off + 24 <= bytes.size()) {
    Leader L;
    if (!parse_leader(bytes.data() + off, bytes.size() - off, L)) {
      if (out.empty()) {
        if (err) {
          err->kind = LoadErrorKind::Parse;
          err->message = "ISO 8211 leader not found";
        }
        return false;
      }
      break;
    }
    if (L.rec_len < 24 || off + L.rec_len > bytes.size()) {
      if (err) {
        err->kind = LoadErrorKind::Parse;
        err->message = "ISO 8211 record length out of range at offset " + std::to_string(off);
      }
      return false;
    }
    Iso8211Record rec;
    rec.offset = off;
    rec.length = L.rec_len;
    rec.leader_id = L.id;
    const std::size_t field_area = off + L.base_addr;
    if (field_area > off + L.rec_len) {
      if (err) {
        err->kind = LoadErrorKind::Parse;
        err->message = "ISO 8211 base address past record end";
      }
      return false;
    }
    const std::uint8_t* dir = bytes.data() + off + 24;
    const std::size_t dir_len = field_area - (off + 24);
    std::size_t i = 0;
    const int esz = L.tag_sz + L.len_sz + L.pos_sz;
    while (i + static_cast<std::size_t>(esz) <= dir_len && dir[i] != kFieldTerm) {
      Iso8211Field f;
      f.tag.assign(reinterpret_cast<const char*>(dir + i), L.tag_sz);
      i += static_cast<std::size_t>(L.tag_sz);
      char lenbuf[16] = {0};
      std::memcpy(lenbuf, dir + i, static_cast<std::size_t>(L.len_sz));
      const int flen = std::atoi(lenbuf);
      i += static_cast<std::size_t>(L.len_sz);
      char posbuf[16] = {0};
      std::memcpy(posbuf, dir + i, static_cast<std::size_t>(L.pos_sz));
      const int fpos = std::atoi(posbuf);
      i += static_cast<std::size_t>(L.pos_sz);
      if (flen < 0 || fpos < 0) continue;
      const std::size_t abs = field_area + static_cast<std::size_t>(fpos);
      if (abs + static_cast<std::size_t>(flen) > off + L.rec_len) continue;
      f.data.assign(bytes.begin() + static_cast<std::ptrdiff_t>(abs),
                    bytes.begin() + static_cast<std::ptrdiff_t>(abs + flen));
      if (!f.data.empty() && f.data.back() == kFieldTerm) f.data.pop_back();
      rec.fields.push_back(std::move(f));
    }
    out.push_back(std::move(rec));
    off += L.rec_len;
  }
  return !out.empty();
}

bool load_iso8211_file(const std::filesystem::path& path, Dataset& out, LoadError* err) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    if (err) {
      err->kind = LoadErrorKind::Io;
      err->message = "cannot open " + path.string();
    }
    return false;
  }
  std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)),
                                  std::istreambuf_iterator<char>());
  if (bytes.empty()) {
    if (err) {
      err->kind = LoadErrorKind::Invalid;
      err->message = "empty cell: " + path.string();
    }
    return false;
  }

  std::vector<Iso8211Record> records;
  if (!parse_iso8211_records(bytes, records, err)) return false;

  Dssi dssi;
  std::unordered_map<std::uint16_t, std::string> ftcs;
  std::unordered_map<std::uint16_t, std::string> atcs;
  // Topology primitives keyed by (RCNM, RCID): 110 point, 120 curve, 125 composite, 130 surface.
  std::unordered_map<SpatialKey, Point, SpatialKeyHash> points;
  std::unordered_map<SpatialKey, std::vector<Point>, SpatialKeyHash> curves;
  struct Ref { SpatialKey key; bool reverse{false}; std::uint8_t usage{1}; };
  std::unordered_map<SpatialKey, std::vector<Ref>, SpatialKeyHash> composites;
  std::unordered_map<SpatialKey, std::vector<Ref>, SpatialKeyHash> surfaces;

  for (const auto& rec : records) {
    if (rec.leader_id != 'D') continue;
    if (!dssi.ok)
      if (auto* f = rec.find("DSSI")) dssi = parse_dssi(f->data);
    if (ftcs.empty())
      if (auto* f = rec.find("FTCS")) ftcs = parse_ftcs(f->data);
    if (atcs.empty())
      if (auto* f = rec.find("ATCS")) atcs = parse_ftcs(f->data);  // same name/code layout

    if (auto* c2it = rec.find("C2IT")) {
      if (auto* id = rec.find("PRID")) {
        auto pts = decode_c2(c2it->data, dssi.cmfx, dssi.cmfy);
        if (!pts.empty()) points[key_from_name_id(id->data)] = pts[0];
      }
    }
    if (auto* c2il = rec.find("C2IL")) {
      if (auto* id = rec.find("CRID")) {
        auto& dst = curves[key_from_name_id(id->data)];
        auto pts = decode_c2(c2il->data, dssi.cmfx, dssi.cmfy);
        dst.insert(dst.end(), pts.begin(), pts.end());  // multiple segments append
      }
    }
    if (auto* cuco = rec.find("CUCO")) {
      if (auto* id = rec.find("CCID")) {
        auto& refs = composites[key_from_name_id(id->data)];
        // CUCO repeating: RRNM(u8) RRID(u32) ORNT(u8)
        for (std::size_t i = 0; i + 6 <= cuco->data.size(); i += 6) {
          Ref r;
          r.key = SpatialKey{cuco->data[i], read_u32(cuco->data.data() + i + 1)};
          r.reverse = cuco->data[i + 5] == 2;
          refs.push_back(r);
        }
      }
    }
    if (auto* rias = rec.find("RIAS")) {
      if (auto* id = rec.find("SRID")) {
        auto& refs = surfaces[key_from_name_id(id->data)];
        // RIAS repeating: RRNM(u8) RRID(u32) ORNT(u8) USAG(u8) RAUI(u8)
        for (std::size_t i = 0; i + 8 <= rias->data.size(); i += 8) {
          Ref r;
          r.key = SpatialKey{rias->data[i], read_u32(rias->data.data() + i + 1)};
          r.reverse = rias->data[i + 5] == 2;
          r.usage = rias->data[i + 6];
          refs.push_back(r);
        }
      }
    }
  }

  // Resolve a curve or composite curve into a point chain (depth-limited for safety).
  std::function<std::vector<Point>(const SpatialKey&, bool, int)> resolve_curve =
      [&](const SpatialKey& k, bool reverse, int depth) -> std::vector<Point> {
    std::vector<Point> pts;
    if (depth > 16) return pts;
    if (k.rcnm == 120) {
      auto it = curves.find(k);
      if (it != curves.end()) pts = it->second;
    } else if (k.rcnm == 125) {
      auto it = composites.find(k);
      if (it != composites.end()) {
        for (const auto& r : it->second) {
          auto part = resolve_curve(r.key, r.reverse, depth + 1);
          if (part.empty()) continue;
          if (!pts.empty() && pts.back().lon == part.front().lon &&
              pts.back().lat == part.front().lat)
            part.erase(part.begin());
          pts.insert(pts.end(), part.begin(), part.end());
        }
      }
    }
    if (reverse) std::reverse(pts.begin(), pts.end());
    return pts;
  };

  auto resolve_surface = [&](const SpatialKey& k, Geometry& g) -> bool {
    auto it = surfaces.find(k);
    if (it == surfaces.end()) return false;
    std::vector<Ring> exterior, interior;
    for (const auto& r : it->second) {
      Ring ring;
      ring.points = resolve_curve(r.key, r.reverse, 0);
      if (ring.points.size() < 3) continue;
      (r.usage == 2 ? interior : exterior).push_back(std::move(ring));
    }
    if (exterior.empty()) return false;
    g = Geometry{};
    g.kind = GeometryKind::Polygon;
    g.polygon = std::move(exterior);  // exterior ring(s) first
    for (auto& ring : interior) g.polygon.push_back(std::move(ring));
    return true;
  };

  out = Dataset{};
  out.path = path;
  out.product = ProductId::S101;
  out.layers.register_layer(LayerId::LAND, "LAND");
  out.layers.register_layer(LayerId::DEPARE, "DEPARE");

  std::uint64_t next_id = 1;
  for (const auto& rec : records) {
    if (rec.leader_id != 'D') continue;
    auto* frid = rec.find("FRID");
    if (!frid) continue;
    const std::uint16_t nftc = nftc_from_frid(frid->data);
    std::string fname = "Feature";
    auto it = ftcs.find(nftc);
    if (it != ftcs.end()) fname = it->second;

    Feature feat;
    feat.id = next_id++;
    feat.feature_type = fname;
    feat.layer = layer_for_feature_type(fname);
    feat.geometry.kind = GeometryKind::Empty;
    if (auto* attr = rec.find("ATTR")) feat.attributes = parse_attributes(attr->data, atcs);

    // SPAS repeating: RRNM(u8) RRID(u32) ORNT(u8) SMIN(u32) SMAX(u32) SAUI(u8) = 15 bytes
    if (auto* spas = rec.find("SPAS")) {
      for (std::size_t i = 0; i + 5 <= spas->data.size(); i += 15) {
        SpatialKey k{spas->data[i], read_u32(spas->data.data() + i + 1)};
        const bool rev = i + 5 < spas->data.size() && spas->data[i + 5] == 2;
        if (k.rcnm == 130) {
          if (resolve_surface(k, feat.geometry)) break;
        } else if (k.rcnm == 120 || k.rcnm == 125) {
          auto pts = resolve_curve(k, rev, 0);
          if (pts.size() >= 2) {
            feat.geometry.kind = GeometryKind::LineString;
            feat.geometry.line.insert(feat.geometry.line.end(), pts.begin(), pts.end());
          }
        } else if (k.rcnm == 110) {
          auto pit = points.find(k);
          if (pit != points.end()) {
            feat.geometry.kind = GeometryKind::Point;
            feat.geometry.point = pit->second;
            break;
          }
        }
      }
    }
    out.features.push_back(std::move(feat));
  }

  if (out.features.empty()) {
    if (err) {
      err->kind = LoadErrorKind::Invalid;
      err->message = "no features decoded from " + path.string();
    }
    return false;
  }
  return true;
}

}  // namespace s100
