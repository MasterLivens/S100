#pragma once
// Host side of S-100 Part 13 (Scripting) + Part 9a (Portrayal): the Host* functions the
// official PC Lua framework calls. One registry per product; see docs/portrayal-contract.md
// rule 7 and docs/testing/host-functions.txt.
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "s100/types.hpp"
#include "s100/coverage.hpp"
#include "s100/dataset.hpp"

struct lua_State;  // Lua stays out of public headers

namespace s100 {

struct Dataset;
struct FeatureCatalogue;

using HostFunction = int (*)(lua_State*);

/** Name -> implementation table for one product's host. Aliases share the implementation of
 *  their target (e.g. S-102 HostFeatureTypeGetCode -> HostFeatureGetCode). */
class HostRegistry {
 public:
  explicit HostRegistry(std::string product = {}) : product_(std::move(product)) {}
  const std::string& product() const { return product_; }

  void add(const std::string& name, HostFunction fn);
  /** Register `alias` with the implementation already registered as `target`. */
  void add_alias(const std::string& alias, const std::string& target);

  bool has(const std::string& name) const { return entries_.count(name) != 0; }
  /** All registered names (implementations and aliases), sorted. */
  std::vector<std::string> names() const;
  bool is_alias(const std::string& name) const;
  /** Name of the implementation behind `name` (itself unless an alias); empty if unknown. */
  std::string target_of(const std::string& name) const;
  HostFunction function(const std::string& name) const;

 private:
  struct Entry {
    std::string target;
    HostFunction fn{nullptr};
  };
  std::string product_;
  std::map<std::string, Entry> entries_;
};

/** One HostPortrayalEmit call. */
struct HostEmission {
  std::string feature_id;
  std::string instructions;  // DEF drawing instructions
  std::string observed;      // observed context parameters (DEF)
};

/** Shared interface of the per-product hosts. The engine installs registry() into a Lua state
 *  (each function gets this adapter as upvalue) and reads emissions() back. */
class HostAdapter {
 public:
  virtual ~HostAdapter() = default;
  HostAdapter(const HostAdapter&) = delete;
  HostAdapter& operator=(const HostAdapter&) = delete;

  const HostRegistry& registry() const { return registry_; }
  const std::string& product() const { return registry_.product(); }
  const FeatureCatalogue& catalogue() const { return *fc_; }

  /** Number of features offered by HostGetFeatureIDs. */
  virtual std::size_t feature_count() const = 0;
  /** Map a Lua feature ID back to the dataset feature id (DrawingInstruction::feature_id). */
  virtual bool feature_id_of(const std::string& lua_id, std::uint64_t& out) const = 0;

  // Run-time sinks (filled by HostPortrayalEmit / HostDebuggerEntry).
  std::vector<HostEmission> emissions;
  std::vector<std::string> trace_errors;    // Debug.Trace('Error: ...') — rule failures
  std::size_t first_chance_errors{0};
  std::size_t warnings{0};
  void reset_run();

 protected:
  HostAdapter(std::string product, const FeatureCatalogue& fc) : registry_(std::move(product)), fc_(&fc) {}
  HostRegistry registry_;
  const FeatureCatalogue* fc_;
};

/** S-101 host: backed by the Dataset feature list (+ topology, information records). */
class S101HostAdapter : public HostAdapter {
 public:
  S101HostAdapter(const Dataset& ds, const FeatureCatalogue& fc);
  std::size_t feature_count() const override;
  bool feature_id_of(const std::string& lua_id, std::uint64_t& out) const override;

  const Dataset& dataset() const { return *ds_; }
  // Lookups used by the host functions (public for tests).
  const Feature* feature(const std::string& lua_id) const;
  const InformationRecord* information(const std::string& lua_id) const;
  static std::string spatial_id(std::uint8_t rcnm, std::uint32_t rcid);
  /** Feature IDs that reference spatial `sid` directly or through composite curves / rings. */
  const std::vector<std::string>& features_of_spatial(const std::string& sid) const;

 private:
  const Dataset* ds_;
  std::unordered_map<std::string, std::size_t> feature_index_;      // lua id -> features[i]
  std::unordered_map<std::uint32_t, std::size_t> feature_by_rcid_;  // FRID RCID -> features[i]
  std::unordered_map<std::uint32_t, std::size_t> info_by_rcid_;     // IRID RCID -> information[i]
  mutable bool spatial_index_built_{false};
  mutable std::unordered_map<std::string, std::vector<std::string>> spatial_features_;
  void build_spatial_index() const;
  friend struct S101HostImpl;
};

/** S-102 host: backed by CoverageDataset (HDF5 loader). No vector spatials or information types. */
class S102HostAdapter : public HostAdapter {
 public:
  S102HostAdapter(const CoverageDataset& cov, const FeatureCatalogue& fc);
  std::size_t feature_count() const override { return cov_->features.size(); }
  bool feature_id_of(const std::string& lua_id, std::uint64_t& out) const override;
  const CoverageFeature* feature(const std::string& lua_id) const;

 private:
  const CoverageDataset* cov_;
  std::unordered_map<std::string, std::size_t> index_;
  friend struct S102HostImpl;
};

}  // namespace s100
