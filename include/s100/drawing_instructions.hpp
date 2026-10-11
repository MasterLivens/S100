#pragma once
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace s100 {

/** `_simple_` (or named) line style defined by a LineStyle state command (Part 9a 9a-11.2.2.3). */
struct LineStyleDef {
  std::string name;            // "_simple_" for SimpleLineStyle
  double interval_length{0};   // 0 = continuous
  double width{0};             // mm
  std::string colour;          // colour token, e.g. "DEPSC"
  double colour_transparency{0};
  bool has_dash{false};
  double dash_start{0};
  double dash_length{0};
};

/** Text style state in effect for a TextInstruction (Part 9a 9a-11.2.2.4). */
struct TextStyle {
  std::string font_colour{"CHBLK"};
  double font_colour_transparency{0};
  double font_size{10};
  std::string font_weight{"Medium"};
  std::string font_slant{"Upright"};
  std::string halign{"Start"};   // TextAlignHorizontal
  std::string valign{"Baseline"};  // TextAlignVertical
  double vertical_offset{0};
  std::string background_colour;
};

/** One product-neutral drawing instruction (S-101 and S-102 engines both emit this; the
 *  renderer and later S-98 interoperability work on it). One entry per drawing command; the
 *  state commands in effect are folded into its fields. */
struct DrawingInstruction {
  enum class Kind { Point, Line, Area, Text, Coverage, Null };

  std::uint64_t feature_id{0};
  Kind kind{Kind::Null};
  std::string command;             // drawing command as emitted, e.g. "ColorFill", "LineInstructionUnsuppressed"

  // Visibility state
  std::string viewing_group;               // first ViewingGroup parameter (drives display category)
  std::vector<std::string> viewing_groups; // all ViewingGroup parameters
  int drawing_priority{0};
  std::string display_plane;               // e.g. UnderRadar / OverRadar
  long scale_minimum{0};                   // 0 = none
  long scale_maximum{0};                   // 0 = none
  bool hover{false};

  // Drawing-command payload
  std::string symbol_ref;       // Point (PointInstruction symbol)
  std::string line_style_ref;   // Line (LineInstruction parameter, e.g. "_simple_" or "ACHARE51")
  std::vector<LineStyleDef> line_styles;  // inline LineStyle definitions referenced by line_style_ref
  std::string fill_ref;         // Area: colour token (ColorFill) or pattern (AreaFillReference, ...)
  double fill_transparency{0};
  std::string text;             // Text (DEF-decoded)
  TextStyle text_style;
  std::string coverage_attribute;          // Coverage (CoverageFill attributeCode)
  std::vector<std::string> coverage_state; // Coverage: CoverageColor / LookupEntry / annotations in effect

  // Transform state
  double rotation{0};
  std::string rotation_crs;     // PortrayalCRS / GeographicCRS / LocalCRS / LineCRS
  double offset_x{0}, offset_y{0};  // LocalOffset (mm)
  double scale_factor{1};
  std::string line_placement;   // LinePlacement raw parameters
  std::string area_placement;   // AreaPlacement raw parameters

  // Geometry state: SpatialReference ids restrict drawing to those spatials (empty = whole
  // feature geometry). Augmented geometry is kept as its explicit commands (not re-derived).
  std::vector<std::string> spatial_refs;
  std::vector<std::string> augmented;      // e.g. "AugmentedRay:GeographicCRS,45,PortrayalCRS,25"

  std::vector<std::string> time_state;     // Date / Time / DateTime / TimeValid in effect
  std::string alert_reference;
};

using DrawingInstructions = std::vector<DrawingInstruction>;

/** Counters from parsing (and running) portrayal. */
struct PortrayalStats {
  std::size_t features{0};            // features offered to the rules
  std::size_t emitted_features{0};    // HostPortrayalEmit calls
  std::size_t instructions{0};        // DrawingInstruction entries produced
  std::map<std::string, std::size_t> by_command;    // drawing command -> count
  std::size_t unparsed{0};            // commands not understood (kept raw, see unparsed_raw)
  std::map<std::string, std::size_t> unparsed_commands;  // command -> count
  std::vector<std::string> unparsed_raw;            // "featureID: element" (first 100)
  std::size_t lua_errors{0};          // rule errors caught by main.lua (Default used) + fatal
  std::vector<std::string> errors;    // messages (first 100)
  std::size_t first_chance_errors{0}; // error() calls seen through HostDebuggerEntry
  std::size_t warnings{0};            // Debug.Trace('Warning: ...')
  std::vector<std::uint64_t> features_without_instructions;
};

/** Parse one HostPortrayalEmit drawing instruction string (DEF, Part 9a 9a-11.2) for one
 *  feature into `out` (appends). Unknown commands are counted in `stats`, never fatal. */
void parse_drawing_instructions(std::uint64_t feature_id, const std::string& def,
                                DrawingInstructions& out, PortrayalStats* stats = nullptr);

/** DEF decode (Part 13 13-6.1): &s ; &c : &m , &a &. */
std::string decode_def_string(const std::string& s);

}  // namespace s100
