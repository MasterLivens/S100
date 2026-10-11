# Public API contracts (P0 freeze)

Owner: SoftwareArchitecture. Implement: SoftwareEngineer. Test: SoftwareTester.

## Entry

- `bool open_dataset(path, Dataset&, LoadError*)` — only public load entry for apps/tests.
- `bool load_s101(...)` — S-101 helper; prefer `open_dataset` in new code.
- S-102 HDF5 loads via `open_dataset` / `load_s102` into `Dataset::coverage` (sample/bounds). S-104/S-111/GML still `Parse` until their loaders land.

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

## Coverage (S-102)

- `Dataset::coverage` holds `CoverageDataset` with **`std::vector<CoverageGrid> grids`** (+ host coverage features). Do not flatten grids into `features`.
- Singular `CoverageDataset::grid` storage removed; temporary shim `grid()` → `grids[0]` allowed one release only.
- `CoverageDataset::sample_wgs84(lon,lat)` routes to the containing grid (smallest envelope, then index). WGS84 façade; UTM/4326 converted inside the grid (see `docs/s102-deepen-contract.md`).
- Portrayal: **one CoverageFill instruction per grid** (N× `Kind::Coverage`).
- Deepen / multi-grid / FourShades·SafetyContour oracles: `docs/s102-deepen-contract.md`.


## Exchange set (P1)

Parallel to `open_dataset` — do not overload the single-file entry. Details: `docs/exchange-set.md`.

- `open_dataset` rejects lone `.001`… (`LoadErrorKind::Invalid`); use `open_dataset_with_updates` / `ChartSession`.
- Updates apply in DSID ED/UPD order (catalogue metadata validated when present); mismatch → `LoadError`, no partial apply.
- `ChartSession` is shared across products; member key = `ProductId` + dataset identity (`S-101:cellStem`).
- FASC/INAS: element-level FAUI/IUIN merge required for Bridge slice — see `docs/association-contract.md`.
  Match key `(rcnm,rcid,association,role)`; unknown FAUI → LoadError with per-feature association rollback.
  Host feature assoc: `associationCode` required; empty `roleCode` = any role. Spatial: `GetAssociatedFeatureIDs(spatialId)` ID-only; `GetAssociatedInformationIDs(spatialId, associationCode, roleCode)` filters info (not FASC).
- Coverage products (S-102 `.h5`) are members of the same `ChartSession` map (`S-102:<stem>`); `open_cell_series` routes them through `open_dataset`.

## Stability

Breaking these hooks needs Architect + Tester ack in the room before merge.
