// IHO-PC-WIRE-111-124 / CTest pc_wire_111_124 — docs/pc-wire-contract.md rev 2.
#include "s100/current.hpp"
#include "s100/dataset.hpp"
#include "s100/feature_catalogue.hpp"
#include "s100/host_adapter.hpp"
#include "s100/nav_warning.hpp"
#include "s100/portrayal_catalogue.hpp"
#include "s100/portrayal_engine.hpp"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

#define CHECK(cond)                                                                \
  do {                                                                             \
    if (!(cond)) {                                                                 \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n"; \
      std::exit(1);                                                                \
    }                                                                              \
  } while (0)

namespace fs = std::filesystem;

static bool is_scarow_id(const std::string& s) {
  if (s.size() != 8 || s.compare(0, 6, "SCAROW") != 0) return false;
  if (s[6] != '0') return false;
  return s[7] >= '1' && s[7] <= '9';
}

void test_pc_wire_111_124() {
  const fs::path src = S100_SOURCE_DIR;

  // -------------------- S-124 --------------------
  const fs::path fc124 = src / "catalogues/S-124/S-124_FC_2.0.0.xml";
  const fs::path pc124 = src / "catalogues/S-124/PC_2.0.0";
  const fs::path gml = src / "testdata/s124/synthetic_navwarning.gml";
  CHECK(fs::exists(fc124));
  CHECK(fs::exists(pc124 / "portrayal_catalogue.xml"));
  CHECK(fs::exists(gml));

  s100::FeatureCatalogue fc_s124;
  s100::PortrayalCatalogue pc_s124;
  std::string e;
  CHECK(s100::load_feature_catalogue(fc124, fc_s124, &e));
  CHECK(s100::load_portrayal_catalogue(pc124, pc_s124, &e));
  CHECK(pc_s124.product.find("124") != std::string::npos || pc_s124.product == "S-124");

  s100::Dataset ds124;
  s100::LoadError lerr;
  CHECK(s100::open_dataset(gml, ds124, &lerr));
  CHECK(ds124.nav_warnings.has_value());
  CHECK(ds124.nav_warnings->warnings.size() == 2);

  // Registry product name
  {
    s100::S124HostAdapter host(*ds124.nav_warnings, fc_s124);
    CHECK(host.product() == "S-124");
    CHECK(host.feature_count() == 2);
    CHECK(host.registry().has("HostGetFeatureIDs"));
    CHECK(host.registry().has("HostPortrayalEmit"));
  }

  s100::DrawingInstructions di124;
  s100::PortrayalStats st124;
  e.clear();
  const bool pc_ok =
      s100::portray_nav_warnings_pc(*ds124.nav_warnings, fc_s124, pc_s124, di124, &st124, &e);
  if (!pc_ok) {
    std::cerr << "portray_nav_warnings_pc failed: " << e << "\n";
    for (const auto& m : st124.errors) std::cerr << "  lua: " << m << "\n";
  }
  CHECK(pc_ok);
  CHECK(!di124.empty());

  // Per warning: ≥1 non-Null DI or explicit Null
  std::set<std::uint64_t> seen_non_null, seen_null;
  bool any_chylw_heuristic = false;
  for (const auto& d : di124) {
    if (d.kind == s100::DrawingInstruction::Kind::Null)
      seen_null.insert(d.feature_id);
    else
      seen_non_null.insert(d.feature_id);
    // Heuristic CHYLW path is not the default when PC is present.
    if (d.fill_ref == "CHYLW" && d.viewing_group == "21010" &&
        d.kind != s100::DrawingInstruction::Kind::Text) {
      // PC uses 31020 / CHMGD / NAVWARN* — CHYLW+21010 is heuristic signature.
      any_chylw_heuristic = true;
    }
  }
  CHECK(!any_chylw_heuristic);
  for (std::uint64_t fid = 1; fid <= 2; ++fid) {
    CHECK(seen_non_null.count(fid) || seen_null.count(fid));
  }
  std::cout << "S-124 PC wire DI=" << di124.size() << " non_null_features=" << seen_non_null.size()
            << " emitted=" << st124.emitted_features << " lua_errors=" << st124.lua_errors << "\n";
  for (std::size_t i = 0; i < std::min<std::size_t>(di124.size(), 6); ++i) {
    const auto& d = di124[i];
    std::cout << "  DI[" << i << "] fid=" << d.feature_id << " kind=" << static_cast<int>(d.kind)
              << " cmd=" << d.command << " symbol=" << d.symbol_ref << " fill=" << d.fill_ref
              << " text=" << d.text << "\n";
  }

  // -------------------- S-111 --------------------
  const fs::path fc111 = src / "catalogues/S-111/S-111_FC_2.0.0.xml";
  const fs::path pc111 = src / "catalogues/S-111/PC_2.0.0";
  const fs::path h5 = src / "testdata/s111/synthetic_current_4326.h5";
  CHECK(fs::exists(fc111));
  CHECK(fs::exists(pc111 / "portrayal_catalogue.xml"));
  CHECK(fs::exists(h5));

  s100::FeatureCatalogue fc_s111;
  s100::PortrayalCatalogue pc_s111;
  e.clear();
  CHECK(s100::load_feature_catalogue(fc111, fc_s111, &e));
  CHECK(s100::load_portrayal_catalogue(pc111, pc_s111, &e));

  // ColorProfiles Day/Dusk/Night present
  const fs::path color_xml = pc111 / "ColorProfiles" / "colorProfile.xml";
  CHECK(fs::exists(color_xml));
  {
    std::ifstream in(color_xml);
    CHECK(in.good());
    std::string body((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    CHECK(body.find("palette name=\"Day\"") != std::string::npos ||
          body.find("name=\"Day\"") != std::string::npos);
    CHECK(body.find("Dusk") != std::string::npos);
    CHECK(body.find("Night") != std::string::npos);
  }

  s100::Dataset ds111;
  CHECK(s100::open_dataset(h5, ds111, &lerr));
  CHECK(ds111.current.has_value());

  auto check_frame = [&](std::size_t frame, const char* expect_ref) {
    CHECK(ds111.current->set_active_frame(frame));
    s100::DrawingInstructions di;
    std::string perr;
    CHECK(s100::portray_current(*ds111.current, di, &perr));
    CHECK(!di.empty());
    for (const auto& d : di) {
      CHECK(d.kind == s100::DrawingInstruction::Kind::Arrow);
      CHECK(is_scarow_id(d.symbol_ref));
      const fs::path sym = pc111 / "Symbols" / (d.symbol_ref + ".svg");
      CHECK(fs::exists(sym));
    }
    // Active-frame band oracle at fixture speeds (all non-zero cells same speed).
    CHECK(di.front().symbol_ref == expect_ref);
    std::cout << "S-111 frame" << frame << " arrows=" << di.size() << " symbol_ref="
              << di.front().symbol_ref << " (expect " << expect_ref << ")\n";
  };
  check_frame(0, "SCAROW02");  // 0.5 m/s → [0.50, 1.00)
  check_frame(1, "SCAROW03");  // 1.5 m/s → [1.00, 2.00)

  // Band helper spot checks (select_arrow.xsl mirror)
  CHECK(s100::scarow_symbol_ref(0.f).empty());
  CHECK(s100::scarow_symbol_ref(0.25f) == "SCAROW01");
  CHECK(s100::scarow_symbol_ref(0.5f) == "SCAROW02");
  CHECK(s100::scarow_symbol_ref(1.5f) == "SCAROW03");
  CHECK(s100::scarow_symbol_ref(13.f) == "SCAROW09");

  // No S-111 *.lua portrayal rules (XSLT PC only; forbidden to invent Lua HostAdapter).
  {
    const fs::path root = src / "catalogues/S-111";
    for (auto it = fs::recursive_directory_iterator(root); it != fs::recursive_directory_iterator();
         ++it) {
      if (!it->is_regular_file()) continue;
      const auto ext = it->path().extension().string();
      if (ext == ".lua" || ext == ".Lua") {
        std::cerr << "forbidden S-111 lua under catalogues: " << it->path() << "\n";
        CHECK(false);
      }
    }
    // src must not grow an S-111 Lua portrayal rules tree.
    for (auto it = fs::recursive_directory_iterator(src / "src"); it != fs::recursive_directory_iterator();
         ++it) {
      if (!it->is_regular_file()) continue;
      const std::string pth = it->path().string();
      if ((it->path().extension() == ".lua") &&
          (pth.find("s111") != std::string::npos || pth.find("S-111") != std::string::npos ||
           pth.find("S111") != std::string::npos)) {
        std::cerr << "forbidden S-111 lua under src: " << it->path() << "\n";
        CHECK(false);
      }
    }
  }

  std::cout << "pc_wire_111_124 OK (IHO-PC-WIRE-111-124)\n";
}

void run_pc_wire_111_124_tests() { test_pc_wire_111_124(); }
