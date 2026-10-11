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
  double cmfz{1};
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
  if (b.size() >= 36) {
    const std::int32_t cmfz = read_i32(b.data() + 32);
    if (cmfz != 0) d.cmfz = static_cast<double>(cmfz);
  }
  d.ok = true;
  return d;
}

std::vector<Point> decode_c2(const std::vector<std::uint8_t>& blob, double cmfx, double cmfy) {
  std::vector<Point> pts;
  for (std::size_t i = 0; i + 8 <= blob.size(); i += 8) {
    // S-101 C2IT/C2IL subfield order is YCOO, XCOO (latitude first).
    const auto y = read_i32(blob.data() + i);
    const auto x = read_i32(blob.data() + i + 4);
    pts.push_back(Point{static_cast<double>(x) / cmfx, static_cast<double>(y) / cmfy});
  }
  return pts;
}

/** C3IL: VCID(u8) then repeating YCOO, XCOO, ZCOO (int32 each, latitude first). */
std::vector<Point3> decode_c3(const std::vector<std::uint8_t>& blob, double cmfx, double cmfy,
                              double cmfz) {
  std::vector<Point3> pts;
  for (std::size_t i = 1; i + 12 <= blob.size(); i += 12) {
    const auto y = read_i32(blob.data() + i);
    const auto x = read_i32(blob.data() + i + 4);
    const auto z = read_i32(blob.data() + i + 8);
    pts.push_back(Point3{static_cast<double>(x) / cmfx, static_cast<double>(y) / cmfy,
                         static_cast<double>(z) / cmfz});
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


/** FASC / INAS field: RRNM(u8) RRID(u32) NFAC|NIAC(u16) NARC(u16) FAUI|IUIN(u8), then ATTR-like
 *  repeating attributes (Part 10a). One association per field; the field itself repeats. */
AssociationRef parse_association(const std::vector<std::uint8_t>& b,
                                 const std::unordered_map<std::uint16_t, std::string>& assoc_codes,
                                 const std::unordered_map<std::uint16_t, std::string>& role_codes,
                                 const std::unordered_map<std::uint16_t, std::string>& atcs) {
  AssociationRef a;
  if (b.size() < 10) return a;
  a.rcnm = b[0];
  a.rcid = read_u32(b.data() + 1);
  const std::uint16_t nac = read_u16(b.data() + 5);
  const std::uint16_t narc = read_u16(b.data() + 7);
  a.instruction = b[9];  // FAUI / IUIN
  auto it = assoc_codes.find(nac);
  a.association = it != assoc_codes.end() ? it->second : ("assoc" + std::to_string(nac));
  auto rt = role_codes.find(narc);
  a.role = rt != role_codes.end() ? rt->second : ("role" + std::to_string(narc));
  std::vector<std::uint8_t> rest(b.begin() + 10, b.end());
  a.attributes = parse_attributes(rest, atcs);
  return a;
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


DatasetIdentification parse_dsid_field(const std::vector<std::uint8_t>& d) {
  DatasetIdentification id;
  if (d.size() < 6) return id;
  // RCNM(b11) RCID(b14) + 7 UT (ENSP..DSTL) + DSRD A(8) + DSLG/DSAB/DSED (+ *DSTC)
  std::size_t i = 5;
  auto read_ut = [&](std::string& out) -> bool {
    const std::size_t start = i;
    while (i < d.size() && d[i] != 0x1f) ++i;
    out.assign(reinterpret_cast<const char*>(d.data() + start), i - start);
    if (i < d.size() && d[i] == 0x1f) {
      ++i;
      return true;
    }
    return i > start || i >= d.size();
  };
  std::string skip;
  for (int n = 0; n < 5; ++n) {
    if (!read_ut(skip)) return id;
  }
  if (!read_ut(id.dataset_name)) return id;  // DSNM
  if (!read_ut(skip)) return id;            // DSTL
  if (i + 8 > d.size()) return id;
  i += 8;  // DSRD A(8)
  std::string dslg, dsab, dsed;
  if (!read_ut(dslg)) return id;
  if (!read_ut(dsab)) return id;
  if (!read_ut(dsed)) return id;
  const auto dot = dsed.find('.');
  if (dot == std::string::npos) return id;
  try {
    id.edition_number = std::stoi(dsed.substr(0, dot));
    id.update_number = std::stoi(dsed.substr(dot + 1));
  } catch (...) {
    return id;
  }
  id.ok = true;
  return id;
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
  std::unordered_map<std::uint16_t, std::string> itcs, iacs, facs, arcs;
  // Topology primitives keyed by (RCNM, RCID): 110 point, 120 curve, 125 composite, 130 surface.
  std::unordered_map<SpatialKey, Point, SpatialKeyHash> points;
  std::unordered_map<SpatialKey, std::vector<Point>, SpatialKeyHash> curves;
  std::unordered_map<SpatialKey, std::vector<Point3>, SpatialKeyHash> multipoints;  // 115
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
    if (itcs.empty())
      if (auto* f = rec.find("ITCS")) itcs = parse_ftcs(f->data);
    if (iacs.empty())
      if (auto* f = rec.find("IACS")) iacs = parse_ftcs(f->data);
    if (facs.empty())
      if (auto* f = rec.find("FACS")) facs = parse_ftcs(f->data);
    if (arcs.empty())
      if (auto* f = rec.find("ARCS")) arcs = parse_ftcs(f->data);

    if (auto* c2it = rec.find("C2IT")) {
      if (auto* id = rec.find("PRID")) {
        auto pts = decode_c2(c2it->data, dssi.cmfx, dssi.cmfy);
        if (!pts.empty()) points[key_from_name_id(id->data)] = pts[0];
      }
    }
    if (auto* c3il = rec.find("C3IL")) {
      if (auto* id = rec.find("MRID")) {
        auto& dst = multipoints[key_from_name_id(id->data)];
        auto pts = decode_c3(c3il->data, dssi.cmfx, dssi.cmfy, dssi.cmfz);
        dst.insert(dst.end(), pts.begin(), pts.end());
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
  for (const auto& rec : records) {
    if (rec.leader_id != 'D') continue;
    if (auto* f = rec.find("DSID")) {
      const auto id = parse_dsid_field(f->data);
      if (id.ok) {
        out.edition_number = id.edition_number;
        out.update_number = id.update_number;
        out.dataset_name = id.dataset_name;
      }
      break;
    }
  }

  // Keep topology + information records for the portrayal host (Part 13 spatial access).
  auto inas_of = [&](const Iso8211Record& rec) {
    std::vector<AssociationRef> v;
    for (const auto& f : rec.fields)
      if (f.tag == "INAS") v.push_back(parse_association(f.data, iacs, arcs, atcs));
    return v;
  };
  for (const auto& rec : records) {
    if (rec.leader_id != 'D') continue;
    if (auto* irid = rec.find("IRID")) {
      InformationRecord ir;
      if (irid->data.size() >= 7) {
        ir.record_id = read_u32(irid->data.data() + 1);
        const std::uint16_t nitc = read_u16(irid->data.data() + 5);
        auto it = itcs.find(nitc);
        ir.information_type = it != itcs.end() ? it->second : ("info" + std::to_string(nitc));
      }
      if (auto* attr = rec.find("ATTR")) ir.attributes = parse_attributes(attr->data, atcs);
      ir.information_associations = inas_of(rec);
      out.information.push_back(std::move(ir));
      continue;
    }
    const Iso8211Field* id = nullptr;
    for (const char* t : {"PRID", "MRID", "CRID", "CCID", "SRID"})
      if ((id = rec.find(t))) break;
    if (!id) continue;
    const SpatialKey k = key_from_name_id(id->data);
    SpatialRecord sr;
    sr.rcnm = k.rcnm;
    sr.rcid = k.rcid;
    if (k.rcnm == 110) {
      auto it = points.find(k);
      if (it != points.end()) sr.coords.push_back(Point3{it->second.lon, it->second.lat, 0});
    } else if (k.rcnm == 115) {
      auto it = multipoints.find(k);
      if (it != multipoints.end()) sr.coords = it->second;
    } else if (k.rcnm == 120) {
      auto it = curves.find(k);
      if (it != curves.end())
        for (const auto& p : it->second) sr.coords.push_back(Point3{p.lon, p.lat, 0});
      if (auto* ptas = rec.find("PTAS")) {
        // PTAS repeating: RRNM(u8) RRID(u32) TOPI(u8): 1 begin, 2 end, 3 begin and end
        for (std::size_t i = 0; i + 6 <= ptas->data.size(); i += 6) {
          const std::uint32_t pid = read_u32(ptas->data.data() + i + 1);
          const std::uint8_t topi = ptas->data[i + 5];
          if (topi == 1 || topi == 3) sr.start_point = pid;
          if (topi == 2 || topi == 3) sr.end_point = pid;
        }
      }
    } else if (k.rcnm == 125) {
      auto it = composites.find(k);
      if (it != composites.end())
        for (const auto& r : it->second)
          sr.parts.push_back(SpatialRef{r.key.rcnm, r.key.rcid, r.reverse, 0, 0, 0});
    } else if (k.rcnm == 130) {
      auto it = surfaces.find(k);
      if (it != surfaces.end())
        for (const auto& r : it->second)
          sr.parts.push_back(SpatialRef{r.key.rcnm, r.key.rcid, r.reverse, r.usage, 0, 0});
    }
    sr.information_associations = inas_of(rec);
    out.spatial.records[SpatialStore::key(k.rcnm, k.rcid)] = std::move(sr);
  }

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
    feat.geometry.kind = GeometryKind::Empty;
    if (auto* attr = rec.find("ATTR")) feat.attributes = parse_attributes(attr->data, atcs);
    if (frid->data.size() >= 5) feat.record_id = read_u32(frid->data.data() + 1);
    for (const auto& f : rec.fields) {
      if (f.tag == "FASC") feat.feature_associations.push_back(parse_association(f.data, facs, arcs, atcs));
      else if (f.tag == "INAS")
        feat.information_associations.push_back(parse_association(f.data, iacs, arcs, atcs));
    }

    // SPAS repeating: RRNM(u8) RRID(u32) ORNT(u8) SMIN(u32) SMAX(u32) SAUI(u8) = 15 bytes
    if (auto* spas = rec.find("SPAS")) {
      for (std::size_t i = 0; i + 15 <= spas->data.size(); i += 15) {
        SpatialRef ref;
        ref.rcnm = spas->data[i];
        ref.rcid = read_u32(spas->data.data() + i + 1);
        ref.reverse = spas->data[i + 5] == 2;
        ref.scale_min = read_u32(spas->data.data() + i + 6);
        ref.scale_max = read_u32(spas->data.data() + i + 10);
        feat.spatial.push_back(ref);
      }
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
        } else if (k.rcnm == 115) {
          auto mit = multipoints.find(k);
          if (mit != multipoints.end()) {
            feat.geometry.kind = GeometryKind::MultiPoint;
            feat.geometry.multipoint.insert(feat.geometry.multipoint.end(), mit->second.begin(),
                                            mit->second.end());
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


bool read_s101_dsid(const std::filesystem::path& path, DatasetIdentification& out, LoadError* err) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    if (err) {
      err->kind = LoadErrorKind::Io;
      err->message = "cannot open for DSID: " + path.string();
    }
    return false;
  }
  std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)),
                                  std::istreambuf_iterator<char>());
  std::vector<Iso8211Record> records;
  if (!parse_iso8211_records(bytes, records, err)) return false;
  for (const auto& rec : records) {
    if (rec.leader_id != 'D') continue;
    if (auto* f = rec.find("DSID")) {
      out = parse_dsid_field(f->data);
      if (!out.ok) {
        if (err) {
          err->kind = LoadErrorKind::Invalid;
          err->message = "DSID DSED missing or unparseable in " + path.string();
        }
        return false;
      }
      return true;
    }
  }
  if (err) {
    err->kind = LoadErrorKind::Invalid;
    err->message = "no DSID record in " + path.string();
  }
  return false;
}

}  // namespace s100
