# S-102 QualityOfBathymetryCoverage meta + §10 min==max broadcast (rev1) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S102-QBC-META`** (**Active / Open**). CTest: **`s102_qbc_meta`**.
Status: **rev1** — Planner freeze (MUST below). Takes QBC deferral and §10 min==max broadcast failure face from `docs/s102-uncertainty-contract.md` (`IHO-S102-UNCERTAINTY` **Closed**).

Builds on `docs/s102-contract.md` / `docs/s102-deepen-contract.md` / `docs/s102-uncertainty-contract.md`. Does **not** reopen OPEN / OVERLAY / DEEPEN / UNCERTAINTY dual-channel VG `13031` painting.

Primary research: `/workspace/research/qbc-meta/BRIEF.md` (+ earlier `/workspace/research/s102-uncertainty/BRIEF.md` + NOAA sample `102US004AK5HQ262267.h5`).

**Status: Active / Open (2026-10-25)** — after **UNCERTAINTY Closed** (tip `0539109`; CI [`40152825887`](https://github.com/MasterLivens/S100/actions/runs/40152825887) dual-green; Closed CI [`40142462597`](https://github.com/MasterLivens/S100/actions/runs/40142462597) @ `13b7c3c`, O1=#1b tip-truth). Eng tip ancestor **`7b20a56`** already on tip — **not greenfield** (optional QoBC U32+FAT / NullInstruction / §10 broadcast elect / fixtures C+BCAST-2 / CTest `QBC-1..4` + `BCAST-1..2`). Ubuntu+Windows already Passed — tip CI [`40098659137`](https://github.com/MasterLivens/S100/actions/runs/40098659137) `#5` + Eng landing [`39107961238`](https://github.com/MasterLivens/S100/actions/runs/39107961238) @ `7b20a56` (native `s100_tests` + HDF5, **not** `viewer_*` — **no Win O1 (a)/(b)**). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from UNCERT / DEEPEN / QBC-MG. Do **not** invent oracles; freeze tip-truth below. Do **not** reopen UNCERT / DEEPEN. Do **not** conflate failure faces **`QBC-*` ≠ `BCAST-*`**. Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). After Closed next: **`IHO-S102-QBC-MULTIGRID`** (Eng `aabbb54` already on main; separate tip).

**Planner freeze (MUST — from rev1 + BRIEF §7 + tip truth):**
- Reuse case id **`IHO-S102-QBC-META`** / CTest **`s102_qbc_meta`**; **forbidden** to invent a second case/test name.
- MUST = **QBC-1..4** (fixture **C** + Q6 real NOAA/CHS) + **BCAST-1..2** (fixture **B** broadcast + fill-minmax no-broadcast) + portray **NullInstruction** / **no** id colour band + prior `s102_uncertainty` / deepen / overlay green.
- Fixtures C / B / BCAST-2 **MUST NOT SKIP** on CI (Linux+Win).
- Failure faces **`QBC-*` ≠ `BCAST-*`** — do not conflate; same CTest binary OK; CHECK prefixes stay separable.
- Eng tip already has **`7b20a56`**; Ubuntu+Win (`40098659137` #5 / Eng `39107961238`) + local Passed → this slice = **Arch docs close** (Open→Closed), not greenfield.
- **`grid()` shim stays Backlog** (DEEPEN O1); QBC may still use `grid()`; retire = separate Backlog, **not** this tip.
- Ubuntu + Windows: `s102_qbc_meta` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **`IHO-S102-QBC-MULTIGRID`** (Eng `aabbb54` already; separate tip). Do **not** Closed QBC-MG on this tip.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `s102_qbc_meta` MUST NOT SKIP, Passed (already on tip CI `40098659137` #5 / Eng `39107961238`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng diff required
- After Close next: **`IHO-S102-QBC-MULTIGRID`**

**Eng notes:** verify `7b20a56` on tip; fixtures C + B + BCAST-2 + `SOURCE.md`; driver `tests/test_s102.cpp` `test_s102_qbc_meta`; cmake `s102_qbc_meta`. UNCERT Closed already on this stack (`0539109`). Do **not** revert broadcast loader / UNCERT #1b tip-truth. Soft pixel WARN-only — do **not** harden EncDotNet. Multi-grid QoBC bind out of scope (QBC-MG).

**O-questions (BRIEF §7 — Planner already answered; freeze tip-truth):**
1. Docs close / verify primarily (Eng `7b20a56` already; no `ci.yml` / no large Eng) — **recommend yes / freeze**.
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` — **recommend yes / freeze**.
3. Notes split failure faces **`QBC-*` ≠ `BCAST-*`** — **recommend yes / freeze**.
4. Closed next → **`IHO-S102-QBC-MULTIGRID`** (Eng `aabbb54` already; separate tip) — **recommend yes / freeze**.
5. `grid()` shim stays Backlog — **freeze, not a question**.

---

## Tester focus — two failure surfaces (do not conflate)

| Surface | Oracle id prefix | What MUST pass | What a FAIL means |
|---------|------------------|----------------|-------------------|
| **Primary — QBC meta** | **`QBC-*`** | Optional `QualityOfBathymetryCoverage` U32 id grid + `featureAttributeTable` load; pick/sample metadata lookup by cell id | QBC load / lookup / NullInstruction / real-sample QoBC assert broken |
| **Separate — §10 broadcast** | **`BCAST-*`** | Depth-only compound + usable `minimumUncertainty==maximumUncertainty` (not fill) → **broadcast** constant; `has_uncertainty==true` with that value | Broadcast election / fixture-B oracle broken — **not** a QBC failure |

CHECK / message text **MUST** carry the prefix (`QBC-1`, `BCAST-1`, …). Failing `BCAST-*` **MUST NOT** be reported or gated as a QBC failure (and vice versa). Same CTest binary `s102_qbc_meta` is fine; surfaces stay separable in logs.

**Do NOT** paint the QBC U32 id raster as a continuous uncertainty pseudocolor / CoverageFill band table.

---

## P1 — S-102 Ed 3.0.0 clause table (QBC + §10 omit)

| Topic | Clause | Requirement (short) |
|-------|--------|---------------------|
| QoBC optional | **§10** | Multiplicity **0..1**; absent → no QBC payload, bathymetry still loads |
| QoBC coding | **§10** | If present, `dataCodingFormat` **must be 9** (`featureOrientedRegularGrid`) |
| QoBC grid | **§4 / §10** | Only **`iD`** in the grid (U32); fill = **`0`**; `featureAttributeTable` **mandatory within** the QoBC group |
| Group_F | Encoding / samples | `/Group_F/QualityOfBathymetryCoverage` row `code=iD`, `fillValue="0"` |
| Omit unc column | **§10** | If all cells’ uncertainty equal, column **may be omitted** + Group_F uncertainty entry omitted |
| min/max attrs | **§4** | `minimumUncertainty` / `maximumUncertainty` on the values group |
| Portrayal | **§9** + PC 3.0.0 | **No** uncertainty/QoBC paint rules; `QualityOfBathymetryCoverage.lua` → **`NullInstruction`** (VG `13030`) |

---

## Findings (NOAA sample + CHS — locked names)

### HDF5 paths (NOAA `102US004AK5HQ262267.h5`)

| What | Path / fact |
|------|-------------|
| QoBC root | `/QualityOfBathymetryCoverage` — `dataCodingFormat` = **9** (`featureOrientedRegularGrid`) |
| Id grid | `/QualityOfBathymetryCoverage/QualityOfBathymetryCoverage.01/Group_001/values` — **U32**, shape matches bathymetry `{1347,1327}`; cell `(700,700)` → **`1`**; fill **`0`** |
| Attribute table | `/QualityOfBathymetryCoverage/featureAttributeTable` — compound dataset, **14** rows |
| Group_F | `/Group_F/featureCode` includes `QualityOfBathymetryCoverage`; `/Group_F/QualityOfBathymetryCoverage` one row `code=iD`, `fillValue="0"` |

### `featureAttributeTable` compound member names (exact HDF5 field names)

Pinned from NOAA h5dump (Eng **MUST** match these strings when reading):

| Field | HDF5 type (sample) | Notes |
|-------|--------------------|-------|
| **`id`** | U32 | Key into table; grid cell stores this id (not row index) |
| **`dataAssessment`** | U8 | |
| **`featuresDetected.leastDepthOfDetectedFeaturesMeasured`** | U8 | |
| **`featuresDetected.significantFeaturesDetected`** | U8 | |
| **`featuresDetected.sizeOfFeaturesDetected`** | F32 | |
| **`featureSizeVar`** | F32 | |
| **`fullSeafloorCoverageAchieved`** | U8 | |
| **`bathyCoverage`** | U8 | |
| **`zoneOfConfidence.horizontalPositionUncertainty.uncertaintyFixed`** | F32 | ZOC-class horizontal fixed |
| **`zoneOfConfidence.horizontalPositionUncertainty.uncertaintyVariableFactor`** | F32 | ZOC-class horizontal variable |
| **`surveyDateRange.dateStart`** | UTF-8 string | e.g. `"1807-02-10"` |
| **`surveyDateRange.dateEnd`** | UTF-8 string | |
| **`sourceSurveyID`** | UTF-8 string | e.g. `"Generalization"`, `"H10064.interpolated"` |
| **`surveyAuthority`** | UTF-8 string | e.g. `"DOC/NOAA/NOS/OCS -- Office of Coast Survey"` |
| **`typeOfBathymetricEstimationUncertainty`** | U8 enum | `unknown=0`, `rawStandardDeviation=1`, `cUBEStandardDeviation=2`, `productUncertainty=3`, `historicalStandardDeviation=4` |

FC feature `QualityOfBathymetryCoverage` binds only **`iD`** on the coverage feature; survey/ZOC fields live in the HDF5 **`featureAttributeTable`**, not as FC bindings on the grid feature.

### CHS

CHS cell `102CA0064682N07116W.h5` (when present): **`Group_F/featureCode` has only `BathymetryCoverage`** — **no** `QualityOfBathymetryCoverage` group. Documented: CHS **may lack** QoBC; that is **not** a FAIL when the cell is exercised without a QoBC group (`has_qbc_coverage == false`).

---

## Decisions

### QBC (primary)

- **Q1 Optional load (0..1).** If `/QualityOfBathymetryCoverage` is absent → success; `CoverageDataset` reports no QBC coverage. If present → load **one** instance (`.01`), U32 id grid + `featureAttributeTable`. Reject / `LoadError` if present but `dataCodingFormat != 9`, or table missing, or id grid unreadable. **Multi-grid attach** (which `grids[i]` receive `qbc_ids`) → **`IHO-S102-QBC-MULTIGRID`** / `docs/s102-qbc-multigrid-contract.md` (rev1; do not hard-bind `grids[0]` only).
- **Q2 Id fill.** Cell id **`0`** → `has_qbc == false` (no table lookup). Non-zero id → look up row where `featureAttributeTable.id == cell_id` (match by **`id` field**, not array index). Missing id in table → `has_qbc == false` (or soft warn — MUST NOT crash); do not invent a record.
- **Q3 Sample / pick lookup.** `sample_wgs84` / `sample_cell` (same cell routing as depth) fill additive QBC fields on the sample when QBC is loaded. Callers may also `lookup_qbc_record(id, …)` on the dataset. Session pick may surface QBC meta on the existing S-102 `CoverageSample` hit (additive fields / attributes) — **no** new session member key.
- **Q4 Official PC is NullInstruction.** Portrayal of QoBC follows shipped `QualityOfBathymetryCoverage.lua` → **`NullInstruction`**. This slice **MUST NOT** emit a CoverageFill / colour band for the id raster. Depth (`13030`) and uncertainty vendor VG (`13031` from UNCERTAINTY) unchanged.
- **Q5 Synthetic fixture C (MUST, no SKIP).** Gittracked under `testdata/s102/` (`.gitignore` `!` + SOURCE.md oracles). CTest **`s102_qbc_meta` MUST NOT SKIP** on Linux + Windows CI.
- **Q6 Real sample via TEST-DATA-ROOTS.** Prefer NOAA `testdata/s102/102US004AK5HQ262267.h5` when present (gitignored). Else CHS via `chs_exercised` / `chs_s100_root()` (`docs/test-data-roots-contract.md`). **Absent** → print not-exercised and **pass** (not CTest SKIP). **`S100_CHS_ROOT` set-but-bad** → **FAIL**. When NOAA present → **MUST** assert QoBC present + pinned cell lookup (see Oracles). When only CHS (no QoBC) → assert load succeeds with `has_qbc_coverage == false` (document lack; do not FAIL).

### BCAST (separate surface — elect the prior MAY)

- **B1 Elect broadcast.** When the BathymetryCoverage `values` compound **omits** the `uncertainty` member (and Group_F omits the uncertainty row) **and** `minimumUncertainty == maximumUncertainty` **and** that constant is **not** the uncertainty fill (`1e6` / `is_fill_uncertainty`), the loader **MUST** broadcast that constant into the in-memory uncertainty channel for every non-fill-depth cell → **`has_uncertainty == true`** and `uncertainty ==` that constant.
- **B2 Unusable min/max (already done).** Depth-only omit **without** a usable constant (attrs missing, or min/max are fill / non-finite / not equal) → **`has_uncertainty == false`** (no invented value). Keep this path green.
- **B3 Fixture B oracle flip.** Prior UNCERTAINTY landing elected *no* broadcast (`synthetic_uncertainty_depth_only_4326.h5`, min==max==**0.75** → `has_uncertainty==false`). **This case elects broadcast** → Eng **MUST** update SOURCE.md + `s102_uncertainty` #1b **and/or** assert under **`BCAST-*`** in `s102_qbc_meta` with `has_uncertainty==true`, `uncertainty≈0.75`. Do **not** overload fixture **C** (QBC) as the broadcast oracle.
- **B4 Same failure isolation.** `BCAST-*` CHECKs use distinct labels; a broadcast regression must not look like a QBC meta failure in CTest output.

### Shared

- **S1 No new ChartSession key.** QBC rides on existing `S-102:<stem>` member.
- **S2 Dual-platform.** `s102_qbc_meta` assertable on Linux + Windows CI (synthetic C + BCAST oracle always; real NOAA/CHS per Q6).

---

## API (additive — sync `api-contracts.md`)

```cpp
/** One row from QualityOfBathymetryCoverage/featureAttributeTable (subset OK). */
struct QbcAttributeRecord {
  std::uint32_t id{0};
  std::string survey_authority;       // "surveyAuthority"
  std::string source_survey_id;       // "sourceSurveyID"
  std::string survey_date_start;      // "surveyDateRange.dateStart"
  std::string survey_date_end;        // "surveyDateRange.dateEnd"
  float zoc_uncertainty_fixed{0};     // zoneOfConfidence.horizontalPositionUncertainty.uncertaintyFixed
  float zoc_uncertainty_variable{0};  // ...uncertaintyVariableFactor
  std::uint8_t data_assessment{0};
  std::uint8_t type_of_bathymetric_estimation_uncertainty{0};  // enum numeric
  bool valid{false};
};

struct CoverageSample {
  float depth{0};
  float uncertainty{0};
  bool has_depth{false};
  bool has_uncertainty{false};
  // additive — QBC:
  std::uint32_t qbc_id{0};   // 0 = fill / none
  bool has_qbc{false};
};

struct CoverageDataset {
  // ...
  bool has_qbc_coverage{false};  // true iff QoBC group loaded (multiplicity 1)
  bool lookup_qbc_record(std::uint32_t id, QbcAttributeRecord& out) const;
  // sample_wgs84 / per-grid sample_cell fill qbc_id/has_qbc when has_qbc_coverage
};
```

Eng may store the full table privately and expose only the fields above on `QbcAttributeRecord` (MUST include **`surveyAuthority`**, **`sourceSurveyID`**, ZOC horizontal pair, **`typeOfBathymetricEstimationUncertainty`** for oracles). Internal grid storage for U32 ids is Eng’s choice (not flattened into `CoverageGrid::uncertainty`).

Portrayal: **no** new `coverage_attribute` for QBC ids; do not reuse `"uncertainty"` / VG `13031` for the id raster.

---

## Fixtures + oracles

| Fixture | File | Surface | Purpose |
|---------|------|---------|---------|
| **C** (QBC) | `synthetic_qbc_meta_4326.h5` (tracked; Eng) | **QBC-*** | EPSG:4326, BathymetryCoverage (depth±unc OK) **plus** QoBC: `dataCodingFormat=9`, U32 id grid, ≥2 `featureAttributeTable` rows with distinct `id`s, known `surveyAuthority` / ZOC fields. Envelope preferably disjoint from deepen / unc A/B (e.g. lon/lat ≈40–40.4). |
| **B** (reuse) | `synthetic_uncertainty_depth_only_4326.h5` (already tracked) | **BCAST-*** | Depth-only; `minimumUncertainty==maximumUncertainty==0.75`. Oracle **flips** to broadcast (`has_uncertainty==true`, unc **0.75**). |
| Real NOAA | `102US004AK5HQ262267.h5` (gitignored) | **QBC-*** | QoBC present; cell id **1** → `surveyAuthority` contains `Office of Coast Survey` (or full pinned string in SOURCE.md). |
| Real CHS | via TEST-DATA-ROOTS | **QBC-*** | May lack QoBC → `has_qbc_coverage==false`, pass. |

### Fixture C oracles (Eng documents exact lon/lat in SOURCE.md)

| Point | Expect |
|-------|--------|
| **C1** | Non-zero id cell → `has_qbc==true`, `qbc_id` matches, `lookup_qbc_record` → pinned `surveyAuthority` / `sourceSurveyID` |
| **C2** | Id fill `0` cell → `has_qbc==false` |
| **C3** | Open → `has_qbc_coverage==true`; `dataCodingFormat` validated as 9 at load |

---

## Oracles — CTest `s102_qbc_meta` (Tester MUST)

| # | Id | Assert | Sev |
|---|-----|--------|-----|
| 1 | **QBC-1** | Open fixture **C**; `has_qbc_coverage==true`; Point C1: `has_qbc` + id + lookup fields match SOURCE | MUST |
| 2 | **QBC-2** | Point C2 (id fill 0): `has_qbc==false` | MUST |
| 3 | **QBC-3** | Portray fixture C: **no** CoverageFill whose attribute is the QBC id raster / continuous unc-from-id; QoBC path is NullInstruction (depth/unc DIs from UNCERTAINTY rules unchanged) | MUST |
| 4 | **QBC-4** | Real NOAA/CHS per Q6: absent → not exercised, pass; set-but-bad `S100_CHS_ROOT` → FAIL; NOAA present → QoBC load + cell `(700,700)` id **1** + `surveyAuthority` oracle; CHS-only without QoBC → `has_qbc_coverage==false`, pass | MUST |
| 5 | **BCAST-1** | Open fixture **B**; sample → **`has_uncertainty==true`**, `uncertainty≈0.75` (broadcast election). Message prefix **`BCAST-1`** | MUST |
| 6 | **BCAST-2** | Depth-only without usable min/max still → `has_uncertainty==false` (regress prior path; may use existing depth-only-with-fill-minmax if added, or document reuse). Prefix **`BCAST-2`** | MUST |
| 7 | — | Prior `s102_uncertainty` / deepen / overlay still green after fixture-B oracle flip | MUST |
| 8 | — | Dual-platform (Linux + Windows): at least QBC-1..3 + BCAST-1 | MUST |

Soft pixel / screenshot: **WARN-only**. EncDotNet: **out**.

---

## Non-goals

- EncDotNet pixel compare; S-98 / IMO **worst-case safety contour + uncertainty buffer**.
- Producer-code matching from SIG backlog.
- **POWERUP** / S-164 power-up certification pack.
- Painting QBC id raster as continuous uncertainty pseudocolor or any CoverageFill colour band for QoBC.
- Editing shipped official `QualityOfBathymetryCoverage.lua` away from **NullInstruction**.
- Reopening UNCERTAINTY VG `13031` band-table design; S-104/S-111 time; hillshade.
- Treating CHS-without-QoBC as a product defect.

---

## Open (historical Eng pins — tip-truth already landed @ `7b20a56`)

- Exact fixture **C** lon/lat / id values — **done** in `testdata/s102/SOURCE.md` (Eng tip).
- Session-pick / CLI pretty-print of QBC strings — fields on sample + `lookup_qbc_record` landed; CLI pretty-print remains optional WARN (not a Close blocker).
- Full FAT field round-trip vs oracle subset — Eng MAY load all HDF5 members internally; public record MUST cover oracle columns (tip-truth).

Planner O-questions (BRIEF §7) → **answered / freeze** in header Status block (docs close; Win no a/b; `QBC-*` ≠ `BCAST-*`; next=QBC-MG; `grid()` Backlog).

---

## Backlog (taken by follow-on)

- **Multi-grid QoBC bind** → **`IHO-S102-QBC-MULTIGRID`** (`docs/s102-qbc-multigrid-contract.md`, **rev1**). §10 QoBC root remains **0..1** / `.01` only (shared across co-located multi-VD `BathymetryCoverage.nn`). Do **not** hard-bind to `grids[0]`; O1 hybrid shape+origin/spacing attach; no matcher → `LoadError`; **`QBC-MG-*`** oracles. This contract’s **`QBC-*`** / **`BCAST-*`** surfaces stay closed.

---

## Implement sketch (Eng)

1. Generator for fixture **C** + `.gitignore` `!` + SOURCE.md oracles (QBC points + NOAA surveyAuthority pin).
2. Loader: optional QoBC group; require `dataCodingFormat==9`; U32 grid + table; id fill 0; `lookup_qbc_record` by `id`.
3. Extend `CoverageSample` / `CoverageDataset` per API; sample fills `qbc_id`/`has_qbc`.
4. Portray: keep NullInstruction; **no** id-raster CoverageFill.
5. **Elect broadcast** (B1); flip fixture **B** / `s102_uncertainty` #1b oracle; assert **`BCAST-*`** separately from **`QBC-*`**.
6. CTest `s102_qbc_meta` (Linux+Win); real branch per Q6 / TEST-DATA-ROOTS.
7. No push from Architect; Eng lands on eng branch after Tester plan.
