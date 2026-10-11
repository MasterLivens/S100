#pragma once
// Portrayal engine: embedded Lua 5.1 running the official PC rules unmodified
// (docs/portrayal-contract.md). Qt-free.
#include <map>
#include <string>

#include "s100/drawing_instructions.hpp"
#include "s100/host_adapter.hpp"
#include "s100/portrayal_catalogue.hpp"

namespace s100 {

struct Dataset;
struct FeatureCatalogue;

/** Mariner settings for one session. Values are textual (as Part 9a
 *  PortrayalSetContextParameter takes them); defaults come from the PC. */
struct PortrayalContext {
  std::map<std::string, std::string> values;  // parameter id -> value

  static PortrayalContext defaults(const PortrayalCatalogue& pc);
  /** Set a parameter the PC declares; false (and *err) for an unknown id. */
  bool set(const PortrayalCatalogue& pc, const std::string& id, const std::string& value,
           std::string* err = nullptr);
  /** Stable hash (hex) of all values: part of the cache key (dataset, PC version, context). */
  std::string hash() const;
};

/** Runs a PC's TopLevelTemplate (main.lua) against one host. A fresh Lua state per run; only
 *  base/package/table/string/math libraries are opened, `require` resolves to <PC>/Rules only. */
class PortrayalEngine {
 public:
  explicit PortrayalEngine(const PortrayalCatalogue& pc) : pc_(&pc) {}
  bool run(HostAdapter& host, const PortrayalContext& ctx, DrawingInstructions& out,
           PortrayalStats* stats = nullptr, std::string* err = nullptr) const;

 private:
  const PortrayalCatalogue* pc_;
};

/** S-101 entry (contract signature). Returns false on a fatal Lua/host error; rule errors that
 *  main.lua recovers from (Default symbology) are counted in stats->lua_errors. */
bool portray(const Dataset& ds, const FeatureCatalogue& fc, const PortrayalCatalogue& pc,
             const PortrayalContext& ctx, DrawingInstructions& out, std::string* err = nullptr,
             PortrayalStats* stats = nullptr);

/** S-102 entry: same engine and output type, coverage-backed host. */
bool portray(const CoverageDataset& cov, const FeatureCatalogue& fc, const PortrayalCatalogue& pc,
             const PortrayalContext& ctx, DrawingInstructions& out, std::string* err = nullptr,
             PortrayalStats* stats = nullptr);

/** Fill LayerInfo viewing_group / drawing_priority / display_plane / display_category from a
 *  portrayal run (contract rule 6). Per layer: most frequent value over its features' first
 *  instruction. Layers keep their FC keys. */
void apply_portrayal_to_layers(Dataset& ds, const DrawingInstructions& di, const PortrayalCatalogue& pc);

/** Session: owns the context; changing a parameter re-runs portrayal on the next portray().
 *  Cache key = (dataset path, PC product+version, context hash). */
class PortrayalSession {
 public:
  PortrayalSession(const FeatureCatalogue& fc, const PortrayalCatalogue& pc)
      : fc_(&fc), pc_(&pc), ctx_(PortrayalContext::defaults(pc)) {}
  const PortrayalContext& context() const { return ctx_; }
  bool set_context_parameter(const std::string& id, const std::string& value, std::string* err = nullptr) {
    return ctx_.set(*pc_, id, value, err);
  }
  /** Portray (or reuse the cached result) and fill ds.layers. nullptr on failure. */
  const DrawingInstructions* portray(Dataset& ds, PortrayalStats* stats = nullptr, std::string* err = nullptr);
  std::string cache_key(const Dataset& ds) const;
  std::size_t runs() const { return runs_; }

 private:
  struct Entry {
    DrawingInstructions instructions;
    PortrayalStats stats;
  };
  const FeatureCatalogue* fc_;
  const PortrayalCatalogue* pc_;
  PortrayalContext ctx_;
  std::map<std::string, Entry> cache_;
  std::size_t runs_{0};
};

}  // namespace s100
