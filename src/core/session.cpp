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
  const std::string member = exchange_member_key(found->product, found->cell_name);
  Dataset ds;
  UpdateReport report;
  const bool ok = open_cell_series(*found, ds, err, &report);
  update_reports_[member] = report;
  if (ok) {
    datasets_[member] = std::move(ds);
    return true;
  }
  // Rejected update: open_cell_series left base + applied prefix in ds (contract Behaviour #2).
  // Keep it as the member so it stays displayable. A base failure has no good state (ds was
  // never filled: empty path), so nothing is inserted.
  const bool have_good_state = report.code != UpdateRejectCode::None && !ds.path.empty() &&
                               report.rejected != found->base;
  if (have_good_state) datasets_[member] = std::move(ds);
  return false;
}

UpdateReport ChartSession::last_update_report(const std::string& key) const {
  auto it = update_reports_.find(key);
  if (it != update_reports_.end()) return it->second;
  for (const auto& s : series_) {
    if (s.cell_name == key) {
      auto it2 = update_reports_.find(exchange_member_key(s.product, s.cell_name));
      if (it2 != update_reports_.end()) return it2->second;
    }
  }
  return UpdateReport{};
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

std::string ChartSession::insert(Dataset ds, std::string dataset_identity) {
  if (dataset_identity.empty()) {
    // Prefer path stem so keys match group_cell_series / exchange_member_key (no .000 suffix).
    if (!ds.path.empty()) dataset_identity = ds.path.stem().string();
    else if (!ds.dataset_name.empty()) dataset_identity = ds.dataset_name;
    else dataset_identity = "unnamed";
  }
  const std::string key = exchange_member_key(ds.product, dataset_identity);
  datasets_[key] = std::move(ds);
  return key;
}

bool ChartSession::open_and_insert(const std::filesystem::path& path, LoadError* err,
                                   std::string* key_out) {
  Dataset ds;
  if (!open_dataset(path, ds, err)) return false;
  const std::string key = insert(std::move(ds));
  if (key_out) *key_out = key;
  return true;
}

void ChartSession::clear() {
  catalogue_ = ExchangeCatalogue{};
  series_.clear();
  datasets_.clear();
  update_reports_.clear();
}

}  // namespace s100
