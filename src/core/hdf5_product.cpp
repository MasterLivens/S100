#include "s100/hdf5_product.hpp"

namespace s100 {

bool load_hdf5_product(ProductId id, const std::filesystem::path& path, Dataset& out,
                       LoadError* err) {
  (void)out;
  if (err) {
    err->kind = LoadErrorKind::Parse;
    err->message = std::string(product_name(id)) + " HDF5 loader not linked yet: " + path.string();
  }
  return false;
}

}  // namespace s100
