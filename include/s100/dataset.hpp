#pragma once
#include "s100/layer.hpp"
#include "s100/product.hpp"
#include "s100/types.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace s100 {

struct Dataset {
  ProductId product{ProductId::Unknown};
  std::filesystem::path path;
  std::vector<Feature> features;
  LayerSet layers;
  DatasetStats stats() const;
  /** WGS84 envelope; valid=false until real spatial decode fills it. */
  Bounds bounds() const;
};

enum class LoadErrorKind { Io, Invalid, Parse };

struct LoadError {
  LoadErrorKind kind;
  std::string message;
};

/** Load an S-101 cell (.000). On failure returns empty optional and fills err. */
bool load_s101(const std::filesystem::path& path, Dataset& out, LoadError* err = nullptr);

/** Product-aware entry: dispatches by extension/header. P0: S-101 .000 only;
 *  S-102/S-104/S-111 HDF5 and S-124 GML return Parse until those loaders land. */
bool open_dataset(const std::filesystem::path& path, Dataset& out, LoadError* err = nullptr);

}  // namespace s100
