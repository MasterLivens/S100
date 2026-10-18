#include "s100/s100.hpp"
#include <cctype>
#include <map>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#ifndef S100_DEFAULT_FC
#define S100_DEFAULT_FC ""
#endif
#ifndef S100_DEFAULT_PC
#define S100_DEFAULT_PC ""
#endif
#ifndef S100_DEFAULT_S111_FC
#define S100_DEFAULT_S111_FC ""
#endif
#ifndef S100_DEFAULT_S111_PC
#define S100_DEFAULT_S111_PC ""
#endif
#ifndef S100_DEFAULT_S124_FC
#define S100_DEFAULT_S124_FC ""
#endif
#ifndef S100_DEFAULT_S124_PC
#define S100_DEFAULT_S124_PC ""
#endif

static const char* kUsage =
    "usage: s100_cli <file.000|.h5|.gml> [--fc <FC.xml>] [--pc <PC dir>] [--layers]\n"
    "                [--s102 <path.h5>] [--s104 <path.h5>] [--s111 <path.h5>] [--s124 <path.gml>]\n"
    "                [--s104-frame <i>] [--s104-time <iso>] [--s111-frame <i>] [--s111-time <iso>]\n"
    "                [--list-frames] [--session-pick <lon> <lat>]\n"
    "                [--portray [--pc <PC dir>] [--set <Param>=<value>]... [--errors]]\n"
    "                [--view-scale <N>] [--ignore-scamin] [--fasc] [--with-updates]\n"
    "       s100_cli <CATALOG.XML|exchange-set-dir> [--list] [--load <key>|--load-all]\n"
    "                [--fc <FC.xml>] [--layers] [--session-pick <lon> <lat>]\n"
    "  note: lone .001 updates are rejected; use exchange set / with_updates\n"
  "  note: --fc/--pc default to pinned S-101; S-111/S-124 use catalogues/S-111|S-124\n"
  "  note: --view-scale <N> filters portray dump by SCAMIN/SCAMAX (omit = no scale filter)\n"
  "  note: --ignore-scamin skips scale_minimum checks only (SCAMAX still applies)\n"
  "  note: --list-frames prints product\\tindex\\ttime_iso\\tactive for S-104/S-111\n"
  "  note: --with-updates applies sibling <stem>.001.. and prints\n"
  "        updates applied=N rejected=<file> code=<Code> (exit 1 when an update is rejected)\n"
  "  note: --fasc prints S-101 FASC as TSV feature_id\\tfeature_type\\tpeer_id\\tpeer_type\\tassociation\\trole\n";

static long long ms_since(std::chrono::steady_clock::time_point t0) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0)
      .count();
}

static bool looks_like_exchange_set(const std::filesystem::path& path) {
  namespace fs = std::filesystem;
  if (!fs::exists(path)) return false;
  if (fs::is_directory(path)) {
    return fs::exists(path / "CATALOG.XML") || fs::exists(path / "catalog.xml") ||
           fs::exists(path / "Catalog.XML");
  }
  auto lower = path.filename().string();
  for (char& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return lower == "catalog.xml";
}


static bool context_ignore_scamin(const std::vector<std::pair<std::string, std::string>>& settings,
                                  bool flag) {
  if (flag) return true;
  for (const auto& [k, v] : settings)
    if (k == "IgnoreScaleMinimum" && (v == "true" || v == "True" || v == "1")) return true;
  return false;
}

static void apply_view_scale_filter(s100::DrawingInstructions& di, long view_scale_denom,
                                    bool ignore_scale_minimum) {
  const std::size_t before = di.size();
  s100::filter_instructions_by_scale(di, view_scale_denom, ignore_scale_minimum);
  std::cout << "scale-filter view=1:" << view_scale_denom
            << " ignore_scamin=" << (ignore_scale_minimum ? "true" : "false")
            << " instructions " << before << " -> " << di.size() << "\n";
}

static const char* kind_name(s100::SessionPickHit::Kind k) {
  using K = s100::SessionPickHit::Kind;
  switch (k) {
    case K::Feature: return "Feature";
    case K::CoverageSample: return "CoverageSample";
    case K::WaterLevel: return "WaterLevel";
    case K::Current: return "Current";
    case K::NavWarning: return "NavWarning";
  }
  return "?";
}

static void print_session_pick(const s100::SessionPickResult& r) {
  std::cout << "session-pick lon=" << r.lon << " lat=" << r.lat << " hits=" << r.hits.size() << "\n";
  for (const auto& h : r.hits) {
    std::cout << "  " << kind_name(h.kind) << " key=" << h.member_key
              << " product=" << s100::product_name(h.product) << " feature_id=" << h.feature_id
              << " type=" << h.feature_type << " label=" << h.label
              << " numeric_valid=" << (h.numeric_valid ? "true" : "false");
    if (h.numeric_valid) {
      if (h.kind == s100::SessionPickHit::Kind::Current)
        std::cout << " speed_ms=" << h.speed_ms << " direction_deg=" << h.direction_deg;
      else
        std::cout << " depth_or_level=" << h.depth_or_level;
    }
    std::cout << "\n";
  }
}


static void apply_frame_selectors(s100::Dataset& ds, int s104_frame, const std::string& s104_time,
                                  int s111_frame, const std::string& s111_time) {
  if (ds.water_level) {
    if (s104_frame >= 0) ds.water_level->set_active_frame(static_cast<std::size_t>(s104_frame));
    if (!s104_time.empty()) ds.water_level->set_active_time(s104_time);
  }
  if (ds.current) {
    if (s111_frame >= 0) ds.current->set_active_frame(static_cast<std::size_t>(s111_frame));
    if (!s111_time.empty()) ds.current->set_active_time(s111_time);
  }
}

static void print_timeline_entries(const char* product, const std::vector<s100::TimelineEntry>& entries) {
  for (const auto& e : entries) {
    std::cout << product << '\t' << e.index << '\t' << e.time_iso << '\t'
              << (e.active ? '1' : '0') << '\n';
  }
}

static bool emit_list_frames_dataset(s100::Dataset& ds) {
  bool any = false;
  if (ds.water_level) {
    std::vector<s100::TimelineEntry> entries;
    if (s100::timeline_entries_water_level(*ds.water_level, entries)) {
      print_timeline_entries("S-104", entries);
      any = true;
    }
  }
  if (ds.current) {
    std::vector<s100::TimelineEntry> entries;
    if (s100::timeline_entries_current(*ds.current, entries)) {
      print_timeline_entries("S-111", entries);
      any = true;
    }
  }
  return any;
}

static int run_session_pick_on(s100::ChartSession& session, double lon, double lat) {
  s100::SessionPickResult pr;
  std::string perr;
  if (!s100::chart_session_pick(session, lon, lat, pr, &perr)) {
    std::cerr << "session-pick failed: " << perr << "\n";
    return 1;
  }
  print_session_pick(pr);
  return 0;
}

static int run_single_cell(const std::filesystem::path& path, const std::filesystem::path& fc_path,
                           const std::filesystem::path& pc_path, bool list_layers, bool do_portray,
                           bool show_errors,
                           const std::vector<std::pair<std::string, std::string>>& settings,
                           const std::vector<std::filesystem::path>& overlay_paths, bool do_session_pick,
                           double pick_lon, double pick_lat, bool have_view_scale, long view_scale_denom,
                           bool ignore_scamin_flag, bool list_frames, int s104_frame,
                           const std::string& s104_time, int s111_frame, const std::string& s111_time,
                           bool with_updates) {
  // Session path: primary + overlays (used for --session-pick and multi-product loads).
  const bool use_session = do_session_pick || !overlay_paths.empty();
  if (use_session) {
    s100::ChartSession session;
    s100::LoadError err;
    const auto t0 = std::chrono::steady_clock::now();
    std::string key;
    if (!path.empty()) {
      if (!session.open_and_insert(path, &err, &key)) {
        std::cerr << "load failed: " << err.message << "\n";
        return 1;
      }
      std::cout << "loaded " << key << " product="
                << s100::product_name(session.find(key)->product) << " ms=" << ms_since(t0) << "\n";
    }
    for (const auto& ov : overlay_paths) {
      std::string ok;
      if (!session.open_and_insert(ov, &err, &ok)) {
        std::cerr << "overlay load failed: " << err.message << "\n";
        return 1;
      }
      std::cout << "loaded " << ok << " product=" << s100::product_name(session.find(ok)->product)
                << "\n";
    }
    if (session.datasets().empty()) {
      std::cerr << "no members loaded\n";
      return 1;
    }
    for (auto& key : session.member_keys()) {
      if (auto* m = session.find(key))
        apply_frame_selectors(*m, s104_frame, s104_time, s111_frame, s111_time);
    }
    if (list_frames) {
      bool any = false;
      for (const auto& key : session.member_keys()) {
        if (auto* m = session.find(key)) any = emit_list_frames_dataset(*m) || any;
      }
      if (!any) {
        std::cerr << "error: --list-frames requires an open S-104 or S-111 dataset\n";
        return 1;
      }
      return 0;
    }
    if (do_session_pick) return run_session_pick_on(session, pick_lon, pick_lat);
    // Fall through to single-dataset report on primary when overlays were loaded without pick.
  }

  s100::Dataset ds;
  s100::LoadError err;
  const auto t0 = std::chrono::steady_clock::now();
  bool ok = false;
  if (path.extension() == ".000") {
    auto updates = s100::discover_update_files(path);
    if (!updates.empty() || with_updates) {
      // docs/update-robustness-contract.md: the CLI only shows the core's report.
      s100::UpdateReport report;
      ok = s100::open_dataset_with_updates(path, updates, ds, &err, &report);
      if (with_updates) {
        std::cout << "updates applied=" << report.applied << " rejected="
                  << (report.rejected.empty() ? std::string("-") : report.rejected.string())
                  << " code=" << s100::update_reject_code_name(report.code) << "\n";
        if (!ok) {
          std::cerr << "warning: " << report.message << "\n";
          if (report.rejected != path)
            std::cerr << "warning: last good state " << ds.edition_number << "."
                      << ds.update_number << " features=" << ds.features.size() << "\n";
          return 1;
        }
      }
    } else {
      ok = s100::open_dataset(path, ds, &err);
    }
  } else {
    ok = s100::open_dataset(path, ds, &err);
  }
  if (!ok) {
    std::cerr << "load failed: " << err.message << "\n";
    return 1;
  }
  apply_frame_selectors(ds, s104_frame, s104_time, s111_frame, s111_time);
  if (list_frames) {
    if (!emit_list_frames_dataset(ds)) {
      std::cerr << "error: --list-frames requires an open S-104 or S-111 dataset\n";
      return 1;
    }
    return 0;
  }
  const auto ms = ms_since(t0);

  bool classified = true;
  s100::FeatureCatalogue fc;
  std::string fc_err;
  const bool have_fc = !fc_path.empty() && s100::load_feature_catalogue(fc_path, fc, &fc_err);
  if (have_fc) {
    s100::ClassifyResult cr;
    classified = s100::classify_layers(ds, fc, &cr);
    for (const auto& c : cr.unknown_codes) std::cerr << "not in FC " << fc.version << ": " << c << "\n";
  } else {
    std::cerr << "warning: no Feature Catalogue (" << (fc_err.empty() ? "no path" : fc_err)
              << "), layers not classified\n";
  }

  const auto st = ds.stats();
  std::cout << "loaded " << path << " product=" << s100::product_name(ds.product)
            << " features=" << st.feature_count << " layers=" << st.layer_count << " ms=" << ms;
  if (ds.nav_warnings)
    std::cout << " warnings=" << ds.nav_warnings->warnings.size();
  std::cout << "\n";

  bool portrayed = true;
  if (do_portray) {
    if (ds.product == s100::ProductId::S124 && ds.nav_warnings) {
      s100::DrawingInstructions di;
      std::string e;
      s100::FeatureCatalogue fc124 = fc;
      s100::PortrayalCatalogue pc124;
      bool used_pc = false;
      // Product-pinned defaults (docs/pc-wire-contract.md); --fc/--pc override when set to
      // non-S-101 paths — prefer explicit S-124 literals when still on S-101 defaults.
      std::string fc_use = std::string(S100_DEFAULT_S124_FC);
      std::string pc_use = std::string(S100_DEFAULT_S124_PC);
      if (!fc_path.empty() && fc_path != std::filesystem::path(S100_DEFAULT_FC))
        fc_use = fc_path.string();
      if (!pc_path.empty() && pc_path != std::filesystem::path(S100_DEFAULT_PC))
        pc_use = pc_path.string();
      if (!fc_use.empty() && !pc_use.empty() &&
          s100::load_feature_catalogue(fc_use, fc124, &e) &&
          s100::load_portrayal_catalogue(pc_use, pc124, &e)) {
        portrayed = s100::portray_nav_warnings_pc(*ds.nav_warnings, fc124, pc124, di, nullptr, &e);
        used_pc = portrayed;
      }
      if (!portrayed) {
        e.clear();
        di.clear();
        portrayed = s100::portray_nav_warnings(*ds.nav_warnings, di, &e);
      }
      if (have_view_scale && portrayed) {
        const bool ign = context_ignore_scamin(settings, ignore_scamin_flag);
        apply_view_scale_filter(di, view_scale_denom, ign);
      }
      std::cout << "portray " << path.filename().string() << " S-124 instructions=" << di.size()
                << (used_pc ? " (PC)" : " (heuristic)")
                << (portrayed ? "" : " FAILED: " + e) << "\n";
    } else if (ds.product == s100::ProductId::S111 && ds.current) {
      s100::DrawingInstructions di;
      std::string e;
      portrayed = s100::portray_current(*ds.current, di, &e);
      if (have_view_scale && portrayed) {
        const bool ign = context_ignore_scamin(settings, ignore_scamin_flag);
        apply_view_scale_filter(di, view_scale_denom, ign);
      }
      std::cout << "portray " << path.filename().string() << " S-111 arrows=" << di.size()
                << (portrayed && !di.empty() ? " symbol=" + di.front().symbol_ref : "")
                << (portrayed ? "" : " FAILED: " + e) << "\n";
    } else {
      s100::PortrayalCatalogue pc;
      std::string e;
      if (!have_fc || !s100::load_portrayal_catalogue(pc_path, pc, &e)) {
        std::cerr << "portray: need FC and PC (" << e << ")\n";
        return 1;
      }
      s100::PortrayalSession session(fc, pc);
      for (const auto& [k, v] : settings)
        if (!session.set_context_parameter(k, v, &e)) {
          std::cerr << e << "\n";
          return 2;
        }
      s100::PortrayalStats ps;
      const auto t1 = std::chrono::steady_clock::now();
      const auto* di = session.portray(ds, &ps, &e);
      portrayed = di != nullptr && ps.lua_errors == 0;
      s100::DrawingInstructions filtered;
      const s100::DrawingInstructions* dump = di;
      if (have_view_scale && di) {
        filtered = *di;
        bool ign = ignore_scamin_flag;
        auto it = session.context().values.find("IgnoreScaleMinimum");
        if (it != session.context().values.end() &&
            (it->second == "true" || it->second == "True" || it->second == "1"))
          ign = true;
        apply_view_scale_filter(filtered, view_scale_denom, ign);
        dump = &filtered;
      }
      std::cout << "portray " << path.filename().string() << " pc=" << pc.product << " " << pc.version
                << " features=" << ps.features << " emitted=" << ps.emitted_features
                << " instructions=" << (dump ? dump->size() : ps.instructions)
                << " lua_errors=" << ps.lua_errors
                << " unparsed=" << ps.unparsed
                << " features_without_instructions=" << ps.features_without_instructions.size()
                << " ms=" << ms_since(t1) << (di ? "" : " FAILED: " + e) << "\n";
      if (have_view_scale && dump) {
        std::map<std::string, std::size_t> by_cmd;
        for (const auto& d : *dump) ++by_cmd[d.command];
        for (const auto& [cmd, n] : by_cmd) std::cout << "  " << cmd << " " << n << "\n";
      } else {
        for (const auto& [cmd, n] : ps.by_command) std::cout << "  " << cmd << " " << n << "\n";
      }
      for (const auto& [cmd, n] : ps.unparsed_commands)
        std::cout << "  unparsed " << cmd << " " << n << "\n";
      if (show_errors)
        for (const auto& m : ps.errors) std::cout << "  error: " << m << "\n";
    }
  }
  if (list_layers)
    for (const auto& [code, li] : ds.layers.all()) {
      std::cout << "  " << code << " [" << li.use_type << "] " << li.feature_count;
      if (!li.viewing_group.empty())
        std::cout << " vg=" << li.viewing_group << " prio=" << li.drawing_priority
                  << " plane=" << li.display_plane << " cat=" << li.display_category;
      std::cout << "\n";
    }
  const bool has_payload =
      st.feature_count > 0 || (ds.nav_warnings && !ds.nav_warnings->warnings.empty()) ||
      ds.coverage.has_value() || ds.water_level.has_value() || ds.current.has_value();
  return has_payload && ms < 5000 && classified && portrayed ? 0 : 1;
}

static int run_exchange_set(const std::filesystem::path& path, const std::filesystem::path& fc_path,
                            bool list_only, bool load_all, const std::string& load_one,
                            bool list_layers, bool do_session_pick, double pick_lon, double pick_lat) {
  s100::ChartSession session;
  s100::LoadError err;
  const auto t0 = std::chrono::steady_clock::now();
  if (!session.open_exchange_set(path, &err)) {
    std::cerr << "catalogue failed: " << err.message << "\n";
    return 1;
  }
  std::cout << "exchange-set " << session.catalogue().identifier
            << " datasets=" << session.catalogue().datasets.size()
            << " cells=" << session.member_keys().size() << " ms=" << ms_since(t0) << "\n";

  for (const auto& s : session.series()) {
    std::cout << "  " << s100::exchange_member_key(s.product, s.cell_name)
              << " product=" << s100::product_name(s.product)
              << " ed=" << s.edition_number << " upd=" << s.latest_update_number
              << " updates=" << s.updates.size();
    if (s.coverage.valid)
      std::cout << " bounds=[" << s.coverage.west << "," << s.coverage.south << "," << s.coverage.east
                << "," << s.coverage.north << "]";
    std::cout << "\n";
  }

  if (list_only && load_one.empty() && !load_all && !do_session_pick) return 0;

  s100::FeatureCatalogue fc;
  std::string fc_err;
  const bool have_fc = !fc_path.empty() && s100::load_feature_catalogue(fc_path, fc, &fc_err);

  auto after_load = [&](const std::string& key, s100::Dataset& ds) {
    if (have_fc) {
      s100::ClassifyResult cr;
      s100::classify_layers(ds, fc, &cr);
      for (const auto& c : cr.unknown_codes)
        std::cerr << "not in FC " << fc.version << ": " << c << "\n";
    }
    const auto st = ds.stats();
    std::cout << "loaded " << key << " features=" << st.feature_count << " layers=" << st.layer_count
              << "\n";
    if (list_layers)
      for (const auto& [code, li] : ds.layers.all())
        std::cout << "  " << code << " [" << li.use_type << "] " << li.feature_count << "\n";
  };

  if (load_all || (do_session_pick && load_one.empty())) {
    s100::LoadError first;
    const auto n = session.load_all_members(&first);
    std::cout << "load-all ok=" << n << "/" << session.member_keys().size() << "\n";
    for (const auto& key : session.member_keys())
      if (auto* ds = session.find(key)) after_load(key, *ds);
    if (do_session_pick) return run_session_pick_on(session, pick_lon, pick_lat);
    return n > 0 ? 0 : 1;
  }

  if (!load_one.empty()) {
    if (!session.load_member(load_one, &err)) {
      std::cerr << "load failed: " << err.message << "\n";
      return 1;
    }
    after_load(load_one, *session.find(load_one));
    if (do_session_pick) return run_session_pick_on(session, pick_lon, pick_lat);
    return 0;
  }

  const auto keys = session.member_keys();
  if (keys.empty()) {
    std::cerr << "no ISO 8211 cell series in catalogue\n";
    return 1;
  }
  if (!session.load_member(keys.front(), &err)) {
    std::cerr << "load failed: " << err.message << "\n";
    return 1;
  }
  after_load(keys.front(), *session.find(keys.front()));
  if (do_session_pick) return run_session_pick_on(session, pick_lon, pick_lat);
  return 0;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << kUsage;
    return 2;
  }
  std::filesystem::path path, fc_path = S100_DEFAULT_FC, pc_path = S100_DEFAULT_PC;
  bool list_layers = false, do_portray = false, show_errors = false;
  bool list_only = false, load_all = false;
  std::string load_one;
  std::vector<std::pair<std::string, std::string>> settings;
  std::vector<std::filesystem::path> overlay_paths;
  bool do_session_pick = false;
  double pick_lon = 0, pick_lat = 0;
  bool have_view_scale = false;
  long view_scale_denom = 0;
  bool ignore_scamin_flag = false;
  bool list_frames = false;
  bool dump_fasc = false;
  bool with_updates = false;
  int s104_frame = -1;
  int s111_frame = -1;
  std::string s104_time, s111_time;
  for (int i = 1; i < argc; ++i) {
    if (!std::strcmp(argv[i], "--fc") && i + 1 < argc)
      fc_path = argv[++i];
    else if (!std::strcmp(argv[i], "--pc") && i + 1 < argc)
      pc_path = argv[++i];
    else if (!std::strcmp(argv[i], "--layers"))
      list_layers = true;
    else if (!std::strcmp(argv[i], "--portray"))
      do_portray = true;
    else if (!std::strcmp(argv[i], "--errors"))
      show_errors = true;
    else if (!std::strcmp(argv[i], "--list"))
      list_only = true;
    else if (!std::strcmp(argv[i], "--load-all"))
      load_all = true;
    else if (!std::strcmp(argv[i], "--load") && i + 1 < argc)
      load_one = argv[++i];
    else if (!std::strcmp(argv[i], "--session-pick") && i + 2 < argc) {
      do_session_pick = true;
      pick_lon = std::atof(argv[++i]);
      pick_lat = std::atof(argv[++i]);
    } else if (!std::strcmp(argv[i], "--s102") && i + 1 < argc)
      overlay_paths.push_back(argv[++i]);
    else if (!std::strcmp(argv[i], "--s104") && i + 1 < argc)
      overlay_paths.push_back(argv[++i]);
    else if (!std::strcmp(argv[i], "--s111") && i + 1 < argc)
      overlay_paths.push_back(argv[++i]);
    else if (!std::strcmp(argv[i], "--s124") && i + 1 < argc) {
      // Preserve prior behaviour: --s124 alone sets primary path; with an existing
      // primary it becomes an overlay member for session pick.
      if (path.empty())
        path = argv[++i];
      else
        overlay_paths.push_back(argv[++i]);
    } else if (!std::strcmp(argv[i], "--s104-frame") && i + 1 < argc) {
      s104_frame = std::atoi(argv[++i]);
    } else if (!std::strcmp(argv[i], "--s104-time") && i + 1 < argc) {
      s104_time = argv[++i];
    } else if (!std::strcmp(argv[i], "--s111-frame") && i + 1 < argc) {
      s111_frame = std::atoi(argv[++i]);
    } else if (!std::strcmp(argv[i], "--s111-time") && i + 1 < argc) {
      s111_time = argv[++i];
    } else if (!std::strcmp(argv[i], "--list-frames")) {
      list_frames = true;
    } else if (!std::strcmp(argv[i], "--set") && i + 1 < argc) {
      const std::string kv = argv[++i];
      const auto eq = kv.find('=');
      if (eq == std::string::npos) {
        std::cerr << kUsage;
        return 2;
      }
      settings.emplace_back(kv.substr(0, eq), kv.substr(eq + 1));
    } else if (!std::strcmp(argv[i], "--view-scale") && i + 1 < argc) {
      have_view_scale = true;
      view_scale_denom = std::atol(argv[++i]);
    } else if (!std::strcmp(argv[i], "--ignore-scamin")) {
      ignore_scamin_flag = true;
    } else if (!std::strcmp(argv[i], "--fasc")) {
      dump_fasc = true;
    } else if (!std::strcmp(argv[i], "--with-updates")) {
      with_updates = true;
    } else
      path = argv[i];
  }

  if (have_view_scale && view_scale_denom <= 0) {
    std::cerr << "error: --view-scale must be > 0 (got " << view_scale_denom << ")\n";
    return 2;
  }

  if (path.empty() && overlay_paths.empty()) {
    if (list_frames) {
      std::cerr << "error: --list-frames requires an open S-104 or S-111 dataset\n";
      return 1;
    }
    std::cerr << kUsage;
    return 2;
  }

  if (dump_fasc) {
    // Raw FASC dump (data evidence; docs/association-contract.md). One row per encoded FASC
    // member, on the side where it is encoded; features without FASC are listed with "-".
    s100::Dataset ds;
    s100::LoadError le;
    if (path.empty() || !s100::open_dataset(path, ds, &le)) {
      std::cerr << "load failed: " << le.message << "\n";
      return 1;
    }
    std::map<std::uint32_t, const s100::Feature*> by_rcid;
    for (const auto& f : ds.features) by_rcid[f.record_id] = &f;
    std::cout << "feature_id\tfeature_type\tpeer_id\tpeer_type\tassociation\trole\n";
    for (const auto& f : ds.features) {
      if (f.feature_associations.empty()) {
        std::cout << f.id << "\t" << f.feature_type << "\t-\t-\t-\t-\n";
        continue;
      }
      for (const auto& a : f.feature_associations) {
        const auto it = a.rcnm == 100 ? by_rcid.find(a.rcid) : by_rcid.end();
        std::cout << f.id << "\t" << f.feature_type << "\t"
                  << (it != by_rcid.end() ? std::to_string(it->second->id) : "dangling:" + std::to_string(a.rcid))
                  << "\t" << (it != by_rcid.end() ? it->second->feature_type : std::string("?")) << "\t"
                  << a.association << "\t" << a.role << "\n";
      }
    }
    return 0;
  }

  if (!path.empty() && looks_like_exchange_set(path))
    return run_exchange_set(path, fc_path, list_only, load_all, load_one, list_layers, do_session_pick,
                            pick_lon, pick_lat);
  if (path.empty() && !overlay_paths.empty()) {
    // e.g. s100_cli --s124 file.gml --session-pick ...
    path = overlay_paths.front();
    overlay_paths.erase(overlay_paths.begin());
  }
  return run_single_cell(path, fc_path, pc_path, list_layers, do_portray, show_errors, settings,
                         overlay_paths, do_session_pick, pick_lon, pick_lat, have_view_scale,
                         view_scale_denom, ignore_scamin_flag, list_frames, s104_frame, s104_time,
                         s111_frame, s111_time, with_updates);
}
