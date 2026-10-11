#include "s100/s100.hpp"
#include <cctype>
#include <chrono>
#include <cstring>
#include <iostream>

#ifndef S100_DEFAULT_FC
#define S100_DEFAULT_FC ""
#endif
#ifndef S100_DEFAULT_PC
#define S100_DEFAULT_PC ""
#endif

static const char* kUsage =
    "usage: s100_cli <file.000> [--fc <FC.xml>] [--layers]\n"
    "                [--portray [--pc <PC dir>] [--set <Param>=<value>]... [--errors]]\n"
    "       s100_cli <CATALOG.XML|exchange-set-dir> [--list] [--load <cell>|--load-all]\n"
    "                [--fc <FC.xml>] [--layers]\n";

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

static int run_single_cell(const std::filesystem::path& path, const std::filesystem::path& fc_path,
                           const std::filesystem::path& pc_path, bool list_layers, bool do_portray,
                           bool show_errors,
                           const std::vector<std::pair<std::string, std::string>>& settings) {
  s100::Dataset ds;
  s100::LoadError err;
  const auto t0 = std::chrono::steady_clock::now();
  bool ok = false;
  if (path.extension() == ".000") {
    auto updates = s100::discover_update_files(path);
    if (!updates.empty())
      ok = s100::open_dataset_with_updates(path, updates, ds, &err);
    else
      ok = s100::open_dataset(path, ds, &err);
  } else {
    ok = s100::open_dataset(path, ds, &err);
  }
  if (!ok) {
    std::cerr << "load failed: " << err.message << "\n";
    return 1;
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
  std::cout << "loaded " << path << " features=" << st.feature_count << " layers=" << st.layer_count
            << " ms=" << ms << "\n";

  bool portrayed = true;
  if (do_portray) {
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
    std::cout << "portray " << path.filename().string() << " pc=" << pc.product << " " << pc.version
              << " features=" << ps.features << " emitted=" << ps.emitted_features
              << " instructions=" << ps.instructions << " lua_errors=" << ps.lua_errors
              << " unparsed=" << ps.unparsed
              << " features_without_instructions=" << ps.features_without_instructions.size()
              << " ms=" << ms_since(t1) << (di ? "" : " FAILED: " + e) << "\n";
    for (const auto& [cmd, n] : ps.by_command) std::cout << "  " << cmd << " " << n << "\n";
    for (const auto& [cmd, n] : ps.unparsed_commands)
      std::cout << "  unparsed " << cmd << " " << n << "\n";
    if (show_errors)
      for (const auto& m : ps.errors) std::cout << "  error: " << m << "\n";
  }
  if (list_layers)
    for (const auto& [code, li] : ds.layers.all()) {
      std::cout << "  " << code << " [" << li.use_type << "] " << li.feature_count;
      if (!li.viewing_group.empty())
        std::cout << " vg=" << li.viewing_group << " prio=" << li.drawing_priority
                  << " plane=" << li.display_plane << " cat=" << li.display_category;
      std::cout << "\n";
    }
  return st.feature_count > 0 && ms < 5000 && classified && portrayed ? 0 : 1;
}

static int run_exchange_set(const std::filesystem::path& path, const std::filesystem::path& fc_path,
                            bool list_only, bool load_all, const std::string& load_one,
                            bool list_layers) {
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
    std::cout << "  " << s.cell_name << " product=" << s100::product_name(s.product)
              << " ed=" << s.edition_number << " upd=" << s.latest_update_number
              << " updates=" << s.updates.size();
    if (s.coverage.valid)
      std::cout << " bounds=[" << s.coverage.west << "," << s.coverage.south << "," << s.coverage.east
                << "," << s.coverage.north << "]";
    std::cout << "\n";
  }

  if (list_only && load_one.empty() && !load_all) return 0;

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

  if (load_all) {
    s100::LoadError first;
    const auto n = session.load_all_members(&first);
    std::cout << "load-all ok=" << n << "/" << session.member_keys().size() << "\n";
    for (const auto& key : session.member_keys())
      if (auto* ds = session.find(key)) after_load(key, *ds);
    return n > 0 ? 0 : 1;
  }

  if (!load_one.empty()) {
    if (!session.load_member(load_one, &err)) {
      std::cerr << "load failed: " << err.message << "\n";
      return 1;
    }
    after_load(load_one, *session.find(load_one));
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
    else if (!std::strcmp(argv[i], "--set") && i + 1 < argc) {
      const std::string kv = argv[++i];
      const auto eq = kv.find('=');
      if (eq == std::string::npos) {
        std::cerr << kUsage;
        return 2;
      }
      settings.emplace_back(kv.substr(0, eq), kv.substr(eq + 1));
    } else
      path = argv[i];
  }

  if (looks_like_exchange_set(path))
    return run_exchange_set(path, fc_path, list_only, load_all, load_one, list_layers);
  return run_single_cell(path, fc_path, pc_path, list_layers, do_portray, show_errors, settings);
}
