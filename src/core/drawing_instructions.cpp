// Parser for the DEF drawing instruction strings emitted through HostPortrayalEmit
// (S-100 Part 9a clause 9a-11.2). State commands update a running state; each drawing command
// produces one DrawingInstruction carrying a snapshot of that state.
#include "s100/drawing_instructions.hpp"

#include <cstdlib>
#include <map>
#include <set>

namespace s100 {
namespace {

std::vector<std::string> split(const std::string& s, char sep) {
  std::vector<std::string> v;
  std::size_t a = 0;
  while (true) {
    const std::size_t b = s.find(sep, a);
    v.push_back(s.substr(a, b == std::string::npos ? std::string::npos : b - a));
    if (b == std::string::npos) break;
    a = b + 1;
  }
  return v;
}

double num(const std::vector<std::string>& p, std::size_t i, double dflt = 0) {
  if (i >= p.size() || p[i].empty()) return dflt;
  char* end = nullptr;
  const double v = std::strtod(p[i].c_str(), &end);
  return end == p[i].c_str() ? dflt : v;
}

std::string str(const std::vector<std::string>& p, std::size_t i) {
  return i < p.size() ? p[i] : std::string();
}

const std::set<std::string>& state_commands() {
  // Part 9a Table 9a-4 (state commands) — recognised; those without a dedicated field are
  // accepted as state and do not count as unparsed.
  static const std::set<std::string> s = {
      "ViewingGroup", "DisplayPlane", "DrawingPriority", "ScaleMinimum", "ScaleMaximum", "Id",
      "Parent", "Hover", "LocalOffset", "LinePlacement", "AreaPlacement", "AreaCRS", "Rotation",
      "ScaleFactor", "Dash", "LineSymbol", "LineStyle", "FontColor", "FontSize", "FontProportion",
      "FontWeight", "FontSlant", "FontSerifs", "FontUnderline", "FontStrikethrough",
      "FontUpperline", "FontReference", "FontBackgroundColor", "TextAlignHorizontal",
      "TextAlignVertical", "TextVerticalOffset", "OverrideColor", "OverrideAll", "ClearOverride",
      "SpatialReference", "AugmentedPoint", "AugmentedRay", "AugmentedPath", "Polyline",
      "ArcByRadius", "Annulus", "ClearGeometry", "NumericAnnotation", "SymbolAnnotation",
      "CoverageColor", "LookupEntry", "Date", "Time", "DateTime", "TimeValid", "ClearTime",
      "AlertReference"};
  return s;
}

}  // namespace

std::string decode_def_string(const std::string& s) {
  if (s.find('&') == std::string::npos) return s;
  std::string o;
  o.reserve(s.size());
  for (std::size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '&' && i + 1 < s.size()) {
      const char c = s[i + 1];
      const char r = c == 's' ? ';' : c == 'c' ? ':' : c == 'm' ? ',' : c == 'a' ? '&' : 0;
      if (r) {
        o += r;
        ++i;
        continue;
      }
    }
    o += s[i];
  }
  return o;
}

void parse_drawing_instructions(std::uint64_t feature_id, const std::string& def,
                                DrawingInstructions& out, PortrayalStats* stats) {
  DrawingInstruction st;  // running state
  st.feature_id = feature_id;
  std::map<std::string, LineStyleDef> line_styles;
  std::vector<std::pair<double, double>> pending_dash;

  for (const auto& element : split(def, ';')) {
    if (element.empty()) continue;
    const std::size_t colon = element.find(':');
    const std::string cmd = element.substr(0, colon);
    const std::string params_s = colon == std::string::npos ? std::string() : element.substr(colon + 1);
    const auto p = split(params_s, ',');

    auto emit = [&](DrawingInstruction::Kind kind) {
      DrawingInstruction di = st;
      di.kind = kind;
      di.command = cmd;
      if (kind == DrawingInstruction::Kind::Point) di.symbol_ref = str(p, 0);
      if (kind == DrawingInstruction::Kind::Line) {
        di.line_style_ref = str(p, 0);
        for (const auto& name : p) {
          auto it = line_styles.find(name);
          if (it != line_styles.end()) di.line_styles.push_back(it->second);
        }
      }
      if (kind == DrawingInstruction::Kind::Area) {
        di.fill_ref = str(p, 0);
        di.fill_transparency = cmd == "ColorFill" ? num(p, 1) : 0;
      }
      if (kind == DrawingInstruction::Kind::Text) di.text = decode_def_string(str(p, 0));
      if (kind == DrawingInstruction::Kind::Coverage) di.coverage_attribute = str(p, 0);
      out.push_back(std::move(di));
      if (stats) {
        ++stats->instructions;
        ++stats->by_command[cmd];
      }
    };

    using K = DrawingInstruction::Kind;
    if (cmd == "PointInstruction") emit(K::Point);
    else if (cmd == "LineInstruction" || cmd == "LineInstructionUnsuppressed") emit(K::Line);
    else if (cmd == "ColorFill" || cmd == "AreaFillReference" || cmd == "PixmapFill" ||
             cmd == "SymbolFill" || cmd == "HatchFill")
      emit(K::Area);
    else if (cmd == "TextInstruction") emit(K::Text);
    else if (cmd == "CoverageFill") emit(K::Coverage);
    else if (cmd == "NullInstruction") emit(K::Null);
    else if (cmd == "ViewingGroup") {
      st.viewing_groups = p;
      st.viewing_group = str(p, 0);
    } else if (cmd == "DrawingPriority") st.drawing_priority = static_cast<int>(num(p, 0));
    else if (cmd == "DisplayPlane") st.display_plane = str(p, 0);
    else if (cmd == "ScaleMinimum") st.scale_minimum = static_cast<long>(num(p, 0));
    else if (cmd == "ScaleMaximum") st.scale_maximum = static_cast<long>(num(p, 0));
    else if (cmd == "Hover") st.hover = str(p, 0) == "true";
    else if (cmd == "LocalOffset") {
      st.offset_x = num(p, 0);
      st.offset_y = num(p, 1);
    } else if (cmd == "Rotation") {
      if (p.size() >= 2) {
        st.rotation_crs = p[0];
        st.rotation = num(p, 1);
      } else {
        st.rotation = num(p, 0);
      }
    } else if (cmd == "ScaleFactor") st.scale_factor = num(p, 0, 1);
    else if (cmd == "LinePlacement") st.line_placement = params_s;
    else if (cmd == "AreaPlacement") st.area_placement = params_s;
    else if (cmd == "Dash") pending_dash.emplace_back(num(p, 0), num(p, 1));
    else if (cmd == "LineStyle") {
      LineStyleDef ls;
      ls.name = str(p, 0);
      ls.interval_length = num(p, 1);
      ls.width = num(p, 2);
      ls.colour = str(p, 3);
      ls.colour_transparency = num(p, 4);
      if (!pending_dash.empty()) {
        ls.has_dash = true;
        ls.dash_start = pending_dash.front().first;
        ls.dash_length = pending_dash.front().second;
      }
      pending_dash.clear();
      line_styles[ls.name] = ls;
    } else if (cmd == "FontColor") {
      st.text_style.font_colour = str(p, 0);
      st.text_style.font_colour_transparency = num(p, 1);
    } else if (cmd == "FontSize") st.text_style.font_size = num(p, 0, 10);
    else if (cmd == "FontWeight") st.text_style.font_weight = str(p, 0);
    else if (cmd == "FontSlant") st.text_style.font_slant = str(p, 0);
    else if (cmd == "FontBackgroundColor") st.text_style.background_colour = str(p, 0);
    else if (cmd == "TextAlignHorizontal") st.text_style.halign = str(p, 0);
    else if (cmd == "TextAlignVertical") st.text_style.valign = str(p, 0);
    else if (cmd == "TextVerticalOffset") st.text_style.vertical_offset = num(p, 0);
    else if (cmd == "SpatialReference") st.spatial_refs.push_back(decode_def_string(str(p, 0)) +
                                                                  (str(p, 1) == "false" ? ",reverse" : ""));
    else if (cmd == "AugmentedPoint" || cmd == "AugmentedRay" || cmd == "AugmentedPath" ||
             cmd == "Polyline" || cmd == "ArcByRadius" || cmd == "Annulus")
      st.augmented.push_back(element);
    else if (cmd == "ClearGeometry") {
      st.spatial_refs.clear();
      st.augmented.clear();
    } else if (cmd == "CoverageColor" || cmd == "LookupEntry" || cmd == "NumericAnnotation" ||
               cmd == "SymbolAnnotation")
      st.coverage_state.push_back(element);
    else if (cmd == "Date" || cmd == "Time" || cmd == "DateTime" || cmd == "TimeValid")
      st.time_state.push_back(element);
    else if (cmd == "ClearTime") st.time_state.clear();
    else if (cmd == "AlertReference") st.alert_reference = params_s;
    else if (state_commands().count(cmd)) {
      // recognised state without a dedicated field (Id, Parent, AreaCRS, LineSymbol, Font*, Override*)
    } else if (stats) {
      ++stats->unparsed;
      ++stats->unparsed_commands[cmd];
      if (stats->unparsed_raw.size() < 100)
        stats->unparsed_raw.push_back(std::to_string(feature_id) + ": " + element);
    }
  }
}

}  // namespace s100
