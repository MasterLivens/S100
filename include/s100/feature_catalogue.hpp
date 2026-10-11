#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace s100 {

/** One feature type from an S-100 Feature Catalogue (FC) XML. */
struct FcFeatureType {
  std::string code;                     // e.g. "DepthArea" — matches FTCS names in the cell
  std::string name;                     // human name, e.g. "Depth Area"
  std::string use_type;                 // featureUseType: geographic | meta | cartographic | theme
  std::vector<std::string> primitives;  // permittedPrimitives: point, curve, surface, pointSet, ...
  std::string super_type;               // abstract parent code, if any
  bool is_abstract{false};
};

struct FeatureCatalogue {
  std::string product;   // productId, e.g. "S-101"
  std::string version;   // versionNumber, e.g. "2.0.0"
  std::string date;      // versionDate
  std::map<std::string, FcFeatureType> feature_types;  // by code

  const FcFeatureType* find(const std::string& code) const {
    auto it = feature_types.find(code);
    return it == feature_types.end() ? nullptr : &it->second;
  }
};

/** Parse an S-100 FC XML (S100FC namespace). Returns false and sets `error` on failure. */
bool load_feature_catalogue(const std::filesystem::path& path, FeatureCatalogue& out,
                            std::string* error = nullptr);

}  // namespace s100
