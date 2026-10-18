// IHO-S164-SIG-PRODUCER — docs/part15-producer-code-contract.md rev1.
// CTest: exchange_signature_producer. MUST NOT SKIP, Linux + Windows.
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
using s100::security::sse_code;
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

void oracle_sse_map() {
  SCHECK(sse_code(SigRejectCode::ProducerCodeMismatch) == 109);
  SCHECK(std::string(s100::security::sse_message(SigRejectCode::ProducerCodeMismatch))
             .find("producer_code_mismatch") != std::string::npos);
}

void oracle_match() {
  std::cout << "oracle producer_match (CN 99ZZ == producerCode 99ZZ):\n";
  const auto root = sig_root() / "producer_match";
  SCHECK(fs::exists(root / "CATALOG.SIGN"));
  const auto r = verify_exchange_set(root, test_trust(), fixed_opts());
  std::cout << "  status=" << static_cast<int>(r.status) << " installed=" << r.installed.size()
            << " skipped=" << r.skipped.size() << "\n";
  SCHECK(r.status == AuthStatus::Ok);
  SCHECK(r.code == SigRejectCode::None);
  SCHECK(r.skipped.empty());
  SCHECK(r.installed.size() >= 2);

  s100::ChartSession session;
  session.set_verify_trust(test_trust());
  session.set_verify_options(fixed_opts());
  s100::LoadError err;
  SCHECK(session.open_exchange_set(root / "CATALOG.XML", &err, VerifyPolicy::Enforce));
  SCHECK(session.last_verification().status == AuthStatus::Ok);
  SCHECK(session.load_all_members(&err) >= 2);
}

void oracle_mismatch() {
  std::cout << "oracle producer_mismatch (CN 99ZZ != producerCode 00AA):\n";
  const auto root = sig_root() / "producer_mismatch";
  SCHECK(fs::exists(root / "CATALOG.SIGN"));
  const auto r = verify_exchange_set(root, test_trust(), fixed_opts());
  std::cout << "  status=" << static_cast<int>(r.status) << " installed=" << r.installed.size()
            << " skipped=" << r.skipped.size() << "\n";
  // Crypto Ok for the set; every dataset with producerCode mismatch is skipped.
  SCHECK(r.status == AuthStatus::Ok);
  SCHECK(r.skipped.size() >= 2);
  SCHECK(r.installed.empty());

  bool saw_reason = false;
  bool saw_109 = false;
  bool saw_code = false;
  for (const auto& f : r.files) {
    if (f.path.extension() != ".000") continue;
    // Reason first (primary), SSE 109 secondary (project-map, not App. B literal, not 126).
    SCHECK(f.message.find("producer_code_mismatch") != std::string::npos);
    saw_reason = true;
    SCHECK(f.sse == 109);
    saw_109 = true;
    SCHECK(f.code == SigRejectCode::ProducerCodeMismatch);
    saw_code = true;
    SCHECK(f.status == AuthStatus::SkippedResource);
  }
  SCHECK(saw_reason);
  SCHECK(saw_109);
  SCHECK(saw_code);

  // Session Enforce: mismatched cells must not load.
  s100::ChartSession session;
  session.set_verify_trust(test_trust());
  session.set_verify_options(fixed_opts());
  s100::LoadError err;
  SCHECK(session.open_exchange_set(root / "CATALOG.XML", &err, VerifyPolicy::Enforce));
  session.load_all_members(&err);
  SCHECK(session.datasets().empty());
}


void oracle_cn_unparseable() {
  // producerCode present + leaf CN not an IHO MRN → Enforce skip; no mismatch reason / no 109.
  std::cout << "oracle producer_cn_unparseable (CN not-an-iho-mrn + producerCode 99ZZ):\n";
  const auto root = sig_root() / "producer_cn_unparseable";
  SCHECK(fs::exists(root / "CATALOG.SIGN"));
  const auto r = verify_exchange_set(root, test_trust(), fixed_opts());
  std::cout << "  status=" << static_cast<int>(r.status) << " installed=" << r.installed.size()
            << " skipped=" << r.skipped.size() << "\n";
  SCHECK(r.status == AuthStatus::Ok);
  SCHECK(r.skipped.size() >= 2);
  SCHECK(r.installed.empty());

  bool saw_skip = false;
  for (const auto& f : r.files) {
    if (f.path.extension() != ".000") continue;
    SCHECK(f.status == AuthStatus::SkippedResource);
    SCHECK(f.code != SigRejectCode::ProducerCodeMismatch);
    SCHECK(f.sse != 109);
    SCHECK(f.sse == 0);
    SCHECK(f.message.find("producer_code_mismatch") == std::string::npos);
    SCHECK(f.message.find("producer_cn_unparseable") != std::string::npos);
    saw_skip = true;
  }
  SCHECK(saw_skip);

  s100::ChartSession session;
  session.set_verify_trust(test_trust());
  session.set_verify_options(fixed_opts());
  s100::LoadError err;
  SCHECK(session.open_exchange_set(root / "CATALOG.XML", &err, VerifyPolicy::Enforce));
  session.load_all_members(&err);
  SCHECK(session.datasets().empty());
}

void oracle_old_positive_na_pass() {
  // Old positive/ has no producerCode → N/A-pass; must stay Ok (do not break exchange_signature).
  std::cout << "oracle old positive N/A-pass (no producerCode):\n";
  const auto r = verify_exchange_set(sig_root() / "positive", test_trust(), fixed_opts());
  SCHECK(r.status == AuthStatus::Ok);
  SCHECK(r.skipped.empty());
  SCHECK(r.installed.size() >= 2);
}

void oracle_off_policy() {
  // Producer-code matching is Enforce-only; Off path is session-level (verify_exchange_set
  // itself always runs the crypto path when called). Confirm mismatch tree still reports
  // the reject code when verify is invoked directly (hook lives in verify_exchange_set).
  std::cout << "oracle Off policy session ignores verify gate:\n";
  s100::ChartSession session;
  s100::LoadError err;
  // Off: open succeeds without calling verify_exchange_set producer gate.
  SCHECK(session.open_exchange_set(sig_root() / "producer_mismatch" / "CATALOG.XML", &err,
                                   VerifyPolicy::Off));
}

}  // namespace

void test_exchange_signature_producer() {
  SCHECK(fs::exists(sig_root() / "producer_match" / "CATALOG.SIGN"));
  SCHECK(fs::exists(sig_root() / "producer_mismatch" / "CATALOG.SIGN"));
  SCHECK(fs::exists(sig_root() / "producer_cn_unparseable" / "CATALOG.SIGN"));
  oracle_sse_map();
  oracle_match();
  oracle_mismatch();
  oracle_cn_unparseable();
  oracle_old_positive_na_pass();
  oracle_off_policy();
  std::cout << "exchange_signature_producer ok\n";
}
