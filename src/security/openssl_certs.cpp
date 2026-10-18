#include "detail.hpp"

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace s100::security::detail {
namespace {

struct BioDel {
  void operator()(BIO* b) const { BIO_free(b); }
};
struct X509Del {
  void operator()(X509* x) const { X509_free(x); }
};

using BioPtr = std::unique_ptr<BIO, BioDel>;
using X509Ptr = std::unique_ptr<X509, X509Del>;

std::string read_file(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return {};
  return std::string(std::istreambuf_iterator<char>(in), {});
}

X509Ptr parse_x509(const std::string& bytes) {
  if (bytes.empty()) return {};
  {
    BioPtr bio(BIO_new_mem_buf(bytes.data(), static_cast<int>(bytes.size())));
    if (bio) {
      X509* raw = nullptr;
      if (PEM_read_bio_X509(bio.get(), &raw, nullptr, nullptr) && raw)
        return X509Ptr(raw);
    }
  }
  const unsigned char* p = reinterpret_cast<const unsigned char*>(bytes.data());
  X509* raw = d2i_X509(nullptr, &p, static_cast<long>(bytes.size()));
  if (raw) return X509Ptr(raw);
  return {};
}

std::string x509_to_der(X509* x) {
  if (!x) return {};
  unsigned char* out = nullptr;
  const int n = i2d_X509(x, &out);
  if (n <= 0 || !out) return {};
  std::string der(reinterpret_cast<char*>(out), static_cast<std::size_t>(n));
  OPENSSL_free(out);
  return der;
}

}  // namespace

std::string load_cert_der_from_pem_or_der_bytes(const std::string& bytes) {
  auto x = parse_x509(bytes);
  if (!x) return {};
  return x509_to_der(x.get());
}

std::string load_cert_der_from_file(const std::string& path) {
  return load_cert_der_from_pem_or_der_bytes(read_file(path));
}

}  // namespace s100::security::detail
