#include "s100/security/trust_store.hpp"

#include "detail.hpp"

#include <fstream>
#include <utility>

namespace s100::security {

struct TrustStore::Impl {
  std::vector<std::string> ders;  // DER-encoded X.509 trust anchors
};

TrustStore::TrustStore() : impl_(std::make_unique<Impl>()) {}
TrustStore::TrustStore(const TrustStore& other) : impl_(std::make_unique<Impl>(*other.impl_)) {}
TrustStore& TrustStore::operator=(const TrustStore& other) {
  if (this != &other) impl_ = std::make_unique<Impl>(*other.impl_);
  return *this;
}
TrustStore::TrustStore(TrustStore&&) noexcept = default;
TrustStore& TrustStore::operator=(TrustStore&&) noexcept = default;
TrustStore::~TrustStore() = default;

TrustStore TrustStore::make_empty() { return TrustStore{}; }

TrustStore TrustStore::builtin_iho_52() {
  TrustStore t;
  const auto pem = detail::builtin_iho_52_pem();
  auto der = detail::load_cert_der_from_pem_or_der_bytes(std::string(pem));
  if (!der.empty()) t.impl_->ders.push_back(std::move(der));
  return t;
}

TrustStore TrustStore::for_tests(const std::filesystem::path& test_sa_pem_or_crt) {
  TrustStore t = builtin_iho_52();
  t.add_user_sa(test_sa_pem_or_crt);
  return t;
}

bool TrustStore::add_user_sa(const std::filesystem::path& pem_or_der_or_crt) {
  auto der = detail::load_cert_der_from_file(pem_or_der_or_crt.string());
  if (der.empty()) return false;
  impl_->ders.push_back(std::move(der));
  return true;
}

bool TrustStore::empty() const { return impl_->ders.empty(); }
std::size_t TrustStore::size() const { return impl_->ders.size(); }
std::vector<std::string> TrustStore::anchor_ders() const { return impl_->ders; }

}  // namespace s100::security
