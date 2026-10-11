# Public API contracts (P0 freeze)

Owner: SoftwareArchitecture. Implement: SoftwareEngineer. Test: SoftwareTester.

## Entry

- `bool open_dataset(path, Dataset&, LoadError*)` — only public load entry for apps/tests.
- `bool load_s101(...)` — S-101 helper; prefer `open_dataset` in new code.
- HDF5 / GML paths return `LoadErrorKind::Parse` until P1/P3 loaders land.

## Dataset (vector / S-101)

- `product`, `path`, `features`, `layers`, `stats()` (`feature_count`, `layer_count`).
- `Bounds bounds() const` → `{west,south,east,north}` WGS84 (`valid` flag). Present now from feature envelope; re-validate when ISO 8211 spatial is real. Tester keys fixtures on this.

## Layers (P1, FC-driven — replaces `LayerId`)

Decision 2026-10-11 (SoftwareArchitecture). Layering comes **only** from the Feature Catalogue; no hard-coded enum, no `Other` fallback.

```cpp
// types.hpp: Feature drops `LayerId layer`; layer key is the FC code already in feature_type.

// layer.hpp
struct LayerInfo {
  std::string code;       // FC feature type code, e.g. "DepthArea" (= layer key)
  std::string name;       // FC human name
  std::string use_type;   // geographic | meta | cartographic | theme
  std::size_t feature_count{0};
  // reserved for portrayal (PC), filled in P1 portrayal step, not now:
  // display_category (base/standard/other), drawing_priority, viewing_group
};

class LayerSet {             // per Dataset, built by classify_layers
 public:
  const LayerInfo* find(const std::string& code) const;
  const std::map<std::string, LayerInfo>& all() const;
};

class LayerVisibility {      // per session/viewer, NOT stored in Dataset
 public:
  void set_layer(const std::string& code, bool on);
  void set_use_type(const std::string& use_type, bool on);  // e.g. hide all "meta"
  bool is_visible(const LayerInfo& layer) const;            // layer rule overrides use_type rule
};

struct ClassifyResult { std::vector<std::string> unknown_codes; };

/** Build ds.layers from FC. Returns false if any feature code is missing from FC
 *  (unknown_codes lists them). No fallback layer. */
bool classify_layers(Dataset& ds, const FeatureCatalogue& fc, ClassifyResult* r = nullptr);
```

Rules:
1. **Codec stays FC-free.** `iso8211` only fills `feature_type`; classification is a separate step. FC is product-level (one per product+version), loaded once and shared by all datasets in a session.
2. `open_dataset` signature unchanged. Session/CLI/tests call `classify_layers` after open.
3. Unknown FC code → `classify_layers` returns false. Tests treat that as failure; viewer shows a warning and does not draw unknown features.
4. Visibility default: geographic on, meta off, cartographic on. ECDIS display categories come later from PC and will add fields to `LayerInfo`, not new keys.
5. Viewer colours keyed by code for now (`LandArea`, `DepthArea`); real symbology is the portrayal step.
6. Migration: delete `LayerId`, `layer_for_feature_type`, and the hard-coded LAND/DEPARE registration; `test_layers` moves to code keys.

## Pick
- `pick_wgs84(px,py,w,h, west,south,east,north)` → WGS84; tolerance check ≤ `1e-5` deg.
- Control points: `testdata/s101/fixtures.toml` after real ISO 8211 spatial records.

## Coverage (P1 — do not implement in P0)

- `sample(lon, lat[, t])` on coverage products; grids must not be flattened into `features`.


## Exchange set (P1)

Parallel to `open_dataset` — do not overload the single-file entry. Details: `docs/exchange-set.md`.

- `parse_exchange_catalogue(path, ExchangeCatalogue&, LoadError*)` — `CATALOG.XML` or catalogue root directory.
- `group_cell_series(cat)` → `ExchangeCellSeries` (base `.000` + ordered `.001`…).
- `open_dataset_with_updates(base, updates, Dataset&, LoadError*)` / `open_cell_series(...)`.
- `ChartSession`: `open_exchange_set` → `member_keys` / `load_member` / `load_all_members` / `find`.
- Coverage products listed in the catalogue are not loaded yet; session is Dataset-keyed for multi-cell S-101 first.

## Stability

Breaking these hooks needs Architect + Tester ack in the room before merge.
