// IHO-S164-SIG — docs/part15-signature-contract.md rev 1.1.
// CTest: exchange_signature. MUST NOT SKIP, Linux + Windows.
#include "s100/security/security.hpp"
#include "s100/session.hpp"
#include "support/test_data_roots.hpp"

#include <chrono>
#include <ctime>
#include <cstdlib>
#include <filesystem>
#include <fstream>
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
  // 2026-01-15T00:00:00Z (Asia/Shanghai 2026-01-15 08:00:00 UTC+8)
  o.verify_time = std::chrono::system_clock::from_time_t(static_cast<std::time_t>(1768435200));
  o.test_mode = test_mode;
  return o;
}

TrustStore test_trust() {
  return TrustStore::for_tests(sig_root() / "keys" / "test_sa.crt");
}

void expect_sse(const fs::path& root, int want_sse, TrustStore trust = test_trust()) {
  const auto r = verify_exchange_set(root, trust, fixed_opts());
  std::cout << "  " << root.filename().string() << " -> status=" << static_cast<int>(r.status)
            << " sse=" << r.sse << " msg=" << r.message << "\n";
  SCHECK(r.status == AuthStatus::RejectedExchange);
  SCHECK(r.sse == want_sse);
}

void oracle_positive() {
  std::cout << "oracle 1 positive:\n";
  const auto root = sig_root() / "positive";
  const auto r = verify_exchange_set(root, test_trust(), fixed_opts());
  std::cout << "  status=" << static_cast<int>(r.status) << " sse=" << r.sse
            << " msg=" << r.message << " installed=" << r.installed.size()
            << " skipped=" << r.skipped.size() << "\n";
  SCHECK(r.status == AuthStatus::Ok);
  SCHECK(r.code == SigRejectCode::None);
  SCHECK(r.skipped.empty());
  SCHECK(r.installed.size() >= 2);

  // Session Enforce + test trust installs members
  s100::ChartSession session;
  session.set_verify_trust(test_trust());
  session.set_verify_options(fixed_opts());
  s100::LoadError err;
  SCHECK(session.open_exchange_set(root / "CATALOG.XML", &err, VerifyPolicy::Enforce));
  SCHECK(session.last_verification().status == AuthStatus::Ok);
  const auto n = session.load_all_members(&err);
  SCHECK(n >= 2);
  SCHECK(session.datasets().size() >= 2);
}

void oracle_negatives() {
  std::cout << "oracle 2a–2k negatives:\n";
  expect_sse(sig_root() / "missing_sign", 134);
  expect_sse(sig_root() / "bad_catalog_sign", 134);
  expect_sse(sig_root() / "unparseable_sign", 128);
  expect_sse(sig_root() / "issuer_not_sa", 126);
  expect_sse(sig_root() / "expired_cert", 122);

  // 2d tampered dataset → 109, only that file skipped
  {
    const auto r = verify_exchange_set(sig_root() / "tampered_dataset", test_trust(), fixed_opts());
    std::cout << "  tampered_dataset -> status=" << static_cast<int>(r.status)
              << " skipped=" << r.skipped.size() << " installed=" << r.installed.size() << "\n";
    SCHECK(r.status == AuthStatus::Ok);  // whole set ok; one resource skipped
    SCHECK(r.skipped.size() == 1);
    SCHECK(r.installed.size() >= 1);
    bool saw109 = false;
    for (const auto& f : r.files) {
      if (f.sse == 109) saw109 = true;
    }
    SCHECK(saw109);

    // Session: pre-state unchanged on whole-set reject already covered; here other members load
    s100::ChartSession session;
    session.set_verify_trust(test_trust());
    session.set_verify_options(fixed_opts());
    s100::LoadError err;
    SCHECK(session.open_exchange_set(sig_root() / "tampered_dataset" / "CATALOG.XML", &err,
                                     VerifyPolicy::Enforce));
    const auto before = session.datasets().size();
    SCHECK(before == 0);
    session.load_all_members(&err);
    SCHECK(session.datasets().size() == r.installed.size());
  }

  // 2f empty trust → 105
  {
    const auto r = verify_exchange_set(sig_root() / "positive", TrustStore::make_empty(), fixed_opts());
    std::cout << "  empty trust -> sse=" << r.sse << "\n";
    SCHECK(r.status == AuthStatus::RejectedExchange);
    SCHECK(r.sse == 105);
  }

  // 2h FC not SA-direct → 129 for that resource
  {
    const auto r = verify_exchange_set(sig_root() / "fc_not_sa_direct", test_trust(), fixed_opts());
    std::cout << "  fc_not_sa_direct -> skipped=" << r.skipped.size() << "\n";
    SCHECK(r.status == AuthStatus::Ok);
    bool saw129 = false;
    for (const auto& f : r.files)
      if (f.sse == 129) saw129 = true;
    SCHECK(saw129);
  }

  // 2i bad algorithm → 109
  {
    const auto r = verify_exchange_set(sig_root() / "bad_algorithm", test_trust(), fixed_opts());
    bool saw109 = false;
    for (const auto& f : r.files)
      if (f.sse == 109) saw109 = true;
    SCHECK(saw109);
  }

  // 2j bad base64 → 109
  {
    const auto r = verify_exchange_set(sig_root() / "bad_base64", test_trust(), fixed_opts());
    bool saw109 = false;
    for (const auto& f : r.files)
      if (f.sse == 109) saw109 = true;
    SCHECK(saw109);
  }

  // 2k wrong file sig → 109 for both (swapped)
  {
    const auto r = verify_exchange_set(sig_root() / "wrong_file_sig", test_trust(), fixed_opts());
    std::cout << "  wrong_file_sig -> skipped=" << r.skipped.size() << "\n";
    SCHECK(r.skipped.size() >= 2);
    for (const auto& f : r.files) {
      if (f.path.extension() == ".000") SCHECK(f.sse == 109);
    }
  }

  // missing dataset sig → 132
  {
    const auto r = verify_exchange_set(sig_root() / "missing_dataset_sig", test_trust(), fixed_opts());
    bool saw132 = false;
    for (const auto& f : r.files)
      if (f.sse == 132) saw132 = true;
    SCHECK(saw132);
  }

  // 2l whole-set reject leaves session unchanged
  {
    s100::ChartSession session;
    s100::Dataset marker;
    s100::LoadError err;
    // Insert a marker member then attempt reject
    SCHECK(session.open_and_insert(fs::path(S100_SOURCE_DIR) / "testdata/s101/101AA00DS0008.000",
                                   &err));
    SCHECK(session.datasets().size() == 1);
    const auto dig_before = session.datasets().begin()->second.features.size();
    session.set_verify_trust(test_trust());
    session.set_verify_options(fixed_opts());
    SCHECK(!session.open_exchange_set(sig_root() / "missing_sign" / "CATALOG.XML", &err,
                                      VerifyPolicy::Enforce));
    SCHECK(session.last_verification().sse == 134);
    SCHECK(session.datasets().size() == 1);
    SCHECK(session.datasets().begin()->second.features.size() == dig_before);
  }
}

void oracle_3() {
  std::cout << "oracle 3a/3b:\n";
  // 3a: builtin IHO rejects test-SA-signed set → exact SSE 126 (not 105)
  {
    const auto r =
        verify_exchange_set(sig_root() / "positive", TrustStore::builtin_iho_52(), fixed_opts(false));
    std::cout << "  builtin vs test-SA -> sse=" << r.sse << " (trust size="
              << TrustStore::builtin_iho_52().size() << ")\n";
    SCHECK(!TrustStore::builtin_iho_52().empty());
    SCHECK(r.status == AuthStatus::RejectedExchange);
    SCHECK(r.sse == 126);
  }
  // 3b self-signed in set
  expect_sse(sig_root() / "self_signed_in_set", 126);
}

void oracle_optional() {
  std::cout << "oracle 13 local optional:\n";
  // IHO_SIGNED PC — look for optional local path
  const fs::path iho_pc = fs::path(S100_SOURCE_DIR) / "testdata/s164_iho_signed_pc";
  if (fs::exists(iho_pc / "CATALOG.SIGN")) {
    const auto r = verify_exchange_set(iho_pc, TrustStore::builtin_iho_52(), fixed_opts(false));
    std::cout << "signatures: iho_pc exercised sse=" << r.sse
              << " status=" << static_cast<int>(r.status) << "\n";
  } else {
    std::cout << "signatures: iho_pc not exercised\n";
  }

  if (s100test::chs_exercised("signatures.chs122")) {
    const auto chs = s100test::chs_s100_root();
    // verify_time after DS notAfter 2026-04-02 → 122
    VerifyOptions o = fixed_opts(false);
    // 2026-06-01T00:00:00Z — after CHS DS notAfter 2026-04-02
    o.verify_time = std::chrono::system_clock::from_time_t(static_cast<std::time_t>(1780272000));
    const auto r = verify_exchange_set(chs, TrustStore::builtin_iho_52(), o);
    std::cout << "signatures: chs122 exercised sse=" << r.sse << "\n";
    SCHECK(r.status == AuthStatus::RejectedExchange);
    SCHECK(r.sse == 122);
  } else {
    std::cout << "signatures: chs122 not exercised\n";
  }
}

void oracle_sse_map_only() {
  // Sanity: mapping table returns expected codes
  SCHECK(sse_code(SigRejectCode::CatalogSignMissing) == 134);
  SCHECK(sse_code(SigRejectCode::CatalogSignParse) == 128);
  SCHECK(sse_code(SigRejectCode::NoSchemeAdministrator) == 105);
  SCHECK(sse_code(SigRejectCode::IssuerNotSa) == 126);
  SCHECK(sse_code(SigRejectCode::CertificateExpired) == 122);
  SCHECK(sse_code(SigRejectCode::DatasetSignatureInvalid) == 109);
  SCHECK(sse_code(SigRejectCode::DatasetSignatureMissing) == 132);
  SCHECK(sse_code(SigRejectCode::CatalogueNotSaDirect) == 129);
}

}  // namespace

void test_exchange_signature() {
  SCHECK(fs::exists(sig_root() / "positive" / "CATALOG.SIGN"));
  SCHECK(fs::exists(sig_root() / "keys" / "test_sa_TEST-ONLY.pem"));
  oracle_sse_map_only();
  oracle_positive();
  oracle_negatives();
  oracle_3();
  oracle_optional();
  std::cout << "exchange_signature ok\n";
}
