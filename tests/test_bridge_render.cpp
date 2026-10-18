// IHO-S101-BRIDGE-RENDER / CTest fasc_bridge_render (+ bridge_render alias) —
// docs/bridge-render-contract.md rev 2.1 (MUST NOT SKIP).
// Bridge symbology comes only from the official S-101 PC 2.0.0 Lua rules (Bridge.lua,
// SpanFixed.lua, SpanOpening.lua, PylonBridgeSupport.lua) through PortrayalSession::portray.
// This test only observes what the PC emits; it never draws or synthesises bridge instructions.
#include "s100/s100.hpp"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;

#define BR_CHECK(cond)                                                                    \
  do {                                                                                    \
    if (!(cond)) {                                                                        \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";        \
      std::exit(1);                                                                       \
    }                                                                                     \
  } while (0)

namespace {

using DI = s100::DrawingInstruction;
using Kind = DI::Kind;

const char* kind_name(Kind k) {
  switch (k) {
    case Kind::Point: return "Point";
    case Kind::Line: return "Line";
    case Kind::Area: return "Area";
    case Kind::Text: return "Text";
    case Kind::Coverage: return "Coverage";
    case Kind::Arrow: return "Arrow";
    case Kind::Null: return "Null";
  }
  return "?";
}

bool is_span_or_bridge(const std::string& t) {
  return t == "Bridge" || t == "SpanFixed" || t == "SpanOpening";
}
bool is_bridge_type(const std::string& t) { return is_span_or_bridge(t) || t == "PylonBridgeSupport"; }

/** "ID=<n> " attribution used by main.lua error messages (see portrayal_all_cells). */
bool error_mentions_feature(const std::string& msg, std::uint64_t id) {
  return msg.find("ID=" + std::to_string(id) + " ") != std::string::npos ||
         msg.find("ID=" + std::to_string(id) + ")") != std::string::npos ||
         msg.find("ID=" + std::to_string(id) + ":") != std::string::npos;
}

/** Feature-association query through the existing S-101 Host surface
 *  (HostFeatureGetAssociatedFeatureIDs, docs/association-contract.md). */
std::vector<std::string> host_associated(lua_State* L, std::uint64_t id, const char* assoc,
                                         const char* role) {
  std::vector<std::string> out;
  lua_getglobal(L, "HostFeatureGetAssociatedFeatureIDs");
  lua_pushstring(L, std::to_string(id).c_str());
  lua_pushstring(L, assoc);
  lua_pushstring(L, role);
  if (lua_pcall(L, 3, 1, 0) != 0) {
    std::cerr << "host error: " << lua_tostring(L, -1) << "\n";
    lua_pop(L, 1);
    BR_CHECK(!"HostFeatureGetAssociatedFeatureIDs raised");
  }
  BR_CHECK(lua_istable(L, -1));
  const int n = static_cast<int>(lua_objlen(L, -1));
  for (int i = 1; i <= n; ++i) {
    lua_rawgeti(L, -1, i);
    if (const char* s = lua_tostring(L, -1)) out.emplace_back(s);
    lua_pop(L, 1);
  }
  lua_pop(L, 1);
  return out;
}

}  // namespace

void bridge_render() {
  const fs::path src = S100_SOURCE_DIR;
  const fs::path fixture = src / "testdata/s101/101AA00DS0006.000";
  const fs::path fc_path = src / "catalogues/S-101/S-101_FC_2.0.0.xml";
  const fs::path pc_path = src / "catalogues/S-101/PC_2.0.0";
  // Missing fixture / catalogues = FAIL, never SKIP.
  if (!fs::exists(fixture)) std::cerr << "FAIL fixture missing: " << fixture << "\n";
  BR_CHECK(fs::exists(fixture));
  for (const char* rule : {"Bridge.lua", "SpanFixed.lua", "SpanOpening.lua", "PylonBridgeSupport.lua"})
    BR_CHECK(fs::exists(pc_path / "Rules" / rule));

  s100::FeatureCatalogue fc;
  s100::PortrayalCatalogue pc;
  std::string e;
  BR_CHECK(s100::load_feature_catalogue(fc_path, fc, &e));
  BR_CHECK(s100::load_portrayal_catalogue(pc_path, pc, &e));
  BR_CHECK(pc.product == "S-101");

  s100::Dataset ds;
  s100::LoadError le;
  BR_CHECK(s100::open_dataset(fixture, ds, &le));
  BR_CHECK(s100::classify_layers(ds, fc));

  std::map<std::uint64_t, const s100::Feature*> by_id;
  std::map<std::string, std::size_t> type_count;
  for (const auto& f : ds.features) {
    by_id[f.id] = &f;
    ++type_count[f.feature_type];
  }
  auto type_of = [&](std::uint64_t id) -> std::string {
    auto it = by_id.find(id);
    return it == by_id.end() ? std::string() : it->second->feature_type;
  };

  // ---- 1. feature counts (MUST)
  std::cout << "row1 counts: Bridge=" << type_count["Bridge"] << " SpanFixed=" << type_count["SpanFixed"]
            << " SpanOpening=" << type_count["SpanOpening"]
            << " PylonBridgeSupport=" << type_count["PylonBridgeSupport"] << "\n";
  BR_CHECK(type_count["Bridge"] == 10);
  BR_CHECK(type_count["SpanFixed"] == 6);
  BR_CHECK(type_count["SpanOpening"] == 4);
  BR_CHECK(type_count["PylonBridgeSupport"] == 6);

  // ---- portray, default context (RadarOverlay=false)
  s100::PortrayalSession session(fc, pc);
  BR_CHECK(session.context().values.count("RadarOverlay"));
  BR_CHECK(session.context().values.at("RadarOverlay") == "false");
  s100::PortrayalStats st;
  const s100::DrawingInstructions* dip = session.portray(ds, &st, &e);
  if (!dip) std::cerr << "portray failed: " << e << "\n";
  BR_CHECK(dip);
  const s100::DrawingInstructions under = *dip;  // copy: cache entry replaced on context change

  // ---- 2. lua errors (MUST)
  std::size_t bridge_errors = 0;
  for (const auto& m : st.errors) {
    std::cerr << "lua error: " << m << "\n";
    for (const auto& f : ds.features)
      if (is_bridge_type(f.feature_type) && error_mentions_feature(m, f.id)) ++bridge_errors;
  }
  std::cout << "row2 lua_errors=" << st.lua_errors << " bridge_type_errors=" << bridge_errors
            << " features=" << st.features << " emitted=" << st.emitted_features << "\n";
  BR_CHECK(st.lua_errors == 0);
  BR_CHECK(st.errors.empty());
  BR_CHECK(bridge_errors == 0);

  // Group DIs by feature.
  std::map<std::uint64_t, std::vector<const DI*>> dis_of;
  for (const auto& d : under) dis_of[d.feature_id].push_back(&d);

  // ---- 12a. per-type DI counts (recorded for Tester pinning)
  {
    std::map<std::string, std::map<std::string, std::size_t>> counts;
    std::map<std::string, std::size_t> totals;
    for (const auto& d : under) {
      const std::string t = type_of(d.feature_id);
      if (!is_bridge_type(t)) continue;
      ++counts[t][kind_name(d.kind)];
      ++totals[t];
    }
    for (const char* t : {"Bridge", "SpanFixed", "SpanOpening", "PylonBridgeSupport"}) {
      std::cout << "di_count " << t << " total=" << totals[t];
      for (const auto& [k, n] : counts[t]) std::cout << " " << k << "=" << n;
      std::cout << "\n";
    }
  }

  // ---- Bridge graph G (rev 2.1): Bridge FASC -> child via record_id; dangling skipped.
  std::map<std::uint32_t, const s100::Feature*> by_rcid;
  for (const auto& f : ds.features) by_rcid[f.record_id] = &f;
  struct Edge {
    std::uint64_t bridge, child;
    std::string child_type, association, role;
  };
  std::vector<Edge> G;
  for (const auto& f : ds.features) {
    if (f.feature_type != "Bridge") continue;
    for (const auto& a : f.feature_associations) {
      if (a.rcnm != 100) continue;
      const auto it = by_rcid.find(a.rcid);
      if (it == by_rcid.end()) continue;
      G.push_back({f.id, it->second->id, it->second->feature_type, a.association, a.role});
    }
    // FASC on the child side pointing at a Bridge is also an edge (rev 2.1 row 3).
  }
  for (const auto& f : ds.features) {
    if (f.feature_type == "Bridge") continue;
    for (const auto& a : f.feature_associations) {
      const auto it = a.rcnm == 100 ? by_rcid.find(a.rcid) : by_rcid.end();
      if (it != by_rcid.end() && it->second->feature_type == "Bridge")
        G.push_back({it->second->id, f.id, f.feature_type, a.association, "(child-side)" + a.role});
    }
  }
  std::set<std::uint64_t> B, C;
  for (const auto& e : G) {
    B.insert(e.bridge);
    C.insert(e.child);
  }
  std::cout << "graph G TSV:\nbridge_id\tchild_id\tchild_type\tassociation\trole\n";
  for (const auto& e : G)
    std::cout << e.bridge << "\t" << e.child << "\t" << e.child_type << "\t" << e.association << "\t" << e.role << "\n";
  std::set<std::uint64_t> BC = B;
  BC.insert(C.begin(), C.end());

  // ---- 3 / 4. graph checks via the existing Host query (HostFeatureGetAssociatedFeatureIDs)
  {
    BR_CHECK(B.size() >= 1);
    BR_CHECK(C.size() >= 1);
    for (const auto& e : G)
      BR_CHECK(e.child_type == "SpanFixed" || e.child_type == "SpanOpening" || e.child_type == "PylonBridgeSupport");

    s100::S101HostAdapter host(ds, fc);
    lua_State* L = luaL_newstate();
    BR_CHECK(L);
    luaL_openlibs(L);
    for (const auto& name : host.registry().names()) {
      lua_pushlightuserdata(L, &host);
      lua_pushcclosure(L, host.registry().function(name), 1);
      lua_setglobal(L, name.c_str());
    }
    // row 3: every Span/Pylon has a Bridge parent, confirmed by the Host query (inverse role).
    std::size_t orphan_pylons = 0;
    std::set<std::uint64_t> orphan_ids;
    for (const auto& f : ds.features) {
      const std::string& t = f.feature_type;
      if (t != "SpanFixed" && t != "SpanOpening" && t != "PylonBridgeSupport") continue;
      const auto parents = host_associated(L, f.id, "BridgeAggregation", "theCollection");
      const auto any_role = host_associated(L, f.id, "BridgeAggregation", "");
      std::cout << "  child " << t << " " << f.id << " rcid=" << f.record_id
                << " own_fasc=" << f.feature_associations.size()
                << " Host(BridgeAggregation,theCollection)=[";
      for (std::size_t i = 0; i < parents.size(); ++i) std::cout << (i ? "," : "") << parents[i];
      std::cout << "]\n";
      BR_CHECK(parents.size() == any_role.size());
      for (const auto& pid : parents) BR_CHECK(type_of(std::stoull(pid)) == "Bridge");
      // Host result must agree with G.
      std::set<std::uint64_t> g_parents, h_parents;
      for (const auto& e : G)
        if (e.child == f.id && e.association == "BridgeAggregation") g_parents.insert(e.bridge);
      for (const auto& pid : parents) h_parents.insert(std::stoull(pid));
      BR_CHECK(g_parents == h_parents);
      if (parents.empty()) {
        if (t == "PylonBridgeSupport") {
          ++orphan_pylons;
          orphan_ids.insert(f.id);
          std::cout << "WARN row3: PylonBridgeSupport " << f.id << " (rcid " << f.record_id
                    << ") has no Bridge FASC parent in 0006: own FASC empty and no Bridge FASC targets it"
                       " (see s100_cli --fasc). (rev2.2 WARN)\n";
        } else {
          std::cerr << t << " " << f.id << " has no Bridge parent\n";
          BR_CHECK(!"span without Bridge parent");  // MUST for spans
        }
      }
    }
    // row 4: Host(bridge, association, "") == G children, per association code.
    for (const auto b : B) {
      std::map<std::string, std::set<std::uint64_t>> want;
      for (const auto& e : G)
        if (e.bridge == b) want[e.association].insert(e.child);
      for (const auto& [assoc, kids] : want) {
        std::set<std::uint64_t> got;
        for (const auto& id : host_associated(L, b, assoc.c_str(), "")) got.insert(std::stoull(id));
        if (got != kids) std::cerr << "Host children mismatch for Bridge " << b << " " << assoc << "\n";
        BR_CHECK(got == kids);
      }
    }
    // rev2.2: orphan pylon set pinned to the measured 0006 result.
    BR_CHECK((orphan_ids == std::set<std::uint64_t>{33, 43, 47, 69}));
    lua_close(L);
    std::cout << "row3/4 |B|=" << B.size() << " |C|=" << C.size() << " Host query == G; orphan pylons="
              << orphan_pylons << " (WARN)\n";
  }

  // ---- 5. DI count > 0 for all four types; non-Null DIs carry 12210 / 24
  std::size_t text_dis = 0, pylon_p18 = 0;
  for (const auto& f : ds.features) {
    if (!is_bridge_type(f.feature_type)) continue;
    const auto it = dis_of.find(f.id);
    if (it == dis_of.end() || it->second.empty())
      std::cerr << "no DI for " << f.feature_type << " " << f.id << "\n";
    BR_CHECK(it != dis_of.end() && !it->second.empty());
    for (const DI* d : it->second) {
      if (d->kind == Kind::Null) continue;
      bool has_feature_vg = d->viewing_group == "12210";
      for (const auto& vg : d->viewing_groups) has_feature_vg |= vg == "12210";
      BR_CHECK(has_feature_vg);
      if (d->kind == Kind::Text) {
        // Official PC (PortrayalModel.lua:241 AddTextInstruction) emits
        // 'ViewingGroup:<textVG>,12210' — first VG is the text group (11 clearance, 21 name);
        // 12210 is carried as the second group (S-100 Part 9: all groups must be on).
        ++text_dis;
        if (d->viewing_group != "11" && d->viewing_group != "21")
          std::cerr << "text DI " << f.id << " vg=" << d->viewing_group << "\n";
        BR_CHECK(d->viewing_group == "11" || d->viewing_group == "21");
        BR_CHECK(d->drawing_priority == 24);
        continue;
      }
      if (d->viewing_group != "12210")
        std::cerr << f.feature_type << " " << f.id << " " << d->command << " vg=" << d->viewing_group << "\n";
      BR_CHECK(d->viewing_group == "12210");
      if (f.feature_type == "PylonBridgeSupport" && f.geometry.kind == s100::GeometryKind::Polygon) {
        // PylonBridgeSupport.lua Surface branch: 'DrawingPriority:18' (official rule).
        BR_CHECK(d->drawing_priority == 18);
        ++pylon_p18;
      } else {
        BR_CHECK(d->drawing_priority == 24);
      }
    }
  }
  std::cout << "row5 ok; DEVIATION (official PC output, not weakened engine): " << text_dis
            << " Text DIs have first viewing_group 11/21 (12210 second), " << pylon_p18
            << " PylonBridgeSupport surface DIs have drawing_priority 18\n";

  // ---- 6. Bridge with empty geometry -> only Null DIs
  std::size_t empty_bridges = 0;
  for (const auto& f : ds.features) {
    if (f.feature_type != "Bridge" || f.geometry.kind != s100::GeometryKind::Empty) continue;
    ++empty_bridges;
    for (const DI* d : dis_of[f.id]) BR_CHECK(d->kind == Kind::Null);
  }
  std::cout << "row6 empty-geometry Bridges=" << empty_bridges << " (Null-only)\n";
  BR_CHECK(empty_bridges > 0);

  // ---- 7. ON (hidden_groups = {}): B and C come out together
  {
    s100::DrawingInstructions on = under;
    s100::filter_instructions_by_viewing_group(on, {});
    BR_CHECK(on.size() == under.size());
    std::map<std::uint64_t, std::size_t> any, non_null;
    for (const auto& d : on) {
      ++any[d.feature_id];
      if (d.kind != Kind::Null) ++non_null[d.feature_id];
    }
    for (const auto b : B) BR_CHECK(any[b] >= 1);
    for (const auto c : C) BR_CHECK(non_null[c] >= 1);
    std::cout << "row7 ON: all " << B.size() << " bridges have DIs, all " << C.size()
              << " children have non-Null DIs\n";
  }

  // ---- 8. OFF (hidden_groups = {"12210"}): B+C filtered together, nothing else affected
  {
    s100::DrawingInstructions off = under;
    s100::filter_instructions_by_viewing_group(off, {"12210"});
    for (const auto& d : off) {
      if (BC.count(d.feature_id))
        std::cerr << "OFF leftover DI " << d.feature_id << " " << d.command << " vg=" << d.viewing_group << "\n";
      BR_CHECK(!BC.count(d.feature_id));
    }
    auto carries_12210 = [](const DI& d) {
      if (d.viewing_group == "12210") return true;
      for (const auto& vg : d.viewing_groups)
        if (vg == "12210") return true;
      return false;
    };
    std::multiset<std::uint64_t> on_ids, off_ids;
    for (const auto& d : under)
      if (!carries_12210(d)) on_ids.insert(d.feature_id);
    for (const auto& d : off) {
      BR_CHECK(!carries_12210(d));
      off_ids.insert(d.feature_id);
    }
    BR_CHECK(on_ids == off_ids);
    std::cout << "row8 OFF {12210}: " << under.size() - off.size() << " DIs removed incl. all of B+C ("
              << BC.size() << " features); " << off_ids.size() << " other DIs unchanged\n";
  }

  // ---- 9. SpanFixed curve/surface -> >=1 Line DI with CHGRD _simple_ style
  std::size_t spanfixed_geom = 0;
  for (const auto& f : ds.features) {
    if (f.feature_type != "SpanFixed") continue;
    const bool curve = f.geometry.kind == s100::GeometryKind::LineString;
    if (!curve && f.geometry.kind != s100::GeometryKind::Polygon) continue;
    ++spanfixed_geom;
    std::size_t chgrd_lines = 0;
    for (const DI* d : dis_of[f.id]) {
      if (d->kind != Kind::Line) continue;
      for (const auto& ls : d->line_styles)
        if (ls.name == d->line_style_ref && ls.colour == "CHGRD" && ls.width > 0) {
          BR_CHECK(ls.width == (curve ? 1.6 : 1.28));  // SpanFixed.lua widths
          ++chgrd_lines;
        }
    }
    if (!chgrd_lines) std::cerr << "SpanFixed " << f.id << " has no CHGRD Line DI\n";
    BR_CHECK(chgrd_lines >= 1);
  }
  BR_CHECK(spanfixed_geom >= 1);  // non-vacuous
  std::cout << "row9 SpanFixed with curve/surface=" << spanfixed_geom << " all have CHGRD _simple_ Line\n";

  // ---- 10. BRIDGE01 -> PC Symbols file via core symbol_path (missing = error); 12 MUST count
  std::size_t bridge01 = 0;
  for (const auto& d : under) {
    if (d.symbol_ref != "BRIDGE01") continue;
    ++bridge01;
    std::string serr;
    const fs::path p = pc.symbol_path(d.symbol_ref, &serr);
    if (p.empty()) std::cerr << serr << "\n";
    BR_CHECK(!p.empty());
    BR_CHECK(fs::equivalent(p, pc_path / "Symbols" / "BRIDGE01.svg"));
  }
  {
    std::string serr;
    BR_CHECK(pc.symbol_path("NO_SUCH_SYMBOL_BR0", &serr).empty());  // missing = error, not silent
    BR_CHECK(!serr.empty());
  }
  std::cout << "row10 BRIDGE01 DIs=" << bridge01 << " all resolve to PC Symbols/BRIDGE01.svg\n";
  // ---- 12. BRIDGE01 count MUST ≥1 (BRIEF Open #3 / contract close tip)
  BR_CHECK(bridge01 >= 1);
  std::cout << "row12 BRIDGE01 count=" << bridge01 << "\n";

  // ---- hook 5/6: VG filter -> SCAMIN filter -> sort; bridge DIs paint above lower-priority fills.
  {
    s100::DrawingInstructions ios = under;
    ios.erase(std::remove_if(ios.begin(), ios.end(), [](const DI& d) { return d.kind == Kind::Null; }),
              ios.end());
    const long view_scale = 22000;
    s100::filter_instructions_by_viewing_group(ios, {});
    s100::filter_instructions_by_scale(ios, view_scale, false);
    s100::sort_instructions_for_draw(ios, &pc.display_planes);
    std::size_t before = 0, after = 0;
    for (const auto& d : under)
      if (d.kind != Kind::Null && is_span_or_bridge(type_of(d.feature_id)) &&
          s100::visible_at_scale(d, view_scale, false))
        ++before;
    long last_low_area = -1, first_bridge = -1;
    for (std::size_t i = 0; i < ios.size(); ++i) {
      const auto& d = ios[i];
      if (is_span_or_bridge(type_of(d.feature_id))) {
        ++after;
        if (first_bridge < 0 && d.display_plane == "UnderRadar") first_bridge = static_cast<long>(i);
      } else if (d.kind == Kind::Area && d.drawing_priority < 24 && d.display_plane == "UnderRadar") {
        last_low_area = static_cast<long>(i);
      }
    }
    BR_CHECK(before > 0 && after == before);
    BR_CHECK(first_bridge >= 0);
    BR_CHECK(last_low_area < first_bridge);
    std::cout << "hook5 VG->SCAMIN->sort: " << after << " Bridge/Span DIs, first at " << first_bridge
              << " after last lower-priority UnderRadar area fill at " << last_low_area << "\n";
  }

  // ---- 11. RadarOverlay=true: same DIs flip to OverRadar
  {
    std::size_t non_null = 0;
    for (const auto& d : under)
      if (d.kind != Kind::Null && BC.count(d.feature_id)) ++non_null;
    BR_CHECK(non_null >= 1);  // non-vacuous

    BR_CHECK(session.set_context_parameter("RadarOverlay", "true", &e));
    s100::PortrayalStats st2;
    const s100::DrawingInstructions* over = session.portray(ds, &st2, &e);
    BR_CHECK(over);
    BR_CHECK(st2.lua_errors == 0);
    BR_CHECK(over->size() == under.size());  // whole-cell total unchanged
    auto pick = [&](const s100::DrawingInstructions& v, bool all_types) {
      std::vector<const DI*> out;
      for (const auto& d : v)
        if (BC.count(d.feature_id) || (all_types && is_bridge_type(type_of(d.feature_id)))) out.push_back(&d);
      return out;
    };
    const auto a = pick(under, true), b = pick(*over, true);
    BR_CHECK(pick(under, false).size() == pick(*over, false).size());  // B+C count unchanged
    BR_CHECK(a.size() == b.size());
    std::size_t flipped = 0, kept_under = 0;
    for (std::size_t i = 0; i < a.size(); ++i) {
      BR_CHECK(a[i]->feature_id == b[i]->feature_id);
      BR_CHECK(a[i]->kind == b[i]->kind);
      BR_CHECK(a[i]->command == b[i]->command);
      BR_CHECK(a[i]->drawing_priority == b[i]->drawing_priority);
      BR_CHECK(a[i]->viewing_group == b[i]->viewing_group);
      BR_CHECK(a[i]->display_plane == "UnderRadar");
      const std::string t = type_of(a[i]->feature_id);
      const bool pylon_surface18 = t == "PylonBridgeSupport" && a[i]->drawing_priority == 18;
      if (pylon_surface18) {
        BR_CHECK(b[i]->display_plane == "UnderRadar");  // PylonBridgeSupport.lua Surface branch
        ++kept_under;
      } else {
        BR_CHECK(b[i]->display_plane == "OverRadar");  // incl. Null and Text DIs
        ++flipped;
      }
    }
    std::cout << "row11 RadarOverlay=true: " << flipped << " DIs flipped UnderRadar->OverRadar, "
              << kept_under << " PylonBridgeSupport priority-18 surface DIs stay UnderRadar; B+C non-Null="
              << non_null << "; total DIs " << over->size() << " unchanged\n";
  }

  std::cout << "WARN row13: viewer pixel check not part of this CTest (viewer_* tests)\n";
  std::cout << "bridge_render OK (IHO-S101-BRIDGE-RENDER)\n";
}
