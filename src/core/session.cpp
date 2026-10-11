#include "s100/session.hpp"

namespace s100 {

bool ChartSession::open_exchange_set(const std::filesystem::path& path, LoadError* err) {
  clear();
  if (!parse_exchange_catalogue(path, catalogue_, err)) return false;
  series_ = group_cell_series(catalogue_);
  return true;
}

std::vector<std::string> ChartSession::member_keys() const {
  std::vector<std::string> keys;
  keys.reserve(series_.size());
  for (const auto& s : series_) keys.push_back(exchange_member_key(s.product, s.cell_name));
  return keys;
}

bool ChartSession::load_member(const std::string& key, LoadError* err) {
  const ExchangeCellSeries* found = nullptr;
  for (const auto& s : series_) {
    if (exchange_member_key(s.product, s.cell_name) == key || s.cell_name == key) {
      found = &s;
      break;
    }
  }
  if (!found) {
    if (err) {
      err->kind = LoadErrorKind::Invalid;
      err->message = "unknown exchange-set member: " + key;
    }
    return false;
  }
  Dataset ds;
  if (!open_cell_series(*found, ds, err)) return false;
  datasets_[exchange_member_key(found->product, found->cell_name)] = std::move(ds);
  return true;
}

std::size_t ChartSession::load_all_members(LoadError* first_err) {
  std::size_t ok = 0;
  bool captured = false;
  for (const auto& s : series_) {
    LoadError err;
    if (load_member(exchange_member_key(s.product, s.cell_name), &err)) {
      ++ok;
    } else if (first_err && !captured) {
      *first_err = err;
      captured = true;
    }
  }
  return ok;
}

Dataset* ChartSession::find(const std::string& key) {
  auto it = datasets_.find(key);
  if (it != datasets_.end()) return &it->second;
  for (const auto& s : series_) {
    if (s.cell_name == key) {
      auto it2 = datasets_.find(exchange_member_key(s.product, s.cell_name));
      return it2 == datasets_.end() ? nullptr : &it2->second;
    }
  }
  return nullptr;
}

const Dataset* ChartSession::find(const std::string& key) const {
  auto it = datasets_.find(key);
  if (it != datasets_.end()) return &it->second;
  for (const auto& s : series_) {
    if (s.cell_name == key) {
      auto it2 = datasets_.find(exchange_member_key(s.product, s.cell_name));
      return it2 == datasets_.end() ? nullptr : &it2->second;
    }
  }
  return nullptr;
}

void ChartSession::clear() {
  catalogue_ = ExchangeCatalogue{};
  series_.clear();
  datasets_.clear();
}

}  // namespace s100
