#pragma once

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

namespace s100::security {

enum class VerifyPolicy { Off, Enforce };

struct VerifyOptions {
  /** REQUIRED. Never defaulted to wall-clock inside the security module. */
  std::chrono::system_clock::time_point verify_time{};
  bool test_mode{false};  // when true, TrustStore::for_tests SA is eligible as anchor
};

enum class AuthStatus {
  Ok,
  Unverified,        // policy Off, or standalone open_dataset
  RejectedExchange,  // whole set (134/105/128/…)
  SkippedResource,   // one file (109/129/132/…)
};

enum class OfficialClass { Official, NonOfficial, Unknown };

/** Internal codes — NEVER serialize as S-98 numbers from call sites; map via sse_code(). */
enum class SigRejectCode {
  None,
  CatalogSignMissing,       // → 134
  CatalogSignParse,         // → 128
  CatalogSignInvalid,       // → 134
  NoSchemeAdministrator,    // → 105
  SaCertificateInvalid,     // → 106 / 108
  IssuerNotSa,              // → 126
  CertificateExpired,       // → 122
  DatasetSignatureInvalid,  // → 109
  DatasetSignatureMissing,  // → 132
  CertificateMissing,       // → 107
  CatalogueNotSaDirect,     // → 129
  UnsupportedDataStatus,    // skip, no SSE number required
  ProducerCodeMismatch,     // → 109 (project-map; not S-98 App.B literal; see part15-producer-code-contract)
};

struct FileVerification {
  std::filesystem::path path;
  AuthStatus status{AuthStatus::Ok};
  SigRejectCode code{SigRejectCode::None};
  int sse{0};
  std::string message;
  OfficialClass official{OfficialClass::Unknown};
};

struct VerificationReport {
  AuthStatus status{AuthStatus::Ok};
  SigRejectCode code{SigRejectCode::None};
  int sse{0};
  std::string message;
  OfficialClass official{OfficialClass::Unknown};
  std::vector<FileVerification> files;
  std::vector<std::filesystem::path> installed;
  std::vector<std::filesystem::path> skipped;
};

}  // namespace s100::security
