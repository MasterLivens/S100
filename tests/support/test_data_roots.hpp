// TEST-DATA-ROOTS — docs/test-data-roots-contract.md rev 1.
// One place that decides where test data lives:
//   * CHS real data (gitignored, testing-only licence): S100_CHS_ROOT, the `testdata/chs` level
//     (the directory that contains COMBINED/). Set and non-empty -> strict, never falls back.
//     Unset/empty -> S100_SOURCE_DIR/testdata/chs if it exists (strict too), else not exercised.
//   * Committed S-101 fixtures: S100_TESTDATA if set and non-empty, else the absolute
//     S100_SOURCE_DIR/testdata/s101 (never cwd-relative).
// A bad root prints FAIL and exits 1; absent CHS is a pass with a message, never a CTest SKIP.
#pragma once

#ifndef S100_SOURCE_DIR
#error "S100_SOURCE_DIR required (test_data_roots.hpp)"
#endif

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace s100test {

struct ChsRoot {
  bool resolved = false;          // true iff CHS data is used (env or default)
  bool from_env = false;          // true iff the root came from S100_CHS_ROOT
  std::filesystem::path root;     // absolute; the testdata/chs level
};

namespace detail {

// Environment value as a path; empty path when unset or empty (both count as unset).
inline std::filesystem::path env_path(const char* name) {
#ifdef _WIN32
  const std::wstring wname(name, name + std::char_traits<char>::length(name));
  const wchar_t* v = _wgetenv(wname.c_str());
  if (!v || !*v) return {};
  return std::filesystem::path(std::wstring(v));
#else
  const char* v = std::getenv(name);
  if (!v || !*v) return {};
  return std::filesystem::path(std::string(v));
#endif
}

// Printable form, native separators, no quoting/escaping (unlike operator<<).
inline std::string show(const std::filesystem::path& p) {
#if defined(_WIN32)
  const auto u = p.u8string();
  return std::string(u.begin(), u.end());
#else
  return p.string();
#endif
}

inline std::filesystem::path default_chs_root() {
  return std::filesystem::path(S100_SOURCE_DIR) / "testdata" / "chs";
}

// The six layout entries every resolved CHS root must carry (contract "Expected layout").
inline std::vector<std::filesystem::path> chs_layout() {
  namespace fs = std::filesystem;
  const fs::path s100 = fs::path("COMBINED") / "S100_ROOT";
  const fs::path s101 = s100 / "S-101" / "DATASET_FILES" / "CA00";
  return {
      s100 / "CATALOG.XML",
      s101 / "101CA00P468N0712W" / "2" / "101CA00P468N0712W.000",
      s101 / "101CA00P468N0712W" / "2" / "101CA00P468N0712W.001",
      s101 / "101CA00T460N0720W" / "6" / "101CA00T460N0720W.000",
      s100 / "S-102" / "DATASET_FILES" / "CA00" / "102CA0064682N07116W" / "16" /
          "102CA0064682N07116W.h5",
      fs::path("COMBINED") / "ENC_ROOT" / "CA" / "CA54GN2A" / "2" / "0" / "CA54GN2A.000",
  };
}

[[noreturn]] inline void fail(const std::string& msg) {
  std::cout.flush();
  std::cerr << "FAIL test_data_roots: " << msg << "\n";
  std::exit(1);
}

inline ChsRoot resolve_chs_root() {
  namespace fs = std::filesystem;
  ChsRoot r;
  std::error_code ec;
  const fs::path env = env_path("S100_CHS_ROOT");
  if (!env.empty()) {
    r.from_env = true;
    r.root = fs::absolute(env, ec);  // relative -> absolute against cwd, once
    if (ec) r.root = env;
    if (!fs::exists(r.root))
      fail("S100_CHS_ROOT=\"" + show(r.root) +
           "\" does not exist (S100_CHS_ROOT must point at the testdata/chs level, the "
           "directory that contains COMBINED/; set-but-missing never falls back to the default)");
    // NC1: old meaning (.../COMBINED/S100_ROOT) is detected, not auto-corrected.
    if (fs::exists(r.root / "CATALOG.XML") && !fs::exists(r.root / "COMBINED"))
      fail("S100_CHS_ROOT=\"" + show(r.root) +
           "\" looks like the old .../COMBINED/S100_ROOT value; S100_CHS_ROOT now means the "
           "testdata/chs level (the directory that contains COMBINED/): set it to its "
           "grandparent \"" + show(r.root.parent_path().parent_path()) + "\"");
  } else {
    const fs::path def = default_chs_root();
    if (!fs::exists(def)) return r;  // not exercised
    r.root = def;
  }
  std::string missing;
  for (const auto& rel : chs_layout())
    if (!fs::exists(r.root / rel)) missing += "\n  missing: " + show(r.root / rel);
  if (!missing.empty())
    fail(std::string(r.from_env ? "S100_CHS_ROOT" : "default CHS root") + " \"" + show(r.root) +
         "\" lacks the CHS layout (a half-copied CHS tree is a bug, not optional data):" + missing);
  r.resolved = true;
  return r;
}

}  // namespace detail

// Resolved once per process; FAIL + std::exit(1) on a bad root.
inline const ChsRoot& chs_root() {
  static const ChsRoot r = detail::resolve_chs_root();
  return r;
}

inline std::filesystem::path chs_s100_root() { return chs_root().root / "COMBINED" / "S100_ROOT"; }
inline std::filesystem::path chs_s101_dir() { return chs_s100_root() / "S-101"; }
inline std::filesystem::path chs_enc_root() { return chs_root().root / "COMBINED" / "ENC_ROOT"; }

// Prints one line for this call site (stdout) and returns true iff a CHS root is resolved.
inline bool chs_exercised(const char* tag) {
  const ChsRoot& r = chs_root();
  if (r.resolved)
    std::cout << "chs: " << tag << " using \"" << detail::show(r.root) << "\" ("
              << (r.from_env ? "S100_CHS_ROOT" : "default") << ")\n";
  else
    std::cout << "chs: " << tag << " not exercised (S100_CHS_ROOT unset, \""
              << detail::show(detail::default_chs_root()) << "\" absent)\n";
  return r.resolved;
}

// S100_TESTDATA verbatim if set and non-empty (FAIL when it lacks 101AA00DS0008.000),
// else the absolute S100_SOURCE_DIR/testdata/s101.
inline std::filesystem::path fixture_root() {
  namespace fs = std::filesystem;
  static const fs::path root = [] {
    const fs::path env = detail::env_path("S100_TESTDATA");
    if (env.empty()) return fs::path(S100_SOURCE_DIR) / "testdata" / "s101";
    if (!fs::exists(env / "101AA00DS0008.000"))
      detail::fail("S100_TESTDATA=\"" + detail::show(env) + "\" lacks 101AA00DS0008.000 (cwd \"" +
                   detail::show(fs::current_path()) + "\"); unset it to use \"" +
                   detail::show(fs::path(S100_SOURCE_DIR) / "testdata" / "s101") + "\"");
    return env;
  }();
  return root;
}

}  // namespace s100test
