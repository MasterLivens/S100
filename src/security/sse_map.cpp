#include "s100/security/sse_map.hpp"

namespace s100::security {

int sse_code(SigRejectCode c) {
  switch (c) {
    case SigRejectCode::None:
      return 0;
    case SigRejectCode::CatalogSignMissing:
    case SigRejectCode::CatalogSignInvalid:
      return 134;
    case SigRejectCode::CatalogSignParse:
      return 128;
    case SigRejectCode::NoSchemeAdministrator:
      return 105;
    case SigRejectCode::SaCertificateInvalid:
      return 108;  // unreadable / wrong format (106 also acceptable for bad SA)
    case SigRejectCode::IssuerNotSa:
      return 126;
    case SigRejectCode::CertificateExpired:
      return 122;
    case SigRejectCode::DatasetSignatureInvalid:
      return 109;
    case SigRejectCode::DatasetSignatureMissing:
      return 132;
    case SigRejectCode::CertificateMissing:
      return 107;
    case SigRejectCode::CatalogueNotSaDirect:
      return 129;
    case SigRejectCode::UnsupportedDataStatus:
      return 0;
    case SigRejectCode::ProducerCodeMismatch:
      return 109;  // project mapping — not S-98 App. B literal (IHO-S164-SIG-PRODUCER)
  }
  return 0;
}

const char* sse_message(SigRejectCode c) {
  switch (c) {
    case SigRejectCode::None:
      return "";
    case SigRejectCode::CatalogSignMissing:
      return "CATALOG.SIGN is missing; exchange set is not authenticated by the Scheme Administrator";
    case SigRejectCode::CatalogSignParse:
      return "Error parsing CATALOG.SIGN";
    case SigRejectCode::CatalogSignInvalid:
      return "This Exchange Set is not authenticated by the IHO acting as the Scheme Administrator";
    case SigRejectCode::NoSchemeAdministrator:
      return "no SA certificate installed";
    case SigRejectCode::SaCertificateInvalid:
      return "SA digital certificate is wrongly formatted or the certificate file is unreadable";
    case SigRejectCode::IssuerNotSa:
      return "ENC is not authenticated by the IHO acting as the SA";
    case SigRejectCode::CertificateExpired:
      return "SA Digital Certificate or an intermediate Domain Controller Certificate has expired";
    case SigRejectCode::DatasetSignatureInvalid:
      return "Dataset Resource Digital Signature is invalid";
    case SigRejectCode::DatasetSignatureMissing:
      return "digitalSignatureValue is missing or fails validation";
    case SigRejectCode::CertificateMissing:
      return "SA signed data server certificate is not present in the Exchange Set Catalogue";
    case SigRejectCode::CatalogueNotSaDirect:
      return "Feature/Portrayal catalogue is not authenticated directly by the Scheme Administrator";
    case SigRejectCode::UnsupportedDataStatus:
      return "unsupported dataStatus (compressed/encrypted)";
    case SigRejectCode::ProducerCodeMismatch:
      return "producer_code_mismatch";
  }
  return "";
}

}  // namespace s100::security
