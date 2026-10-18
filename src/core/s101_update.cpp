#include "s100/exchange_set.hpp"
#include "s100/iso8211.hpp"
#include "s101_update_internal.hpp"

#include <algorithm>
#include <cstring>
#include <exception>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <string>
#include <unordered_map>

namespace s100 {
namespace {

std::int32_t read_i32(const std::uint8_t* p) {
  std::int32_t v; std::memcpy(&v, p, 4); return v;
}
std::uint32_t read_u32(const std::uint8_t* p) {
  std::uint32_t v; std::memcpy(&v, p, 4); return v;
}
std::uint16_t read_u16(const std::uint8_t* p) {
  std::uint16_t v; std::memcpy(&v, p, 2); return v;
}

struct Dssi {
  double cmfx{1e7}, cmfy{1e7}, cmfz{1};
  bool ok{false};
};

Dssi parse_dssi(const std::vector<std::uint8_t>& b) {
  Dssi d;
  if (b.size() < 36) return d;
  const auto cmfx = read_i32(b.data() + 24), cmfy = read_i32(b.data() + 28), cmfz = read_i32(b.data() + 32);
  if (cmfx) d.cmfx = cmfx;
  if (cmfy) d.cmfy = cmfy;
  if (cmfz) d.cmfz = cmfz;
  d.ok = true;
  return d;
}

std::unordered_map<std::uint16_t, std::string> parse_ftcs(const std::vector<std::uint8_t>& b) {
  std::unordered_map<std::uint16_t, std::string> m;
  std::size_t i = 0;
  while (i < b.size()) {
    const std::size_t start = i;
    while (i < b.size() && b[i] != 0x1f) ++i;
    if (i >= b.size()) break;
    std::string name(reinterpret_cast<const char*>(b.data() + start), i - start);
    ++i;
    if (i + 2 > b.size()) break;
    m[read_u16(b.data() + i)] = std::move(name);
    i += 2;
  }
  return m;
}

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
    if (i < b.size()) ++i;
    auto it = atcs.find(natc);
    a.code = it != atcs.end() ? it->second : ("attr" + std::to_string(natc));
    out.push_back(std::move(a));
  }
  return out;
}

AssociationRef parse_association(const std::vector<std::uint8_t>& b,
                                 const std::unordered_map<std::uint16_t, std::string>& assoc_codes,
                                 const std::unordered_map<std::uint16_t, std::string>& role_codes,
                                 const std::unordered_map<std::uint16_t, std::string>& atcs) {
  AssociationRef a;
  if (b.size() < 10) return a;
  a.rcnm = b[0];
  a.rcid = read_u32(b.data() + 1);
  const auto nac = read_u16(b.data() + 5), narc = read_u16(b.data() + 7);
  a.instruction = b[9];
  auto it = assoc_codes.find(nac);
  a.association = it != assoc_codes.end() ? it->second : ("assoc" + std::to_string(nac));
  auto rt = role_codes.find(narc);
  a.role = rt != role_codes.end() ? rt->second : ("role" + std::to_string(narc));
  a.attributes = parse_attributes(std::vector<std::uint8_t>(b.begin() + 10, b.end()), atcs);
  return a;
}

std::vector<Point> decode_c2(const std::vector<std::uint8_t>& blob, double cmfx, double cmfy) {
  std::vector<Point> pts;
  for (std::size_t i = 0; i + 8 <= blob.size(); i += 8)
    pts.push_back(Point{static_cast<double>(read_i32(blob.data() + i + 4)) / cmfx,
                        static_cast<double>(read_i32(blob.data() + i)) / cmfy});
  return pts;
}

std::vector<Point3> decode_c3(const std::vector<std::uint8_t>& blob, double cmfx, double cmfy,
                              double cmfz) {
  std::vector<Point3> pts;
  for (std::size_t i = 1; i + 12 <= blob.size(); i += 12)
    pts.push_back(Point3{static_cast<double>(read_i32(blob.data() + i + 4)) / cmfx,
                         static_cast<double>(read_i32(blob.data() + i)) / cmfy,
                         static_cast<double>(read_i32(blob.data() + i + 8)) / cmfz});
  return pts;
}

bool attr_match(const Attribute& a, const Attribute& b) {
  return a.code == b.code && a.index == b.index && a.parent == b.parent;
}

void apply_atin(std::vector<Attribute>& attrs, const std::vector<Attribute>& patch) {
  for (const auto& p : patch) {
    const std::uint8_t insn = p.instruction == 0 ? 1 : p.instruction;
    if (insn == 1) {
      Attribute copy = p;
      copy.instruction = 1;
      attrs.push_back(std::move(copy));
    } else if (insn == 2) {
      attrs.erase(std::remove_if(attrs.begin(), attrs.end(),
                                 [&](const Attribute& a) { return attr_match(a, p); }),
                  attrs.end());
    } else if (insn == 3) {
      for (auto& a : attrs)
        if (attr_match(a, p)) {
          a.value = p.value;
          break;
        }
    }
  }
}

std::vector<SpatialRef> parse_spas(const std::vector<std::uint8_t>& b) {
  std::vector<SpatialRef> out;
  for (std::size_t i = 0; i + 15 <= b.size(); i += 15) {
    SpatialRef ref;
    ref.rcnm = b[i];
    ref.rcid = read_u32(b.data() + i + 1);
    ref.reverse = b[i + 5] == 2;
    ref.scale_min = read_u32(b.data() + i + 6);
    ref.scale_max = read_u32(b.data() + i + 10);
    out.push_back(ref);
  }
  return out;
}

void apply_spas_sau(std::vector<SpatialRef>& spas, const std::vector<std::uint8_t>& b) {
  for (std::size_t i = 0; i + 15 <= b.size(); i += 15) {
    SpatialRef ref;
    ref.rcnm = b[i];
    ref.rcid = read_u32(b.data() + i + 1);
    ref.reverse = b[i + 5] == 2;
    ref.scale_min = read_u32(b.data() + i + 6);
    ref.scale_max = read_u32(b.data() + i + 10);
    const std::uint8_t saui = b[i + 14] == 0 ? 1 : b[i + 14];
    auto same = [&](const SpatialRef& s) { return s.rcnm == ref.rcnm && s.rcid == ref.rcid; };
    if (saui == 1) spas.push_back(ref);
    else if (saui == 2)
      spas.erase(std::remove_if(spas.begin(), spas.end(), same), spas.end());
    else if (saui == 3) {
      for (auto& s : spas)
        if (same(s)) {
          s = ref;
          break;
        }
    }
  }
}

void rebuild_feature_geometries(Dataset& ds) {
  auto resolve_curve = [&](auto&& self, std::uint8_t rcnm, std::uint32_t rcid, bool reverse,
                           int depth) -> std::vector<Point> {
    std::vector<Point> pts;
    if (depth > 16) return pts;
    const auto* sr = ds.spatial.find(rcnm, rcid);
    if (!sr) return pts;
    if (rcnm == 120) {
      for (const auto& c : sr->coords) pts.push_back(Point{c.lon, c.lat});
    } else if (rcnm == 125) {
      for (const auto& part : sr->parts) {
        auto chunk = self(self, part.rcnm, part.rcid, part.reverse, depth + 1);
        if (chunk.empty()) continue;
        if (!pts.empty() && pts.back().lon == chunk.front().lon &&
            pts.back().lat == chunk.front().lat)
          chunk.erase(chunk.begin());
        pts.insert(pts.end(), chunk.begin(), chunk.end());
      }
    }
    if (reverse) std::reverse(pts.begin(), pts.end());
    return pts;
  };

  for (auto& feat : ds.features) {
    if (feat.spatial.empty()) continue;
    feat.geometry = Geometry{};
    feat.geometry.kind = GeometryKind::Empty;
    for (const auto& ref : feat.spatial) {
      if (ref.rcnm == 130) {
        const auto* sr = ds.spatial.find(ref.rcnm, ref.rcid);
        if (!sr) continue;
        std::vector<Ring> exterior, interior;
        for (const auto& part : sr->parts) {
          Ring ring;
          ring.points = resolve_curve(resolve_curve, part.rcnm, part.rcid, part.reverse, 0);
          if (ring.points.size() < 3) continue;
          (part.usage == 2 ? interior : exterior).push_back(std::move(ring));
        }
        if (!exterior.empty()) {
          feat.geometry.kind = GeometryKind::Polygon;
          feat.geometry.polygon = std::move(exterior);
          for (auto& r : interior) feat.geometry.polygon.push_back(std::move(r));
          break;
        }
      } else if (ref.rcnm == 120 || ref.rcnm == 125) {
        auto pts = resolve_curve(resolve_curve, ref.rcnm, ref.rcid, ref.reverse, 0);
        if (pts.size() >= 2) {
          feat.geometry.kind = GeometryKind::LineString;
          feat.geometry.line.insert(feat.geometry.line.end(), pts.begin(), pts.end());
        }
      } else if (ref.rcnm == 115) {
        if (const auto* sr = ds.spatial.find(ref.rcnm, ref.rcid); sr && !sr->coords.empty()) {
          feat.geometry.kind = GeometryKind::MultiPoint;
          feat.geometry.multipoint.insert(feat.geometry.multipoint.end(), sr->coords.begin(),
                                          sr->coords.end());
        }
      } else if (ref.rcnm == 110) {
        if (const auto* sr = ds.spatial.find(ref.rcnm, ref.rcid); sr && !sr->coords.empty()) {
          feat.geometry.kind = GeometryKind::Point;
          feat.geometry.point = Point{sr->coords.front().lon, sr->coords.front().lat};
          break;
        }
      }
    }
  }
}

SpatialRecord decode_spatial(const Iso8211Record& rec, const char* id_tag, const Dssi& dssi,
                             const std::unordered_map<std::uint16_t, std::string>& iacs,
                             const std::unordered_map<std::uint16_t, std::string>& arcs,
                             const std::unordered_map<std::uint16_t, std::string>& atcs) {
  SpatialRecord sr;
  auto* id = rec.find(id_tag);
  if (!id || id->data.size() < 5) return sr;
  sr.rcnm = id->data[0];
  sr.rcid = read_u32(id->data.data() + 1);
  if (auto* c2it = rec.find("C2IT")) {
    auto pts = decode_c2(c2it->data, dssi.cmfx, dssi.cmfy);
    if (!pts.empty()) sr.coords.push_back(Point3{pts[0].lon, pts[0].lat, 0});
  }
  if (auto* c3il = rec.find("C3IL")) {
    auto pts = decode_c3(c3il->data, dssi.cmfx, dssi.cmfy, dssi.cmfz);
    sr.coords.insert(sr.coords.end(), pts.begin(), pts.end());
  }
  if (auto* c2il = rec.find("C2IL")) {
    for (const auto& p : decode_c2(c2il->data, dssi.cmfx, dssi.cmfy))
      sr.coords.push_back(Point3{p.lon, p.lat, 0});
  }
  if (auto* ptas = rec.find("PTAS")) {
    for (std::size_t i = 0; i + 6 <= ptas->data.size(); i += 6) {
      const auto pid = read_u32(ptas->data.data() + i + 1);
      const auto topi = ptas->data[i + 5];
      if (topi == 1 || topi == 3) sr.start_point = pid;
      if (topi == 2 || topi == 3) sr.end_point = pid;
    }
  }
  if (auto* cuco = rec.find("CUCO")) {
    for (std::size_t i = 0; i + 6 <= cuco->data.size(); i += 6)
      sr.parts.push_back(
          SpatialRef{cuco->data[i], read_u32(cuco->data.data() + i + 1), cuco->data[i + 5] == 2, 0, 0, 0});
  }
  if (auto* rias = rec.find("RIAS")) {
    for (std::size_t i = 0; i + 8 <= rias->data.size(); i += 8)
      sr.parts.push_back(SpatialRef{rias->data[i], read_u32(rias->data.data() + i + 1),
                                    rias->data[i + 5] == 2, rias->data[i + 6], 0, 0});
  }
  for (const auto& f : rec.fields)
    if (f.tag == "INAS")
      sr.information_associations.push_back(parse_association(f.data, iacs, arcs, atcs));
  return sr;
}

Feature decode_feature(const Iso8211Record& rec,
                       const std::unordered_map<std::uint16_t, std::string>& ftcs,
                       const std::unordered_map<std::uint16_t, std::string>& atcs,
                       const std::unordered_map<std::uint16_t, std::string>& facs,
                       const std::unordered_map<std::uint16_t, std::string>& arcs,
                       const std::unordered_map<std::uint16_t, std::string>& iacs,
                       std::uint64_t id) {
  Feature feat;
  feat.id = id;
  auto* frid = rec.find("FRID");
  if (!frid || frid->data.size() < 7) return feat;
  feat.record_id = read_u32(frid->data.data() + 1);
  const auto nftc = read_u16(frid->data.data() + 5);
  auto it = ftcs.find(nftc);
  feat.feature_type = it != ftcs.end() ? it->second : ("Feature" + std::to_string(nftc));
  if (auto* attr = rec.find("ATTR")) feat.attributes = parse_attributes(attr->data, atcs);
  if (auto* spas = rec.find("SPAS")) feat.spatial = parse_spas(spas->data);
  for (const auto& f : rec.fields) {
    if (f.tag == "FASC")
      feat.feature_associations.push_back(parse_association(f.data, facs, arcs, atcs));
    else if (f.tag == "INAS")
      feat.information_associations.push_back(parse_association(f.data, iacs, arcs, atcs));
  }
  return feat;
}

std::uint8_t ruin_from_id(const std::vector<std::uint8_t>& b, bool has_type_code) {
  if (has_type_code) {
    if (b.size() >= 10) return b[9];
  } else if (b.size() >= 8) {
    return b[7];
  }
  return 1;
}

}  // namespace

bool apply_association_updates(std::vector<AssociationRef>& existing,
                               const std::vector<AssociationRef>& patch, LoadError* err) {
  auto same = [](const AssociationRef& a, const AssociationRef& b) {
    return a.rcnm == b.rcnm && a.rcid == b.rcid && a.association == b.association && a.role == b.role;
  };
  bool any_control = false;
  for (const auto& p : patch)
    if (p.instruction != 0) {
      any_control = true;
      break;
    }
  if (!any_control) {
    existing = patch;
    for (auto& a : existing) a.instruction = 0;
    return true;
  }
  std::vector<AssociationRef> work = existing;
  for (const auto& p : patch) {
    const std::uint8_t insn = p.instruction;
    if (insn == 1) {
      AssociationRef copy = p;
      copy.instruction = 0;
      work.push_back(std::move(copy));
    } else if (insn == 2) {
      work.erase(std::remove_if(work.begin(), work.end(),
                                [&](const AssociationRef& a) { return same(a, p); }),
                 work.end());
    } else if (insn == 3) {
      bool matched = false;
      for (auto& a : work) {
        if (!same(a, p)) continue;
        a.attributes = p.attributes;
        matched = true;
      }
      if (!matched) {
        if (err) {
          err->kind = LoadErrorKind::Invalid;
          err->message = "FAUI/IUIN Modify: no association member matching (rcnm,rcid,association,role)";
        }
        return false;
      }
    } else {
      if (err) {
        err->kind = LoadErrorKind::Invalid;
        err->message = "unknown FAUI/IUIN value " + std::to_string(static_cast<unsigned>(insn));
      }
      return false;
    }
  }
  existing = std::move(work);
  return true;
}

const char* update_reject_code_name(UpdateRejectCode code) {
  switch (code) {
    case UpdateRejectCode::None: return "None";
    case UpdateRejectCode::EditionMismatch: return "EditionMismatch";
    case UpdateRejectCode::OutOfSequence: return "OutOfSequence";
    case UpdateRejectCode::NotUpdateFile: return "NotUpdateFile";
    case UpdateRejectCode::Truncated: return "Truncated";
    case UpdateRejectCode::BadStructure: return "BadStructure";
    case UpdateRejectCode::BadInstruction: return "BadInstruction";
    case UpdateRejectCode::MissingTarget: return "MissingTarget";
    case UpdateRejectCode::CatalogueMismatch: return "CatalogueMismatch";
    case UpdateRejectCode::DatasetCancelled: return "DatasetCancelled";
    case UpdateRejectCode::NewerEditionPending: return "NewerEditionPending";
    case UpdateRejectCode::DatasetNameMismatch: return "DatasetNameMismatch";
  }
  return "Unknown";
}

namespace {

// ---- ISO 8211 structural pre-scan (contract Behaviour #3: Truncated, then BadStructure) ----

struct Leader8211 {
  std::size_t rec_len{0};
  std::size_t base_addr{0};
  char id{'?'};
  int len_sz{0}, pos_sz{0}, tag_sz{0};
};

bool all_digits(const std::uint8_t* p, std::size_t n) {
  for (std::size_t i = 0; i < n; ++i)
    if (p[i] < '0' || p[i] > '9') return false;
  return true;
}

std::size_t digits_value(const std::uint8_t* p, std::size_t n) {
  std::size_t v = 0;
  for (std::size_t i = 0; i < n; ++i) v = v * 10 + static_cast<std::size_t>(p[i] - '0');
  return v;
}

/** Same acceptance rules as the loader's leader parser (iso8211.cpp), plus sane entry sizes. */
bool parse_leader_8211(const std::uint8_t* p, std::size_t n, Leader8211& L) {
  if (n < 24) return false;
  if (!all_digits(p, 5) || !all_digits(p + 12, 5)) return false;
  if (!all_digits(p + 20, 2) || !all_digits(p + 23, 1)) return false;
  L.rec_len = digits_value(p, 5);
  L.base_addr = digits_value(p + 12, 5);
  L.id = static_cast<char>(p[6]);
  L.len_sz = p[20] - '0';
  L.pos_sz = p[21] - '0';
  L.tag_sz = p[23] - '0';
  return L.rec_len >= 24 && L.len_sz > 0 && L.pos_sz > 0 && L.tag_sz > 0;
}

enum class Scan { Ok, Truncated, BadStructure };

/** Pass 1: every leader we can read must fit in the file (file size vs leader). */
Scan scan_truncation(const std::vector<std::uint8_t>& b, std::string& why) {
  if (b.size() < 24) {
    why = "file is " + std::to_string(b.size()) + " bytes, shorter than one ISO 8211 leader (24)";
    return Scan::Truncated;
  }
  std::size_t off = 0;
  while (off < b.size()) {
    const std::size_t rem = b.size() - off;
    if (rem < 24) {
      // A partial leader whose record-length digits are intact is a cut-off record.
      if (all_digits(b.data() + off, std::min<std::size_t>(rem, 5))) {
        why = "partial leader at offset " + std::to_string(off) + " (" + std::to_string(rem) +
              " bytes left)";
        return Scan::Truncated;
      }
      return Scan::Ok;  // garbage tail: structural pass decides
    }
    Leader8211 L;
    if (!parse_leader_8211(b.data() + off, rem, L)) return Scan::Ok;  // unreadable: pass 2
    if (L.rec_len > rem) {
      why = "record at offset " + std::to_string(off) + " claims " + std::to_string(L.rec_len) +
            " bytes, only " + std::to_string(rem) + " on disk";
      return Scan::Truncated;
    }
    off += L.rec_len;
  }
  return Scan::Ok;
}

/** Pass 2: leader / directory / field layout. */
Scan scan_structure(const std::vector<std::uint8_t>& b, std::string& why) {
  std::size_t off = 0;
  std::size_t index = 0;
  while (off < b.size()) {
    const std::size_t rem = b.size() - off;
    Leader8211 L;
    if (!parse_leader_8211(b.data() + off, rem, L)) {
      why = "bad ISO 8211 leader at offset " + std::to_string(off);
      return Scan::BadStructure;
    }
    if (L.rec_len > rem) {
      why = "record length past end of file at offset " + std::to_string(off);
      return Scan::Truncated;
    }
    const char want = index == 0 ? 'L' : 'D';
    if (L.id != want && !(index > 0 && L.id == 'R')) {
      why = std::string("unexpected leader identifier '") + L.id + "' at offset " +
            std::to_string(off);
      return Scan::BadStructure;
    }
    if (L.base_addr < 25 || L.base_addr > L.rec_len) {
      why = "base address out of range at offset " + std::to_string(off);
      return Scan::BadStructure;
    }
    const std::uint8_t* rec = b.data() + off;
    const std::size_t dir_len = L.base_addr - 24;
    if (rec[L.base_addr - 1] != 0x1e) {
      why = "directory not terminated at offset " + std::to_string(off);
      return Scan::BadStructure;
    }
    const std::size_t esz = static_cast<std::size_t>(L.tag_sz + L.len_sz + L.pos_sz);
    if ((dir_len - 1) % esz != 0 || dir_len - 1 == 0) {
      why = "directory size not a multiple of entry size at offset " + std::to_string(off);
      return Scan::BadStructure;
    }
    const std::size_t field_area = L.rec_len - L.base_addr;
    for (std::size_t i = 0; i + esz <= dir_len - 1; i += esz) {
      const std::uint8_t* e = rec + 24 + i;
      const std::uint8_t* len_p = e + L.tag_sz;
      const std::uint8_t* pos_p = len_p + L.len_sz;
      if (!all_digits(len_p, static_cast<std::size_t>(L.len_sz)) ||
          !all_digits(pos_p, static_cast<std::size_t>(L.pos_sz))) {
        why = "non-numeric directory entry at offset " + std::to_string(off);
        return Scan::BadStructure;
      }
      const std::size_t flen = digits_value(len_p, static_cast<std::size_t>(L.len_sz));
      const std::size_t fpos = digits_value(pos_p, static_cast<std::size_t>(L.pos_sz));
      if (flen == 0 || fpos + flen > field_area) {
        why = "directory entry past record end at offset " + std::to_string(off);
        return Scan::BadStructure;
      }
      if (rec[L.base_addr + fpos + flen - 1] != 0x1e) {
        why = "field not terminated at offset " + std::to_string(off);
        return Scan::BadStructure;
      }
    }
    off += L.rec_len;
    ++index;
  }
  return Scan::Ok;
}

std::string ed_upd(int ed, int upd) { return std::to_string(ed) + "." + std::to_string(upd); }

bool instruction_ok(std::uint8_t v) { return v <= 3; }

/** Context for one apply: produces the single code + message on rejection. */
struct Reject {
  const std::filesystem::path& path;
  LoadError* err;
  UpdateRejectCode* code;
  std::string* message;
  std::string file_ed_upd{"?.? (DSID unreadable)"};
  bool operator()(UpdateRejectCode c, LoadErrorKind kind, const std::string& detail) const {
    const std::string msg = "update rejected: " + path.string() + " ED.UPD " + file_ed_upd +
                            " code=" + update_reject_code_name(c) + ": " + detail;
    if (err) {
      err->kind = kind;
      err->message = msg;
    }
    if (code) *code = c;
    if (message) *message = msg;
    return false;
  }
};

/** Validate ATIN / FAUI / IUIN values on one data record. Returns false + detail on the first
 *  unknown value. */
bool check_record_instructions(const Iso8211Record& rec,
                               const std::unordered_map<std::uint16_t, std::string>& atcs,
                               std::string& detail) {
  for (const auto& f : rec.fields) {
    if (f.tag == "ATTR") {
      for (const auto& a : parse_attributes(f.data, atcs))
        if (!instruction_ok(a.instruction)) {
          detail = "unknown ATIN " + std::to_string(static_cast<unsigned>(a.instruction)) +
                   " on attribute " + a.code;
          return false;
        }
    } else if (f.tag == "FASC" || f.tag == "INAS") {
      if (f.data.size() < 10) continue;
      if (!instruction_ok(f.data[9])) {
        detail = std::string("unknown ") + (f.tag == "FASC" ? "FAUI " : "IUIN ") +
                 std::to_string(static_cast<unsigned>(f.data[9]));
        return false;
      }
      std::vector<std::uint8_t> tail(f.data.begin() + 10, f.data.end());
      for (const auto& a : parse_attributes(tail, atcs))
        if (!instruction_ok(a.instruction)) {
          detail = "unknown ATIN " + std::to_string(static_cast<unsigned>(a.instruction)) +
                   " on association attribute " + a.code;
          return false;
        }
    }
  }
  return true;
}

bool ruin_ok(std::uint8_t r) { return r >= 1 && r <= 3; }

/** Applies one update to `ds` in place, in contract check order. Caller guarantees `ds` is a
 *  working copy (strong guarantee lives in apply_one). */
/** Per-call options / side results of one apply (docs/cancel-reissue-contract.md). */
struct ApplyOptions {
  int reissue_upd{0};  // base is a re-issue at UPD n > 0: same-ED updates with UPD <= n skip (D2)
};
struct ApplyOutcome {
  bool skipped{false};    // already contained in a re-issue base; dataset unchanged
  bool cancelled{false};  // a cancellation was applied
  std::string notice;
};

/** DSNM stem: file identifier without its extension ("101AA00DS0003.001" -> "101AA00DS0003"). */
std::string dsnm_stem(const std::string& dsnm) {
  const auto slash = dsnm.find_last_of("/\\");
  std::string name = slash == std::string::npos ? dsnm : dsnm.substr(slash + 1);
  const auto dot = name.find_last_of('.');
  return dot == std::string::npos ? name : name.substr(0, dot);
}

std::string dataset_stem(const Dataset& ds) {
  if (!ds.dataset_name.empty()) return dsnm_stem(ds.dataset_name);
  return ds.path.stem().string();
}

bool apply_in_place(Dataset& ds, const std::filesystem::path& update_path,
                    const ExchangeDatasetEntry* entry, const Reject& reject_in,
                    const ApplyOptions& opts, ApplyOutcome& outcome) {
  Reject reject = reject_in;
  std::vector<std::uint8_t> bytes;
  {
    std::ifstream in(update_path, std::ios::binary);
    if (!in)
      return reject(UpdateRejectCode::Truncated, LoadErrorKind::Io,
                    "cannot open update (0 bytes readable)");
    bytes.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
  }

  // 1. file size vs leader -> Truncated; 2. ISO 8211 parse -> BadStructure.
  std::string why;
  const Scan t = scan_truncation(bytes, why);
  const Scan st = t == Scan::Ok ? scan_structure(bytes, why) : t;
  if (st != Scan::Ok) {
    // Oracle 6 (rev 1.3): DSID is not read from a file that fails the scan -> "?.?".
    return reject(st == Scan::Truncated ? UpdateRejectCode::Truncated : UpdateRejectCode::BadStructure,
                  LoadErrorKind::Parse, why);
  }
  std::vector<Iso8211Record> records;
  {
    LoadError perr{LoadErrorKind::Parse, {}};
    if (!parse_iso8211_records(bytes, records, &perr))
      return reject(UpdateRejectCode::BadStructure, LoadErrorKind::Parse, perr.message);
  }

  // DSID (missing -> NotUpdateFile: nothing names a target; unparseable DSED -> BadStructure).
  const Iso8211Field* dsid = nullptr;
  for (const auto& rec : records) {
    if (rec.leader_id != 'D') continue;
    if ((dsid = rec.find("DSID"))) break;
  }
  if (!dsid)
    return reject(UpdateRejectCode::NotUpdateFile, LoadErrorKind::Invalid,
                  "no DSID record: not an S-101 dataset");
  const DatasetIdentification upd_id = parse_s101_dsid(dsid->data);
  if (!upd_id.ok)
    return reject(UpdateRejectCode::BadStructure, LoadErrorKind::Invalid,
                  "DSID DSED missing or unparseable");
  reject.file_ed_upd = ed_upd(upd_id.edition_number, upd_id.update_number);

  // Check order: docs/cancel-reissue-contract.md rev 2.1 Behaviour #1.
  // 3. DSNM stem must name this dataset (D5), cancel included.
  const std::string target = dataset_stem(ds);
  const std::string named = dsnm_stem(upd_id.dataset_name);
  if (!target.empty() && named != target)
    return reject(UpdateRejectCode::DatasetNameMismatch, LoadErrorKind::Invalid,
                  "update DSNM '" + upd_id.dataset_name + "' does not name dataset '" + target +
                      "'");
  // 4. target already cancelled (D1).
  if (ds.cancelled)
    return reject(UpdateRejectCode::DatasetCancelled, LoadErrorKind::Invalid,
                  "dataset " + target + " is cancelled; no further updates apply");
  // 5. cancellation (DSED ED 0): DSID-only, PROF "2"; not sequence-checked (D4).
  if (upd_id.edition_number == 0) {
    std::size_t data_records = 0;
    bool only_dsid = true;
    for (const auto& rec : records) {
      if (rec.leader_id == 'L') continue;
      ++data_records;
      for (const auto& f : rec.fields)
        if (f.tag != "DSID" && f.tag != "0001") only_dsid = false;
    }
    if (upd_id.profile != "2")
      return reject(UpdateRejectCode::BadStructure, LoadErrorKind::Invalid,
                    "cancellation DSID PROF '" + upd_id.profile + "' is not \"2\"");
    if (data_records != 1 || !only_dsid)
      return reject(UpdateRejectCode::BadStructure, LoadErrorKind::Invalid,
                    "cancellation file must contain only the DSID record");
    ds.features.clear();
    ds.information.clear();
    ds.spatial.records.clear();
    ds.layers.clear();
    ds.cancelled = true;
    outcome.cancelled = true;
    outcome.notice = "cancelled: " + (ds.dataset_name.empty() ? target : ds.dataset_name);
    return true;  // ED/UPD unchanged
  }
  // 6. base / non-S-101 -> NotUpdateFile.
  if (upd_id.product_specification.find("S-101") == std::string::npos)
    return reject(UpdateRejectCode::NotUpdateFile, LoadErrorKind::Invalid,
                  "DSID PRSP '" + upd_id.product_specification + "' is not S-101");
  if (upd_id.update_number == 0)
    return reject(UpdateRejectCode::NotUpdateFile, LoadErrorKind::Invalid,
                  "DSID UPD 0: base cell, not an update");

  // 7. catalogue vs DSID -> CatalogueMismatch.
  if (entry && ((entry->has_edition_meta && entry->edition_number != upd_id.edition_number) ||
                (entry->has_update_meta && entry->update_number != upd_id.update_number)))
    return reject(UpdateRejectCode::CatalogueMismatch, LoadErrorKind::Invalid,
                  "catalogue ED/UPD " +
                      (entry->has_edition_meta ? std::to_string(entry->edition_number) : "?") + "." +
                      (entry->has_update_meta ? std::to_string(entry->update_number) : "?") +
                      " does not match DSID " + reject.file_ed_upd);

  // 8. ED > installed -> NewerEditionPending (D3); 9. ED < installed -> EditionMismatch.
  if (upd_id.edition_number > ds.edition_number) {
    outcome.notice = "newer edition " + std::to_string(upd_id.edition_number) + " pending";
    return reject(UpdateRejectCode::NewerEditionPending, LoadErrorKind::Invalid,
                  "newer edition " + std::to_string(upd_id.edition_number) +
                      " exists, dataset not up to date (installed " +
                      ed_upd(ds.edition_number, ds.update_number) + ")");
  }
  if (upd_id.edition_number != ds.edition_number)
    return reject(UpdateRejectCode::EditionMismatch, LoadErrorKind::Invalid,
                  "update DSID ED/UPD " + reject.file_ed_upd + " edition differs from dataset " +
                      ed_upd(ds.edition_number, ds.update_number));
  // 10. UPD -> OutOfSequence, except updates already contained in a re-issue base (D2).
  if (opts.reissue_upd > 0 && upd_id.update_number <= opts.reissue_upd) {
    outcome.skipped = true;
    return true;  // dataset unchanged
  }
  if (upd_id.update_number != ds.update_number + 1)
    return reject(UpdateRejectCode::OutOfSequence, LoadErrorKind::Invalid,
                  "update DSID ED/UPD " + reject.file_ed_upd + " mismatch: expected " +
                      ed_upd(ds.edition_number, ds.update_number + 1) + " (dataset at " +
                      ed_upd(ds.edition_number, ds.update_number) + ")");

  // 11. records: BadInstruction / MissingTarget, first failure in record order wins.
  Dssi dssi;
  std::unordered_map<std::uint16_t, std::string> ftcs, atcs, itcs, iacs, facs, arcs;
  for (const auto& rec : records) {
    if (rec.leader_id != 'D') continue;
    if (!dssi.ok)
      if (auto* f = rec.find("DSSI")) dssi = parse_dssi(f->data);
    if (ftcs.empty())
      if (auto* f = rec.find("FTCS")) ftcs = parse_ftcs(f->data);
    if (atcs.empty())
      if (auto* f = rec.find("ATCS")) atcs = parse_ftcs(f->data);
    if (itcs.empty())
      if (auto* f = rec.find("ITCS")) itcs = parse_ftcs(f->data);
    if (iacs.empty())
      if (auto* f = rec.find("IACS")) iacs = parse_ftcs(f->data);
    if (facs.empty())
      if (auto* f = rec.find("FACS")) facs = parse_ftcs(f->data);
    if (arcs.empty())
      if (auto* f = rec.find("ARCS")) arcs = parse_ftcs(f->data);
  }

  std::uint64_t next_id = 1;
  for (const auto& f : ds.features) next_id = std::max(next_id, f.id + 1);

  auto bad = [&](const std::string& d) {
    return reject(UpdateRejectCode::BadInstruction, LoadErrorKind::Invalid, d);
  };
  auto missing = [&](const std::string& d) {
    return reject(UpdateRejectCode::MissingTarget, LoadErrorKind::Invalid, d);
  };
  auto rcid_text = [](const char* what, unsigned rcnm, std::uint32_t rcid) {
    return std::string(what) + " (" + std::to_string(rcnm) + "," + std::to_string(rcid) + ")";
  };

  for (const auto& rec : records) {
    if (rec.leader_id != 'D') continue;
    std::string detail;
    if (!check_record_instructions(rec, atcs, detail)) return bad(detail);

    for (const char* tag : {"PRID", "MRID", "CRID", "CCID", "SRID"}) {
      auto* id = rec.find(tag);
      if (!id || id->data.size() < 8 || id->data[0] < 100) continue;
      const auto ruin = ruin_from_id(id->data, false);
      const auto rcnm = id->data[0];
      const auto rcid = read_u32(id->data.data() + 1);
      if (!ruin_ok(ruin))
        return bad("unknown RUIN " + std::to_string(static_cast<unsigned>(ruin)) + " on " +
                   rcid_text(tag, rcnm, rcid));
      const auto key = SpatialStore::key(rcnm, rcid);
      const bool exists = ds.spatial.records.count(key) != 0;
      if ((ruin == 2 || ruin == 3) && !exists)
        return missing(std::string(ruin == 2 ? "Delete" : "Modify") + " of non-existent spatial " +
                       rcid_text(tag, rcnm, rcid));
      if (ruin == 2)
        ds.spatial.records.erase(key);
      else
        ds.spatial.records[key] = decode_spatial(rec, tag, dssi, iacs, arcs, atcs);
    }

    if (auto* irid = rec.find("IRID"); irid && irid->data.size() >= 10 && irid->data[0] >= 100) {
      const auto ruin = ruin_from_id(irid->data, true);
      const auto rcid = read_u32(irid->data.data() + 1);
      if (!ruin_ok(ruin))
        return bad("unknown RUIN " + std::to_string(static_cast<unsigned>(ruin)) + " on " +
                   rcid_text("IRID", irid->data[0], rcid));
      auto target = std::find_if(ds.information.begin(), ds.information.end(),
                                 [&](const InformationRecord& ir) { return ir.record_id == rcid; });
      if ((ruin == 2 || ruin == 3) && target == ds.information.end())
        return missing(std::string(ruin == 2 ? "Delete" : "Modify") + " of non-existent " +
                       rcid_text("IRID", irid->data[0], rcid));
      if (ruin == 2) {
        ds.information.erase(std::remove_if(ds.information.begin(), ds.information.end(),
                                            [&](const InformationRecord& ir) {
                                              return ir.record_id == rcid;
                                            }),
                             ds.information.end());
      } else {
        InformationRecord ir;
        ir.record_id = rcid;
        const auto nitc = read_u16(irid->data.data() + 5);
        auto it = itcs.find(nitc);
        ir.information_type = it != itcs.end() ? it->second : ("info" + std::to_string(nitc));
        if (auto* attr = rec.find("ATTR")) ir.attributes = parse_attributes(attr->data, atcs);
        for (const auto& f : rec.fields)
          if (f.tag == "INAS")
            ir.information_associations.push_back(parse_association(f.data, iacs, arcs, atcs));
        if (ruin == 1) {
          ds.information.push_back(std::move(ir));
        } else {
          auto& existing = *target;
          if (!ir.attributes.empty()) apply_atin(existing.attributes, ir.attributes);
          if (rec.find("INAS")) {
            LoadError aerr{LoadErrorKind::Invalid, {}};
            if (!apply_association_updates(existing.information_associations,
                                           ir.information_associations, &aerr))
              return missing(aerr.message + " on " + rcid_text("IRID", irid->data[0], rcid));
          }
          if (!ir.information_type.empty()) existing.information_type = ir.information_type;
        }
      }
    }

    if (auto* frid = rec.find("FRID"); frid && frid->data.size() >= 10 && frid->data[0] == 100) {
      const auto ruin = ruin_from_id(frid->data, true);
      const auto rcid = read_u32(frid->data.data() + 1);
      if (!ruin_ok(ruin))
        return bad("unknown RUIN " + std::to_string(static_cast<unsigned>(ruin)) + " on " +
                   rcid_text("FRID", 100, rcid));
      Feature* target = nullptr;
      for (auto& f : ds.features)
        if (f.record_id == rcid) {
          target = &f;
          break;
        }
      if ((ruin == 2 || ruin == 3) && !target)
        return missing(std::string(ruin == 2 ? "Delete" : "Modify") + " of non-existent " +
                       rcid_text("FRID", 100, rcid));
      if (ruin == 2) {
        ds.features.erase(std::remove_if(ds.features.begin(), ds.features.end(),
                                         [&](const Feature& f) { return f.record_id == rcid; }),
                          ds.features.end());
      } else if (ruin == 1) {
        Feature feat = decode_feature(rec, ftcs, atcs, facs, arcs, iacs, next_id++);
        for (auto& a : feat.attributes) a.instruction = 1;
        ds.features.push_back(std::move(feat));
      } else {
        Feature patch = decode_feature(rec, ftcs, atcs, facs, arcs, iacs, 0);
        if (!patch.feature_type.empty() && patch.feature_type.rfind("Feature", 0) != 0)
          target->feature_type = patch.feature_type;
        if (rec.find("ATTR")) apply_atin(target->attributes, patch.attributes);
        if (auto* spas = rec.find("SPAS")) apply_spas_sau(target->spatial, spas->data);
        LoadError aerr{LoadErrorKind::Invalid, {}};
        if (rec.find("FASC") && !apply_association_updates(target->feature_associations,
                                                           patch.feature_associations, &aerr))
          return missing(aerr.message + " on " + rcid_text("FRID", 100, rcid));
        if (rec.find("INAS") && !apply_association_updates(target->information_associations,
                                                           patch.information_associations, &aerr))
          return missing(aerr.message + " on " + rcid_text("FRID", 100, rcid));
      }
    }
  }

  rebuild_feature_geometries(ds);
  ds.edition_number = upd_id.edition_number;
  ds.update_number = upd_id.update_number;
  // D5: an update never overwrites ds.dataset_name (it names its target, not a new identity).
  return true;
}

/** Strong guarantee (contract Behaviour #1): apply on a copy, swap only on success. */
bool apply_one(Dataset& ds, const std::filesystem::path& update_path,
               const ExchangeDatasetEntry* entry, LoadError* err, UpdateRejectCode* code,
               std::string* message, const ApplyOptions& opts = {},
               ApplyOutcome* outcome_out = nullptr) {
  if (code) *code = UpdateRejectCode::None;
  Dataset work = ds;
  const Reject reject{update_path, err, code, message};
  ApplyOutcome outcome;
  bool ok = false;
  try {
    ok = apply_in_place(work, update_path, entry, reject, opts, outcome);
  } catch (const std::exception& e) {
    // Contract rule 9: nothing escapes; a decode that throws is a structural defect.
    ok = reject(UpdateRejectCode::BadStructure, LoadErrorKind::Parse,
                std::string("exception while decoding: ") + e.what());
  }
  if (outcome_out) *outcome_out = outcome;
  if (!ok) return false;
  if (outcome.skipped) return true;  // nothing changed
  using std::swap;
  swap(ds, work);
  return true;
}

/** Code for a base cell that load_s101 refused (not an update, but the open call must still
 *  name one code): file-level scan result, else BadStructure. */
UpdateRejectCode classify_base_failure(const std::filesystem::path& base, std::string& why) {
  std::ifstream in(base, std::ios::binary);
  if (!in) {
    why = "cannot open base cell";
    return UpdateRejectCode::Truncated;
  }
  std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)),
                                  std::istreambuf_iterator<char>());
  const Scan t = scan_truncation(bytes, why);
  if (t == Scan::Truncated) return UpdateRejectCode::Truncated;
  const Scan s = scan_structure(bytes, why);
  if (s == Scan::Truncated) return UpdateRejectCode::Truncated;
  if (why.empty()) why = "base cell did not load";
  return UpdateRejectCode::BadStructure;
}

}  // namespace

bool apply_s101_update(Dataset& ds, const std::filesystem::path& update_path, LoadError* err,
                       UpdateRejectCode* code) {
  return apply_one(ds, update_path, nullptr, err, code, nullptr);
}

namespace detail {

bool open_s101_with_updates(const std::filesystem::path& base,
                            const std::vector<std::filesystem::path>& updates,
                            const std::vector<const ExchangeDatasetEntry*>* entries, Dataset& out,
                            LoadError* err, UpdateReport* report) {
  if (report) *report = UpdateReport{};
  Dataset tmp;
  LoadError base_err{LoadErrorKind::Invalid, {}};
  if (!load_s101(base, tmp, &base_err)) {
    std::string why;
    const auto c = classify_base_failure(base, why);
    if (err) *err = base_err;
    if (report) {
      report->rejected = base;
      report->code = c;
      report->message = "base rejected: " + base.string() + " code=" + update_reject_code_name(c) +
                        ": " + base_err.message;
    }
    return false;
  }
  tmp.path = base;
  tmp.product = ProductId::S101;

  // Contract Behaviour #2/#3a: list order, sequenced by DSID; stop at the first rejection and
  // keep base + the applied prefix in `out` (last good state, usable for display).
  // Re-issue (cancel-reissue Behaviour #4): a base at UPD n > 0 already contains 1..n.
  ApplyOptions opts;
  opts.reissue_upd = tmp.update_number;
  int applied = 0, skipped = 0;
  bool cancelled = false;
  std::string notice;
  for (std::size_t i = 0; i < updates.size(); ++i) {
    const ExchangeDatasetEntry* entry =
        entries && i < entries->size() ? (*entries)[i] : nullptr;
    UpdateRejectCode code = UpdateRejectCode::None;
    std::string message;
    ApplyOutcome outcome;
    if (!apply_one(tmp, updates[i], entry, err, &code, &message, opts, &outcome)) {
      if (report) {
        report->applied = applied;
        report->skipped = skipped;
        report->cancelled = cancelled;
        report->rejected = updates[i];
        report->code = code;
        report->message = message;
        report->notice = !outcome.notice.empty() ? outcome.notice : notice;
      }
      out = std::move(tmp);
      return false;
    }
    if (outcome.skipped) {
      ++skipped;
      continue;
    }
    if (outcome.cancelled) {
      cancelled = true;
      notice = outcome.notice;
    }
    ++applied;
  }
  if (report) {
    report->applied = applied;
    report->skipped = skipped;
    report->cancelled = cancelled;
    report->notice = notice;
  }
  out = std::move(tmp);
  return true;
}

}  // namespace detail

bool open_dataset_with_updates(const std::filesystem::path& base,
                               const std::vector<std::filesystem::path>& updates, Dataset& out,
                               LoadError* err, UpdateReport* report) {
  return detail::open_s101_with_updates(base, updates, nullptr, out, err, report);
}

}  // namespace s100
