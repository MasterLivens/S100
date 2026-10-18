#pragma once
// Internal to s100_core (not installed): shared by s101_update.cpp and exchange_set.cpp so the
// catalogue-vs-DSID check runs at its contract position (docs/update-robustness-contract.md
// Behaviour #3) inside the per-update check order.
#include "s100/exchange_set.hpp"

#include <filesystem>
#include <vector>

namespace s100 {
namespace detail {

/** Base + updates in list order. `entries`, when non-null, has one entry pointer per update
 *  (nullptr = no catalogue metadata for that file). Fills `report` (may be null). */
bool open_s101_with_updates(const std::filesystem::path& base,
                            const std::vector<std::filesystem::path>& updates,
                            const std::vector<const ExchangeDatasetEntry*>* entries, Dataset& out,
                            LoadError* err, UpdateReport* report);

}  // namespace detail
}  // namespace s100
