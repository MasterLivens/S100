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
  std::string cell_name;
  std::filesystem::path base;
  std::vector<std::filesystem::path> updates;
  int edition_number{0};
  int latest_update_number{0};
  Bounds coverage;
  std::string description;
};

std::vector<ExchangeCellSeries> group_cell_series(const ExchangeCatalogue& cat);
std::vector<std::filesystem::path> discover_update_files(const std::filesystem::path& base_000);
bool open_dataset_with_updates(const std::filesystem::path& base,
                               const std::vector<std::filesystem::path>& updates, Dataset& out,
                               LoadError* err = nullptr);
bool open_cell_series(const ExchangeCellSeries& series, Dataset& out, LoadError* err = nullptr);
bool apply_s101_update(Dataset& ds, const std::filesystem::path& update_path,
                       LoadError* err = nullptr);

}  // namespace s100
