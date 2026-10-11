#pragma once
#include "s100/dataset.hpp"

namespace s100 {

/**
 * Future HDF5 product loaders (S-102 / S-104 / S-111).
 * Link against HDF5 when P1 starts; keep this facade so viewers stay product-agnostic.
 */
bool load_hdf5_product(ProductId id, const std::filesystem::path& path, Dataset& out,
                       LoadError* err = nullptr);

}  // namespace s100
