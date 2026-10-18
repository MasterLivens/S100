#include "s100/security/verify.hpp"

#include "detail.hpp"

#include <pugixml.hpp>

#include <openssl/asn1.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace s100::security {
namespace {

struct BioDel {
  void operator()(BIO* b) const { BIO_free(b); }
};
struct X509Del {
  void operator()(X509* x) const { X509_free(x); }
};
struct EvpDel {
  void operator()(EVP_PKEY* p) const { EVP_PKEY_free(p); }
};
struct MdCtxDel {
  void operator()(EVP_MD_CTX* c) const { EVP_MD_CTX_free(c); }
};

using BioPtr = std::unique_ptr<BIO, BioDel>;
using X509Ptr = std::unique_ptr<X509, X509Del>;
using EvpPtr = std::unique_ptr<EVP_PKEY, EvpDel>;
using MdCtxPtr = std::unique_ptr<EVP_MD_CTX, MdCtxDel>;

const char* local_name(const char* name) {
  if (!name) return "";
  const char* c = std::strchr(name, ':');
  return c ? c + 1 : name;
}

bool name_is(const pugi::xml_node& n, const char* local) {
  return std::strcmp(local_name(n.name()), local) == 0;
}

pugi::xml_node child_local(const pugi::xml_node& n, const char* local) {
  for (auto c : n.children())
    if (name_is(c, local)) return c;
  return {};
}

std::string text_of(const pugi::xml_node& n) {
  if (!n) return {};
  for (auto c : n.children()) {
    const char* ln = local_name(c.name());
    if (std::strcmp(ln, "CharacterString") == 0 || std::strcmp(ln, "Decimal") == 0)
      return c.text().as_string();
  }
  return n.text().as_string();
}

std::string collapse_ws(std::string s) {
  s.erase(std::remove_if(s.begin(), s.end(),
                         [](unsigned char c) { return std::isspace(c) != 0; }),
          s.end());
  return s;
}

std::string read_file_bytes(const std::filesystem::path& p) {
  std::ifstream in(p, std::ios::binary);
  if (!in) return {};
  return std::string(std::istreambuf_iterator<char>(in), {});
}

std::string b64_decode(const std::string& in) {
  if (in.empty()) return {};
  BioPtr b64(BIO_new(BIO_f_base64()));
  if (!b64) return {};
  BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);
  BioPtr mem(BIO_new_mem_buf(in.data(), static_cast<int>(in.size())));
  if (!mem) return {};
  BIO_push(b64.get(), mem.release());
  std::string out(in.size(), '\0');
  const int n = BIO_read(b64.get(), out.data(), static_cast<int>(out.size()));
  BIO_pop(b64.get());
  if (n <= 0) return {};
  out.resize(static_cast<std::size_t>(n));
  return out;
}

X509Ptr parse_cert_body(const std::string& body_b64_or_pem) {
  std::string body = collapse_ws(body_b64_or_pem);
  if (body.empty()) return {};
  // Prefer PEM reconstruction from base64 body (Part 15 §15-8.6 omits headers in XML).
  if (body.find("BEGIN") == std::string::npos) {
    std::ostringstream pem;
    pem << "-----BEGIN CERTIFICATE-----\n";
    for (std::size_t i = 0; i < body.size(); i += 64)
      pem << body.substr(i, 64) << "\n";
    pem << "-----END CERTIFICATE-----\n";
    body = pem.str();
  }
  BioPtr bio(BIO_new_mem_buf(body.data(), static_cast<int>(body.size())));
  if (!bio) return {};
  X509* raw = nullptr;
  if (PEM_read_bio_X509(bio.get(), &raw, nullptr, nullptr) && raw) return X509Ptr(raw);
  // DER fallback
  const auto der = b64_decode(collapse_ws(body_b64_or_pem));
  if (der.empty()) return {};
  const unsigned char* p = reinterpret_cast<const unsigned char*>(der.data());
  raw = d2i_X509(nullptr, &p, static_cast<long>(der.size()));
  return raw ? X509Ptr(raw) : X509Ptr{};
}

X509Ptr parse_der(const std::string& der) {
  if (der.empty()) return {};
  const unsigned char* p = reinterpret_cast<const unsigned char*>(der.data());
  X509* raw = d2i_X509(nullptr, &p, static_cast<long>(der.size()));
  return raw ? X509Ptr(raw) : X509Ptr{};
}

bool cert_ders_equal(X509* a, X509* b) {
  if (!a || !b) return false;
  unsigned char *da = nullptr, *db = nullptr;
  const int na = i2d_X509(a, &da);
  const int nb = i2d_X509(b, &db);
  bool eq = na > 0 && na == nb && da && db && std::memcmp(da, db, static_cast<std::size_t>(na)) == 0;
  OPENSSL_free(da);
  OPENSSL_free(db);
  return eq;
}

bool is_self_signed(X509* x) {
  if (!x) return false;
  EvpPtr pub(X509_get_pubkey(x));
  if (!pub) return false;
  return X509_verify(x, pub.get()) == 1;
}

std::string cert_st(X509* x) {
  if (!x) return {};
  char buf[256];
  const int n = X509_NAME_get_text_by_NID(X509_get_subject_name(x), NID_stateOrProvinceName, buf,
                                         static_cast<int>(sizeof(buf)));
  if (n <= 0) return {};
  return std::string(buf, static_cast<std::size_t>(n));
}

std::string cert_cn(X509* x) {
  if (!x) return {};
  char buf[512];
  const int n = X509_NAME_get_text_by_NID(X509_get_subject_name(x), NID_commonName, buf,
                                         static_cast<int>(sizeof(buf)));
  if (n <= 0) return {};
  return std::string(buf, static_cast<std::size_t>(n));
}

std::string to_upper_ascii(std::string s) {
  for (char& c : s) {
    if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
  }
  return s;
}

/** IHO MRN producer token = namespace segment 3 only (after urn:mrn:iho: or urn:mrn:iho:org:).
 *  Segment 4 (cert instance id) is never returned. Empty if CN is not a conforming MRN. */
std::string cn_producer_segment(const std::string& cn) {
  if (cn.empty()) return {};
  std::string lower = cn;
  for (char& c : lower) {
    if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
  }
  const std::string kOrg = "urn:mrn:iho:org:";
  const std::string kIho = "urn:mrn:iho:";
  std::string rest;
  if (lower.rfind(kOrg, 0) == 0) {
    rest = cn.substr(kOrg.size());
  } else if (lower.rfind(kIho, 0) == 0) {
    rest = cn.substr(kIho.size());
  } else {
    return {};
  }
  // rest is "<producer>:<id>" or just "<producer>"; take first colon-separated token only
  const auto colon = rest.find(':');
  std::string token = (colon == std::string::npos) ? rest : rest.substr(0, colon);
  // reject empty or nested leftovers that look like bare "org" without org: prefix handling
  if (token.empty()) return {};
  return token;
}

OfficialClass classify_st(const std::string& st) {
  if (st == "DATA_PRODUCER" || st == "DATA_AGGREGATOR" || st == "SCHEME_ADMINISTRATOR")
    return OfficialClass::Official;
  if (st.empty()) return OfficialClass::Unknown;
  return OfficialClass::NonOfficial;
}

bool cert_time_valid(X509* x, std::time_t when) {
  if (!x) return false;
  const ASN1_TIME* nb = X509_get0_notBefore(x);
  const ASN1_TIME* na = X509_get0_notAfter(x);
  if (!nb || !na) return false;
  // ASN1_TIME_cmp_time_t: returns -1 if ASN1_TIME <= time, 0 equal-ish, 1 if ASN1_TIME > time
  // Valid when notBefore <= when <= notAfter.
  if (ASN1_TIME_cmp_time_t(nb, when) > 0) return false;  // notBefore > when
  if (ASN1_TIME_cmp_time_t(na, when) < 0) return false;  // notAfter < when
  return true;
}

bool cert_sig_alg_ok(X509* x) {
  if (!x) return false;
  const X509_ALGOR* alg = nullptr;
  X509_get0_signature(nullptr, &alg, x);
  if (!alg) return false;
  int nid = NID_undef;
  X509_ALGOR_get0(nullptr, &nid, nullptr, const_cast<X509_ALGOR*>(alg));
  // Prefer OBJ_obj2nid on algorithm
  const ASN1_OBJECT* obj = nullptr;
  X509_ALGOR_get0(&obj, nullptr, nullptr, const_cast<X509_ALGOR*>(alg));
  if (!obj) return false;
  nid = OBJ_obj2nid(obj);
  return nid == NID_ecdsa_with_SHA256 || nid == NID_ecdsa_with_SHA384 ||
         nid == NID_ecdsa_with_SHA512;
}

bool pubkey_is_p384(EVP_PKEY* key) {
  if (!key) return false;
  char gname[64] = {};
  const size_t n = sizeof(gname);
  if (EVP_PKEY_get_group_name(key, gname, n, nullptr) != 1) return false;
  return std::strcmp(gname, "secp384r1") == 0 || std::strcmp(gname, "P-384") == 0;
}

bool verify_ecdsa_sha384(EVP_PKEY* key, const std::string& data, const std::string& sig_der) {
  if (!key || data.empty() || sig_der.empty()) return false;
  if (!pubkey_is_p384(key)) return false;
  MdCtxPtr ctx(EVP_MD_CTX_new());
  if (!ctx) return false;
  if (EVP_DigestVerifyInit(ctx.get(), nullptr, EVP_sha384(), nullptr, key) != 1) return false;
  return EVP_DigestVerify(ctx.get(), reinterpret_cast<const unsigned char*>(sig_der.data()),
                          sig_der.size(), reinterpret_cast<const unsigned char*>(data.data()),
                          data.size()) == 1;
}

struct CatalogCert {
  std::string id;
  std::string issuer;  // id of issuer (or SA id / "IHO")
  X509Ptr cert;
  bool from_exchange{true};
};

struct WholeFail {
  SigRejectCode code;
  std::string message;
};

FileVerification make_file(const std::filesystem::path& path, SigRejectCode code,
                           OfficialClass oc = OfficialClass::Unknown) {
  FileVerification fv;
  fv.path = path;
  fv.code = code;
  fv.sse = sse_code(code);
  fv.message = sse_message(code);
  fv.official = oc;
  if (code == SigRejectCode::None) {
    fv.status = AuthStatus::Ok;
  } else if (code == SigRejectCode::UnsupportedDataStatus) {
    fv.status = AuthStatus::SkippedResource;
  } else {
    fv.status = AuthStatus::SkippedResource;
  }
  return fv;
}

VerificationReport reject_whole(SigRejectCode code, const std::string& extra = {}) {
  VerificationReport r;
  r.status = AuthStatus::RejectedExchange;
  r.code = code;
  r.sse = sse_code(code);
  r.message = sse_message(code);
  if (!extra.empty()) {
    if (!r.message.empty()) r.message += ": ";
    r.message += extra;
  }
  return r;
}

std::filesystem::path resolve_exchange_paths(const std::filesystem::path& exchange_root,
                                             std::filesystem::path& catalog_xml,
                                             std::filesystem::path& catalog_sign,
                                             std::filesystem::path& root) {
  namespace fs = std::filesystem;
  fs::path p = exchange_root;
  if (fs::is_regular_file(p) && p.filename() == "CATALOG.XML") {
    catalog_xml = p;
    root = p.parent_path();
  } else if (fs::is_directory(p)) {
    root = p;
    catalog_xml = p / "CATALOG.XML";
  } else {
    // maybe a dir path that doesn't exist yet, or CATALOG.XML path
    if (p.filename() == "CATALOG.XML") {
      catalog_xml = p;
      root = p.parent_path();
    } else {
      root = p;
      catalog_xml = p / "CATALOG.XML";
    }
  }
  catalog_sign = root / "CATALOG.SIGN";
  return root;
}

std::filesystem::path resolve_file_name(const std::filesystem::path& root, std::string name) {
  if (name.rfind("file:", 0) == 0) {
    name.erase(0, 5);
    while (!name.empty() && name[0] == '/') name.erase(name.begin());
  }
  std::filesystem::path rel(name);
  std::filesystem::path abs = root / rel;
  std::error_code ec;
  auto canon = std::filesystem::weakly_canonical(abs, ec);
  return ec ? abs.lexically_normal() : canon;
}

void collect_certificates(const pugi::xml_node& parent, std::map<std::string, CatalogCert>& out) {
  for (auto c : parent.children()) {
    if (!name_is(c, "certificate")) continue;
    CatalogCert cc;
    cc.id = c.attribute("id").as_string();
    cc.issuer = c.attribute("issuer").as_string();
    cc.cert = parse_cert_body(c.text().as_string());
    if (cc.id.empty() || !cc.cert) continue;
    out[cc.id] = std::move(cc);
  }
}

/** Build chain by id/issuer (D1). Returns reject code or None. Sets *leaf_official. */
SigRejectCode authenticate_cert(const std::string& cert_id,
                                const std::map<std::string, CatalogCert>& catalog_certs,
                                const std::vector<X509Ptr>& anchors, std::time_t when,
                                bool require_sa_direct, OfficialClass* leaf_official,
                                std::string* detail) {
  if (cert_id.empty()) {
    if (detail) *detail = "empty certificateRef";
    return SigRejectCode::CertificateMissing;
  }
  auto it = catalog_certs.find(cert_id);
  if (it == catalog_certs.end() || !it->second.cert) {
    if (detail) *detail = "certificateRef not in catalogue: " + cert_id;
    return SigRejectCode::CertificateMissing;
  }

  // Walk id/issuer; verify each cert with issuer pubkey; terminate at trust-store SA.
  std::string cur_id = cert_id;
  int depth = 0;
  const int kMax = 8;
  bool reached_sa = false;
  X509* leaf = it->second.cert.get();
  if (leaf_official) *leaf_official = classify_st(cert_st(leaf));

  while (depth < kMax) {
    auto cit = catalog_certs.find(cur_id);
    if (cit == catalog_certs.end() || !cit->second.cert) {
      if (detail) *detail = "missing cert id " + cur_id;
      return SigRejectCode::CertificateMissing;
    }
    X509* cur = cit->second.cert.get();
    if (!cert_sig_alg_ok(cur)) {
      if (detail) *detail = "unsupported certificate signature algorithm";
      return SigRejectCode::DatasetSignatureInvalid;
    }
    if (!cert_time_valid(cur, when)) {
      if (detail) *detail = "certificate expired or not yet valid: " + cur_id;
      return SigRejectCode::CertificateExpired;
    }

    // Is this cert itself a trust-store SA (exact DER match)?
    for (const auto& a : anchors) {
      if (cert_ders_equal(cur, a.get())) {
        reached_sa = true;
        break;
      }
    }
    if (reached_sa) break;

    // Self-signed inside the exchange set is never a trust anchor (D4).
    const std::string issuer_id = cit->second.issuer;
    if (issuer_id.empty() || issuer_id == cur_id || is_self_signed(cur)) {
      // Try matching subject against anchors via signature verify with anchor pubkey
      bool verified_by_anchor = false;
      for (const auto& a : anchors) {
        EvpPtr apub(X509_get_pubkey(a.get()));
        if (apub && X509_verify(cur, apub.get()) == 1) {
          // Also check anchor validity
          if (!cert_time_valid(a.get(), when)) {
            if (detail) *detail = "SA trust anchor expired";
            return SigRejectCode::CertificateExpired;
          }
          reached_sa = true;
          verified_by_anchor = true;
          break;
        }
      }
      if (verified_by_anchor) break;
      if (detail) *detail = "chain does not reach SA (self-signed or missing issuer)";
      return SigRejectCode::IssuerNotSa;
    }

    // Look up issuer in catalogue certs first
    auto iit = catalog_certs.find(issuer_id);
    X509* issuer_cert = nullptr;
    if (iit != catalog_certs.end() && iit->second.cert) {
      issuer_cert = iit->second.cert.get();
    } else {
      // Issuer id may name the SA (e.g. "IHO") — match against schemeAdministrator / anchors
      for (const auto& a : anchors) {
        EvpPtr apub(X509_get_pubkey(a.get()));
        if (apub && X509_verify(cur, apub.get()) == 1) {
          if (!cert_time_valid(a.get(), when)) return SigRejectCode::CertificateExpired;
          reached_sa = true;
          issuer_cert = a.get();
          break;
        }
      }
      if (reached_sa) break;
      if (detail) *detail = "issuer not found: " + issuer_id;
      return SigRejectCode::IssuerNotSa;
    }

    EvpPtr ipub(X509_get_pubkey(issuer_cert));
    if (!ipub || X509_verify(cur, ipub.get()) != 1) {
      if (detail) *detail = "certificate signature verify failed for " + cur_id;
      return SigRejectCode::IssuerNotSa;
    }

    // If issuer is an anchor, done
    for (const auto& a : anchors) {
      if (cert_ders_equal(issuer_cert, a.get())) {
        reached_sa = true;
        break;
      }
    }
    if (reached_sa) break;

    cur_id = issuer_id;
    ++depth;
  }

  if (!reached_sa) {
    if (detail) *detail = "chain depth exceeded or did not reach SA";
    return SigRejectCode::IssuerNotSa;
  }

  if (require_sa_direct) {
    // S-98 / SSE 129: FC/PC signing certificate must BE an SA trust-anchor, not merely
    // chain to one (a Data Server cert signed by the SA is not "directly" SA).
    bool leaf_is_sa = false;
    for (const auto& a : anchors) {
      if (cert_ders_equal(leaf, a.get())) {
        leaf_is_sa = true;
        break;
      }
    }
    if (!leaf_is_sa) {
      if (detail) *detail = "catalogue not signed directly by SA";
      return SigRejectCode::CatalogueNotSaDirect;
    }
  }
  return SigRejectCode::None;
}

}  // namespace

VerificationReport verify_exchange_set(const std::filesystem::path& exchange_root,
                                       const TrustStore& trust, const VerifyOptions& options) {
  namespace fs = std::filesystem;
  fs::path catalog_xml, catalog_sign, root;
  resolve_exchange_paths(exchange_root, catalog_xml, catalog_sign, root);

  const auto when = std::chrono::system_clock::to_time_t(options.verify_time);

  // A7.1 CATALOG.SIGN exists
  if (!fs::exists(catalog_sign) || !fs::is_regular_file(catalog_sign))
    return reject_whole(SigRejectCode::CatalogSignMissing, catalog_sign.string());

  // A7.2 parse StandaloneDigitalSignature
  pugi::xml_document sign_doc;
  {
    const auto pref = sign_doc.load_file(catalog_sign.c_str());
    if (!pref) return reject_whole(SigRejectCode::CatalogSignParse, pref.description());
  }
  pugi::xml_node sign_root = sign_doc.document_element();
  if (!sign_root || !name_is(sign_root, "StandaloneDigitalSignature"))
    return reject_whole(SigRejectCode::CatalogSignParse, "root is not StandaloneDigitalSignature");

  // A7.3 trust store has SA
  if (trust.empty()) return reject_whole(SigRejectCode::NoSchemeAdministrator);

  std::vector<X509Ptr> anchors;
  for (const auto& der : trust.anchor_ders()) {
    auto x = parse_der(der);
    if (!x) return reject_whole(SigRejectCode::SaCertificateInvalid, "failed to parse trust anchor");
    anchors.push_back(std::move(x));
  }
  if (anchors.empty()) return reject_whole(SigRejectCode::NoSchemeAdministrator);

  // A7.4 collect certificates from CATALOG.SIGN (+ catalogue)
  std::map<std::string, CatalogCert> certs;
  std::string scheme_admin_id;
  if (auto certs_node = child_local(sign_root, "certificates")) {
    if (auto sa = child_local(certs_node, "schemeAdministrator"))
      scheme_admin_id = sa.attribute("id").as_string();
    collect_certificates(certs_node, certs);
  }

  if (!fs::exists(catalog_xml))
    return reject_whole(SigRejectCode::CatalogSignInvalid, "CATALOG.XML missing");

  pugi::xml_document cat_doc;
  {
    const auto pref = cat_doc.load_file(catalog_xml.c_str());
    if (!pref) return reject_whole(SigRejectCode::CatalogSignInvalid, "CATALOG.XML unreadable");
  }
  auto cat_el = cat_doc.document_element();
  if (auto ccerts = child_local(cat_el, "certificates")) {
    if (scheme_admin_id.empty())
      if (auto sa = child_local(ccerts, "schemeAdministrator"))
        scheme_admin_id = sa.attribute("id").as_string();
    collect_certificates(ccerts, certs);
  }

  // Ignore self-signed exchange-set certs as trust anchors: do not add them to anchors.
  // (They remain in `certs` for id lookup, but authenticate_cert refuses them as termination.)

  // Find catalogue digitalSignature
  pugi::xml_node dig_sig;
  for (auto c : sign_root.children()) {
    if (name_is(c, "digitalSignature")) {
      dig_sig = c;
      break;
    }
  }
  if (!dig_sig) return reject_whole(SigRejectCode::CatalogSignParse, "digitalSignature missing");

  const std::string catalog_cert_ref = dig_sig.attribute("certificateRef").as_string();
  OfficialClass exchange_official = OfficialClass::Unknown;
  std::string detail;
  const SigRejectCode chain_rc =
      authenticate_cert(catalog_cert_ref, certs, anchors, when, /*require_sa_direct=*/false,
                        &exchange_official, &detail);
  if (chain_rc != SigRejectCode::None) {
    // Map catalogue-level chain failures to whole-set codes
    if (chain_rc == SigRejectCode::CertificateExpired) return reject_whole(chain_rc, detail);
    if (chain_rc == SigRejectCode::IssuerNotSa) return reject_whole(chain_rc, detail);
    if (chain_rc == SigRejectCode::CertificateMissing) return reject_whole(SigRejectCode::CatalogSignInvalid, detail);
    if (chain_rc == SigRejectCode::SaCertificateInvalid) return reject_whole(chain_rc, detail);
    return reject_whole(SigRejectCode::CatalogSignInvalid, detail);
  }

  // A7.8 verify CATALOG.XML bytes
  {
    auto cit = certs.find(catalog_cert_ref);
    if (cit == certs.end() || !cit->second.cert)
      return reject_whole(SigRejectCode::CatalogSignInvalid, "signing cert missing");
    EvpPtr pub(X509_get_pubkey(cit->second.cert.get()));
    const std::string xml_bytes = read_file_bytes(catalog_xml);
    const std::string sig_der = b64_decode(collapse_ws(dig_sig.text().as_string()));
    if (sig_der.empty() || !verify_ecdsa_sha384(pub.get(), xml_bytes, sig_der))
      return reject_whole(SigRejectCode::CatalogSignInvalid, "CATALOG.XML signature verify failed");
  }

  VerificationReport report;
  report.status = AuthStatus::Ok;
  report.official = exchange_official;

  auto handle_resource = [&](pugi::xml_node meta, bool is_catalogue_resource,
                             bool check_producer_code) {
    const std::string file_name = text_of(child_local(meta, "fileName"));
    if (file_name.empty()) return;
    const fs::path path = resolve_file_name(root, file_name);

    // dataStatus / signature fields
    auto ref_node = child_local(meta, "digitalSignatureReference");
    auto val_node = child_local(meta, "digitalSignatureValue");

    if (!val_node) {
      auto fv = make_file(path, SigRejectCode::DatasetSignatureMissing);
      report.files.push_back(fv);
      report.skipped.push_back(path);
      return;
    }

    // Find SignatureOnData / S100_SE_DigitalSignature / any S100_SE_* child with text
    pugi::xml_node sig_el;
    std::string data_status = "unencrypted";
    for (auto c : val_node.children()) {
      const char* ln = local_name(c.name());
      if (std::strcmp(ln, "S100_SE_SignatureOnData") == 0 ||
          std::strcmp(ln, "S100_SE_DigitalSignature") == 0 ||
          std::strcmp(ln, "SignatureOnData") == 0 ||
          std::strncmp(ln, "S100_SE_", 8) == 0) {
        sig_el = c;
        break;
      }
    }
    if (!sig_el) {
      // Maybe text directly under digitalSignatureValue
      if (std::strlen(val_node.text().as_string()) > 0) sig_el = val_node;
    }
    if (!sig_el) {
      auto fv = make_file(path, SigRejectCode::DatasetSignatureMissing);
      report.files.push_back(fv);
      report.skipped.push_back(path);
      return;
    }

    if (auto ds = sig_el.attribute("dataStatus")) data_status = ds.as_string();
    if (data_status.empty()) data_status = "unencrypted";
    if (data_status != "unencrypted") {
      auto fv = make_file(path, SigRejectCode::UnsupportedDataStatus);
      report.files.push_back(fv);
      report.skipped.push_back(path);
      return;
    }

    const std::string alg = ref_node ? text_of(ref_node) : std::string{};
    if (alg != "ECDSA-384-SHA2") {
      auto fv = make_file(path, SigRejectCode::DatasetSignatureInvalid);
      fv.message = std::string(sse_message(SigRejectCode::DatasetSignatureInvalid)) +
                   " (unsupported algorithm)";
      report.files.push_back(fv);
      report.skipped.push_back(path);
      return;
    }

    const std::string sig_b64 = collapse_ws(sig_el.text().as_string());
    const std::string sig_der = b64_decode(sig_b64);
    if (sig_der.empty()) {
      auto fv = make_file(path, SigRejectCode::DatasetSignatureInvalid);
      fv.message += " (bad Base64/DER)";
      report.files.push_back(fv);
      report.skipped.push_back(path);
      return;
    }

    std::string cert_ref = sig_el.attribute("certificateRef").as_string();
    OfficialClass oc = OfficialClass::Unknown;
    std::string d2;
    const SigRejectCode crc =
        authenticate_cert(cert_ref, certs, anchors, when, is_catalogue_resource, &oc, &d2);
    if (crc != SigRejectCode::None) {
      auto fv = make_file(path, crc, oc);
      if (!d2.empty()) {
        fv.message += ": ";
        fv.message += d2;
      }
      report.files.push_back(fv);
      report.skipped.push_back(path);
      return;
    }

    if (!fs::exists(path)) {
      auto fv = make_file(path, SigRejectCode::DatasetSignatureInvalid);
      fv.message += " (file missing)";
      report.files.push_back(fv);
      report.skipped.push_back(path);
      return;
    }

    auto cit = certs.find(cert_ref);
    EvpPtr pub(X509_get_pubkey(cit->second.cert.get()));
    const std::string bytes = read_file_bytes(path);
    if (!verify_ecdsa_sha384(pub.get(), bytes, sig_der)) {
      auto fv = make_file(path, SigRejectCode::DatasetSignatureInvalid, oc);
      report.files.push_back(fv);
      report.skipped.push_back(path);
      return;
    }

    // IHO-S164-SIG-PRODUCER: after crypto Ok, match CN producer seg3 vs CATALOG producerCode.
    // Datasets only; FC/PC out of scope; support deferred. Missing producerCode → N/A-pass.
    // ST=DATA_AGGREGATOR → exempt this slice.
    if (check_producer_code) {
      X509* leaf = cit->second.cert.get();
      const std::string st = cert_st(leaf);
      if (st != "DATA_AGGREGATOR") {
        // producerCode is optional (0..1); CharacterString wrapper or bare text
        std::string declared;
        if (auto pc = child_local(meta, "producerCode")) {
          declared = text_of(pc);
          if (declared.empty()) declared = pc.text().as_string();
          // trim surrounding whitespace only (do NOT strip leading zeros inside the code)
          while (!declared.empty() && std::isspace(static_cast<unsigned char>(declared.front())))
            declared.erase(declared.begin());
          while (!declared.empty() && std::isspace(static_cast<unsigned char>(declared.back())))
            declared.pop_back();
        }
        if (!declared.empty()) {
          const std::string seg = cn_producer_segment(cert_cn(leaf));
          if (!seg.empty()) {
            if (to_upper_ascii(seg) != to_upper_ascii(declared)) {
              auto fv = make_file(path, SigRejectCode::ProducerCodeMismatch, oc);
              // Primary report reason (CTest asserts this first); SSE 109 is secondary.
              fv.message = "producer_code_mismatch";
              report.files.push_back(fv);
              report.skipped.push_back(path);
              return;
            }
          } else {
            // CN unparseable + producerCode present: cannot prove must-match → skip file.
            // MUST NOT emit producer_code_mismatch / SSE 109 (109 = parse-Ok mismatch only).
            // NonOfficial via A9 may coexist; no new SSE from this slice.
            FileVerification fv;
            fv.path = path;
            fv.status = AuthStatus::SkippedResource;
            fv.code = SigRejectCode::None;
            fv.sse = 0;
            fv.message = "producer_cn_unparseable";
            fv.official = oc;
            report.files.push_back(fv);
            report.skipped.push_back(path);
            return;
          }
        }
      }
    }

    auto fv = make_file(path, SigRejectCode::None, oc);
    fv.status = AuthStatus::Ok;
    fv.message = "ok";
    report.files.push_back(fv);
    report.installed.push_back(path);
  };

  // Dataset + support + catalogue discovery metadata
  // Producer-code matching applies to datasets only (FC/PC out of scope; support deferred).
  auto walk_discovery = [&](const char* container, const char* item, bool is_cat,
                            bool check_producer) {
    auto cont = child_local(cat_el, container);
    if (!cont) return;
    for (auto c : cont.children()) {
      if (name_is(c, item)) handle_resource(c, is_cat, check_producer);
    }
  };
  // Sometimes discovery entries are direct children of catalogue root
  for (auto c : cat_el.children()) {
    if (name_is(c, "S100_DatasetDiscoveryMetadata") || name_is(c, "datasetDiscoveryMetadata")) {
      if (name_is(c, "datasetDiscoveryMetadata")) {
        for (auto d : c.children())
          if (name_is(d, "S100_DatasetDiscoveryMetadata")) handle_resource(d, false, true);
      } else {
        handle_resource(c, false, true);
      }
    }
  }
  walk_discovery("datasetDiscoveryMetadata", "S100_DatasetDiscoveryMetadata", false, true);
  walk_discovery("supportFileDiscoveryMetadata", "S100_SupportFileDiscoveryMetadata", false, false);
  walk_discovery("catalogueDiscoveryMetadata", "S100_CatalogueDiscoveryMetadata", true, false);

  // Also handle support/catalogue if nested differently — already covered by walk.

  // Deduplicate installed/skipped if walk double-counted (datasetDiscoveryMetadata container
  // + direct). Use path string uniqueness.
  auto uniq = [](std::vector<fs::path>& v) {
    std::vector<fs::path> out;
    std::vector<std::string> seen;
    for (auto& p : v) {
      const std::string s = p.lexically_normal().string();
      if (std::find(seen.begin(), seen.end(), s) != seen.end()) continue;
      seen.push_back(s);
      out.push_back(p);
    }
    v.swap(out);
  };
  uniq(report.installed);
  uniq(report.skipped);
  // Dedup files similarly (keep first)
  {
    std::vector<FileVerification> out;
    std::vector<std::string> seen;
    for (auto& f : report.files) {
      const std::string s = f.path.lexically_normal().string();
      if (std::find(seen.begin(), seen.end(), s) != seen.end()) continue;
      seen.push_back(s);
      out.push_back(std::move(f));
    }
    report.files.swap(out);
  }

  return report;
}

}  // namespace s100::security
