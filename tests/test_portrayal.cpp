// Portrayal contract tests (docs/portrayal-contract.md, "Test hooks").
// One executable, one CTest entry per case: s100_portrayal_tests <case>.
#include "s100/s100.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>

namespace fs = std::filesystem;

#define CHECK(cond)                                                                    \
  do {                                                                                 \
    if (!(cond)) {                                                                     \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";     \
      std::exit(1);                                                                    \
    }                                                                                  \
  } while (0)

namespace {

const fs::path kSrc = S100_SOURCE_DIR;
const fs::path kS101Pc = kSrc / "catalogues/S-101/PC_2.0.0";
const fs::path kS102Pc = kSrc / "catalogues/S-102/PC_3.0.0";

const s100::FeatureCatalogue& s101_fc() {
  static s100::FeatureCatalogue fc;
  static bool loaded = false;
  if (!loaded) {
    std::string e;
    CHECK(s100::load_feature_catalogue(kSrc / "catalogues/S-101/S-101_FC_2.0.0.xml", fc, &e));
    loaded = true;
  }
  return fc;
}

const s100::PortrayalCatalogue& pc_at(const fs::path& dir) {
  static std::map<fs::path, s100::PortrayalCatalogue> cache;
  auto it = cache.find(dir);
  if (it == cache.end()) {
    s100::PortrayalCatalogue pc;
    std::string e;
    if (!s100::load_portrayal_catalogue(dir, pc, &e)) std::cerr << e << "\n";
    CHECK(!pc.product.empty());
    it = cache.emplace(dir, std::move(pc)).first;
  }
  return it->second;
}

/** 3 IHO cells (required) + CHS cells (gitignored; skipped when absent). */
std::vector<fs::path> cells() {
  std::vector<fs::path> v;
  for (const char* c : {"101AA00DS0003.000", "101AA00DS0006.000", "101AA00DS0008.000"})
    v.push_back(kSrc / "testdata/s101" / c);
  const fs::path chs = kSrc / "testdata/chs/COMBINED/S100_ROOT/S-101";
  if (fs::exists(chs)) {
    std::vector<fs::path> c;
    for (const auto& d : fs::recursive_directory_iterator(chs))
      if (d.path().extension() == ".000") c.push_back(d.path());
    std::sort(c.begin(), c.end());
    v.insert(v.end(), c.begin(), c.end());
  } else {
    std::cerr << "skip CHS cells: " << chs << " absent\n";
  }
  return v;
}

s100::Dataset open_cell(const fs::path& p) {
  s100::Dataset ds;
  s100::LoadError le;
  CHECK(s100::open_dataset(p, ds, &le));
  CHECK(s100::classify_layers(ds, s101_fc()));
  return ds;
}

// Rule errors the official S-101 PC 2.0.0 raises on its own (OBSTRN07.lua line 19: error() when
// an Obstruction has neither valueOfSounding nor defaultClearanceDepth); main.lua then falls back
// to Default symbology by design. Not a host defect; the host must not invent depths. Listed
// exactly (cell, feature id) so any other Lua error fails. Open question for the architect.
struct KnownPcError {
  const char* cell;
  const char* feature_id;
  const char* message;
};
const KnownPcError kKnownPcErrors[] = {
    {"101CA00P468N0712W.000", "1160", "Neither valueOfSounding or defaultClearanceDepth have a value"},
    {"101CA00P468N0712W.000", "1164", "Neither valueOfSounding or defaultClearanceDepth have a value"},
    {"101CA00T460N0720W.000", "78", "Neither valueOfSounding or defaultClearanceDepth have a value"},
};

bool known_pc_error(const std::string& cell, const std::string& msg) {
  for (const auto& k : kKnownPcErrors)
    if (cell == k.cell && msg.find(k.message) != std::string::npos &&
        msg.find(std::string("ID=") + k.feature_id + " ") != std::string::npos)
      return true;
  return false;
}

// ---------------------------------------------------------------- cases

/** All cells portray: no fatal error, no Lua errors (beyond the listed PC 2.0.0 ones), every
 *  feature yields >= 1 instruction (Null counts), layers get PC display info. */
void portrayal_all_cells() {
  const auto& pc = pc_at(kS101Pc);
  for (const auto& cell : cells()) {
    auto ds = open_cell(cell);
    s100::PortrayalSession session(s101_fc(), pc);
    s100::PortrayalStats st;
    std::string err;
    const auto* di = session.portray(ds, &st, &err);
    if (!di) std::cerr << cell.filename() << ": " << err << "\n";
    CHECK(di);
    std::size_t unexpected = 0;
    for (const auto& m : st.errors)
      if (!known_pc_error(cell.filename().string(), m)) {
        std::cerr << cell.filename() << " lua error: " << m << "\n";
        ++unexpected;
      }
    CHECK(unexpected == 0);
    CHECK(st.lua_errors == st.errors.size());  // nothing truncated / fatal
    CHECK(st.features == ds.features.size());
    CHECK(st.emitted_features >= ds.features.size());
    for (auto id : st.features_without_instructions) std::cerr << cell.filename() << " no instruction: " << id << "\n";
    CHECK(st.features_without_instructions.empty());
    std::set<std::uint64_t> fids;
    for (const auto& d : *di) fids.insert(d.feature_id);
    for (const auto& f : ds.features) CHECK(fids.count(f.id));
    for (const auto& [code, li] : ds.layers.all()) {
      CHECK(!li.viewing_group.empty());
      CHECK(!li.display_plane.empty());
    }
    std::cout << "portray " << cell.filename().string() << " features=" << ds.features.size()
              << " instructions=" << st.instructions << " lua_errors=" << st.lua_errors
              << " (known PC 2.0.0: " << st.lua_errors - unexpected << ") unparsed=" << st.unparsed << "\n";
    for (const auto& [c, n] : st.unparsed_commands) std::cout << "  unparsed " << c << " " << n << "\n";
  }
}

/** Every Host* referenced by a PC's Rules/*.lua, and every name in docs/testing/host-functions.txt,
 *  is registered in that product's registry. Lua function parameters named Host* (PortrayalAPI.lua
 *  LookupAttributeValue) are registered too, per the tester list. */
void host_functions_static() {
  std::map<std::string, std::set<std::string>> listed;  // section -> names
  {
    std::ifstream in(kSrc / "docs/testing/host-functions.txt");
    CHECK(in);
    std::string line, section;
    while (std::getline(in, line)) {
      if (!line.empty() && line.back() == '\r') line.pop_back();
      if (line.empty() || line[0] == '#') continue;
      if (line[0] == '[') section = line;
      else listed[section].insert(line);
    }
  }
  CHECK(listed["[S-101 PC 2.0.0]"].size() == 28);
  CHECK(listed["[S-102 PC 3.0.0]"].size() == 29);

  s100::Dataset ds;
  s100::S101HostAdapter h101(ds, s101_fc());
  s100::CoverageDataset cov;
  s100::S102HostAdapter h102(cov, s101_fc());
  struct P {
    const s100::HostAdapter* host;
    fs::path rules;
    const char* section;
  } products[] = {{&h101, kS101Pc / "Rules", "[S-101 PC 2.0.0]"}, {&h102, kS102Pc / "Rules", "[S-102 PC 3.0.0]"}};
  const std::regex host_re("\\bHost[A-Za-z]+\\b");
  for (const auto& p : products) {
    std::set<std::string> referenced;
    for (const auto& f : fs::directory_iterator(p.rules)) {
      if (f.path().extension() != ".lua") continue;
      std::ifstream in(f.path(), std::ios::binary);
      std::stringstream ss;
      ss << in.rdbuf();
      const std::string src = ss.str();
      for (auto it = std::sregex_iterator(src.begin(), src.end(), host_re); it != std::sregex_iterator(); ++it)
        referenced.insert(it->str());
    }
    for (const auto& n : referenced) {
      if (!p.host->registry().has(n)) std::cerr << p.section << " referenced but not registered: " << n << "\n";
      CHECK(p.host->registry().has(n));
    }
    for (const auto& n : listed[p.section]) {
      if (!p.host->registry().has(n)) std::cerr << p.section << " listed but not registered: " << n << "\n";
      CHECK(p.host->registry().has(n));
      CHECK(p.host->registry().function(n) != nullptr);
    }
    CHECK(referenced == listed[p.section]);  // list is in sync with the official rules
    std::cout << p.host->product() << ": " << referenced.size() << " Host* referenced, "
              << p.host->registry().names().size() << " registered\n";
  }
  // Rule 7: name variants are one implementation registered as an alias.
  const auto& r102 = h102.registry();
  CHECK(r102.is_alias("HostFeatureTypeGetCode"));
  CHECK(r102.target_of("HostFeatureTypeGetCode") == "HostFeatureGetCode");
  CHECK(r102.function("HostFeatureTypeGetCode") == r102.function("HostFeatureGetCode"));
  CHECK(!r102.is_alias("HostFeatureGetCode"));
  CHECK(!h101.registry().has("HostFeatureTypeGetCode"));  // S-101 PC does not use it
}

/** An unregistered Host* must raise a Lua error, not read as nil. */
void host_missing_function() {
  const fs::path dir = fs::temp_directory_path() / "s100_missing_host_pc";
  fs::create_directories(dir / "Rules");
  {
    std::ofstream x(dir / "portrayal_catalogue.xml");
    x << "<?xml version=\"1.0\"?><pc:portrayalCatalog xmlns:pc=\"http://www.iho.int/S100PortrayalCatalog/5.2\" "
         "productId=\"TEST\" version=\"0\"><rules><ruleFile id=\"main\"><fileName>main.lua</fileName>"
         "<ruleType>TopLevelTemplate</ruleType></ruleFile></rules></pc:portrayalCatalog>";
    std::ofstream m(dir / "Rules/main.lua");
    // HostSpatialRelate is Part 13 13-8.2.3.1 but not used by the PCs, hence not registered.
    m << "function PortrayalCreateContextParameter() return {} end\n"
         "function PortrayalInitializeContextParameters() end\n"
         "function PortrayalMain() local r = HostSpatialRelate('a', 'b', 'Contains') return true end\n";
  }
  s100::PortrayalCatalogue pc;
  std::string e;
  CHECK(s100::load_portrayal_catalogue(dir, pc, &e));
  s100::Dataset ds;
  s100::S101HostAdapter host(ds, s101_fc());
  s100::DrawingInstructions di;
  s100::PortrayalStats st;
  CHECK(!s100::PortrayalEngine(pc).run(host, s100::PortrayalContext::defaults(pc), di, &st, &e));
  std::cout << "missing host function -> " << e << "\n";
  CHECK(e.find("HostSpatialRelate") != std::string::npos);
  CHECK(e.find("not implemented") != std::string::npos);
  fs::remove_all(dir);
}

std::map<std::uint64_t, std::vector<std::string>> depth_area_fills(const s100::Dataset& ds,
                                                                   const s100::DrawingInstructions& di) {
  std::set<std::uint64_t> depth;
  for (const auto& f : ds.features)
    if (f.feature_type == "DepthArea") depth.insert(f.id);
  std::map<std::uint64_t, std::vector<std::string>> m;
  for (const auto& d : di)
    if (depth.count(d.feature_id) && d.command == "ColorFill") m[d.feature_id].push_back(d.fill_ref);
  return m;
}

/** Changing SafetyContour in the session re-runs portrayal and changes DepthArea fills. */
void safety_contour() {
  const auto& pc = pc_at(kS101Pc);
  auto ds = open_cell(kSrc / "testdata/s101/101AA00DS0008.000");
  s100::PortrayalSession session(s101_fc(), pc);
  const auto* a = session.portray(ds);
  CHECK(a);
  const auto fills_default = depth_area_fills(ds, *a);
  CHECK(!fills_default.empty());
  CHECK(session.portray(ds) == a && session.runs() == 1);  // same context: cached

  const std::string key0 = session.cache_key(ds);
  std::string e;
  CHECK(!session.set_context_parameter("NoSuchParameter", "1", &e));
  CHECK(session.set_context_parameter("SafetyContour", "5", &e));
  CHECK(session.cache_key(ds) != key0);
  const auto* b = session.portray(ds);
  CHECK(b && session.runs() == 2);
  const auto fills_5 = depth_area_fills(ds, *b);
  std::size_t changed = 0;
  for (const auto& [id, f] : fills_default) {
    auto it = fills_5.find(id);
    if (it != fills_5.end() && it->second != f) {
      if (!changed)
        std::cout << "DepthArea " << id << " SafetyContour 30 -> 5: " << f.front() << " -> " << it->second.front() << "\n";
      ++changed;
    }
  }
  std::cout << "SafetyContour change: " << changed << " of " << fills_default.size() << " DepthArea fills changed\n";
  CHECK(changed > 0);
}

/** S-102 reuses the engine and emits the same DrawingInstructions type (coverage host). */
void s102_engine() {
  const auto& pc = pc_at(kS102Pc);
  CHECK(pc.product == "S-102");
  // The S-102 FC is not staged yet (open item); a minimal in-memory FC with the one feature
  // type the PC 3.0.0 rules portray stands in for type information.
  s100::FeatureCatalogue fc;
  fc.product = "S-102";
  fc.version = "test";
  s100::FcFeatureType bc;
  bc.code = "BathymetryCoverage";
  bc.name = "Bathymetry Coverage";
  bc.use_type = "geographic";
  bc.primitives = {"coverage"};
  fc.feature_types[bc.code] = bc;
  s100::CoverageDataset cov;
  cov.features.push_back(s100::CoverageFeature{1, "BathymetryCoverage", {}});

  auto run = [&](const s100::PortrayalContext& ctx) {
    s100::DrawingInstructions di;
    s100::PortrayalStats st;
    std::string e;
    CHECK(s100::portray(cov, fc, pc, ctx, di, &e, &st));
    for (const auto& m : st.errors) std::cerr << "S-102 lua error: " << m << "\n";
    CHECK(st.lua_errors == 0);
    CHECK(st.unparsed == 0);
    CHECK(di.size() == 1);
    CHECK(di[0].kind == s100::DrawingInstruction::Kind::Coverage);
    CHECK(di[0].command == "CoverageFill" && di[0].coverage_attribute == "depth");
    CHECK(di[0].viewing_group == "13030" && di[0].drawing_priority == 3);
    return di[0];
  };
  auto ctx = s100::PortrayalContext::defaults(pc);
  const auto d30 = run(ctx);
  CHECK(ctx.set(pc, "SafetyContour", "10"));
  const auto d10 = run(ctx);
  CHECK(d30.coverage_state != d10.coverage_state);
  std::string joined;
  for (const auto& s : d10.coverage_state) joined += s + ";";
  CHECK(joined.find("LookupEntry:Shallow Water,0,10,") != std::string::npos);
  std::cout << "S-102 coverage: " << joined << "\n";
}

/** Parser: known commands map to fields; unknown commands are counted, never fatal. */
void drawing_instruction_parser() {
  s100::DrawingInstructions di;
  s100::PortrayalStats st;
  s100::parse_drawing_instructions(
      7,
      "ViewingGroup:13030,90020;DrawingPriority:3;DisplayPlane:UnderRadar;ColorFill:DEPVS,0.5;"
      "AreaFillReference:DIAMOND1;Dash:0,3.6;LineStyle:_simple_,5.4,0.64,DEPSC;LineInstruction:_simple_;"
      "Rotation:PortrayalCRS,45;PointInstruction:BOYLAT13;FontColor:CHBLK;TextInstruction:A&m B&s C&c D&a;"
      "SpatialReference:C12;ClearGeometry;FutureCommand:1,2;;NullInstruction",
      di, &st);
  CHECK(di.size() == 6);
  CHECK(st.instructions == 6);
  CHECK(st.unparsed == 1 && st.unparsed_commands["FutureCommand"] == 1);
  using K = s100::DrawingInstruction::Kind;
  CHECK(di[0].kind == K::Area && di[0].fill_ref == "DEPVS" && di[0].fill_transparency == 0.5);
  CHECK(di[0].viewing_group == "13030" && di[0].viewing_groups.size() == 2 && di[0].drawing_priority == 3);
  CHECK(di[0].display_plane == "UnderRadar" && di[0].feature_id == 7);
  CHECK(di[1].kind == K::Area && di[1].command == "AreaFillReference" && di[1].fill_ref == "DIAMOND1");
  CHECK(di[2].kind == K::Line && di[2].line_style_ref == "_simple_" && di[2].line_styles.size() == 1);
  CHECK(di[2].line_styles[0].colour == "DEPSC" && di[2].line_styles[0].has_dash);
  CHECK(di[3].kind == K::Point && di[3].symbol_ref == "BOYLAT13" && di[3].rotation == 45);
  CHECK(di[4].kind == K::Text && di[4].text == "A, B; C: D&");
  CHECK(di[5].kind == K::Null && di[5].spatial_refs.empty());
  std::cout << "parser ok\n";
}


/** SpatialReference ids are resolved into DrawingInstruction::geometries by portray(). */
void spatial_reference_resolve() {
  s100::Dataset ds;
  s100::LoadError le;
  CHECK(s100::load_s101(kSrc / "testdata/s101/101AA00DS0008.000", ds, &le));
  const auto& pc = pc_at(kS101Pc);
  s100::DrawingInstructions di;
  s100::PortrayalStats st;
  std::string e;
  auto ctx = s100::PortrayalContext::defaults(pc);
  CHECK(s100::portray(ds, s101_fc(), pc, ctx, di, &e, &st));
  std::size_t with_sr = 0, resolved = 0;
  for (const auto& d : di) {
    if (d.spatial_refs.empty()) {
      CHECK(d.geometries.empty());
      continue;
    }
    ++with_sr;
    CHECK(!d.geometries.empty());
    CHECK(d.geometries.size() == d.spatial_refs.size());
    for (const auto& g : d.geometries) {
      CHECK(g.kind == s100::GeometryKind::LineString);
      CHECK(g.line.size() >= 2);
    }
    ++resolved;
  }
  CHECK(with_sr > 0);
  CHECK(resolved == with_sr);
  // Direct resolve of a known reverse token on CHS-style id.
  s100::DrawingInstructions one;
  s100::DrawingInstruction sample;
  sample.spatial_refs = {"C2"};
  one.push_back(sample);
  s100::resolve_spatial_references(ds, one);
  CHECK(one[0].geometries.size() == 1);
  CHECK(one[0].geometries[0].kind == s100::GeometryKind::LineString);
  std::cout << "spatial_reference_resolve: " << with_sr << " instructions clipped\n";
}

}  // namespace


/** Colour tokens on instructions stay symbolic names; Day/Dusk/Night remap must not rewrite them. */
void colour_tokens_symbolic() {
  auto ds = open_cell(kSrc / "testdata/s101/101AA00DS0008.000");
  s100::PortrayalSession session(s101_fc(), pc_at(kS101Pc));
  s100::PortrayalStats st;
  std::string err;
  const auto* di = session.portray(ds, &st, &err);
  CHECK(di);
  std::size_t named = 0;
  for (const auto& d : *di) {
    if (!d.fill_ref.empty()) {
      CHECK(d.fill_ref[0] != '#');
      CHECK(d.fill_ref.size() >= 3 && d.fill_ref.size() <= 16);
      ++named;
    }
    if (!d.text_style.font_colour.empty()) {
      CHECK(d.text_style.font_colour[0] != '#');
    }
  }
  CHECK(named > 0);
  std::cout << "colour tokens symbolic: " << named << " ColorFill refs\n";
}

int main(int argc, char** argv) {
  const std::map<std::string, std::function<void()>> cases = {
      {"portrayal_all_cells", portrayal_all_cells},
      {"host_functions_static", host_functions_static},
      {"host_missing_function", host_missing_function},
      {"safety_contour", safety_contour},
      {"s102_engine", s102_engine},
      {"drawing_instruction_parser", drawing_instruction_parser},
      {"spatial_reference_resolve", spatial_reference_resolve},
      {"colour_tokens_symbolic", colour_tokens_symbolic},
  };
  if (argc < 2) {
    for (const auto& [n, f] : cases) f();
  } else {
    auto it = cases.find(argv[1]);
    if (it == cases.end()) {
      std::cerr << "unknown case " << argv[1] << "\n";
      return 2;
    }
    it->second();
  }
  std::cout << "ok\n";
  return 0;
}
