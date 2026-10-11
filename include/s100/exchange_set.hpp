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
bool open_dataset_with_updates(const std::filesystem::path& base,
                               const std::vector<std::filesystem::path>& updates, Dataset& out,
                               LoadError* err = nullptr);
bool open_cell_series(const ExchangeCellSeries& series, Dataset& out, LoadError* err = nullptr);
bool apply_s101_update(Dataset& ds, const std::filesystem::path& update_path,
                       LoadError* err = nullptr);

/** FAUI/IUIN element merge (docs/association-contract.md). instruction 0 on all → replace. */
bool apply_association_updates(std::vector<AssociationRef>& existing,
                               const std::vector<AssociationRef>& patch, LoadError* err = nullptr);


}  // namespace s100
