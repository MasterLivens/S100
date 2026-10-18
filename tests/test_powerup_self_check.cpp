// IHO-S164-POWERUP — docs/powerup-self-check-contract.md rev1.
// CTest: powerup_self_check. MUST NOT SKIP, Linux + Windows.
#include "s100/powerup.hpp"
#include "s100/dataset.hpp"
#include "s100/session.hpp"
#include "support/test_data_roots.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

namespace fs = std::filesystem;
using s100::PowerUpSeverity;
using s100::PowerUpSurface;

#define SCHECK(cond)                                                                \
  do {                                                                              \
    if (!(cond)) {                                                                  \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";  \
      std::exit(1);                                                                 \
    }                                                                               \
  } while (0)

const s100::PowerUpItem* find_item(const s100::PowerUpReport& r, const char* id) {
  for (const auto& it : r.items)
    if (it.id == id) return &it;
  return nullptr;
}

void expect_item(const s100::PowerUpReport& r, const char* id, PowerUpSeverity want) {
  const auto* it = find_item(r, id);
  SCHECK(it != nullptr);
  if (it->severity != want) {
    std::cerr << "FAIL " << id << " severity want=" << static_cast<int>(want)
              << " got=" << static_cast<int>(it->severity) << " msg=" << it->message << "\n";
    std::exit(1);
  }
}

s100::PowerUpOptions good_opts(PowerUpSurface surface = PowerUpSurface::Library) {
  auto o = s100::default_powerup_options(surface);
  SCHECK(!o.catalogue_root.empty());
  SCHECK(fs::is_directory(o.catalogue_root));
  SCHECK(fs::exists(o.smoke_exchange));
  SCHECK(fs::is_regular_file(o.test_sa_cert));
  return o;
}

void oracle_happy_path() {
  std::cout << "oracle happy path (PU-1…6):\n";
  s100::reset_powerup_self_check_for_tests();
  const auto r = s100::run_powerup_self_check(good_opts(PowerUpSurface::Cli));
  s100::print_powerup_report(r);
  SCHECK(r.overall == PowerUpSeverity::Pass);
  SCHECK(r.items.size() == 6);
  expect_item(r, "PU-1", PowerUpSeverity::Pass);
  expect_item(r, "PU-2", PowerUpSeverity::Pass);
  expect_item(r, "PU-3", PowerUpSeverity::Pass);
  expect_item(r, "PU-4", PowerUpSeverity::Pass);
  expect_item(r, "PU-5", PowerUpSeverity::Pass);
  expect_item(r, "PU-6", PowerUpSeverity::Pass);  // CLI N/A → Pass
  SCHECK(!r.software_identity.empty());
  SCHECK(r.software_identity.find("s100-chart") != std::string::npos);
}

void oracle_pu6_viewer_warn() {
  std::cout << "oracle PU-6 viewer WARN stub:\n";
  s100::reset_powerup_self_check_for_tests();
  const auto r = s100::run_powerup_self_check(good_opts(PowerUpSurface::Viewer));
  expect_item(r, "PU-6", PowerUpSeverity::Warn);
  SCHECK(r.overall == PowerUpSeverity::Warn);
  SCHECK(find_item(r, "PU-6")->message.find("display_check_stub") != std::string::npos);
}

void oracle_pu2_missing_blocks() {
  std::cout << "oracle PU-2 missing catalogue → Block:\n";
  s100::reset_powerup_self_check_for_tests();
  auto o = good_opts();
  o.catalogue_root = fs::path(S100_SOURCE_DIR) / "testdata" / "s101" / "signatures" / "positive";
  // Not a product root — catalogues/… absent under this path.
  const auto r = s100::run_powerup_self_check(o);
  expect_item(r, "PU-2", PowerUpSeverity::Block);
  SCHECK(r.overall == PowerUpSeverity::Block);
}

void oracle_pu3_empty_blocks() {
  std::cout << "oracle PU-3 empty trust → Block:\n";
  s100::reset_powerup_self_check_for_tests();
  auto o = good_opts();
  auto empty = s100::security::TrustStore::make_empty();
  o.trust_for_pu3 = &empty;
  const auto r = s100::run_powerup_self_check(o);
  expect_item(r, "PU-3", PowerUpSeverity::Block);
  SCHECK(r.overall == PowerUpSeverity::Block);
}

void oracle_block_refuses_exchange_allows_dataset() {
  std::cout << "oracle Block refuses open_exchange_set; open_dataset still allowed:\n";
  s100::reset_powerup_self_check_for_tests();
  auto o = good_opts();
  auto empty = s100::security::TrustStore::make_empty();
  o.trust_for_pu3 = &empty;
  const auto r = s100::run_powerup_self_check(o);
  SCHECK(r.overall == PowerUpSeverity::Block);

  s100::ChartSession session;
  session.set_verify_trust(s100::security::TrustStore::for_tests(o.test_sa_cert));
  session.set_verify_options(o.smoke_verify_options);
  s100::LoadError err;
  const fs::path catalog = o.smoke_exchange / "CATALOG.XML";
  SCHECK(!session.open_exchange_set(catalog, &err, s100::security::VerifyPolicy::Enforce));
  SCHECK(err.message.find("POWERUP BLOCK") != std::string::npos);

  // SIG D7: standalone open_dataset remains allowed (UNVERIFIED).
  const fs::path cell = fs::path(S100_SOURCE_DIR) / "testdata/s101/101AA00DS0003.000";
  SCHECK(fs::is_regular_file(cell));
  s100::Dataset ds;
  s100::LoadError derr;
  SCHECK(s100::open_dataset(cell, ds, &derr));
  SCHECK(ds.stats().feature_count > 0);

  // Off policy still allowed under Block (diagnostics).
  s100::ChartSession off;
  SCHECK(off.open_exchange_set(catalog, &err, s100::security::VerifyPolicy::Off));
}

void oracle_security_self_test_thin() {
  std::cout << "oracle PU-4 security_self_test thin wrap:\n";
  auto o = good_opts();
  const auto trust = s100::security::TrustStore::for_tests(o.test_sa_cert);
  const auto a = s100::security_self_test(o.smoke_exchange, trust, o.smoke_verify_options);
  const auto b =
      s100::security::verify_exchange_set(o.smoke_exchange, trust, o.smoke_verify_options);
  SCHECK(a.status == b.status);
  SCHECK(a.sse == b.sse);
  SCHECK(a.status == s100::security::AuthStatus::Ok);
}

}  // namespace

void test_powerup_self_check() {
  oracle_happy_path();
  oracle_pu6_viewer_warn();
  oracle_pu2_missing_blocks();
  oracle_pu3_empty_blocks();
  oracle_block_refuses_exchange_allows_dataset();
  oracle_security_self_test_thin();

  // Leave process in Pass so later suite tests (Enforce) are not poisoned.
  s100::reset_powerup_self_check_for_tests();
  const auto r = s100::run_powerup_self_check(good_opts(PowerUpSurface::Cli));
  SCHECK(r.overall == PowerUpSeverity::Pass);
  std::cout << "powerup_self_check ok\n";
}
