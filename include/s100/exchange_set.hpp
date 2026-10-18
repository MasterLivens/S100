#pragma once
#include "s100/dataset.hpp"
#include "s100/product.hpp"
#include "s100/types.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace s100 {

/** One datasetDiscoveryMetadata row from CATALOG.XML (any product). */
struct ExchangeDatasetEntry {
  ProductId product{ProductId::Unknown};
  std::string product_version;
  std::filesystem::path file_path;
  std::string file_name;
  int edition_number{0};
  int update_number{0};
  bool has_edition_meta{false};  // true when CATALOG.XML had editionNumber
  bool has_update_meta{false};   // true when CATALOG.XML had updateNumber
  std::string purpose;
  std::string description;
  Bounds coverage;
};

/** Parsed S-100 exchange catalogue (S100_ExchangeCatalogue / S100XC:...). */
struct ExchangeCatalogue {
  std::string identifier;
  std::string date_time;
  std::filesystem::path root;
  std::filesystem::path catalog_path;
  std::vector<ExchangeDatasetEntry> datasets;
};

bool parse_exchange_catalogue(const std::filesystem::path& path, ExchangeCatalogue& out,
                              LoadError* err = nullptr);

struct ExchangeCellSeries {
  ProductId product{ProductId::Unknown};
  std::string cell_name;  // dataset identity (file stem / DSID DSNM stem)
  std::filesystem::path base;
  std::vector<std::filesystem::path> updates;  // ordered by catalogue ED/UPD when present
  /** Catalogue ED/UPD for each path in updates (same size); has_* false if omitted in XML. */
  std::vector<ExchangeDatasetEntry> update_entries;
  ExchangeDatasetEntry base_entry{};
  int edition_number{0};
  int latest_update_number{0};
  Bounds coverage;
  std::string description;
};

/** ChartSession member key: ProductId name + dataset identity (no parallel product maps). */
std::string exchange_member_key(ProductId product, const std::string& dataset_identity);

std::vector<ExchangeCellSeries> group_cell_series(const ExchangeCatalogue& cat);
std::vector<std::filesystem::path> discover_update_files(const std::filesystem::path& base_000);
/** Why an S-101 update was rejected (docs/update-robustness-contract.md rev 1.2).
 *  Additive: the CANCEL-REISSUE slice extends this enum without breaking callers. */
enum class UpdateRejectCode {
  None,
  EditionMismatch,   // update ED != dataset ED
  OutOfSequence,     // update UPD != dataset UPD + 1 (covers duplicate, gap, reversed)
  NotUpdateFile,     // DSID says base / wrong product / not S-101
  Truncated,         // file shorter than leader/directory claims
  BadStructure,      // ISO 8211 leader / directory / field parse error
  BadInstruction,    // unknown RUIN / FAUI / IUIN / ATIN value
  MissingTarget,     // Delete/Modify of a record id that does not exist
  CatalogueMismatch  // CATALOG.XML ED/UPD disagrees with DSID of the file
};

/** Stable name of a reject code ("None", "OutOfSequence", ...). */
const char* update_reject_code_name(UpdateRejectCode code);

struct UpdateReport {
  int applied{0};                          // updates applied successfully, in order
  std::filesystem::path rejected;          // first rejected file, empty if none
  UpdateRejectCode code{UpdateRejectCode::None};
  std::string message;                     // human text, includes file name + ED.UPD
};

/** Apply one S-101 update. Strong guarantee: on false `ds` is unchanged and `code` (when
 *  given) names the single reason; on true `code` is None. */
bool apply_s101_update(Dataset& ds, const std::filesystem::path& update_path,
                       LoadError* err = nullptr, UpdateRejectCode* code = nullptr);
/** Base + updates in the given order (sequenced by DSID ED/UPD, not file name). Stops at the
 *  first rejected update: returns false, `out` holds base + the applied prefix. */
bool open_dataset_with_updates(const std::filesystem::path& base,
                               const std::vector<std::filesystem::path>& updates, Dataset& out,
                               LoadError* err = nullptr, UpdateReport* report = nullptr);
bool open_cell_series(const ExchangeCellSeries& series, Dataset& out, LoadError* err = nullptr,
                      UpdateReport* report = nullptr);

/** FAUI/IUIN element merge (docs/association-contract.md). instruction 0 on all → replace. */
bool apply_association_updates(std::vector<AssociationRef>& existing,
                               const std::vector<AssociationRef>& patch, LoadError* err = nullptr);


}  // namespace s100
