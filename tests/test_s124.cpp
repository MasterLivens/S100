#include "s100/dataset.hpp"
#include "s100/nav_warning.hpp"
#include "s100/session.hpp"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
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

static bool near(double a, double b, double eps = 1e-9) { return std::fabs(a - b) < eps; }

/** IHO-S124-OPEN-LIST / CTest s124_open_list — MUST rows from docs/s124-contract.md. */
void test_s124_open_list() {
  const fs::path src = S100_SOURCE_DIR;
  const fs::path fixture = src / "testdata/s124/synthetic_navwarning.gml";
  CHECK(fs::exists(fixture));

  s100::LoadError err;
  s100::Dataset ds;
  CHECK(s100::open_dataset(fixture, ds, &err));
  CHECK(ds.product == s100::ProductId::S124);
  CHECK(ds.nav_warnings.has_value());
  CHECK(!ds.coverage.has_value());
  CHECK(!ds.water_level.has_value());
  CHECK(!ds.current.has_value());
  CHECK(ds.features.empty());  // no Feature mirroring
  CHECK(ds.nav_warnings->warnings.size() == 2);

  const auto* nw1 = ds.nav_warnings->find("NW.1");
  const auto* nw2 = ds.nav_warnings->find("NW.2");
  CHECK(nw1 != nullptr);
  CHECK(nw2 != nullptr);
  CHECK(nw1->title == "Synthetic Hazard Alpha");
  CHECK(nw2->title == "Synthetic Area Bravo");
  CHECK(nw1->time_iso == "2024-01-01T00:00:00Z");
  CHECK(nw2->time_iso == "2024-01-01T06:00:00Z");

  // NW.1 point lon=0.2 lat=0.2
  CHECK(nw1->geometries.size() >= 1);
  CHECK(nw1->geometries[0].kind == s100::GeometryKind::Point);
  CHECK(near(nw1->geometries[0].point.lon, 0.2));
  CHECK(near(nw1->geometries[0].point.lat, 0.2));

  // NW.2 ring exact posList
  CHECK(nw2->geometries.size() >= 1);
  CHECK(nw2->geometries[0].kind == s100::GeometryKind::Polygon);
  CHECK(!nw2->geometries[0].polygon.empty());
  const auto& ring = nw2->geometries[0].polygon.front().points;
  CHECK(ring.size() == 5);
  const double expect[][2] = {{0.0, 0.0}, {0.4, 0.0}, {0.4, 0.4}, {0.0, 0.4}, {0.0, 0.0}};
  for (std::size_t i = 0; i < 5; ++i) {
    CHECK(near(ring[i].lon, expect[i][0]));
    CHECK(near(ring[i].lat, expect[i][1]));
  }

  // Dataset::bounds intersects fixture envelope [0,0]–[0.4,0.4]
  const auto b = ds.bounds();
  CHECK(b.valid);
  CHECK(b.west <= 0.0 + 1e-6);
  CHECK(b.south <= 0.0 + 1e-6);
  CHECK(b.east >= 0.4 - 1e-6);
  CHECK(b.north >= 0.4 - 1e-6);
  const auto nwb = ds.nav_warnings->bounds();
  CHECK(nwb.valid);
  CHECK(near(nwb.west, b.west) && near(nwb.east, b.east));

  std::cout << "S-124 open warnings=" << ds.nav_warnings->warnings.size()
            << " ids=" << nw1->id << "," << nw2->id << "\n";

  // Portray: ≥2 geom + ≥2 Text
  s100::DrawingInstructions di;
  std::string perr;
  CHECK(s100::portray_nav_warnings(*ds.nav_warnings, di, &perr));
  std::size_t geom_n = 0, text_n = 0;
  for (const auto& d : di) {
    if (d.kind == s100::DrawingInstruction::Kind::Text) {
      ++text_n;
      CHECK(!d.text.empty());
      CHECK(d.fill_ref == "CHYLW" || d.text_style.font_colour == "CHYLW");
    } else if (d.kind == s100::DrawingInstruction::Kind::Point ||
               d.kind == s100::DrawingInstruction::Kind::Line ||
               d.kind == s100::DrawingInstruction::Kind::Area) {
      ++geom_n;
      CHECK(d.fill_ref == "CHYLW" || (!d.line_styles.empty() && d.line_styles[0].colour == "CHYLW"));
      CHECK(!d.geometries.empty());
    }
  }
  CHECK(geom_n >= 2);
  CHECK(text_n >= 2);
  std::cout << "S-124 portray geom=" << geom_n << " text=" << text_n << " total_di=" << di.size()
            << "\n";

  // ChartSession S-101 + S-124
  const fs::path s101 = src / "testdata/s101/101AA00DS0008.000";
  CHECK(fs::exists(s101));
  s100::ChartSession session;
  std::string k101, k124;
  CHECK(session.open_and_insert(s101, &err, &k101));
  CHECK(k101.rfind("S-101:", 0) == 0);
  CHECK(session.open_and_insert(fixture, &err, &k124));
  CHECK(k124.rfind("S-124:", 0) == 0);
  CHECK(k124 == "S-124:synthetic_navwarning");
  CHECK(session.find(k124) != nullptr);
  CHECK(session.find(k124)->nav_warnings.has_value());
  CHECK(session.find(k124)->product == s100::ProductId::S124);
  std::cout << "S-124 session overlay " << k101 << " + " << k124 << "\n";

  // Empty / whitespace → Invalid, nav_warnings empty
  {
    const fs::path empty_path = src / "testdata/s124/_empty_tmp.gml";
    {
      std::ofstream(empty_path) << "   \n\t  \n";
    }
    s100::Dataset bad;
    s100::LoadError e2;
    CHECK(!s100::open_dataset(empty_path, bad, &e2));
    CHECK(e2.kind == s100::LoadErrorKind::Invalid);
    CHECK(!bad.nav_warnings.has_value());
    fs::remove(empty_path);

    s100::NavWarningDataset nw;
    s100::LoadError e3;
    CHECK(!s100::load_s124(fixture.parent_path() / "_missing_no_such.gml", nw, &e3));
    // Io or Invalid acceptable for missing; ensure warnings empty
    CHECK(nw.warnings.empty());
  }

  // Well-formed XML, zero NavigationalWarning → Parse
  {
    const fs::path zero_path = src / "testdata/s124/_zero_nw_tmp.gml";
    {
      std::ofstream o(zero_path);
      o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<S124:Dataset xmlns:S124=\"http://www.iho.int/S124/2.0\" productId=\"S-124\">\n"
        << "</S124:Dataset>\n";
    }
    s100::Dataset z;
    s100::LoadError ez;
    CHECK(!s100::open_dataset(zero_path, z, &ez));
    CHECK(ez.kind == s100::LoadErrorKind::Parse);
    CHECK(ez.message.find("zero") != std::string::npos ||
          ez.message.find("Zero") != std::string::npos ||
          ez.message.find("NavigationalWarning") != std::string::npos);
    CHECK(!z.nav_warnings.has_value());
    fs::remove(zero_path);
  }

  // Wrong product → error; must NOT fill nav_warnings
  {
    const fs::path wrong_path = src / "testdata/s124/_wrong_product_tmp.gml";
    {
      std::ofstream o(wrong_path);
      o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<S127:Dataset xmlns:S127=\"http://www.iho.int/S127/1.0\" productId=\"S-127\">\n"
        << "  <S127:Something/>\n"
        << "</S127:Dataset>\n";
    }
    s100::Dataset w;
    s100::LoadError ew;
    CHECK(!s100::open_dataset(wrong_path, w, &ew));
    CHECK(ew.kind == s100::LoadErrorKind::Invalid || ew.kind == s100::LoadErrorKind::Parse);
    CHECK(!w.nav_warnings.has_value());

    s100::NavWarningDataset nw;
    s100::LoadError e4;
    CHECK(!s100::load_s124(wrong_path, nw, &e4));
    CHECK(nw.warnings.empty());
    fs::remove(wrong_path);
  }

  std::cout << "S-124 open_list OK (IHO-S124-OPEN-LIST)\n";
}

void run_s124_tests() { test_s124_open_list(); }
