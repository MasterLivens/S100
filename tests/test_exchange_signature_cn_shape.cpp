// IHO-S164-SIG-CN-SHAPE — docs/part15-cn-shape-contract.md rev1.
// CTest: exchange_signature_cn_shape. MUST NOT SKIP, Linux + Windows.
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
using s100::security::OfficialClass;
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

VerifyOptions fixed_opts() {
  VerifyOptions o;
  // 2026-01-15T00:00:00Z
  o.verify_time = std::chrono::system_clock::from_time_t(static_cast<std::time_t>(1768435200));
  o.test_mode = true;
  return o;
}

TrustStore test_trust() {
  return TrustStore::for_tests(sig_root() / "keys" / "test_sa.crt");
}

void oracle_cn_shape_ok() {
  std::cout << "oracle cn_shape_ok (urn:mrn:iho:org:99ZZ:90001 → Official + install):\n";
  const auto root = sig_root() / "cn_shape_ok";
  SCHECK(fs::exists(root / "CATALOG.SIGN"));
  const auto r = verify_exchange_set(root, test_trust(), fixed_opts());
  std::cout << "  status=" << static_cast<int>(r.status) << " official="
            << static_cast<int>(r.official) << " installed=" << r.installed.size()
            << " skipped=" << r.skipped.size() << "\n";
  SCHECK(r.status == AuthStatus::Ok);
  SCHECK(r.code == SigRejectCode::None);
  SCHECK(r.skipped.empty());
  SCHECK(r.installed.size() >= 2);
  SCHECK(r.official == OfficialClass::Official);
  bool saw_file_official = false;
  for (const auto& f : r.files) {
    if (f.path.extension() != ".000") continue;
    SCHECK(f.status == AuthStatus::Ok);
    SCHECK(f.official == OfficialClass::Official);
    saw_file_official = true;
  }
  SCHECK(saw_file_official);

  s100::ChartSession session;
  session.set_verify_trust(test_trust());
  session.set_verify_options(fixed_opts());
  s100::LoadError err;
  SCHECK(session.open_exchange_set(root / "CATALOG.XML", &err, VerifyPolicy::Enforce));
  SCHECK(session.last_verification().status == AuthStatus::Ok);
  SCHECK(session.last_verification().official == OfficialClass::Official);
  SCHECK(session.load_all_members(&err) >= 2);
}

void oracle_cn_shape_bad() {
  std::cout << "oracle cn_shape_bad (bad CN + crypto Ok → NonOfficial + still install):\n";
  const auto root = sig_root() / "cn_shape_bad";
  SCHECK(fs::exists(root / "CATALOG.SIGN"));
  const auto r = verify_exchange_set(root, test_trust(), fixed_opts());
  std::cout << "  status=" << static_cast<int>(r.status) << " official="
            << static_cast<int>(r.official) << " installed=" << r.installed.size()
            << " skipped=" << r.skipped.size() << "\n";
  // No producerCode → PRODUCER N/A-pass; this slice MUST NOT skip.
  SCHECK(r.status == AuthStatus::Ok);
  SCHECK(r.code == SigRejectCode::None);
  SCHECK(r.skipped.empty());
  SCHECK(r.installed.size() >= 2);
  SCHECK(r.official == OfficialClass::NonOfficial);
  bool saw = false;
  for (const auto& f : r.files) {
    if (f.path.extension() != ".000") continue;
    SCHECK(f.status == AuthStatus::Ok);
    SCHECK(f.code == SigRejectCode::None);
    SCHECK(f.sse == 0);
    SCHECK(f.message.find("producer_code_mismatch") == std::string::npos);
    SCHECK(f.message.find("producer_cn_unparseable") == std::string::npos);
    SCHECK(f.official == OfficialClass::NonOfficial);
    saw = true;
  }
  SCHECK(saw);

  s100::ChartSession session;
  session.set_verify_trust(test_trust());
  session.set_verify_options(fixed_opts());
  s100::LoadError err;
  SCHECK(session.open_exchange_set(root / "CATALOG.XML", &err, VerifyPolicy::Enforce));
  SCHECK(session.last_verification().status == AuthStatus::Ok);
  SCHECK(session.last_verification().official == OfficialClass::NonOfficial);
  SCHECK(session.load_all_members(&err) >= 2);
  SCHECK(!session.datasets().empty());
}

void oracle_old_positive_still_official() {
  std::cout << "oracle old positive still Official (form-1 CN):\n";
  const auto r = verify_exchange_set(sig_root() / "positive", test_trust(), fixed_opts());
  SCHECK(r.status == AuthStatus::Ok);
  SCHECK(r.official == OfficialClass::Official);
  SCHECK(r.skipped.empty());
}

}  // namespace

void test_exchange_signature_cn_shape() {
  SCHECK(fs::exists(sig_root() / "cn_shape_ok" / "CATALOG.SIGN"));
  SCHECK(fs::exists(sig_root() / "cn_shape_bad" / "CATALOG.SIGN"));
  oracle_cn_shape_ok();
  oracle_cn_shape_bad();
  oracle_old_positive_still_official();
  std::cout << "exchange_signature_cn_shape ok\n";
}
