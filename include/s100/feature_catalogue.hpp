#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace s100 {

/** Attribute binding (feature/information type or complex attribute sub-binding). */
struct FcAttributeBinding {
  std::string attribute;            // attribute code
  int lower{0};
  int upper{-1};                    // -1 = unbounded (infinite)
  bool sequential{false};
  std::vector<int> permitted_values;
};

/** informationBinding / featureBinding of a type. */
struct FcTypeBinding {
  std::vector<std::string> types;   // target informationType / featureType codes
  int lower{0};
  int upper{-1};                    // -1 = unbounded
  std::string role_type;            // association | aggregation | composition
  std::string role;
  std::string association;
};

/** One feature type from an S-100 Feature Catalogue (FC) XML. */
struct FcFeatureType {
  std::string code;                     // e.g. "DepthArea" — matches FTCS names in the cell
  std::string name;                     // human name, e.g. "Depth Area"
  std::string definition;
  std::vector<std::string> alias;
  std::string use_type;                 // featureUseType: geographic | meta | cartographic | theme
  std::vector<std::string> primitives;  // permittedPrimitives: point, curve, surface, pointSet, ...
  std::string super_type;               // abstract parent code, if any
  std::vector<std::string> sub_types;   // derived from super_type of other types
  bool is_abstract{false};
  std::vector<FcAttributeBinding> attribute_bindings;  // own + inherited (supertype first)
  std::vector<FcTypeBinding> information_bindings;     // own + inherited
  std::vector<FcTypeBinding> feature_bindings;         // own + inherited
};

struct FcInformationType {
  std::string code, name, definition, super_type;
  std::vector<std::string> alias, sub_types;
  bool is_abstract{false};
  std::vector<FcAttributeBinding> attribute_bindings;  // own + inherited
  std::vector<FcTypeBinding> information_bindings;
};

struct FcListedValue {
  std::string label, definition;
  int code{0};
};

struct FcSimpleAttribute {
  std::string code, name, definition;
  std::vector<std::string> alias;
  std::string value_type;  // boolean | enumeration | integer | real | text | date | time | ...
  std::string uom;         // unit of measure name, if any
  std::string quantity_specification;
  std::vector<FcListedValue> listed_values;
};

struct FcComplexAttribute {
  std::string code, name, definition;
  std::vector<std::string> alias;
  std::vector<FcAttributeBinding> sub_attribute_bindings;
};

/** Role, information association or feature association. */
struct FcNamedItem {
  std::string code, name, definition;
  std::vector<std::string> roles;  // associations only
  std::string super_type;
  bool is_abstract{false};
};

struct FeatureCatalogue {
  std::string product;   // productId, e.g. "S-101"
  std::string version;   // versionNumber, e.g. "2.0.0"
  std::string date;      // versionDate
  std::map<std::string, FcFeatureType> feature_types;  // by code
  std::map<std::string, FcInformationType> information_types;
  std::map<std::string, FcSimpleAttribute> simple_attributes;
  std::map<std::string, FcComplexAttribute> complex_attributes;
  std::map<std::string, FcNamedItem> roles;
  std::map<std::string, FcNamedItem> information_associations;
  std::map<std::string, FcNamedItem> feature_associations;

  const FcFeatureType* find(const std::string& code) const {
    auto it = feature_types.find(code);
    return it == feature_types.end() ? nullptr : &it->second;
  }
};

/** Parse an S-100 FC XML (S100FC namespace). Returns false and sets `error` on failure. */
bool load_feature_catalogue(const std::filesystem::path& path, FeatureCatalogue& out,
                            std::string* error = nullptr);

}  // namespace s100
