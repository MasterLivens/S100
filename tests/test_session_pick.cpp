#include "s100/dataset.hpp"
#include "s100/pick.hpp"
#include "s100/session.hpp"

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

/** IHO-SESSION-PICK-UNIFIED / CTest session_pick_unified — docs/session-pick-contract.md. */
void test_session_pick_unified() {
  const fs::path src = S100_SOURCE_DIR;
  const fs::path s101 = src / "testdata/s101/101AA00DS0008.000";
  const fs::path s124 = src / "testdata/s124/synthetic_navwarning.gml";
  const fs::path s102 = src / "testdata/s102/synthetic_multigrid_4326.h5";
  const fs::path s104 = src / "testdata/s104/synthetic_waterlevel_4326.h5";
  const fs::path s111 = src / "testdata/s111/synthetic_current_4326.h5";
  CHECK(fs::exists(s101));
  CHECK(fs::exists(s124));
  CHECK(fs::exists(s102));
  CHECK(fs::exists(s104));
  CHECK(fs::exists(s111));

  s100::LoadError err;

  // --- Oracle A: S-101 0008 only → ≥1 Feature at known land point ---
  {
    s100::ChartSession session;
    CHECK(session.open_and_insert(s101, &err));
    // LandRegion id=6 point (62.6722, -32.5291) from fixture cell
    constexpr double kLon = 62.6721855, kLat = -32.5290868;
    s100::SessionPickResult r;
    CHECK(s100::chart_session_pick(session, kLon, kLat, r, nullptr));
    std::size_t feat_n = 0;
    for (const auto& h : r.hits) {
      if (h.kind == s100::SessionPickHit::Kind::Feature) {
        ++feat_n;
        CHECK(h.product == s100::ProductId::S101);
        CHECK(!h.numeric_valid);
        CHECK(h.feature_id > 0);
      }
    }
    CHECK(feat_n >= 1);
    std::cout << "oracle A: Feature hits=" << feat_n << " total=" << r.hits.size() << "\n";
  }

  // --- Oracle B: S-124 synthetic only @ (0.2,0.2) → NW.1 and NW.2 ---
  {
    s100::ChartSession session;
    CHECK(session.open_and_insert(s124, &err));
    s100::SessionPickResult r;
    CHECK(s100::chart_session_pick(session, 0.2, 0.2, r, nullptr));
    bool saw_nw1 = false, saw_nw2 = false;
    std::vector<std::string> nw_order;
    for (const auto& h : r.hits) {
      CHECK(h.kind == s100::SessionPickHit::Kind::NavWarning);
      CHECK(h.product == s100::ProductId::S124);
      CHECK(h.feature_id == 0);
      CHECK(!h.numeric_valid);
      CHECK(h.attributes.count("id"));
      CHECK(h.attributes.at("id") == h.feature_type);
      nw_order.push_back(h.feature_type);
      if (h.feature_type == "NW.1") {
        saw_nw1 = true;
        CHECK(h.label == "Synthetic Hazard Alpha");
        CHECK(h.attributes.at("title") == "Synthetic Hazard Alpha");
        CHECK(h.attributes.at("time_iso") == "2024-01-01T00:00:00Z");
      }
      if (h.feature_type == "NW.2") {
        saw_nw2 = true;
        CHECK(h.label == "Synthetic Area Bravo");
      }
    }
    CHECK(saw_nw1 && saw_nw2);
    // id ascending: NW.1 before NW.2
    CHECK(nw_order.size() >= 2);
    const auto i1 = std::find(nw_order.begin(), nw_order.end(), "NW.1");
    const auto i2 = std::find(nw_order.begin(), nw_order.end(), "NW.2");
    CHECK(i1 != nw_order.end() && i2 != nw_order.end() && i1 < i2);
    std::cout << "oracle B: NW.1+NW.2 hits=" << r.hits.size() << "\n";
  }

  // --- Oracle C: S-102+S-104+S-111 @ (0.2,0.2) → Coverage+WL+Current, numeric_valid ---
  {
    s100::ChartSession session;
    CHECK(session.open_and_insert(s102, &err));
    CHECK(session.open_and_insert(s104, &err));
    CHECK(session.open_and_insert(s111, &err));
    s100::SessionPickResult r;
    CHECK(s100::chart_session_pick(session, 0.2, 0.2, r, nullptr));
    bool saw_cov = false, saw_wl = false, saw_cur = false;
    int phase = 0;  // 0=cov,1=wl,2=cur expected order among these kinds
    for (const auto& h : r.hits) {
      CHECK(h.kind != s100::SessionPickHit::Kind::Feature);
      CHECK(h.kind != s100::SessionPickHit::Kind::NavWarning);
      if (h.kind == s100::SessionPickHit::Kind::CoverageSample) {
        CHECK(phase <= 0);
        phase = 1;
        saw_cov = true;
        CHECK(h.numeric_valid);
        CHECK(h.product == s100::ProductId::S102);
      } else if (h.kind == s100::SessionPickHit::Kind::WaterLevel) {
        CHECK(phase <= 1);
        phase = 2;
        saw_wl = true;
        CHECK(h.numeric_valid);
        CHECK(h.product == s100::ProductId::S104);
      } else if (h.kind == s100::SessionPickHit::Kind::Current) {
        CHECK(phase <= 2);
        phase = 3;
        saw_cur = true;
        CHECK(h.numeric_valid);
        CHECK(h.product == s100::ProductId::S111);
      }
    }
    CHECK(saw_cov && saw_wl && saw_cur);
    std::cout << "oracle C: Coverage+WaterLevel+Current hits=" << r.hits.size() << "\n";
  }

  // --- Oracle D: synthetics-only @ (0.2,0.2) → no S-101 Feature ---
  {
    s100::ChartSession session;
    CHECK(session.open_and_insert(s124, &err));
    CHECK(session.open_and_insert(s102, &err));
    CHECK(session.open_and_insert(s104, &err));
    CHECK(session.open_and_insert(s111, &err));
    s100::SessionPickResult r;
    CHECK(s100::chart_session_pick(session, 0.2, 0.2, r, nullptr));
    for (const auto& h : r.hits) {
      CHECK(h.kind != s100::SessionPickHit::Kind::Feature);
      CHECK(h.product != s100::ProductId::S101);
      CHECK(h.kind == s100::SessionPickHit::Kind::NavWarning ||
            h.kind == s100::SessionPickHit::Kind::CoverageSample ||
            h.kind == s100::SessionPickHit::Kind::WaterLevel ||
            h.kind == s100::SessionPickHit::Kind::Current);
    }
    // Full stack order: NW → 102 → 104 → 111
    int last = -1;
    auto rank = [](s100::SessionPickHit::Kind k) {
      using K = s100::SessionPickHit::Kind;
      if (k == K::NavWarning) return 0;
      if (k == K::CoverageSample) return 1;
      if (k == K::WaterLevel) return 2;
      if (k == K::Current) return 3;
      return 9;
    };
    for (const auto& h : r.hits) {
      const int rk = rank(h.kind);
      CHECK(rk >= last);
      last = rk;
    }
    std::cout << "oracle D: synthetics-only hits=" << r.hits.size() << " (no Feature)\n";
  }

  // --- Oracle E: (180,0) → true + empty hits ---
  {
    s100::ChartSession session;
    CHECK(session.open_and_insert(s124, &err));
    CHECK(session.open_and_insert(s102, &err));
    CHECK(session.open_and_insert(s104, &err));
    CHECK(session.open_and_insert(s111, &err));
    s100::SessionPickResult r;
    std::string perr = "x";
    CHECK(s100::chart_session_pick(session, 180.0, 0.0, r, &perr));
    CHECK(r.hits.empty());
    CHECK(r.lon == 180.0 && r.lat == 0.0);
    std::cout << "oracle E: ocean (180,0) hits=0 ok\n";
  }

  std::cout << "session_pick_unified OK (IHO-SESSION-PICK-UNIFIED)\n";
}
