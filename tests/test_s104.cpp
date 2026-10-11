#include "s100/dataset.hpp"
#include "s100/hdf5_product.hpp"
#include "s100/session.hpp"
#include "s100/water_level.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#define CHECK(cond)                                                                \
  do {                                                                             \
    if (!(cond)) {                                                                 \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n"; \
      std::exit(1);                                                                \
    }                                                                              \
  } while (0)

namespace fs = std::filesystem;

/** IHO-S104-OPEN-FRAME / CTest s104_open_frame — MUST rows from docs/s104-contract.md. */
void test_s104_open_frame() {
  const fs::path src = S100_SOURCE_DIR;
  const fs::path synth = src / "testdata/s104/synthetic_waterlevel_4326.h5";
  CHECK(fs::exists(synth));

  s100::LoadError err;
  s100::Dataset ds;
  CHECK(s100::open_dataset(synth, ds, &err));
  CHECK(ds.product == s100::ProductId::S104);
  CHECK(ds.water_level.has_value());
  CHECK(!ds.coverage.has_value());  // must not reuse coverage for S-104
  CHECK(ds.water_level->frames.size() >= 2);
  CHECK(ds.water_level->frames[0].grid.horizontal_crs == 4326);
  std::cout << "S-104 open frames=" << ds.water_level->frames.size()
            << " times=" << ds.water_level->frames[0].time_iso << ","
            << ds.water_level->frames[1].time_iso << "\n";

  // set_active_frame / set_active_time
  CHECK(ds.water_level->set_active_frame(0));
  CHECK(ds.water_level->active_frame == 0);
  CHECK(!ds.water_level->set_active_frame(99));
  CHECK(ds.water_level->active_frame == 0);
  CHECK(ds.water_level->set_active_time("2024-01-01T05:30:00Z"));
  CHECK(ds.water_level->active_frame == 1);  // nearer to 06:00 than 00:00
  CHECK(ds.water_level->set_active_time("2024-01-01T00:00:00Z"));
  CHECK(ds.water_level->active_frame == 0);

  // sample_wgs84 at documented point (0.2, 0.2) differs frame0 vs frame1
  constexpr double kLon = 0.2, kLat = 0.2;
  CHECK(ds.water_level->set_active_frame(0));
  s100::CoverageSample s0, s1;
  CHECK(ds.water_level->sample_wgs84(kLon, kLat, s0));
  CHECK(s0.has_depth);
  CHECK(std::fabs(s0.depth - 1.0f) < 1e-3f);
  CHECK(ds.water_level->set_active_frame(1));
  CHECK(ds.water_level->sample_wgs84(kLon, kLat, s1));
  CHECK(s1.has_depth);
  CHECK(std::fabs(s1.depth - 2.5f) < 1e-3f);
  CHECK(std::fabs(s0.depth - s1.depth) > 0.5f);
  std::cout << "S-104 sample (0.2,0.2) frame0=" << s0.depth << " frame1=" << s1.depth << "\n";

  const auto b = ds.bounds();
  CHECK(b.valid);
  CHECK(b.west <= 0.0 + 1e-6 && b.east >= 0.4 - 1e-3);

  // Portray ≥1 CoverageFill for active frame; tokens change across frames
  auto collect_tokens = [](const s100::DrawingInstructions& di) {
    std::vector<std::string> toks;
    for (const auto& d : di) {
      for (const auto& el : d.coverage_state) {
        if (el.rfind("CoverageColor:", 0) == 0) {
          const auto rest = el.substr(14);
          const auto comma = rest.find(',');
          toks.push_back(comma == std::string::npos ? rest : rest.substr(0, comma));
        }
      }
    }
    return toks;
  };
  auto join_state = [](const s100::DrawingInstructions& di) {
    std::string s;
    for (const auto& d : di)
      for (const auto& el : d.coverage_state) s += el + ";";
    return s;
  };

  CHECK(ds.water_level->set_active_frame(0));
  s100::DrawingInstructions di0;
  std::string perr;
  CHECK(s100::portray_water_level(*ds.water_level, di0, &perr));
  CHECK(!di0.empty());
  CHECK(di0[0].kind == s100::DrawingInstruction::Kind::Coverage);
  CHECK(di0[0].command == "CoverageFill");
  CHECK(di0[0].coverage_attribute == "depth");

  CHECK(ds.water_level->set_active_frame(1));
  s100::DrawingInstructions di1;
  CHECK(s100::portray_water_level(*ds.water_level, di1, &perr));
  CHECK(!di1.empty());
  const auto tok0 = collect_tokens(di0);
  const auto tok1 = collect_tokens(di1);
  CHECK(!tok0.empty() && !tok1.empty());
  CHECK(tok0.front() != tok1.front());  // active-level token changes with frame
  CHECK(join_state(di0) != join_state(di1));
  std::cout << "S-104 portray tokens frame0=" << tok0.front() << " frame1=" << tok1.front()
            << "\n";

  // Day→Dusk: tokens stable for same frame (palette is paint-time only)
  s100::DrawingInstructions di_again;
  CHECK(s100::portray_water_level(*ds.water_level, di_again, &perr));
  CHECK(collect_tokens(di_again) == tok1);
  std::cout << "S-104 Day→Dusk tokens stable (same frame re-portray)\n";

  // ChartSession S-101 + S-104 overlay
  const fs::path s101 = src / "testdata/s101/101AA00DS0008.000";
  CHECK(fs::exists(s101));
  s100::ChartSession session;
  std::string k101, k104;
  CHECK(session.open_and_insert(s101, &err, &k101));
  CHECK(k101.rfind("S-101:", 0) == 0);
  CHECK(session.open_and_insert(synth, &err, &k104));
  CHECK(k104.rfind("S-104:", 0) == 0);
  CHECK(session.find(k104) != nullptr);
  CHECK(session.find(k104)->water_level.has_value());
  CHECK(session.find(k104)->product == s100::ProductId::S104);
  std::cout << "S-104 session overlay " << k101 << " + " << k104 << "\n";

  // Wrong product claim: S-102 file must not fill water_level
  const fs::path s102 = src / "testdata/s102/synthetic_multigrid_4326.h5";
  if (fs::exists(s102)) {
    s100::Dataset ds102;
    CHECK(s100::open_dataset(s102, ds102, &err));
    CHECK(ds102.product == s100::ProductId::S102);
    CHECK(ds102.coverage.has_value());
    CHECK(!ds102.water_level.has_value());
  }

  std::cout << "S-104 open_frame OK (IHO-S104-OPEN-FRAME)\n";
}

void run_s104_tests() { test_s104_open_frame(); }
