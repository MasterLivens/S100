// IHO-S164-SIG-AGGREGATOR — docs/part15-aggregator-producer-contract.md rev1.
// CTest: exchange_signature_aggregator. MUST NOT SKIP, Linux + Windows.
#include "s100/security/security.hpp"
#include "s100/session.hpp"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <string>

namespace {

namespace fs = std::filesystem;
using s100::security::AuthStatus;
using s100::security::SigRejectCode;
using s100::security::TrustStore;
using s100::security::VerifyOptions;
using s100::security::VerifyPolicy;
using s100::security::verify_exchange_set;

#define SCHECK(cond)                                                                \
  do {                                                                              \
    if (!(cond)) {                                                                  \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << " " << #cond << "\n";  \
      std::exit(1);                                                                 \
    }                                                                               \
  } while (0)

fs::path sig_root() { return fs::path(S100_SOURCE_DIR) / "testdata/s101/signatures"; }

VerifyOptions fixed_opts(bool test_mode = true) {
  VerifyOptions o;
  // 2026-01-15T00:00:00Z
  o.verify_time = std::chrono::system_clock::from_time_t(static_cast<std::time_t>(1768435200));
  o.test_mode = test_mode;
  return o;
}

TrustStore test_trust() {
  return TrustStore::for_tests(sig_root() / "keys" / "test_sa.crt");
}

void oracle_aggregator_exempt() {
  // ST=DATA_AGGREGATOR, CN 00AG, producerCode=99MB ≠ CN → Enforce still installs.
  std::cout << "oracle aggregator_exempt (DATA_AGGREGATOR + member producerCode ≠ CN):\n";
  const auto root = sig_root() / "aggregator_exempt";
  SCHECK(fs::exists(root / "CATALOG.SIGN"));
  const auto r = verify_exchange_set(root, test_trust(), fixed_opts());
  std::cout << "  status=" << static_cast<int>(r.status) << " installed=" << r.installed.size()
            << " skipped=" << r.skipped.size() << "\n";
  SCHECK(r.status == AuthStatus::Ok);
  SCHECK(r.code == SigRejectCode::None);
  SCHECK(r.skipped.empty());
  SCHECK(r.installed.size() >= 2);

  bool saw_warn = false;
  for (const auto& f : r.files) {
    if (f.path.extension() != ".000") continue;
    SCHECK(f.status == AuthStatus::Ok);
    SCHECK(f.code == SigRejectCode::None);
    SCHECK(f.sse == 0);
    SCHECK(f.sse != 109);
    SCHECK(f.message.find("producer_code_mismatch") == std::string::npos);
    if (f.message.find("aggregator_producer_code_divergent") != std::string::npos) {
      saw_warn = true;
    }
  }
  // WARN is SHOULD/MUST-log preferred; assert when present (engine emits it).
  SCHECK(saw_warn);

  s100::ChartSession session;
  session.set_verify_trust(test_trust());
  session.set_verify_options(fixed_opts());
  s100::LoadError err;
  SCHECK(session.open_exchange_set(root / "CATALOG.XML", &err, VerifyPolicy::Enforce));
  SCHECK(session.last_verification().status == AuthStatus::Ok);
  SCHECK(session.load_all_members(&err) >= 2);
  SCHECK(!session.datasets().empty());
}

void oracle_producer_mismatch_unchanged() {
  // DATA_PRODUCER path must still skip on mismatch (AG-2).
  std::cout << "oracle DATA_PRODUCER mismatch still skips:\n";
  const auto r = verify_exchange_set(sig_root() / "producer_mismatch", test_trust(), fixed_opts());
  SCHECK(r.status == AuthStatus::Ok);
  SCHECK(r.skipped.size() >= 2);
  bool saw = false;
  for (const auto& f : r.files) {
    if (f.path.extension() != ".000") continue;
    SCHECK(f.message.find("producer_code_mismatch") != std::string::npos);
    SCHECK(f.sse == 109);
    saw = true;
  }
  SCHECK(saw);
}

}  // namespace

void test_exchange_signature_aggregator() {
  SCHECK(fs::exists(sig_root() / "aggregator_exempt" / "CATALOG.SIGN"));
  oracle_aggregator_exempt();
  oracle_producer_mismatch_unchanged();
  std::cout << "exchange_signature_aggregator ok\n";
}
