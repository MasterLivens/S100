#include "s100/nav_warning.hpp"
#include "s100/dataset.hpp"

#include <pugixml.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

namespace s100 {
namespace {

void set_err(LoadError* err, LoadErrorKind kind, std::string message) {
  if (!err) return;
  err->kind = kind;
  err->message = std::move(message);
}

const char* local(const pugi::xml_node& n) {
  const char* name = n.name();
  const char* c = std::strchr(name, ':');
  return c ? c + 1 : name;
}

pugi::xml_node child(const pugi::xml_node& n, const char* name) {
  for (auto c : n.children())
    if (std::strcmp(local(c), name) == 0) return c;
  return {};
}

std::vector<pugi::xml_node> children(const pugi::xml_node& n, const char* name) {
  std::vector<pugi::xml_node> v;
  for (auto c : n.children())
    if (std::strcmp(local(c), name) == 0) v.push_back(c);
  return v;
}

std::string text_of(const pugi::xml_node& n) {
  if (!n) return {};
  return n.text().as_string();
}

std::string child_text(const pugi::xml_node& n, const char* name) {
  return text_of(child(n, name));
}

bool is_all_whitespace(const std::string& s) {
  return std::all_of(s.begin(), s.end(),
                     [](unsigned char c) { return std::isspace(c) != 0; });
}

std::string trim_copy(std::string s) {
  auto not_space = [](unsigned char c) { return std::isspace(c) == 0; };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
  s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
  return s;
}

bool parse_doubles(const std::string& s, std::vector<double>& out) {
  out.clear();
  std::istringstream iss(s);
  double v = 0;
  while (iss >> v) out.push_back(v);
  return !out.empty() && iss.eof();
}

Bounds bounds_from_geometry(const Geometry& g) {
  Bounds b;
  bool any = false;
  auto consider = [&](double lon, double lat) {
    if (!any) {
      b.west = b.east = lon;
      b.south = b.north = lat;
      any = true;
      return;
    }
    b.west = std::min(b.west, lon);
    b.east = std::max(b.east, lon);
    b.south = std::min(b.south, lat);
    b.north = std::max(b.north, lat);
  };
  if (g.kind == GeometryKind::Point) {
    consider(g.point.lon, g.point.lat);
  } else if (g.kind == GeometryKind::LineString) {
    for (const auto& p : g.line) consider(p.lon, p.lat);
  } else if (g.kind == GeometryKind::Polygon) {
    for (const auto& ring : g.polygon)
      for (const auto& p : ring.points) consider(p.lon, p.lat);
  }
  b.valid = any;
  return b;
}

void union_bounds(Bounds& acc, const Bounds& o) {
  if (!o.valid) return;
  if (!acc.valid) {
    acc = o;
    return;
  }
  acc.west = std::min(acc.west, o.west);
  acc.east = std::max(acc.east, o.east);
  acc.south = std::min(acc.south, o.south);
  acc.north = std::max(acc.north, o.north);
}

bool parse_point(const pugi::xml_node& point_node, Geometry& g) {
  const auto pos = child(point_node, "pos");
  if (!pos) return false;
  std::vector<double> vals;
  if (!parse_doubles(text_of(pos), vals) || vals.size() < 2) return false;
  g.kind = GeometryKind::Point;
  g.point.lon = vals[0];
  g.point.lat = vals[1];
  return true;
}

bool parse_polygon(const pugi::xml_node& poly_node, Geometry& g) {
  auto exterior = child(poly_node, "exterior");
  if (!exterior) return false;
  auto ring_node = child(exterior, "LinearRing");
  if (!ring_node) ring_node = exterior;
  const auto pos_list = child(ring_node, "posList");
  if (!pos_list) return false;
  std::vector<double> vals;
  if (!parse_doubles(text_of(pos_list), vals) || vals.size() < 6 || (vals.size() % 2) != 0)
    return false;
  Ring ring;
  ring.points.reserve(vals.size() / 2);
  for (std::size_t i = 0; i + 1 < vals.size(); i += 2)
    ring.points.push_back(Point{vals[i], vals[i + 1]});
  g.kind = GeometryKind::Polygon;
  g.polygon.push_back(std::move(ring));
  return true;
}

bool parse_linestring(const pugi::xml_node& line_node, Geometry& g) {
  const auto pos_list = child(line_node, "posList");
  std::vector<double> vals;
  if (pos_list) {
    if (!parse_doubles(text_of(pos_list), vals) || vals.size() < 4 || (vals.size() % 2) != 0)
      return false;
  } else {
    for (auto pos : children(line_node, "pos")) {
      std::vector<double> one;
      if (!parse_doubles(text_of(pos), one) || one.size() < 2) return false;
      vals.push_back(one[0]);
      vals.push_back(one[1]);
    }
    if (vals.size() < 4) return false;
  }
  g.kind = GeometryKind::LineString;
  for (std::size_t i = 0; i + 1 < vals.size(); i += 2)
    g.line.push_back(Point{vals[i], vals[i + 1]});
  return true;
}

bool parse_geometry_node(const pugi::xml_node& geom_wrap, Geometry& g) {
  // geometry may wrap Point/Polygon/LineString, or be them directly.
  for (auto c : geom_wrap.children()) {
    const char* ln = local(c);
    if (std::strcmp(ln, "Point") == 0) return parse_point(c, g);
    if (std::strcmp(ln, "Polygon") == 0) return parse_polygon(c, g);
    if (std::strcmp(ln, "LineString") == 0) return parse_linestring(c, g);
  }
  const char* self = local(geom_wrap);
  if (std::strcmp(self, "Point") == 0) return parse_point(geom_wrap, g);
  if (std::strcmp(self, "Polygon") == 0) return parse_polygon(geom_wrap, g);
  if (std::strcmp(self, "LineString") == 0) return parse_linestring(geom_wrap, g);
  return false;
}

Point centroid_of(const Geometry& g) {
  if (g.kind == GeometryKind::Point) return g.point;
  if (g.kind == GeometryKind::LineString && !g.line.empty()) {
    double lon = 0, lat = 0;
    for (const auto& p : g.line) {
      lon += p.lon;
      lat += p.lat;
    }
    const double n = static_cast<double>(g.line.size());
    return Point{lon / n, lat / n};
  }
  if (g.kind == GeometryKind::Polygon && !g.polygon.empty() && !g.polygon.front().points.empty()) {
    // Average of exterior ring vertices EXCLUDING the repeated closing vertex (contract).
    const auto& pts = g.polygon.front().points;
    std::size_t n = pts.size();
    if (n >= 2 && pts.front().lon == pts.back().lon && pts.front().lat == pts.back().lat) --n;
    if (n == 0) return {};
    double lon = 0, lat = 0;
    for (std::size_t i = 0; i < n; ++i) {
      lon += pts[i].lon;
      lat += pts[i].lat;
    }
    const double dn = static_cast<double>(n);
    return Point{lon / dn, lat / dn};
  }
  return {};
}

bool looks_like_s124(const pugi::xml_node& root, const pugi::xml_document& /*doc*/) {
  const auto pid = trim_copy(root.attribute("productId").as_string());
  if (!pid.empty()) {
    std::string upper = pid;
    for (char& c : upper) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    if (upper == "S-124" || upper == "S124") return true;
    return false;  // explicit wrong productId
  }
  // No productId: accept if root or children use NavigationalWarning / S124 Dataset local name.
  const char* rn = local(root);
  if (std::strcmp(rn, "Dataset") == 0 || std::strcmp(rn, "NavigationalWarning") == 0) {
    // Prefer S124 namespace hint on root name (S124:Dataset) or child warnings.
    const char* raw = root.name();
    if (std::strncmp(raw, "S124:", 5) == 0) return true;
    if (!children(root, "NavigationalWarning").empty()) return true;
  }
  if (!children(root, "NavigationalWarning").empty()) return true;
  return false;
}

DrawingInstruction::Kind geom_kind_to_di(GeometryKind k) {
  switch (k) {
    case GeometryKind::Point:
      return DrawingInstruction::Kind::Point;
    case GeometryKind::LineString:
      return DrawingInstruction::Kind::Line;
    case GeometryKind::Polygon:
      return DrawingInstruction::Kind::Area;
    default:
      return DrawingInstruction::Kind::Null;
  }
}

}  // namespace

Bounds NavWarningDataset::bounds() const {
  Bounds b;
  for (const auto& w : warnings) union_bounds(b, w.bounds);
  return b;
}

const NavWarning* NavWarningDataset::find(std::string_view id) const {
  for (const auto& w : warnings)
    if (w.id == id) return &w;
  return nullptr;
}

bool load_s124(const std::filesystem::path& path, NavWarningDataset& out, LoadError* err) {
  out = NavWarningDataset{};
  out.product = ProductId::S124;
  out.path = path;

  std::ifstream in(path, std::ios::binary);
  if (!in) {
    set_err(err, LoadErrorKind::Io, "cannot open: " + path.string());
    return false;
  }
  std::ostringstream oss;
  oss << in.rdbuf();
  const std::string bytes = oss.str();
  if (bytes.empty() || is_all_whitespace(bytes)) {
    set_err(err, LoadErrorKind::Invalid, "empty or whitespace-only S-124 input: " + path.string());
    out.warnings.clear();
    return false;
  }

  pugi::xml_document doc;
  pugi::xml_parse_result pr = doc.load_buffer(bytes.data(), bytes.size());
  if (!pr) {
    set_err(err, LoadErrorKind::Parse, std::string("S-124 XML parse failed: ") + pr.description());
    return false;
  }

  const pugi::xml_node root = doc.document_element();
  if (!root) {
    set_err(err, LoadErrorKind::Parse, "S-124 XML has no document element");
    return false;
  }

  if (!looks_like_s124(root, doc)) {
    set_err(err, LoadErrorKind::Invalid,
            "not an S-124 navigational-warning product: " + path.string());
    out.warnings.clear();
    return false;
  }

  std::vector<pugi::xml_node> nw_nodes = children(root, "NavigationalWarning");
  // Some packages nest under a member/featureMember wrapper.
  if (nw_nodes.empty()) {
    for (auto c : root.children()) {
      auto nested = children(c, "NavigationalWarning");
      nw_nodes.insert(nw_nodes.end(), nested.begin(), nested.end());
      if (std::strcmp(local(c), "NavigationalWarning") == 0) nw_nodes.push_back(c);
    }
  }

  if (nw_nodes.empty()) {
    set_err(err, LoadErrorKind::Parse,
            "S-124 has zero NavigationalWarning elements: " + path.string());
    out.warnings.clear();
    return false;
  }

  out.warnings.reserve(nw_nodes.size());
  for (const auto& node : nw_nodes) {
    NavWarning w;
    w.id = node.attribute("gml:id").as_string();
    if (w.id.empty()) w.id = node.attribute("id").as_string();
    // pugixml stores attributes without namespace prefix by default depending on parse;
    // also try gml id via iterate.
    if (w.id.empty()) {
      for (auto a : node.attributes()) {
        const char* an = a.name();
        const char* c = std::strchr(an, ':');
        const char* local_a = c ? c + 1 : an;
        if (std::strcmp(local_a, "id") == 0) {
          w.id = a.value();
          break;
        }
      }
    }
    w.title = child_text(node, "title");
    w.message = child_text(node, "message");
    w.time_iso = child_text(node, "timeISO");
    if (w.time_iso.empty()) w.time_iso = child_text(node, "time_iso");

    for (auto geom_wrap : children(node, "geometry")) {
      Geometry g;
      if (parse_geometry_node(geom_wrap, g)) {
        union_bounds(w.bounds, bounds_from_geometry(g));
        w.geometries.push_back(std::move(g));
      }
    }
    // Geometry may also sit directly under the warning.
    if (w.geometries.empty()) {
      for (auto c : node.children()) {
        const char* ln = local(c);
        if (std::strcmp(ln, "Point") == 0 || std::strcmp(ln, "Polygon") == 0 ||
            std::strcmp(ln, "LineString") == 0) {
          Geometry g;
          if (parse_geometry_node(c, g)) {
            union_bounds(w.bounds, bounds_from_geometry(g));
            w.geometries.push_back(std::move(g));
          }
        }
      }
    }
    out.warnings.push_back(std::move(w));
  }

  return true;
}

bool portray_nav_warnings(const NavWarningDataset& nw, DrawingInstructions& out, std::string* err) {
  out.clear();
  if (nw.warnings.empty()) {
    if (err) *err = "S-124 portray: no navigational warnings";
    return false;
  }

  std::uint64_t fid = 1;
  for (const auto& w : nw.warnings) {
    if (w.geometries.empty()) {
      ++fid;
      continue;
    }
    for (const auto& g : w.geometries) {
      DrawingInstruction di;
      di.feature_id = fid;
      di.kind = geom_kind_to_di(g.kind);
      if (di.kind == DrawingInstruction::Kind::Null) continue;
      di.fill_ref = "CHYLW";
      di.viewing_group = "21010";
      di.drawing_priority = 0;
      di.display_plane = "OverRadar";
      if (di.kind == DrawingInstruction::Kind::Area) {
        di.command = "ColorFill";
      } else if (di.kind == DrawingInstruction::Kind::Line) {
        di.command = "LineInstruction";
        LineStyleDef ls;
        ls.name = "_simple_";
        ls.width = 0.32;
        ls.colour = "CHYLW";
        di.line_styles.push_back(ls);
        di.line_style_ref = "_simple_";
      } else {
        di.command = "PointInstruction";
        di.symbol_ref = "";  // viewer falls back to coloured marker via fill_ref path
      }
      di.geometries.push_back(g);
      out.push_back(std::move(di));
    }

    // Title text at point or polygon centroid.
    DrawingInstruction text;
    text.feature_id = fid;
    text.kind = DrawingInstruction::Kind::Text;
    text.command = "TextInstruction";
    text.text = w.title.empty() ? w.id : w.title;
    text.fill_ref = "CHYLW";
    text.text_style.font_colour = "CHYLW";
    text.text_style.font_size = 10;
    text.text_style.halign = "Center";
    text.text_style.valign = "Center";
    text.viewing_group = "21010";
    text.drawing_priority = 1;
    text.display_plane = "OverRadar";
    Geometry anchor;
    anchor.kind = GeometryKind::Point;
    anchor.point = centroid_of(w.geometries.front());
    text.geometries.push_back(std::move(anchor));
    out.push_back(std::move(text));
    ++fid;
  }

  if (out.empty()) {
    if (err) *err = "S-124 portray: no drawable geometries";
    return false;
  }
  return true;
}

}  // namespace s100
