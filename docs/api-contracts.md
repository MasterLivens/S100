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
- **Sample channels:** `CoverageSample` is dual-channel — `depth`/`has_depth` and `uncertainty`/`has_uncertainty`. Vertical uncertainty metres come from BathymetryCoverage compound field **`uncertainty`**. Missing unc → `has_uncertainty == false` via `is_fill_uncertainty` (fill default `1.0e6f`). **Depth-only compound (PS §10):** column + Group_F omitted → load MUST succeed. UNCERTAINTY rev1: fixture B → `has_uncertainty == false`. **`IHO-S102-QBC-META` elects broadcast** when `minimumUncertainty==maximumUncertainty` and not fill → `has_uncertainty == true` with that constant (`BCAST-*` oracle; see `docs/s102-qbc-meta-contract.md`).
- **QBC meta (Active):** optional `QualityOfBathymetryCoverage` (multiplicity 0..1; `dataCodingFormat` must be **9**; U32 id grid fill **0** + `featureAttributeTable`). Additive on sample: `qbc_id` / `has_qbc`; on dataset: `has_qbc_coverage`, `lookup_qbc_record(id, QbcAttributeRecord&)`. HDF5 table field names pinned in the QBC contract (e.g. `surveyAuthority`, `sourceSurveyID`, `zoneOfConfidence.horizontalPositionUncertainty.*`, `typeOfBathymetricEstimationUncertainty`). Official PC → **NullInstruction** — **do not** paint id raster as continuous unc. CTest `s102_qbc_meta` / case `IHO-S102-QBC-META`.
- Portrayal: **one depth CoverageFill per grid** (N× `Kind::Coverage`, `coverage_attribute=="depth"`, VG `13030`) plus, when unc channel present, **one uncertainty CoverageFill per grid** (`coverage_attribute=="uncertainty"`, project-private vendor-extension VG **`13031`** — not a PS/PC clause; independently toggleable; hide `13031` must not strip `13030` depth). See `docs/s102-uncertainty-contract.md` (`IHO-S102-UNCERTAINTY` rev1 / CTest `s102_uncertainty`). No QBC CoverageFill colour band.
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
- **PC wiring Active:** `docs/pc-wire-contract.md` (`IHO-PC-WIRE-111-124`). S-124: Lua PC via `S124HostAdapter` / `portray_nav_warnings_pc`. S-111: Arrow `symbol_ref`→`SCAROW*` + ColorProfiles (XSLT host still WARN; **no** invented S-111 Lua).


## SCAMIN / scale filter

- Details: `docs/scamin-contract.md` (`IHO-SCAMIN-FILTER`).
- API: `visible_at_scale(di, view_scale_denom, ignore_scale_minimum)`; `filter_instructions_by_scale(...)`.
- Hide when `view_denom > scale_minimum` (unless IgnoreScaleMinimum) or `view_denom < scale_maximum`.
- CLI: `--view-scale <N>`, `--ignore-scamin`. CTest `scamin_filter` MUST NOT SKIP.


## Display priority / draw order

- Details: `docs/display-priority-contract.md` (`IHO-DISPLAY-PRIORITY`).
- API: `display_plane_rank`, `draw_order_less`, `sort_instructions_for_draw`.
- Order: plane rank ascending, then `drawing_priority` ascending; `stable_sort`.
- CTest `display_priority_order` MUST NOT SKIP.


## Timeline UI (S-104 / S-111)

- Details: `docs/timeline-ui-contract.md` (`IHO-TIMELINE-UI`).
- Core: `timeline_entries_water_level` / `timeline_entries_current`; optional `timeline_step_frame`.
- CLI: `--list-frames` (`product\tindex\ttime_iso\tactive`); reuse `--s104-frame` / `--s111-frame` / time flags.
- Viewer: independent scrubbers; re-portray on frame change.
- CTest `timeline_ui` MUST NOT SKIP.


## Bridge rendering (S-101)

- Details: `docs/bridge-render-contract.md` (`IHO-S101-BRIDGE-RENDER`).
- Official PC Lua only; no hand-drawn bridge glyphs. Fixture `101AA00DS0006.000`.
- CTest `fasc_bridge_render` MUST NOT SKIP (Linux + Windows).
- New pure API: `filter_instructions_by_viewing_group(DrawingInstructions&, const std::set<std::string>& hidden_groups)` in `include/s100/viewing_group_filter.hpp`.


## Update robustness (S-101)

- Details: `docs/update-robustness-contract.md` (`IHO-S164-UPD-SEQ`, `IHO-S164-UPD-CORRUPT`; cancel/reissue next slice).
- Additive: `UpdateRejectCode`, `UpdateReport`, optional out-params on `apply_s101_update` / `open_dataset_with_updates` / `open_cell_series`.
- `apply_s101_update` strong guarantee; series stops at first rejection with last good state.
- CTest `exchange_update_robustness` MUST NOT SKIP (Linux + Windows).


## Cancel / re-issue / new edition (S-101)

- Details: `docs/cancel-reissue-contract.md` (`IHO-S164-CANCEL-REISSUE`).
- Additive: `UpdateRejectCode::DatasetCancelled`, `NewerEditionPending`, `DatasetNameMismatch`; `UpdateReport::skipped/cancelled/superseded/notice`; `Dataset::cancelled`; DSED bare integer accepted.
- CTest `exchange_cancel_reissue` MUST NOT SKIP (Linux + Windows).

## Edition hardening (S-101)

- Details: `docs/edition-hardening-contract.md` (`IHO-S164-EDITION-HARDEN`).
- Additive: `UpdateEditionRelation` + `classify_update_edition(installed_ed, update_ed)` (single edition rule shared by `group_cell_series` and the update check order).
- Behaviour: on every series path (`open_dataset_with_updates`, `open_cell_series`, `ChartSession`, viewer, CLI) an update with 0 < ED < base ED is skipped and listed in `UpdateReport::superseded` (no error). Standalone `apply_s101_update` keeps `EditionMismatch`; ED > installed keeps `NewerEditionPending`.
- `load_s101` / `open_dataset(.000)`: base with missing DSID, unparseable DSED or DSED edition 0 → `false` (`LoadErrorKind::Invalid`; `BadStructure` with `rejected == base` on update paths); `out` untouched on any `false`. Signatures unchanged.
- CTest `exchange_edition_hardening` MUST NOT SKIP (Linux + Windows).


## Part 15 signature verification (exchange set)

- Details: `docs/part15-signature-contract.md` (`IHO-S164-SIG`).
- New module `s100_security` (`include/s100/security/…`): `verify_exchange_set(root, TrustStore, VerifyOptions) → VerificationReport`; `VerifyPolicy {Off, Enforce}`; `TrustStore` value type; `sse_code` / `sse_message` in **one** map TU.
- `ChartSession::open_exchange_set` / `open_cell_series` gain optional `VerifyPolicy` (**library default `Enforce`** — security boundary; a forgotten policy must refuse unsigned data). Existing tests pass `VerifyPolicy::Off` **explicitly** (named deviation for frozen test files: call-arg only). Product CLI and viewer **hardcode `Enforce`**. No `--allow-unsigned` / env escape in the product. See `docs/part15-signature-contract.md` rev 1.1.
- Standalone `open_dataset` / `.000` unchanged; CLI and viewer show `UNVERIFIED`.
- Build: `find_package(OpenSSL 3.0 REQUIRED)`; target defines `OPENSSL_API_COMPAT=30000` and `OPENSSL_NO_DEPRECATED`.
- CTest `exchange_signature` MUST NOT SKIP (Linux + Windows).


## Power-up / start-up self-check (Track B)

- Details: `docs/powerup-self-check-contract.md` (`IHO-S164-POWERUP`) — **rev0 skeleton / pending Planner checklist**.
- App start: structured `PowerUpReport` (identity, critical files, SA trust store, `s100_security` self-test, catalogue/exchange smoke under `VerifyPolicy::Enforce`). Block vs warn-and-continue; CLI / viewer / `ChartSession` surface TBD in rev1.
- **Calls into** existing Part 15 verify + `open_exchange_set` — does **not** reimplement SIG; **no** producer-code matching in this slice.
- CTest `powerup_self_check` MUST NOT SKIP (Linux + Windows) — Eng after Planner freeze.

## Stability

Breaking these hooks needs Architect + Tester ack in the room before merge.
