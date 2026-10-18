#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace s100::security {

/** Value type; injectable. Default = built-in IHO 5.2 root only.
 *  OpenSSL types never appear in this header. */
class TrustStore {
 public:
  TrustStore();
  TrustStore(const TrustStore& other);
  TrustStore& operator=(const TrustStore& other);
  TrustStore(TrustStore&&) noexcept;
  TrustStore& operator=(TrustStore&&) noexcept;
  ~TrustStore();

  static TrustStore builtin_iho_52();
  static TrustStore for_tests(const std::filesystem::path& test_sa_pem_or_crt);
  static TrustStore make_empty();

  bool add_user_sa(const std::filesystem::path& pem_or_der_or_crt);

  bool empty() const;
  std::size_t size() const;

  /** DER-encoded trust-anchor certificates (for verify TU). */
  std::vector<std::string> anchor_ders() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace s100::security
