#pragma once
#include "s100/coverage.hpp"
#include "s100/dataset.hpp"
#include "s100/water_level.hpp"

namespace s100 {

/** Load an S-102 HDF5 cell into a CoverageDataset (bounds + grid + host features). */
bool load_s102(const std::filesystem::path& path, CoverageDataset& out, LoadError* err = nullptr);

/** Load an S-104 HDF5 cell into a WaterLevelDataset (≥1 time frame). */
bool load_s104(const std::filesystem::path& path, WaterLevelDataset& out, LoadError* err = nullptr);

/**
 * HDF5 product façade. S-102 fills `out.coverage`; S-104 fills `out.water_level`.
 * Detects product from productSpecification / group layout. Wrong/unreadable S-104 claim → LoadError.
 * S-111 still returns Parse until its loader lands.
 */
bool load_hdf5_product(ProductId id, const std::filesystem::path& path, Dataset& out,
                       LoadError* err = nullptr);

}  // namespace s100
