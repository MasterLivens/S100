#pragma once
// Canonical content digest of an S-101 Dataset (docs/update-robustness-contract.md rev 1.2,
// oracle #4). One shared helper: compared before vs after every rejected update.
#include "s100/dataset.hpp"

#include <string>

namespace s100test {

/** Canonical serialisation (exposed for diffing when a digest comparison fails). */
std::string dataset_canonical(const s100::Dataset& ds);
/** sha256 hex of dataset_canonical(ds). */
std::string dataset_digest(const s100::Dataset& ds);

}  // namespace s100test
