#pragma once
#include "s100/coverage.hpp"
#include "s100/dataset.hpp"

namespace s100 {

/** Load an S-102 HDF5 cell into a CoverageDataset (bounds + grid + host features). */
bool load_s102(const std::filesystem::path& path, CoverageDataset& out, LoadError* err = nullptr);

/**
 * HDF5 product façade. S-102 fills `out.coverage` and sets product/path; does not populate
 * vector `features`. S-104/S-111 still return Parse until their loaders land.
 */
bool load_hdf5_product(ProductId id, const std::filesystem::path& path, Dataset& out,
                       LoadError* err = nullptr);

}  // namespace s100
