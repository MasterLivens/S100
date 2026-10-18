#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace s100::security::detail {
std::string_view builtin_iho_52_pem();
/** Load PEM or DER bytes into a DER-encoded X.509 blob; empty on failure. */
std::string load_cert_der_from_file(const std::string& path);
std::string load_cert_der_from_pem_or_der_bytes(const std::string& bytes);
}  // namespace s100::security::detail
