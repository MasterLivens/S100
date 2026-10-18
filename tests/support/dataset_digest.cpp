#include "dataset_digest.hpp"
#include "sha256.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <tuple>
#include <vector>

namespace s100test {
namespace {

// Coordinates are hashed by their IEEE-754 bit pattern: "bit-for-bit equal" (rule 1).
std::string bits(double v) {
  std::uint64_t u;
  std::memcpy(&u, &v, sizeof u);
  std::ostringstream os;
  os << std::hex << u;
  return os.str();
}

std::string pt(const s100::Point& p) { return "(" + bits(p.lon) + "," + bits(p.lat) + ")"; }
std::string pt3(const s100::Point3& p) {
  return "(" + bits(p.lon) + "," + bits(p.lat) + "," + bits(p.z) + ")";
}

std::string esc(const std::string& s) {
  std::string o;
  for (char c : s) {
    if (c == '\\' || c == '|' || c == '{' || c == '}' || c == ';') o.push_back('\\');
    o.push_back(c);
  }
  return o;
}

/** Attributes as a nested tree (PAIX = 1-based parent position); siblings sorted. */
std::string attr_subtree(const std::vector<s100::Attribute>& attrs, std::uint16_t parent_pos) {
  std::vector<std::string> kids;
  for (std::size_t i = 0; i < attrs.size(); ++i) {
    const auto& a = attrs[i];
    if (a.parent != parent_pos) continue;
    const auto pos = static_cast<std::uint16_t>(i + 1);
    std::string k = esc(a.code) + "|" + std::to_string(a.index) + "|" + esc(a.value) + "|" +
                    std::to_string(a.instruction);
    if (pos != 0 && pos != parent_pos) k += "{" + attr_subtree(attrs, pos) + "}";
    kids.push_back(std::move(k));
  }
  std::sort(kids.begin(), kids.end());
  std::string o;
  for (const auto& k : kids) o += k + ";";
  return o;
}

std::string attrs_str(const std::vector<s100::Attribute>& attrs) {
  // Orphans (parent past the list) would be dropped by the tree walk; count them explicitly.
  std::size_t orphans = 0;
  for (const auto& a : attrs)
    if (a.parent > attrs.size()) ++orphans;
  return "n=" + std::to_string(attrs.size()) + ",orphans=" + std::to_string(orphans) + "[" +
         attr_subtree(attrs, 0) + "]";
}

std::string assocs_str(const std::vector<s100::AssociationRef>& v) {
  std::vector<std::string> items;
  for (const auto& a : v)
    items.push_back(std::to_string(a.rcnm) + "/" + std::to_string(a.rcid) + "|" +
                    esc(a.association) + "|" + esc(a.role) + "|" + std::to_string(a.instruction) +
                    "|" + attrs_str(a.attributes));
  std::sort(items.begin(), items.end());
  std::string o = "n=" + std::to_string(v.size()) + "[";
  for (const auto& s : items) o += s + ";";
  return o + "]";
}

std::string refs_str(const std::vector<s100::SpatialRef>& v) {
  std::string o;  // record order is meaningful (rings / curve chains): not sorted
  for (const auto& r : v)
    o += std::to_string(r.rcnm) + "/" + std::to_string(r.rcid) + (r.reverse ? "r" : "f") +
         std::to_string(r.usage) + ":" + std::to_string(r.scale_min) + ":" +
         std::to_string(r.scale_max) + ";";
  return o;
}

std::string geom_str(const s100::Geometry& g) {
  std::string o = "kind=" + std::to_string(static_cast<int>(g.kind)) + " pt=" + pt(g.point) + " line=";
  for (const auto& p : g.line) o += pt(p);
  o += " poly=";
  for (const auto& r : g.polygon) {
    o += "[";
    for (const auto& p : r.points) o += pt(p);
    o += "]";
  }
  o += " mp=";
  for (const auto& p : g.multipoint) o += pt3(p);
  return o;
}

}  // namespace

std::string dataset_canonical(const s100::Dataset& ds) {
  std::ostringstream os;
  os << "ED=" << ds.edition_number << " UPD=" << ds.update_number << "\n";
  os << "features=" << ds.features.size() << "\n";
  std::vector<const s100::Feature*> feats;
  for (const auto& f : ds.features) feats.push_back(&f);
  std::sort(feats.begin(), feats.end(), [](const s100::Feature* a, const s100::Feature* b) {
    return std::tie(a->id, a->record_id) < std::tie(b->id, b->record_id);
  });
  for (const auto* f : feats) {
    os << "F id=" << f->id << " rcid=" << f->record_id << " type=" << esc(f->feature_type)
       << "\n  attrs=" << attrs_str(f->attributes) << "\n  geom=" << geom_str(f->geometry)
       << "\n  spas=" << refs_str(f->spatial) << "\n  fasc=" << assocs_str(f->feature_associations)
       << "\n  inas=" << assocs_str(f->information_associations) << "\n";
  }
  std::vector<const s100::InformationRecord*> infos;
  for (const auto& ir : ds.information) infos.push_back(&ir);
  std::stable_sort(infos.begin(), infos.end(),
                   [](const s100::InformationRecord* a, const s100::InformationRecord* b) {
                     return a->record_id < b->record_id;
                   });
  os << "information=" << infos.size() << "\n";
  for (const auto* ir : infos)
    os << "I id=" << ir->record_id << " type=" << esc(ir->information_type)
       << " attrs=" << attrs_str(ir->attributes) << " inas=" << assocs_str(ir->information_associations)
       << "\n";
  std::vector<const s100::SpatialRecord*> sps;
  for (const auto& kv : ds.spatial.records) sps.push_back(&kv.second);
  std::sort(sps.begin(), sps.end(), [](const s100::SpatialRecord* a, const s100::SpatialRecord* b) {
    return std::tie(a->rcnm, a->rcid) < std::tie(b->rcnm, b->rcid);
  });
  os << "spatial=" << sps.size() << "\n";
  for (const auto* s : sps) {
    os << "S " << static_cast<int>(s->rcnm) << "/" << s->rcid << " coords=";
    for (const auto& p : s->coords) os << pt3(p);
    os << " sp=" << s->start_point << " ep=" << s->end_point << " parts=" << refs_str(s->parts)
       << " inas=" << assocs_str(s->information_associations) << "\n";
  }
  return os.str();
}

std::string dataset_digest(const s100::Dataset& ds) { return sha256_hex(dataset_canonical(ds)); }

}  // namespace s100test
