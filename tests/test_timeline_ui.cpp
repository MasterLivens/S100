#include "s100/dataset.hpp"
#include "s100/hdf5_product.hpp"
#include "s100/timeline.hpp"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#if !defined(_WIN32)
#include <sys/wait.h>
#endif
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifndef S100_SOURCE_DIR
#error "S100_SOURCE_DIR required"
#endif
#ifndef S100_CLI_PATH
#error "S100_CLI_PATH required"
#endif

#define CHECK(cond)                                                                \
  do {                                                                             \
    if (!(cond)) {                                                                 \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n"; \
      std::exit(1);                                                                \
    }                                                                              \
  } while (0)

namespace fs = std::filesystem;

namespace {

struct CliResult {
  int exit_code{0};
  std::string stdout_text;
  std::string stderr_text;
};

CliResult run_cli(const std::string& args) {
  const fs::path out_path = fs::temp_directory_path() /
                            ("s100_timeline_ui_stdout_" + std::to_string(std::rand()) + ".txt");
  const fs::path err_path = fs::temp_directory_path() /
                            ("s100_timeline_ui_stderr_" + std::to_string(std::rand()) + ".txt");
  // Windows cmd.exe mangles a line that starts with a quoted exe + redirects.
  // Use cmd /S /C ""exe" args 1>"out" 2>"err"" (documented CreateProcess/cmd form).
#if defined(_WIN32)
  const std::string cmd = std::string("cmd /S /C \"\"") + S100_CLI_PATH + "\" " + args +
                          " 1>\"" + out_path.string() + "\" 2>\"" + err_path.string() + "\"\"";
#else
  const std::string cmd = std::string("\"") + S100_CLI_PATH + "\" " + args + " >\"" +
                          out_path.string() + "\" 2>\"" + err_path.string() + "\"";
#endif
  CliResult r;
  r.exit_code = std::system(cmd.c_str());
  // Normalize wait status to exit code on POSIX; WinCRT returns exit code directly.
#if !defined(_WIN32)
  if (WIFEXITED(r.exit_code)) r.exit_code = WEXITSTATUS(r.exit_code);
#endif
  auto slurp = [](const fs::path& p) {
    std::FILE* f = std::fopen(p.string().c_str(), "rb");
    if (!f) return std::string{};
    std::string s;
    char buf[4096];
    while (true) {
      const size_t n = std::fread(buf, 1, sizeof(buf), f);
      if (!n) break;
      s.append(buf, n);
    }
    std::fclose(f);
    return s;
  };
  r.stdout_text = slurp(out_path);
  r.stderr_text = slurp(err_path);
  return r;
}

struct FrameLine {
  std::string product;
  std::size_t index{0};
  std::string time_iso;
  int active{0};
};

std::vector<FrameLine> parse_frame_lines(const std::string& out, const char* product_filter) {
  std::vector<FrameLine> lines;
  std::istringstream iss(out);
  std::string line;
  while (std::getline(iss, line)) {
    if (line.empty()) continue;
    // Skip non-TSV chatter if any
    std::istringstream ls(line);
    FrameLine fl;
    std::string idx, active;
    if (!std::getline(ls, fl.product, '\t')) continue;
    if (product_filter && fl.product != product_filter) continue;
    if (!std::getline(ls, idx, '\t')) continue;
    if (!std::getline(ls, fl.time_iso, '\t')) continue;
    if (!std::getline(ls, active, '\t')) continue;
    fl.index = static_cast<std::size_t>(std::stoul(idx));
    fl.active = std::stoi(active);
    lines.push_back(fl);
  }
  return lines;
}

void check_entries(const std::vector<s100::TimelineEntry>& entries, std::size_t n,
                   std::size_t expect_active) {
  CHECK(entries.size() == n);
  std::size_t active_count = 0;
  for (std::size_t i = 0; i < entries.size(); ++i) {
    CHECK(entries[i].index == i);
    if (entries[i].active) {
      ++active_count;
      CHECK(entries[i].index == expect_active);
    }
  }
  CHECK(active_count == 1);
}

}  // namespace

/** IHO-TIMELINE-UI / CTest timeline_ui — MUST oracles from docs/timeline-ui-contract.md. */
void test_timeline_ui() {
  const fs::path src = S100_SOURCE_DIR;
  const fs::path s104 = src / "testdata/s104/synthetic_waterlevel_4326.h5";
  const fs::path s111 = src / "testdata/s111/synthetic_current_4326.h5";
  CHECK(fs::exists(s104));
  CHECK(fs::exists(s111));
  if (!fs::exists(S100_CLI_PATH)) {
    std::cerr << "FAIL missing CLI S100_CLI_PATH=" << S100_CLI_PATH << "\n";
  }
  CHECK(fs::exists(S100_CLI_PATH));

  // --- Oracle 1–2: timeline_entries_water_level ---
  {
    s100::Dataset ds;
    s100::LoadError err;
    CHECK(s100::open_dataset(s104, ds, &err));
    CHECK(ds.water_level.has_value());
    auto& wl = *ds.water_level;
    CHECK(wl.frames.size() >= 2);
    std::vector<s100::TimelineEntry> entries;
    CHECK(s100::timeline_entries_water_level(wl, entries));
    check_entries(entries, wl.frames.size(), wl.active_frame);

    CHECK(wl.set_active_frame(1));
    CHECK(s100::timeline_entries_water_level(wl, entries));
    check_entries(entries, wl.frames.size(), 1);
    std::cout << "timeline_ui oracle 1–2 water_level entries ok\n";
  }

  // --- Oracle 3: timeline_entries_current ---
  {
    s100::Dataset ds;
    s100::LoadError err;
    CHECK(s100::open_dataset(s111, ds, &err));
    CHECK(ds.current.has_value());
    auto& cur = *ds.current;
    CHECK(cur.frames.size() >= 2);
    std::vector<s100::TimelineEntry> entries;
    CHECK(s100::timeline_entries_current(cur, entries));
    check_entries(entries, cur.frames.size(), cur.active_frame);

    CHECK(cur.set_active_frame(1));
    CHECK(s100::timeline_entries_current(cur, entries));
    check_entries(entries, cur.frames.size(), 1);
    std::cout << "timeline_ui oracle 3 current entries ok\n";
  }

  // --- Oracle 4: timeline_step_frame ---
  {
    s100::Dataset ds104, ds111;
    s100::LoadError err;
    CHECK(s100::open_dataset(s104, ds104, &err));
    CHECK(s100::open_dataset(s111, ds111, &err));
    auto& wl = *ds104.water_level;
    auto& cur = *ds111.current;
    CHECK(wl.set_active_frame(0));
    CHECK(cur.set_active_frame(0));
    CHECK(s100::timeline_step_frame(wl, +1));
    CHECK(wl.active_frame == 1);
    CHECK(s100::timeline_step_frame(cur, +1));
    CHECK(cur.active_frame == 1);

    const std::size_t last_wl = wl.frames.size() - 1;
    const std::size_t last_cur = cur.frames.size() - 1;
    CHECK(wl.set_active_frame(last_wl));
    CHECK(cur.set_active_frame(last_cur));
    CHECK(!s100::timeline_step_frame(wl, +1));
    CHECK(wl.active_frame == last_wl);
    CHECK(!s100::timeline_step_frame(cur, +1));
    CHECK(cur.active_frame == last_cur);
    std::cout << "timeline_ui oracle 4 step_frame ok\n";
  }

  // --- Oracle 5: CLI --s104 --list-frames ---
  {
    const auto r = run_cli(std::string("--s104 \"") + s104.string() + "\" --list-frames");
    if (r.exit_code != 0) {
      std::cerr << "CLI s104 list-frames exit=" << r.exit_code << " stderr=" << r.stderr_text << "\n";
    }
    CHECK(r.exit_code == 0);
    const auto lines = parse_frame_lines(r.stdout_text, "S-104");
    CHECK(lines.size() >= 2);
    std::size_t active = 0;
    for (const auto& l : lines) {
      CHECK(l.product == "S-104");
      if (l.active == 1) ++active;
    }
    CHECK(active == 1);
    std::cout << "timeline_ui oracle 5 CLI S-104 list-frames ok lines=" << lines.size() << "\n";
  }

  // --- Oracle 6: CLI --s111 --list-frames ---
  {
    const auto r = run_cli(std::string("--s111 \"") + s111.string() + "\" --list-frames");
    CHECK(r.exit_code == 0);
    const auto lines = parse_frame_lines(r.stdout_text, "S-111");
    CHECK(lines.size() >= 2);
    std::size_t active = 0;
    for (const auto& l : lines) {
      CHECK(l.product == "S-111");
      if (l.active == 1) ++active;
    }
    CHECK(active == 1);
    std::cout << "timeline_ui oracle 6 CLI S-111 list-frames ok lines=" << lines.size() << "\n";
  }

  // --- Oracle 7: --list-frames alone → non-zero ---
  {
    const auto r = run_cli("--list-frames");
    CHECK(r.exit_code != 0);
    std::cout << "timeline_ui oracle 7 list-frames alone non-zero ok\n";
  }

  // --- Oracle 8: --s104-frame 1 --list-frames → active index 1 ---
  {
    const auto r = run_cli(std::string("--s104 \"") + s104.string() +
                           "\" --s104-frame 1 --list-frames");
    CHECK(r.exit_code == 0);
    const auto lines = parse_frame_lines(r.stdout_text, "S-104");
    CHECK(lines.size() >= 2);
    bool found = false;
    for (const auto& l : lines) {
      if (l.active == 1) {
        CHECK(l.index == 1);
        found = true;
      }
    }
    CHECK(found);
    std::cout << "timeline_ui oracle 8 s104-frame 1 active ok\n";
  }

  std::cout << "timeline_ui OK (IHO-TIMELINE-UI)\n";
}
