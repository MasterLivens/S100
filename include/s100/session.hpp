#pragma once
#include "s100/dataset.hpp"
#include "s100/exchange_set.hpp"
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace s100 {

class ChartSession {
 public:
  bool open_exchange_set(const std::filesystem::path& path, LoadError* err = nullptr);
  const ExchangeCatalogue& catalogue() const { return catalogue_; }
  const std::vector<ExchangeCellSeries>& series() const { return series_; }
  std::vector<std::string> member_keys() const;
  bool load_member(const std::string& key, LoadError* err = nullptr);
  std::size_t load_all_members(LoadError* first_err = nullptr);
  Dataset* find(const std::string& key);
  const Dataset* find(const std::string& key) const;
  const std::map<std::string, Dataset>& datasets() const { return datasets_; }
  void clear();

 private:
  ExchangeCatalogue catalogue_;
  std::vector<ExchangeCellSeries> series_;
  std::map<std::string, Dataset> datasets_;
};

}  // namespace s100
