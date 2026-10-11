#include "s100/current.hpp"
#include "s100/dataset.hpp"
#include "s100/hdf5_product.hpp"
#include "s100/session.hpp"

#include <cmath>
#include <cstdlib>
#include <filesystem>
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

/** IHO-S111-OPEN-FRAME / CTest s111_open_frame — MUST rows from docs/s111-contract.md. */
void test_s111_open_frame() {
  const fs::path src = S100_SOURCE_DIR;
  const fs::path synth = src / "testdata/s111/synthetic_current_4326.h5";
  CHECK(fs::exists(synth));

  s100::LoadError err;
  s100::Dataset ds;
  CHECK(s100::open_dataset(synth, ds, &err));
  CHECK(ds.product == s100::ProductId::S111);
  CHECK(ds.current.has_value());
  CHECK(!ds.coverage.has_value());
  CHECK(!ds.water_level.has_value());
  CHECK(ds.current->frames.size() >= 2);
  CHECK(ds.current->frames[0].speed.horizontal_crs == 4326);
  std::cout << "S-111 open frames=" << ds.current->frames.size()
            << " times=" << ds.current->frames[0].time_iso << ","
            << ds.current->frames[1].time_iso << "\n";

  // set_active_frame OOR → false, no change
  CHECK(ds.current->set_active_frame(0));
  CHECK(ds.current->active_frame == 0);
  CHECK(!ds.current->set_active_frame(99));
  CHECK(ds.current->active_frame == 0);

  // set_active_time nearest |Δt|; ties → lower index
  CHECK(ds.current->set_active_time("2024-01-01T05:30:00Z"));
  CHECK(ds.current->active_frame == 1);
  CHECK(ds.current->set_active_time("2024-01-01T00:00:00Z"));
  CHECK(ds.current->active_frame == 0);
  // Exact midpoint 03:00 → |Δt| equal → lower index (0)
  CHECK(ds.current->set_active_time("2024-01-01T03:00:00Z"));
  CHECK(ds.current->active_frame == 0);

  // sample_wgs84 oracle at (0.2, 0.2)
  constexpr double kLon = 0.2, kLat = 0.2;
  CHECK(ds.current->set_active_frame(0));
  s100::CurrentSample s0, s1;
  CHECK(ds.current->sample_wgs84(kLon, kLat, s0));
  CHECK(s0.valid);
  CHECK(std::fabs(s0.speed_ms - 0.5f) < 1e-3f);
  CHECK(std::fabs(s0.direction_deg - 90.f) < 1e-3f);
  CHECK(ds.current->set_active_frame(1));
  CHECK(ds.current->sample_wgs84(kLon, kLat, s1));
  CHECK(s1.valid);
  CHECK(std::fabs(s1.speed_ms - 1.5f) < 1e-3f);
  CHECK(std::fabs(s1.direction_deg - 180.f) < 1e-3f);
  std::cout << "S-111 sample (0.2,0.2) frame0=(" << s0.speed_ms << "," << s0.direction_deg
            << ") frame1=(" << s1.speed_ms << "," << s1.direction_deg << ")\n";

  const auto b = ds.bounds();
  CHECK(b.valid);
  CHECK(b.west <= 0.0 + 1e-6 && b.east >= 0.4 - 1e-3);

  // Portray active frame: Arrow count follows stride; ≥1 Arrow
  CHECK(ds.current->set_active_frame(0));
  s100::DrawingInstructions di0;
  std::string perr;
  CHECK(s100::portray_current(*ds.current, di0, &perr));
  CHECK(!di0.empty());
  std::size_t arrow_n = 0;
  for (const auto& d : di0) {
    if (d.kind == s100::DrawingInstruction::Kind::Arrow) {
      ++arrow_n;
      CHECK(d.command == "Arrow" || d.symbol_ref == "Arrow");
      CHECK(!d.geometries.empty());
      CHECK(d.geometries[0].kind == s100::GeometryKind::Point);
      CHECK(d.fill_ref == "CHGRD" || !d.fill_ref.empty());
    }
  }
  CHECK(arrow_n >= 1);
  CHECK(arrow_n == di0.size());
  // 5×5 stride 2 edges inclusive → rows/cols {0,2,4} → 9
  CHECK(arrow_n == 9);
  CHECK(std::fabs(di0[0].rotation - 90.0) < 1e-3);
  CHECK(std::fabs(di0[0].scale_factor - 0.5) < 1e-3);

  CHECK(ds.current->set_active_frame(1));
  s100::DrawingInstructions di1;
  CHECK(s100::portray_current(*ds.current, di1, &perr));
  CHECK(di1.size() == arrow_n);
  CHECK(std::fabs(di1[0].rotation - 180.0) < 1e-3);
  CHECK(std::fabs(di1[0].scale_factor - 1.5) < 1e-3);
  std::cout << "S-111 Arrow count=" << arrow_n << " (kS111ArrowStride=" << s100::kS111ArrowStride
            << ")\n";

  // ChartSession S-101 + S-111
  const fs::path s101 = src / "testdata/s101/101AA00DS0008.000";
  CHECK(fs::exists(s101));
  s100::ChartSession session;
  std::string k101, k111;
  CHECK(session.open_and_insert(s101, &err, &k101));
  CHECK(k101.rfind("S-101:", 0) == 0);
  CHECK(session.open_and_insert(synth, &err, &k111));
  CHECK(k111.rfind("S-111:", 0) == 0);
  CHECK(session.find(k111) != nullptr);
  CHECK(session.find(k111)->current.has_value());
  CHECK(session.find(k111)->product == s100::ProductId::S111);
  std::cout << "S-111 session overlay " << k101 << " + " << k111 << "\n";

  // Wrong product → current empty (S-102 / S-104)
  const fs::path s102 = src / "testdata/s102/synthetic_multigrid_4326.h5";
  CHECK(fs::exists(s102));
  {
    s100::Dataset ds102;
    CHECK(s100::open_dataset(s102, ds102, &err));
    CHECK(ds102.product == s100::ProductId::S102);
    CHECK(ds102.coverage.has_value());
    CHECK(!ds102.current.has_value());
  }
  const fs::path s104 = src / "testdata/s104/synthetic_waterlevel_4326.h5";
  CHECK(fs::exists(s104));
  {
    s100::Dataset ds104;
    CHECK(s100::open_dataset(s104, ds104, &err));
    CHECK(ds104.product == s100::ProductId::S104);
    CHECK(ds104.water_level.has_value());
    CHECK(!ds104.current.has_value());
  }
  // Direct load_s111 on wrong product → LoadError
  {
    s100::CurrentDataset bad;
    s100::LoadError e2;
    CHECK(!s100::load_s111(s104, bad, &e2));
    CHECK(bad.frames.empty());
  }

  std::cout << "S-111 open_frame OK (IHO-S111-OPEN-FRAME)\n";
}

void run_s111_tests() { test_s111_open_frame(); }
