#pragma once

#include "s100/security/trust_store.hpp"
#include "s100/security/types.hpp"
#include "s100/security/verify.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace s100 {

enum class PowerUpSeverity { Pass, Warn, Block };

struct PowerUpItem {
  std::string id;  // "PU-1" … "PU-6"
  PowerUpSeverity severity{PowerUpSeverity::Pass};
  std::string message;
};

enum class PowerUpSurface { Cli, Viewer, Library };

struct PowerUpOptions {
  /** Product install / source root; PU-2 paths are relative to this (catalogues/…). */
  std::filesystem::path catalogue_root;
  /** Synthetic mini exchange set for PU-4 / PU-5 (TEST-ONLY fixtures). */
  std::filesystem::path smoke_exchange;
  /** SA cert for TrustStore::for_tests (PU-4 / PU-5 smoke only). */
  std::filesystem::path test_sa_cert;
  /** Injected verify_time for PU-4/PU-5 — never wall clock (SIG D2). */
  security::VerifyOptions smoke_verify_options{};
  /** Optional override for PU-3; nullptr → TrustStore::builtin_iho_52(). */
  const security::TrustStore* trust_for_pu3{nullptr};
  PowerUpSurface surface{PowerUpSurface::Library};
  /** When true (install-tree / env override), missing smoke → Pass smoke_na_install_tree. */
  bool allow_smoke_na{false};
};

struct PowerUpReport {
  PowerUpSeverity overall{PowerUpSeverity::Pass};  // max of items (Block > Warn > Pass)
  std::string software_identity;                   // PU-1
  std::vector<PowerUpItem> items;
};

/** Thin wrapper — MUST call verify_exchange_set (no second crypto path). */
security::VerificationReport security_self_test(const std::filesystem::path& exchange_root,
                                                const security::TrustStore& trust,
                                                const security::VerifyOptions& options);

/** Run at process start (CLI/viewer MUST) or lazy before first operational open. Idempotent
 *  when opts match the cached run; always updates the process-wide last report. */
PowerUpReport run_powerup_self_check(const PowerUpOptions& opts = {});

/** Last report from run_powerup_self_check (default-constructed if never run). */
const PowerUpReport& last_powerup_report();

/** True when a report has been produced in this process. */
bool powerup_has_run();

/** Test helper: clear cached report so the next open can re-run or re-inject. */
void reset_powerup_self_check_for_tests();

/** Format one-line status + per-item lines. Warn items also mirrored to stderr by helper. */
std::string format_powerup_report(const PowerUpReport& r);
void print_powerup_report(const PowerUpReport& r);

/** Default options from compile-time pins (S100_CATALOGUE_ROOT / smoke paths / version). */
PowerUpOptions default_powerup_options(PowerUpSurface surface = PowerUpSurface::Library);

/** Resolve catalogue root: env S100_CATALOGUE_ROOT → compile pin if exists →
 *  exe-relative ../share/s100 (install layout). Optional override wins when non-empty dir. */
std::filesystem::path resolve_catalogue_root(
    const std::filesystem::path& override_root = {});

/** Join resolve_catalogue_root() with a path relative to that root (e.g. catalogues/S-101/…). */
std::filesystem::path catalogue_path(const std::filesystem::path& relative_under_root,
                                     const std::filesystem::path& override_root = {});

}  // namespace s100
