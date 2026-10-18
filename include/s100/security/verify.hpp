#pragma once

#include "s100/security/sse_map.hpp"
#include "s100/security/trust_store.hpp"
#include "s100/security/types.hpp"

namespace s100::security {

/** Verify S-100 Part 15 signatures on an exchange set.
 *  `exchange_root` may be the directory containing CATALOG.XML, or the CATALOG.XML path itself.
 *  Decision order: whole-set failures (missing/bad CATALOG.SIGN → SSE 134, …) before per-file.
 *  OpenSSL types do not appear in this header. */
VerificationReport verify_exchange_set(const std::filesystem::path& exchange_root,
                                       const TrustStore& trust, const VerifyOptions& options);

}  // namespace s100::security
