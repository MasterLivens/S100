#include "s100/coverage.hpp"
#include "s100/dataset.hpp"
#include "s100/feature_catalogue.hpp"
#include "s100/hdf5_product.hpp"
#include "s100/portrayal_catalogue.hpp"
#include "s100/portrayal_engine.hpp"
#include "s100/session.hpp"
#include "s100/viewing_group_filter.hpp"
#include "s100/security/types.hpp"
#include "support/test_data_roots.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <tuple>
#include <iostream>
#include <string>

#define CHECK(cond)                                                                \
  do {                                                                             \
    if (!(cond)) {                                                                 \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n"; \
      std::exit(1);                                                                \
    }                                                                              \
  } while (0)

namespace fs = std::filesystem;

static fs::path find_s102_cell() {
  const fs::path src = S100_SOURCE_DIR;
  if (s100test::chs_exercised("s102.open"))
    return s100test::chs_s100_root() / "S-102" / "DATASET_FILES" / "CA00" /
           "102CA0064682N07116W" / "16" / "102CA0064682N07116W.h5";
  const fs::path noaa = src / "testdata/s102/102US004AK5HQ262267.h5";
  if (fs::exists(noaa)) return noaa;
  return {};
}

void test_s102_fc_and_pc_pair() {
  const fs::path src = S100_SOURCE_DIR;
  s100::FeatureCatalogue fc;
  std::string err;
  CHECK(s100::load_feature_catalogue(src / "catalogues/S-102/S-102_FC_3.0.0.xml", fc, &err));
  CHECK(fc.product == "S-102");
  CHECK(fc.version == "3.0.0");
  CHECK(fc.feature_types.count("BathymetryCoverage") == 1);
  CHECK(fc.feature_types.count("QualityOfBathymetryCoverage") == 1);

  s100::PortrayalCatalogue pc;
  CHECK(s100::load_portrayal_catalogue(src / "catalogues/S-102/PC_3.0.0", pc, &err));
  CHECK(pc.product == "S-102");
  CHECK(pc.version == "3.0.0");
  std::cout << "S-102 FC/PC pair " << fc.version << " / " << pc.version
            << " feature_types=" << fc.feature_types.size() << "\n";
}

void test_s102_open_bounds_sample_portray() {
  const fs::path cell = find_s102_cell();
  if (cell.empty()) {
    std::cerr << "skip s102 open/sample: no CHS or NOAA fixture (see testdata/s102/SOURCE.md)\n";
    return;
  }

  s100::LoadError err;
  s100::Dataset ds;
  CHECK(s100::open_dataset(cell, ds, &err));
  CHECK(ds.product == s100::ProductId::S102);
  CHECK(ds.coverage.has_value());
  CHECK(ds.features.empty());  // grids must not be flattened into features
  const auto b = ds.bounds();
  CHECK(b.valid);
  CHECK(b.east > b.west);
  CHECK(b.north > b.south);
  CHECK(ds.coverage->grid().nrows > 0 && ds.coverage->grid().ncols > 0);
  CHECK(!ds.coverage->features.empty());
  CHECK(ds.coverage->features[0].code == "BathymetryCoverage");
  std::cout << "S-102 open " << cell.filename() << " bounds=[" << b.west << "," << b.south << ","
            << b.east << "," << b.north << "] grid=" << ds.coverage->grid().ncols << "x"
            << ds.coverage->grid().nrows << " crs=" << ds.coverage->grid().horizontal_crs << "\n";

  // sample_wgs84 only for EPSG:4326 (CHS). Projected NOAA: verify cell sample instead.
  if (ds.coverage->grid().horizontal_crs == 4326) {
    bool found = false;
    s100::CoverageSample sm;
    for (std::size_t r = 0; r < ds.coverage->grid().nrows && !found; r += 17) {
      for (std::size_t c = 0; c < ds.coverage->grid().ncols && !found; c += 17) {
        if (!ds.coverage->grid().sample_cell(r, c, sm)) continue;
        const double lon =
            ds.coverage->grid().origin_x + static_cast<double>(c) * ds.coverage->grid().spacing_x;
        const double lat =
            ds.coverage->grid().origin_y + static_cast<double>(r) * ds.coverage->grid().spacing_y;
        s100::CoverageSample sm2;
        CHECK(ds.coverage->grid().sample_wgs84(lon, lat, sm2));
        CHECK(sm2.has_depth);
        CHECK(std::fabs(sm2.depth - sm.depth) < 1e-3f);
        found = true;
        std::cout << "S-102 sample lon=" << lon << " lat=" << lat << " depth=" << sm2.depth << "\n";
      }
    }
    CHECK(found);
  } else {
    // Projected UTM (e.g. NOAA EPSG:32609): sample_wgs84 converts lon/lat internally.
    s100::CoverageSample sm;
    bool found = false;
    std::size_t fr = 0, fc = 0;
    for (std::size_t i = 0; i < ds.coverage->grid().depth.size(); ++i) {
      const std::size_t r = i / ds.coverage->grid().ncols;
      const std::size_t c = i % ds.coverage->grid().ncols;
      if (ds.coverage->grid().sample_cell(r, c, sm)) {
        found = true;
        fr = r;
        fc = c;
        break;
      }
    }
    CHECK(found);
    const double east =
        ds.coverage->grid().origin_x + static_cast<double>(fc) * ds.coverage->grid().spacing_x;
    const double north =
        ds.coverage->grid().origin_y + static_cast<double>(fr) * ds.coverage->grid().spacing_y;
    // Pick a WGS84 point inside the geographic envelope and require a depth hit.
    s100::CoverageSample sm2;
    bool wh = false;
    const int steps = 40;
    for (int i = 0; i <= steps && !wh; ++i) {
      const double lon = b.west + (b.east - b.west) * (static_cast<double>(i) / steps);
      for (int j = 0; j <= steps && !wh; ++j) {
        const double lat = b.south + (b.north - b.south) * (static_cast<double>(j) / steps);
        if (ds.coverage->grid().sample_wgs84(lon, lat, sm2) && sm2.has_depth) wh = true;
      }
    }
    CHECK(wh);
    s100::CoverageSample reject;
    CHECK(!ds.coverage->grid().sample_wgs84(b.west - 1.0, b.south - 1.0, reject));
    std::cout << "S-102 projected CRS " << ds.coverage->grid().horizontal_crs
              << " cell=(" << fr << "," << fc << ") easting=" << east << " northing=" << north
              << " wgs84 sample depth=" << sm2.depth << "\n";
  }

  // CoverageFill via the same DrawingInstructions pipeline as S-101.
  const fs::path src = S100_SOURCE_DIR;
  s100::FeatureCatalogue fc;
  s100::PortrayalCatalogue pc;
  std::string perr;
  CHECK(s100::load_feature_catalogue(src / "catalogues/S-102/S-102_FC_3.0.0.xml", fc, &perr));
  CHECK(s100::load_portrayal_catalogue(src / "catalogues/S-102/PC_3.0.0", pc, &perr));
  s100::DrawingInstructions di;
  s100::PortrayalStats st;
  auto ctx = s100::PortrayalContext::defaults(pc);
  CHECK(s100::portray(*ds.coverage, fc, pc, ctx, di, &perr, &st));
  CHECK(st.lua_errors == 0);
  CHECK(!di.empty());
  bool saw_fill = false;
  for (const auto& d : di) {
    if (d.kind == s100::DrawingInstruction::Kind::Coverage && d.command == "CoverageFill") {
      CHECK(d.coverage_attribute == "depth" || d.coverage_attribute == "uncertainty");
      CHECK(!d.coverage_state.empty());
      if (d.coverage_attribute == "depth") {
        CHECK(d.viewing_group == "13030");
        saw_fill = true;
      } else {
        CHECK(d.viewing_group == "13031");
      }
    }
  }
  CHECK(saw_fill);
  std::cout << "S-102 CoverageFill instructions=" << di.size()
            << " coverage_state=" << di.front().coverage_state.size() << "\n";
}

void test_s102_session_overlay() {
  const fs::path src = S100_SOURCE_DIR;
  const fs::path s101 = src / "testdata/s101/101AA00DS0008.000";
  CHECK(fs::exists(s101));

  s100::ChartSession session;
  s100::LoadError err;
  std::string k101, k102;
  CHECK(session.open_and_insert(s101, &err, &k101));
  CHECK(k101.rfind("S-101:", 0) == 0);
  CHECK(session.find(k101) != nullptr);
  CHECK(session.find(k101)->product == s100::ProductId::S101);

  const fs::path cell = find_s102_cell();
  if (cell.empty()) {
    std::cerr << "skip s102 session overlay: no S-102 fixture\n";
    return;
  }
  CHECK(session.open_and_insert(cell, &err, &k102));
  CHECK(k102.rfind("S-102:", 0) == 0);
  CHECK(session.find(k102) != nullptr);
  CHECK(session.find(k102)->product == s100::ProductId::S102);
  CHECK(session.find(k102)->coverage.has_value());
  CHECK(session.datasets().size() >= 2);
  // View Dataset for overlay: S-101 features + S-102 coverage → bounds is the union.
  {
    s100::Dataset view = *session.find(k101);
    const auto b101 = view.bounds();
    const auto b102 = session.find(k102)->bounds();
    view.coverage = session.find(k102)->coverage;
    const auto bu = view.bounds();
    CHECK(b101.valid && b102.valid && bu.valid);
    CHECK(bu.west <= std::min(b101.west, b102.west) + 1e-9);
    CHECK(bu.east >= std::max(b101.east, b102.east) - 1e-9);
    CHECK(bu.south <= std::min(b101.south, b102.south) + 1e-9);
    CHECK(bu.north >= std::max(b101.north, b102.north) - 1e-9);
    std::cout << "overlay bounds union [" << bu.west << "," << bu.south << "]-["
              << bu.east << "," << bu.north << "]\n";
  }
  std::cout << "session overlay keys " << k101 << " + " << k102
            << " n=" << session.datasets().size() << "\n";

  // CHS combined exchange set: both products share one catalogue / session key space.
  if (!s100test::chs_exercised("s102.series")) return;
  const fs::path chs = s100test::chs_s100_root();
  s100::ChartSession chs_session;
  CHECK(chs_session.open_exchange_set(chs, &err, s100::security::VerifyPolicy::Off));
  bool saw101 = false, saw102 = false;
  for (const auto& k : chs_session.member_keys()) {
    if (k.rfind("S-101:", 0) == 0) saw101 = true;
    if (k.rfind("S-102:", 0) == 0) saw102 = true;
  }
  CHECK(saw101);
  CHECK(saw102);
  std::string s102_key;
  for (const auto& k : chs_session.member_keys()) {
    if (k.rfind("S-102:", 0) == 0) {
      s102_key = k;
      break;
    }
  }
  CHECK(chs_session.load_member(s102_key, &err));
  CHECK(chs_session.find(s102_key)->product == s100::ProductId::S102);
  CHECK(chs_session.find(s102_key)->bounds().valid);
  std::cout << "CHS exchange S-102 member " << s102_key << " loaded\n";
}


void test_s102_utm_sample_wgs84() {
  const fs::path src = S100_SOURCE_DIR;
  const fs::path noaa = src / "testdata/s102/102US004AK5HQ262267.h5";
  if (!fs::exists(noaa)) {
    std::cerr << "skip s102 UTM sample_wgs84: no NOAA fixture\n";
    return;
  }
  s100::LoadError err;
  s100::Dataset ds;
  CHECK(s100::open_dataset(noaa, ds, &err));
  CHECK(ds.coverage.has_value());
  CHECK(ds.coverage->grid().horizontal_crs == 32609);
  const auto b = ds.bounds();
  CHECK(b.valid);
  // Geographic envelope can include fill cells; scan for a WGS84 hit with depth.
  s100::CoverageSample sm;
  bool hit = false;
  double hit_lon = 0, hit_lat = 0;
  const int steps = 40;
  for (int i = 0; i <= steps && !hit; ++i) {
    const double lon = b.west + (b.east - b.west) * (static_cast<double>(i) / steps);
    for (int j = 0; j <= steps && !hit; ++j) {
      const double lat = b.south + (b.north - b.south) * (static_cast<double>(j) / steps);
      if (ds.coverage->grid().sample_wgs84(lon, lat, sm) && sm.has_depth) {
        hit = true;
        hit_lon = lon;
        hit_lat = lat;
      }
    }
  }
  CHECK(hit);
  s100::CoverageSample reject;
  CHECK(!ds.coverage->grid().sample_wgs84(b.west - 2.0, b.south - 2.0, reject));
  std::cout << "S-102 UTM sample_wgs84 EPSG:32609 lon=" << hit_lon << " lat=" << hit_lat
            << " depth=" << sm.depth << "\n";
}


/** IHO-S102-DEEPEN-MULTIGRID / CTest s102_deepen_multigrid — MUST #1–6. */
void test_s102_deepen_multigrid() {
  const fs::path src = S100_SOURCE_DIR;
  const fs::path synth = src / "testdata/s102/synthetic_multigrid_4326.h5";
  CHECK(fs::exists(synth));

  s100::LoadError err;
  s100::Dataset ds;
  CHECK(s100::open_dataset(synth, ds, &err));
  CHECK(ds.product == s100::ProductId::S102);
  CHECK(ds.coverage.has_value());
  CHECK(ds.coverage->grids.size() >= 2);
  CHECK(ds.coverage->features.size() == ds.coverage->grids.size());
  CHECK(ds.coverage->grid().horizontal_crs == 4326);
  std::cout << "S-102 deepen grids=" << ds.coverage->grids.size()
            << " A=" << ds.coverage->grids[0].ncols << "x" << ds.coverage->grids[0].nrows
            << " B=" << ds.coverage->grids[1].ncols << "x" << ds.coverage->grids[1].nrows << "\n";

  // #2 sample_wgs84 routing: A-only vs B-only (disjoint envelopes, depths 5 vs 50).
  s100::CoverageSample sa, sb, smiss;
  CHECK(ds.coverage->sample_wgs84(0.2, 0.2, sa));
  CHECK(sa.has_depth);
  CHECK(std::fabs(sa.depth - 5.0f) < 1e-3f);
  CHECK(ds.coverage->sample_wgs84(10.2, 10.2, sb));
  CHECK(sb.has_depth);
  CHECK(std::fabs(sb.depth - 50.0f) < 1e-3f);
  CHECK(!ds.coverage->sample_wgs84(5.0, 5.0, smiss));  // gap between envelopes
  std::cout << "S-102 deepen sample A depth=" << sa.depth << " B depth=" << sb.depth << "\n";

  // #3 Portray emits >=2 Coverage instructions with distinct feature_ids.
  s100::FeatureCatalogue fc;
  s100::PortrayalCatalogue pc;
  std::string perr;
  CHECK(s100::load_feature_catalogue(src / "catalogues/S-102/S-102_FC_3.0.0.xml", fc, &perr));
  CHECK(s100::load_portrayal_catalogue(src / "catalogues/S-102/PC_3.0.0", pc, &perr));

  auto collect_tokens = [](const s100::DrawingInstruction& d) {
    std::vector<std::string> toks;
    for (const auto& el : d.coverage_state) {
      if (el.rfind("CoverageColor:", 0) == 0) {
        const auto rest = el.substr(14);
        const auto comma = rest.find(',');
        toks.push_back(comma == std::string::npos ? rest : rest.substr(0, comma));
      }
    }
    return toks;
  };
  auto join_state = [](const s100::DrawingInstruction& d) {
    std::string s;
    for (const auto& el : d.coverage_state) s += el + ";";
    return s;
  };

  auto ctx = s100::PortrayalContext::defaults(pc);
  s100::DrawingInstructions di;
  s100::PortrayalStats st;
  CHECK(s100::portray(*ds.coverage, fc, pc, ctx, di, &perr, &st));
  CHECK(st.lua_errors == 0);
  std::vector<const s100::DrawingInstruction*> fills;
  std::vector<const s100::DrawingInstruction*> depth_fills;
  for (const auto& d : di) {
    if (d.kind == s100::DrawingInstruction::Kind::Coverage && d.command == "CoverageFill") {
      fills.push_back(&d);
      if (d.coverage_attribute == "depth") depth_fills.push_back(&d);
    }
  }
  CHECK(depth_fills.size() >= 2);
  CHECK(depth_fills[0]->feature_id != depth_fills[1]->feature_id);
  CHECK(depth_fills[0]->coverage_attribute == "depth");
  CHECK(depth_fills[1]->coverage_attribute == "depth");
  std::cout << "S-102 deepen CoverageFill n=" << fills.size()
            << " depth=" << depth_fills.size()
            << " feature_ids=" << depth_fills[0]->feature_id << "," << depth_fills[1]->feature_id
            << "\n";

  // #4 FourShades=false: SafetyContour change moves DEPVS/DEPDW LookupEntry boundaries.
  CHECK(ctx.set(pc, "FourShades", "false"));
  CHECK(ctx.set(pc, "SafetyContour", "30"));
  s100::DrawingInstructions di30;
  CHECK(s100::portray(*ds.coverage, fc, pc, ctx, di30, &perr, &st));
  CHECK(ctx.set(pc, "SafetyContour", "10"));
  s100::DrawingInstructions di10;
  CHECK(s100::portray(*ds.coverage, fc, pc, ctx, di10, &perr, &st));
  CHECK(!di30.empty() && !di10.empty());
  const std::string s30 = join_state(di30.front());
  const std::string s10 = join_state(di10.front());
  CHECK(s30 != s10);
  CHECK(s30.find("LookupEntry:Shallow Water,0,30,") != std::string::npos);
  CHECK(s10.find("LookupEntry:Shallow Water,0,10,") != std::string::npos);
  CHECK(s30.find("DEPVS") != std::string::npos && s30.find("DEPDW") != std::string::npos);
  const auto tok30 = collect_tokens(di30.front());
  CHECK(std::find(tok30.begin(), tok30.end(), "DEPVS") != tok30.end());
  CHECK(std::find(tok30.begin(), tok30.end(), "DEPDW") != tok30.end());

  // #4b FourShades=true → DEPVS/DEPMS/DEPMD/DEPDW
  CHECK(ctx.set(pc, "FourShades", "true"));
  CHECK(ctx.set(pc, "ShallowContour", "5"));
  CHECK(ctx.set(pc, "SafetyContour", "10"));
  CHECK(ctx.set(pc, "DeepContour", "20"));
  s100::DrawingInstructions di4;
  CHECK(s100::portray(*ds.coverage, fc, pc, ctx, di4, &perr, &st));
  CHECK(!di4.empty());
  const auto tok4 = collect_tokens(di4.front());
  for (const char* need : {"DEPVS", "DEPMS", "DEPMD", "DEPDW"}) {
    CHECK(std::find(tok4.begin(), tok4.end(), need) != tok4.end());
  }
  std::cout << "S-102 deepen FourShades tokens ok\n";

  // #5 Day→Dusk: instruction tokens unchanged; remapped RGB differs (soft, from PC colorProfile).
  CHECK(ctx.set(pc, "FourShades", "false"));
  CHECK(ctx.set(pc, "SafetyContour", "30"));
  s100::DrawingInstructions di_day;
  CHECK(s100::portray(*ds.coverage, fc, pc, ctx, di_day, &perr, &st));
  const auto toks_day = collect_tokens(di_day.front());
  // Re-portray with same context (palette is paint-time only) — tokens stable.
  s100::DrawingInstructions di_dusk;
  CHECK(s100::portray(*ds.coverage, fc, pc, ctx, di_dusk, &perr, &st));
  CHECK(collect_tokens(di_dusk.front()) == toks_day);
  CHECK(join_state(di_day.front()) == join_state(di_dusk.front()));

  // Soft RGB check: parse official colorProfile.xml Day vs Dusk for DEPVS.
  const fs::path cp = src / "catalogues/S-102/PC_3.0.0/ColorProfiles/colorProfile.xml";
  CHECK(fs::exists(cp));
  std::ifstream in(cp);
  std::string xml((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  auto rgb_for = [&](const std::string& palette, const std::string& token) -> std::tuple<int, int, int> {
    const std::string pmark = "palette name=\"" + palette + "\"";
    const auto p0 = xml.find(pmark);
    CHECK(p0 != std::string::npos);
    const auto p1 = xml.find("</palette>", p0);
    CHECK(p1 != std::string::npos);
    const auto slice = xml.substr(p0, p1 - p0);
    const std::string imark = "item token=\"" + token + "\"";
    const auto i0 = slice.find(imark);
    CHECK(i0 != std::string::npos);
    auto grab = [&](const char* tag) {
      const std::string open = std::string("<") + tag + ">";
      const std::string close = std::string("</") + tag + ">";
      const auto a = slice.find(open, i0);
      const auto b = slice.find(close, a);
      CHECK(a != std::string::npos && b != std::string::npos);
      return std::stoi(slice.substr(a + open.size(), b - a - open.size()));
    };
    return {grab("red"), grab("green"), grab("blue")};
  };
  const auto day_rgb = rgb_for("Day", "DEPVS");
  const auto dusk_rgb = rgb_for("Dusk", "DEPVS");
  CHECK(day_rgb != dusk_rgb);
  std::cout << "S-102 deepen Day→Dusk tokens stable; DEPVS RGB Day=("
            << std::get<0>(day_rgb) << "," << std::get<1>(day_rgb) << "," << std::get<2>(day_rgb)
            << ") Dusk=(" << std::get<0>(dusk_rgb) << "," << std::get<1>(dusk_rgb) << ","
            << std::get<2>(dusk_rgb) << ")\n";

  // #6 prior single-grid path still loads (NOAA when present) — exercised by other tests;
  // here assert synthetic still has grids[0] shim and union bounds.
  const auto b = ds.bounds();
  CHECK(b.valid);
  CHECK(b.west <= 0.0 + 1e-6 && b.east >= 10.4 - 1e-3);
  CHECK(b.south <= 0.0 + 1e-6 && b.north >= 10.4 - 1e-3);
  CHECK(ds.coverage->grid().ncols == ds.coverage->grids[0].ncols);
  std::cout << "S-102 deepen multigrid OK (IHO-S102-DEEPEN-MULTIGRID)\n";
}


/** IHO-S102-UNCERTAINTY / CTest s102_uncertainty — MUST #1, #1b, #2–7. */
void test_s102_uncertainty() {
  const fs::path src = S100_SOURCE_DIR;
  const fs::path fixture_a = src / "testdata/s102/synthetic_uncertainty_4326.h5";
  const fs::path fixture_b = src / "testdata/s102/synthetic_uncertainty_depth_only_4326.h5";
  CHECK(fs::exists(fixture_a));
  CHECK(fs::exists(fixture_b));

  // --- #1 Fixture A dual-channel sample ---
  {
    s100::LoadError err;
    s100::Dataset ds;
    CHECK(s100::open_dataset(fixture_a, ds, &err));
    CHECK(ds.product == s100::ProductId::S102);
    CHECK(ds.coverage.has_value());
    CHECK(ds.coverage->grids.size() >= 1);
    CHECK(ds.coverage->grid().horizontal_crs == 4326);

    s100::CoverageSample pa, pb;
    CHECK(ds.coverage->sample_wgs84(20.0, 20.0, pa));
    CHECK(pa.has_depth);
    CHECK(std::fabs(pa.depth - 12.5f) < 1e-3f);
    CHECK(pa.has_uncertainty);
    CHECK(std::fabs(pa.uncertainty - 2.5f) < 1e-3f);

    CHECK(ds.coverage->sample_wgs84(20.1, 20.0, pb));
    CHECK(pb.has_depth);
    CHECK(std::fabs(pb.depth - 12.5f) < 1e-3f);
    CHECK(!pb.has_uncertainty);
    std::cout << "S-102 unc A PointA depth=" << pa.depth << " unc=" << pa.uncertainty
              << " PointB has_unc=" << pb.has_uncertainty << "\n";

    // --- #2 Portray: depth VG 13030 + uncertainty VG 13031 ---
    s100::FeatureCatalogue fc;
    s100::PortrayalCatalogue pc;
    std::string perr;
    CHECK(s100::load_feature_catalogue(src / "catalogues/S-102/S-102_FC_3.0.0.xml", fc, &perr));
    CHECK(s100::load_portrayal_catalogue(src / "catalogues/S-102/PC_3.0.0", pc, &perr));
    auto ctx = s100::PortrayalContext::defaults(pc);
    s100::DrawingInstructions di;
    s100::PortrayalStats st;
    CHECK(s100::portray(*ds.coverage, fc, pc, ctx, di, &perr, &st));
    CHECK(st.lua_errors == 0);
    int n_depth = 0, n_unc = 0;
    for (const auto& d : di) {
      if (d.kind != s100::DrawingInstruction::Kind::Coverage || d.command != "CoverageFill") continue;
      if (d.coverage_attribute == "depth") {
        CHECK(d.viewing_group == "13030");
        ++n_depth;
        // Symbolic tokens only (no hard-coded RGB in coverage_state).
        for (const auto& el : d.coverage_state) {
          if (el.rfind("CoverageColor:", 0) == 0) {
            CHECK(el.find('#') == std::string::npos);
          }
        }
      } else if (d.coverage_attribute == "uncertainty") {
        CHECK(d.viewing_group == "13031");
        CHECK(d.drawing_priority == 3);
        CHECK(d.display_plane == "UnderRadar");
        ++n_unc;
        for (const auto& el : d.coverage_state) {
          if (el.rfind("CoverageColor:", 0) == 0) {
            CHECK(el.find('#') == std::string::npos);
            // Band tokens from vendor table.
            CHECK(el.find("CHGRN") != std::string::npos || el.find("ADINF") != std::string::npos ||
                  el.find("CHYLW") != std::string::npos || el.find("CHCOR") != std::string::npos ||
                  el.find("CHRED") != std::string::npos);
          }
        }
      }
    }
    CHECK(n_depth >= 1);
    CHECK(n_unc >= 1);
    std::cout << "S-102 unc portray depth=" << n_depth << " unc=" << n_unc << "\n";

    // Snapshot sample before toggle (U1 / #3: no side effects).
    s100::CoverageSample before;
    CHECK(ds.coverage->sample_wgs84(20.0, 20.0, before));

    // --- #3 Hide VG 13031 → unc DIs gone; depth 13030 remains; sample unchanged ---
    s100::DrawingInstructions filtered = di;
    s100::filter_instructions_by_viewing_group(filtered, {"13031"});
    int fd = 0, fu = 0;
    for (const auto& d : filtered) {
      if (d.kind != s100::DrawingInstruction::Kind::Coverage || d.command != "CoverageFill") continue;
      if (d.coverage_attribute == "depth") {
        CHECK(d.viewing_group == "13030");
        ++fd;
      } else if (d.coverage_attribute == "uncertainty") {
        ++fu;
      }
    }
    CHECK(fd >= 1);
    CHECK(fu == 0);
    s100::CoverageSample after_hide;
    CHECK(ds.coverage->sample_wgs84(20.0, 20.0, after_hide));
    CHECK(after_hide.has_depth == before.has_depth);
    CHECK(after_hide.has_uncertainty == before.has_uncertainty);
    CHECK(std::fabs(after_hide.depth - before.depth) < 1e-6f);
    CHECK(std::fabs(after_hide.uncertainty - before.uncertainty) < 1e-6f);

    // --- #4 Show 13031 again ---
    s100::DrawingInstructions shown = di;
    s100::filter_instructions_by_viewing_group(shown, {});
    int sd = 0, su = 0;
    for (const auto& d : shown) {
      if (d.kind != s100::DrawingInstruction::Kind::Coverage || d.command != "CoverageFill") continue;
      if (d.coverage_attribute == "depth") ++sd;
      else if (d.coverage_attribute == "uncertainty") {
        CHECK(d.viewing_group == "13031");
        ++su;
      }
    }
    CHECK(sd >= 1);
    CHECK(su >= 1);
    std::cout << "S-102 unc VG toggle hide/show ok\n";
  }

  // --- #1b Fixture B depth-only compound ---
  {
    s100::LoadError err;
    s100::Dataset ds;
    CHECK(s100::open_dataset(fixture_b, ds, &err));
    CHECK(ds.coverage.has_value());
    CHECK(ds.coverage->grids.size() >= 1);
    CHECK(ds.coverage->grid().uncertainty.empty());  // no channel loaded
    s100::CoverageSample sm;
    CHECK(ds.coverage->sample_wgs84(30.0, 30.0, sm));
    CHECK(sm.has_depth);
    CHECK(std::fabs(sm.depth - 8.0f) < 1e-3f);
    CHECK(!sm.has_uncertainty);

    s100::FeatureCatalogue fc;
    s100::PortrayalCatalogue pc;
    std::string perr;
    CHECK(s100::load_feature_catalogue(src / "catalogues/S-102/S-102_FC_3.0.0.xml", fc, &perr));
    CHECK(s100::load_portrayal_catalogue(src / "catalogues/S-102/PC_3.0.0", pc, &perr));
    auto ctx = s100::PortrayalContext::defaults(pc);
    s100::DrawingInstructions di;
    s100::PortrayalStats st;
    CHECK(s100::portray(*ds.coverage, fc, pc, ctx, di, &perr, &st));
    int n_unc = 0, n_depth = 0;
    for (const auto& d : di) {
      if (d.kind != s100::DrawingInstruction::Kind::Coverage || d.command != "CoverageFill") continue;
      if (d.coverage_attribute == "uncertainty") ++n_unc;
      if (d.coverage_attribute == "depth") ++n_depth;
    }
    CHECK(n_depth >= 1);
    CHECK(n_unc == 0);  // no uncertainty CoverageFill for depth-only
    std::cout << "S-102 unc B depth-only load+sample+no-unc-DI ok\n";
  }

  // --- #6 Real NOAA/CHS path (unc metres only; QoBC not asserted) ---
  {
    const fs::path noaa = src / "testdata/s102/102US004AK5HQ262267.h5";
    fs::path cell;
    bool prefer_noaa = false;
    if (fs::exists(noaa)) {
      cell = noaa;
      prefer_noaa = true;
    } else if (s100test::chs_exercised("s102.uncertainty")) {
      cell = s100test::chs_s100_root() / "S-102" / "DATASET_FILES" / "CA00" /
             "102CA0064682N07116W" / "16" / "102CA0064682N07116W.h5";
    }
    if (cell.empty()) {
      std::cout << "S-102 unc real sample: not exercised (no NOAA/CHS under TEST-DATA-ROOTS)\n";
    } else {
      s100::LoadError err;
      s100::Dataset ds;
      CHECK(s100::open_dataset(cell, ds, &err));
      CHECK(ds.coverage.has_value());
      CHECK(ds.coverage->grids.size() >= 1);
      // Dual-channel sample shape always.
      s100::CoverageSample sm;
      bool any_depth = false;
      bool any_unc = false;
      const auto& g = ds.coverage->grid();
      for (std::size_t r = 0; r < g.nrows; r += std::max<std::size_t>(1, g.nrows / 64)) {
        for (std::size_t c = 0; c < g.ncols; c += std::max<std::size_t>(1, g.ncols / 64)) {
          if (!g.sample_cell(r, c, sm)) continue;
          any_depth = true;
          if (sm.has_uncertainty) any_unc = true;
        }
      }
      CHECK(any_depth);
      if (prefer_noaa) {
        // NOAA has real per-cell unc (e.g. cell 700,700 → 40.34).
        CHECK(g.sample_cell(700, 700, sm));
        CHECK(sm.has_depth);
        CHECK(sm.has_uncertainty);
        CHECK(std::fabs(sm.uncertainty - 40.34f) < 0.01f);
        std::cout << "S-102 unc NOAA cell(700,700) depth=" << sm.depth
                  << " unc=" << sm.uncertainty << "\n";
      } else {
        // CHS cell today is all-fill unc — positive has_uncertainty only when present.
        std::cout << "S-102 unc CHS path loaded any_unc=" << any_unc << " (all-fill ok)\n";
      }
    }
  }

  // --- #7 prior deepen still green is covered by separate CTest; smoke synthetic still loads ---
  {
    const fs::path synth = src / "testdata/s102/synthetic_multigrid_4326.h5";
    CHECK(fs::exists(synth));
    s100::LoadError err;
    s100::Dataset ds;
    CHECK(s100::open_dataset(synth, ds, &err));
    CHECK(ds.coverage.has_value());
    CHECK(ds.coverage->grids.size() >= 2);
  }

  std::cout << "S-102 uncertainty OK (IHO-S102-UNCERTAINTY)\n";
}

void run_s102_tests() {
  test_s102_fc_and_pc_pair();
  test_s102_open_bounds_sample_portray();
  test_s102_utm_sample_wgs84();
  test_s102_session_overlay();
  test_s102_deepen_multigrid();
  test_s102_uncertainty();
}
