#include "s100/powerup.hpp"
#include "s100/session.hpp"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <sstream>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#ifndef S100_VERSION
#define S100_VERSION ""
#endif
#ifndef S100_CATALOGUE_ROOT
#define S100_CATALOGUE_ROOT ""
#endif
#ifndef S100_POWERUP_SMOKE_EXCHANGE
#define S100_POWERUP_SMOKE_EXCHANGE ""
#endif
#ifndef S100_POWERUP_TEST_SA
#define S100_POWERUP_TEST_SA ""
#endif

namespace s100 {
namespace {

std::mutex g_mu;
bool g_has_run = false;
PowerUpReport g_last{};

std::filesystem::path executable_dir() {
  namespace fs = std::filesystem;
#if defined(_WIN32)
  wchar_t buf[MAX_PATH];
  DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
  if (n == 0 || n >= MAX_PATH) return {};
  return fs::path(buf).parent_path();
#else
  std::error_code ec;
  auto p = fs::read_symlink("/proc/self/exe", ec);
  if (ec) return {};
  return p.parent_path();
#endif
}

bool dir_ok(const std::filesystem::path& p) {
  std::error_code ec;
  return !p.empty() && std::filesystem::is_directory(p, ec) && !ec;
}

PowerUpSeverity max_sev(PowerUpSeverity a, PowerUpSeverity b) {
  const auto rank = [](PowerUpSeverity s) {
    switch (s) {
      case PowerUpSeverity::Block:
        return 2;
      case PowerUpSeverity::Warn:
        return 1;
      case PowerUpSeverity::Pass:
        return 0;
    }
    return 0;
  };
  return rank(a) >= rank(b) ? a : b;
}

const char* sev_name(PowerUpSeverity s) {
  switch (s) {
    case PowerUpSeverity::Pass:
      return "PASS";
    case PowerUpSeverity::Warn:
      return "WARN";
    case PowerUpSeverity::Block:
      return "BLOCK";
  }
  return "PASS";
}

bool is_printable_identity(const std::string& s) {
  if (s.empty()) return false;
  for (unsigned char c : s) {
    if (c < 0x20 || c > 0x7e) return false;
  }
  return true;
}

std::string software_identity_string() {
  const std::string v = S100_VERSION;
  if (!v.empty()) return std::string("s100-chart ") + v;
  return {};
}

void add_item(PowerUpReport& r, const char* id, PowerUpSeverity sev, std::string msg) {
  PowerUpItem it;
  it.id = id;
  it.severity = sev;
  it.message = std::move(msg);
  r.overall = max_sev(r.overall, sev);
  r.items.push_back(std::move(it));
}

void check_pu1(PowerUpReport& r) {
  r.software_identity = software_identity_string();
  if (!is_printable_identity(r.software_identity)) {
    add_item(r, "PU-1", PowerUpSeverity::Block,
             "software identity/version empty or non-printable (S100_VERSION)");
    return;
  }
  add_item(r, "PU-1", PowerUpSeverity::Pass, r.software_identity);
}

void check_pu2(PowerUpReport& r, const std::filesystem::path& catalogue_root) {
  namespace fs = std::filesystem;
  if (catalogue_root.empty() || !fs::is_directory(catalogue_root)) {
    add_item(r, "PU-2", PowerUpSeverity::Block,
             "catalogue root not resolvable: " + catalogue_root.string());
    return;
  }

  const std::vector<fs::path> must = {
      "catalogues/S-101/S-101_FC_2.0.0.xml",
      "catalogues/S-101/PC_2.0.0/portrayal_catalogue.xml",
      "catalogues/S-102/S-102_FC_3.0.0.xml",
      "catalogues/S-102/PC_3.0.0/portrayal_catalogue.xml",
      "catalogues/S-104/S-104_FC_2.0.0.xml",
      "catalogues/S-111/S-111_FC_2.0.0.xml",
      "catalogues/S-111/PC_2.0.0/portrayal_catalogue.xml",
      "catalogues/S-124/S-124_FC_2.0.0.xml",
      "catalogues/S-124/PC_2.0.0/portrayal_catalogue.xml",
  };

  std::vector<std::string> missing;
  for (const auto& rel : must) {
    const fs::path p = catalogue_root / rel;
    if (!fs::is_regular_file(p)) missing.push_back(rel.string());
  }

  const fs::path s104_pc = catalogue_root / "catalogues/S-104/PC_2.0.0";
  const bool forged_pc = fs::exists(s104_pc);

  if (!missing.empty() || forged_pc) {
    std::ostringstream oss;
    if (!missing.empty()) {
      oss << "missing critical catalogues:";
      for (const auto& m : missing) oss << " " << m;
    }
    if (forged_pc) {
      if (!missing.empty()) oss << "; ";
      oss << "S-104 PC_2.0.0 must not exist (official N/A)";
    }
    add_item(r, "PU-2", PowerUpSeverity::Block, oss.str());
    return;
  }
  add_item(r, "PU-2", PowerUpSeverity::Pass,
           "critical catalogues present (S-98 20.1 / S-164 §2.1.1; sha in catalogue_provenance)");
}

void check_pu3(PowerUpReport& r, const security::TrustStore* trust_override) {
  security::TrustStore builtin = security::TrustStore::builtin_iho_52();
  const security::TrustStore& store = trust_override ? *trust_override : builtin;
  if (store.empty() || store.size() < 1) {
    add_item(r, "PU-3", PowerUpSeverity::Block,
             "SA trust store empty (SSE 105 semantics; builtin_iho_52 has no anchors)");
    return;
  }
  add_item(r, "PU-3", PowerUpSeverity::Pass,
           "trust anchors=" + std::to_string(store.size()) + " (builtin_iho_52 or override)");
}

void check_pu4(PowerUpReport& r, const PowerUpOptions& opts) {
  namespace fs = std::filesystem;
  if (opts.smoke_exchange.empty() || !fs::exists(opts.smoke_exchange) ||
      opts.test_sa_cert.empty() || !fs::is_regular_file(opts.test_sa_cert)) {
    if (opts.allow_smoke_na) {
      add_item(r, "PU-4", PowerUpSeverity::Pass, "smoke_na_install_tree");
      return;
    }
    if (opts.smoke_exchange.empty() || !fs::exists(opts.smoke_exchange)) {
      add_item(r, "PU-4", PowerUpSeverity::Block,
               "smoke exchange fixture missing: " + opts.smoke_exchange.string());
      return;
    }
    add_item(r, "PU-4", PowerUpSeverity::Block,
             "test SA cert missing: " + opts.test_sa_cert.string());
    return;
  }
  security::VerifyOptions vo = opts.smoke_verify_options;
  if (vo.verify_time == std::chrono::system_clock::time_point{}) {
    vo.verify_time = std::chrono::system_clock::from_time_t(static_cast<std::time_t>(1768435200));
  }
  vo.test_mode = true;
  const auto trust = security::TrustStore::for_tests(opts.test_sa_cert);
  const auto report = security_self_test(opts.smoke_exchange, trust, vo);
  if (report.status != security::AuthStatus::Ok) {
    add_item(r, "PU-4", PowerUpSeverity::Block,
             "security_self_test failed sse=" + std::to_string(report.sse) + " " + report.message);
    return;
  }
  add_item(r, "PU-4", PowerUpSeverity::Pass, "security_self_test ok (verify_exchange_set mini gold)");
}

void check_pu5(PowerUpReport& r, const PowerUpOptions& opts) {
  namespace fs = std::filesystem;
  if (opts.smoke_exchange.empty() || !fs::exists(opts.smoke_exchange) ||
      opts.test_sa_cert.empty() || !fs::is_regular_file(opts.test_sa_cert)) {
    if (opts.allow_smoke_na) {
      add_item(r, "PU-5", PowerUpSeverity::Pass, "smoke_na_install_tree");
      return;
    }
    if (opts.smoke_exchange.empty() || !fs::exists(opts.smoke_exchange)) {
      add_item(r, "PU-5", PowerUpSeverity::Block,
               "smoke exchange fixture missing: " + opts.smoke_exchange.string());
      return;
    }
    add_item(r, "PU-5", PowerUpSeverity::Block,
             "test SA cert missing: " + opts.test_sa_cert.string());
    return;
  }
  security::VerifyOptions vo = opts.smoke_verify_options;
  if (vo.verify_time == std::chrono::system_clock::time_point{}) {
    vo.verify_time = std::chrono::system_clock::from_time_t(static_cast<std::time_t>(1768435200));
  }
  vo.test_mode = true;

  ChartSession session;
  session.set_verify_trust(security::TrustStore::for_tests(opts.test_sa_cert));
  session.set_verify_options(vo);
  session.set_powerup_gate_bypass(true);
  LoadError err;
  const fs::path catalog = fs::is_directory(opts.smoke_exchange)
                               ? (opts.smoke_exchange / "CATALOG.XML")
                               : opts.smoke_exchange;
  if (!session.open_exchange_set(catalog, &err, security::VerifyPolicy::Enforce)) {
    add_item(r, "PU-5", PowerUpSeverity::Block,
             "open_exchange_set Enforce failed: " + err.message);
    return;
  }
  if (session.last_verification().status != security::AuthStatus::Ok) {
    add_item(r, "PU-5", PowerUpSeverity::Block, "open_exchange_set verification not Ok");
    return;
  }
  add_item(r, "PU-5", PowerUpSeverity::Pass, "open_exchange_set Enforce smoke ok");
}

void check_pu6(PowerUpReport& r, PowerUpSurface surface) {
  switch (surface) {
    case PowerUpSurface::Cli:
      add_item(r, "PU-6", PowerUpSeverity::Pass, "N/A (CLI)");
      break;
    case PowerUpSurface::Viewer:
      add_item(r, "PU-6", PowerUpSeverity::Warn, "display_check_stub");
      break;
    case PowerUpSurface::Library:
      add_item(r, "PU-6", PowerUpSeverity::Pass, "N/A (library)");
      break;
  }
}

}  // namespace

security::VerificationReport security_self_test(const std::filesystem::path& exchange_root,
                                                const security::TrustStore& trust,
                                                const security::VerifyOptions& options) {
  return security::verify_exchange_set(exchange_root, trust, options);
}

std::filesystem::path resolve_catalogue_root(const std::filesystem::path& override_root) {
  namespace fs = std::filesystem;
  if (dir_ok(override_root)) return fs::weakly_canonical(override_root);

  if (const char* env = std::getenv("S100_CATALOGUE_ROOT"); env && *env) {
    fs::path ep(env);
    if (dir_ok(ep)) return fs::weakly_canonical(ep);
  }

  const fs::path compile_pin = S100_CATALOGUE_ROOT;
  if (dir_ok(compile_pin)) return fs::weakly_canonical(compile_pin);

  const fs::path exe = executable_dir();
  if (!exe.empty()) {
    const fs::path cand = exe / ".." / "share" / "s100";
    if (dir_ok(cand)) return fs::weakly_canonical(cand);
  }
  return compile_pin;
}

std::filesystem::path catalogue_path(const std::filesystem::path& relative_under_root,
                                     const std::filesystem::path& override_root) {
  return resolve_catalogue_root(override_root) / relative_under_root;
}

PowerUpOptions default_powerup_options(PowerUpSurface surface) {
  PowerUpOptions o;
  const char* env = std::getenv("S100_CATALOGUE_ROOT");
  const bool env_set = env && *env;
  o.catalogue_root = resolve_catalogue_root();
  o.smoke_verify_options.verify_time =
      std::chrono::system_clock::from_time_t(static_cast<std::time_t>(1768435200));
  o.smoke_verify_options.test_mode = true;
  o.surface = surface;
  if (env_set) {
    o.allow_smoke_na = true;
    o.smoke_exchange.clear();
    o.test_sa_cert.clear();
  } else {
    o.smoke_exchange = S100_POWERUP_SMOKE_EXCHANGE;
    o.test_sa_cert = S100_POWERUP_TEST_SA;
    o.allow_smoke_na = false;
  }
  return o;
}

PowerUpReport run_powerup_self_check(const PowerUpOptions& opts_in) {
  PowerUpOptions opts = opts_in;
  if (opts.catalogue_root.empty()) opts.catalogue_root = resolve_catalogue_root();
  if (!opts.allow_smoke_na) {
    if (opts.smoke_exchange.empty()) opts.smoke_exchange = S100_POWERUP_SMOKE_EXCHANGE;
    if (opts.test_sa_cert.empty()) opts.test_sa_cert = S100_POWERUP_TEST_SA;
  }
  if (opts.smoke_verify_options.verify_time == std::chrono::system_clock::time_point{}) {
    opts.smoke_verify_options.verify_time =
        std::chrono::system_clock::from_time_t(static_cast<std::time_t>(1768435200));
  }

  PowerUpReport r;
  r.overall = PowerUpSeverity::Pass;
  check_pu1(r);
  check_pu2(r, opts.catalogue_root);
  check_pu3(r, opts.trust_for_pu3);
  check_pu4(r, opts);
  check_pu5(r, opts);
  check_pu6(r, opts.surface);

  {
    std::lock_guard<std::mutex> lock(g_mu);
    g_last = r;
    g_has_run = true;
  }
  return r;
}

const PowerUpReport& last_powerup_report() {
  std::lock_guard<std::mutex> lock(g_mu);
  return g_last;
}

bool powerup_has_run() {
  std::lock_guard<std::mutex> lock(g_mu);
  return g_has_run;
}

void reset_powerup_self_check_for_tests() {
  std::lock_guard<std::mutex> lock(g_mu);
  g_has_run = false;
  g_last = PowerUpReport{};
}

std::string format_powerup_report(const PowerUpReport& r) {
  std::ostringstream oss;
  oss << "powerup status=" << sev_name(r.overall) << "\n";
  for (const auto& it : r.items) {
    oss << "  " << it.id << " " << sev_name(it.severity) << ": " << it.message << "\n";
  }
  return oss.str();
}

void print_powerup_report(const PowerUpReport& r) {
  std::cout << format_powerup_report(r);
  for (const auto& it : r.items) {
    if (it.severity == PowerUpSeverity::Warn) {
      std::cerr << "POWERUP WARN: " << it.id << ": " << it.message << "\n";
    }
    if (it.severity == PowerUpSeverity::Block) {
      std::cerr << "POWERUP BLOCK: " << it.id << ": " << it.message << "\n";
    }
  }
}

}  // namespace s100
