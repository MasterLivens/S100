#pragma once
#include "s100/dataset.hpp"
#include "s100/exchange_set.hpp"
#include "s100/security/types.hpp"
#include "s100/security/trust_store.hpp"
#include "s100/security/verify.hpp"
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace s100 {

/** One session for all products (S-101, S-102, …). Keys = product + dataset id. */
class ChartSession {
 public:
  /** Library default VerifyPolicy::Enforce (Part 15 security boundary).
   *  Off is for tests / explicit opt-out only — not a product escape hatch. */
  bool open_exchange_set(const std::filesystem::path& path, LoadError* err = nullptr,
                         security::VerifyPolicy policy = security::VerifyPolicy::Enforce);
  const ExchangeCatalogue& catalogue() const { return catalogue_; }
  const std::vector<ExchangeCellSeries>& series() const { return series_; }
  /** Keys are exchange_member_key(product, cell_name) — shared map, no per-product parallel. */
  std::vector<std::string> member_keys() const;
  /** Open one exchange-set member (docs/update-robustness-contract.md rev 1.3, Session).
   *  On a rejected update the last good state (base + applied prefix) is KEPT as the member
   *  and false is returned; the report is kept either way (last_update_report). A base cell
   *  that fails to load leaves no member. Under Enforce, resources in last_verification().skipped
   *  are not loaded. */
  bool load_member(const std::string& key, LoadError* err = nullptr);
  /** UpdateReport of the last load_member for `key` (member key or bare cell name); a
   *  default report (code None, applied 0) when that member was never loaded. */
  UpdateReport last_update_report(const std::string& key) const;
  std::size_t load_all_members(LoadError* first_err = nullptr);
  Dataset* find(const std::string& key);
  const Dataset* find(const std::string& key) const;
  const std::map<std::string, Dataset>& datasets() const { return datasets_; }
  /** Insert an already-loaded Dataset under exchange_member_key(product, identity).
   *  identity defaults to dataset_name or path stem. Used to overlay S-102 on S-101. */
  std::string insert(Dataset ds, std::string dataset_identity = {});
  /** open_dataset(path) then insert under the shared session key. */
  bool open_and_insert(const std::filesystem::path& path, LoadError* err = nullptr,
                       std::string* key_out = nullptr);
  void clear();

  void set_verify_trust(security::TrustStore trust) { trust_ = std::move(trust); }
  void set_verify_options(security::VerifyOptions options) { verify_options_ = std::move(options); }
  const security::VerificationReport& last_verification() const { return last_verification_; }
  security::VerifyPolicy verify_policy() const { return verify_policy_; }

 private:
  bool path_is_skipped(const std::filesystem::path& p) const;

  ExchangeCatalogue catalogue_;
  std::vector<ExchangeCellSeries> series_;
  std::map<std::string, Dataset> datasets_;  // shared across S-101/S-102/…
  std::map<std::string, UpdateReport> update_reports_;  // by exchange_member_key
  security::TrustStore trust_{security::TrustStore::builtin_iho_52()};
  security::VerifyOptions verify_options_{};
  security::VerificationReport last_verification_{};
  security::VerifyPolicy verify_policy_{security::VerifyPolicy::Enforce};
};

}  // namespace s100
