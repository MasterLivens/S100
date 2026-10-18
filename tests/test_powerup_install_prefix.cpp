// IHO-S164-POWERUP-INSTALL-PREFIX — docs/powerup-install-prefix-contract.md rev1.
// Invoked by tests/check_powerup_install_prefix.cmake after cmake --install.
// CTest name: powerup_install_prefix. MUST NOT SKIP.
#include "s100/powerup.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

namespace fs = std::filesystem;
using s100::PowerUpSeverity;

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

}  // namespace

void test_powerup_install_prefix() {
  const char* env = std::getenv("S100_CATALOGUE_ROOT");
  SCHECK(env && *env);
  SCHECK(fs::is_directory(env));

  s100::reset_powerup_self_check_for_tests();
  auto opts = s100::default_powerup_options(s100::PowerUpSurface::Cli);
  SCHECK(opts.allow_smoke_na);
  SCHECK(opts.catalogue_root == fs::weakly_canonical(env) ||
         fs::equivalent(opts.catalogue_root, env));

  const auto r = s100::run_powerup_self_check(opts);
  s100::print_powerup_report(r);
  SCHECK(r.overall == PowerUpSeverity::Pass);
  SCHECK(find_item(r, "PU-2") != nullptr);
  SCHECK(find_item(r, "PU-2")->severity == PowerUpSeverity::Pass);
  SCHECK(find_item(r, "PU-4") != nullptr);
  SCHECK(find_item(r, "PU-4")->severity == PowerUpSeverity::Pass);
  SCHECK(find_item(r, "PU-4")->message.find("smoke_na_install_tree") != std::string::npos);
  SCHECK(find_item(r, "PU-5") != nullptr);
  SCHECK(find_item(r, "PU-5")->message.find("smoke_na_install_tree") != std::string::npos);

  // Shared DEFAULT path resolve under same root.
  const auto fc = s100::catalogue_path("catalogues/S-101/S-101_FC_2.0.0.xml");
  SCHECK(fs::is_regular_file(fc));
  SCHECK(fc.string().find("share/s100") != std::string::npos ||
         fc.string().find(env) != std::string::npos);

  std::cout << "powerup_install_prefix ok\n";
}
