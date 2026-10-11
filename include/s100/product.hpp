#pragma once
#include <cstdint>
#include <string_view>

namespace s100 {

/** S-100 product family. P0 implements S101; others are reserved hooks. */
enum class ProductId : std::uint16_t {
  Unknown = 0,
  S101 = 101,  // ENC vector (ISO 8211)
  S102 = 102,  // bathymetric surface (HDF5) — P1
  S104 = 104,  // water level (HDF5) — P2
  S111 = 111,  // surface currents (HDF5) — P2
  S124 = 124,  // navigational warnings (GML) — P3
};

inline std::string_view product_name(ProductId id) {
  switch (id) {
    case ProductId::S101: return "S-101";
    case ProductId::S102: return "S-102";
    case ProductId::S104: return "S-104";
    case ProductId::S111: return "S-111";
    case ProductId::S124: return "S-124";
    default: return "Unknown";
  }
}

}  // namespace s100
