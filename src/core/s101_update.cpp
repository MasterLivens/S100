#include "s100/exchange_set.hpp"
#include "s100/iso8211.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
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

bool apply_s101_update(Dataset& ds, const std::filesystem::path& update_path, LoadError* err) {
  std::ifstream in(update_path, std::ios::binary);
  if (!in) {
    if (err) {
      err->kind = LoadErrorKind::Io;
      err->message = "cannot open update " + update_path.string();
    }
    return false;
  }
  std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(in)),
                                  std::istreambuf_iterator<char>());
  if (bytes.empty()) {
    if (err) {
      err->kind = LoadErrorKind::Invalid;
      err->message = "empty update: " + update_path.string();
    }
    return false;
  }

  std::vector<Iso8211Record> records;
  if (!parse_iso8211_records(bytes, records, err)) return false;

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

  auto apply_spatial = [&](const Iso8211Record& rec, const char* tag) {
    auto* id = rec.find(tag);
    if (!id || id->data.size() < 8 || id->data[0] < 100) return;
    const auto ruin = ruin_from_id(id->data, false);
    const auto rcnm = id->data[0];
    const auto rcid = read_u32(id->data.data() + 1);
    const auto key = SpatialStore::key(rcnm, rcid);
    if (ruin == 2) {
      ds.spatial.records.erase(key);
      return;
    }
    if (ruin == 1 || ruin == 3)
      ds.spatial.records[key] = decode_spatial(rec, tag, dssi, iacs, arcs, atcs);
  };

  for (const auto& rec : records) {
    if (rec.leader_id != 'D') continue;
    if (rec.find("PRID")) apply_spatial(rec, "PRID");
    if (rec.find("MRID")) apply_spatial(rec, "MRID");
    if (rec.find("CRID")) apply_spatial(rec, "CRID");
    if (rec.find("CCID")) apply_spatial(rec, "CCID");
    if (rec.find("SRID")) apply_spatial(rec, "SRID");

    if (auto* irid = rec.find("IRID"); irid && irid->data.size() >= 10 && irid->data[0] >= 100) {
      const auto ruin = ruin_from_id(irid->data, true);
      const auto rcid = read_u32(irid->data.data() + 1);
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
        } else if (ruin == 3) {
          bool found = false;
          for (auto& existing : ds.information) {
            if (existing.record_id != rcid) continue;
            if (!ir.attributes.empty()) apply_atin(existing.attributes, ir.attributes);
            if (!ir.information_associations.empty())
              existing.information_associations = std::move(ir.information_associations);
            if (!ir.information_type.empty()) existing.information_type = ir.information_type;
            found = true;
            break;
          }
          if (!found) ds.information.push_back(std::move(ir));
        }
      }
    }

    if (auto* frid = rec.find("FRID"); frid && frid->data.size() >= 10 && frid->data[0] == 100) {
      const auto ruin = ruin_from_id(frid->data, true);
      const auto rcid = read_u32(frid->data.data() + 1);
      if (ruin == 2) {
        ds.features.erase(std::remove_if(ds.features.begin(), ds.features.end(),
                                         [&](const Feature& f) { return f.record_id == rcid; }),
                          ds.features.end());
      } else if (ruin == 1) {
        Feature feat = decode_feature(rec, ftcs, atcs, facs, arcs, iacs, next_id++);
        for (auto& a : feat.attributes) a.instruction = 1;
        ds.features.push_back(std::move(feat));
      } else if (ruin == 3) {
        Feature patch = decode_feature(rec, ftcs, atcs, facs, arcs, iacs, 0);
        Feature* target = nullptr;
        for (auto& f : ds.features)
          if (f.record_id == rcid) {
            target = &f;
            break;
          }
        if (!target) {
          patch.id = next_id++;
          ds.features.push_back(std::move(patch));
        } else {
          if (!patch.feature_type.empty() && patch.feature_type.rfind("Feature", 0) != 0)
            target->feature_type = patch.feature_type;
          if (rec.find("ATTR")) apply_atin(target->attributes, patch.attributes);
          if (auto* spas = rec.find("SPAS")) apply_spas_sau(target->spatial, spas->data);
          if (rec.find("FASC"))
            target->feature_associations = std::move(patch.feature_associations);
          if (rec.find("INAS"))
            target->information_associations = std::move(patch.information_associations);
        }
      }
    }
  }

  rebuild_feature_geometries(ds);
  return true;
}

bool open_dataset_with_updates(const std::filesystem::path& base,
                               const std::vector<std::filesystem::path>& updates, Dataset& out,
                               LoadError* err) {
  if (!load_s101(base, out, err)) return false;
  for (const auto& u : updates) {
    if (!apply_s101_update(out, u, err)) return false;
  }
  out.path = base;
  out.product = ProductId::S101;
  return !out.features.empty();
}

}  // namespace s100
