# Public API contracts (P0 freeze)

Owner: SoftwareArchitecture. Implement: SoftwareEngineer. Test: SoftwareTester.

## Entry

- `bool open_dataset(path, Dataset&, LoadError*)` — only public load entry for apps/tests.
- `bool load_s101(...)` — S-101 helper; prefer `open_dataset` in new code.
- S-102 → `coverage`; S-104 → `water_level`; S-111 → `current`; S-124 GML → `nav_warnings` (see S-124 section). Other GML still `Parse` until loaders land.

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
- `pick_wgs84(px,py,w,h, west,south,east,north)` → WGS84; tolerance check ≤ `1e-5` deg (view helper).
- Control points: `testdata/s101/fixtures.toml`.
- **Session:** `chart_session_pick(session, lon, lat, …)` — unified multi-product report; see `docs/session-pick-contract.md`. CLI `--session-pick`. Returns `true` with zero hits when nothing matches; `false` only on hard failure.

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


## S-104 (water level — first slice)

Details: `docs/s104-contract.md`.

- `open_dataset` on S-104 HDF5 fills `Dataset::water_level` (`WaterLevelDataset`), not `coverage`.
- `WaterLevelDataset::{set_active_frame,set_active_time,sample_wgs84,bounds}`; sample is **active frame only**.
- `ChartSession` key `S-104:<stem>`; CLI `--s104` / `--s104-frame` / `--s104-time`.
- CI fixture `testdata/s104/synthetic_waterlevel_4326.h5` (tracked); primary CTest `s104_open_frame` MUST not SKIP.

## S-111 (surface current — first slice)

Details: `docs/s111-contract.md`.

- `open_dataset` fills `Dataset::current` (`CurrentDataset`), not coverage/water_level.
- `CurrentDataset::{set_active_frame,set_active_time,sample_wgs84,bounds}`; sample is active frame; façade returns speed + direction.
- Portray: `DrawingInstruction::Kind::Arrow` (direction→`rotation`, speed→`scale_factor`); stride `kS111ArrowStride=2`.
- Wrong HDF5 product must not fill `Dataset::current` (`LoadError`).
- `ChartSession` key `S-111:<stem>`; CLI `--s111` / `--s111-frame` / `--s111-time`.
- CI fixture `testdata/s111/synthetic_current_4326.h5` (oracle in SOURCE.md); CTest `s111_open_frame` MUST not SKIP.

## S-124 (navigational warnings — first slice)

Details: `docs/s124-contract.md`.

- `open_dataset` / CLI `--s124` fill `Dataset::nav_warnings` (`NavWarningDataset`). No Feature mirroring.
- `Dataset::bounds()` unions `nav_warnings.bounds()`.
- Portray each warning: geom DI + `Kind::Text` title (`CHYLW`); fixture MUST yield `warnings.size()==2`.
- Empty file → `Invalid`; zero NW → `Parse`; wrong product → error and no `nav_warnings` fill.
- `ChartSession` key `S-124:<stem>`. Fixture shipped at `testdata/s124/synthetic_navwarning.gml`.
- CTest `s124_open_list` / case `IHO-S124-OPEN-LIST` MUST not SKIP.

## Catalogues (FC/PC)

- S-101/S-102 pinned as today.
- S-111/S-124 MUST ship `S-10X_FC_2.0.0.xml` + `PC_2.0.0/portrayal_catalogue.xml` (sha + FC/PC edition pair).
- S-104 MUST ship **FC only** (`S-104_FC_2.0.0.xml`); official PC is **N/A** for Ed.2.0.0 — never forge; provenance skips PC pairing for S-104.
- Details: `docs/catalogue-complete-contract.md`. CTest `catalogue_provenance`: required-file/sha/pair failures → **FATAL** (S-104 PC checks N/A).
- Wiring loaders to S-111/S-124 PC is follow-up (WARN).

## Stability

Breaking these hooks needs Architect + Tester ack in the room before merge.
