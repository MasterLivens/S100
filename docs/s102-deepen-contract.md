# S-102 deepen (rev 2) (Closed)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/s102-contract.md`. Case id: **`IHO-S102-DEEPEN-MULTIGRID`** (**Closed / Done** 2026-10-19).  
CTest name: **`s102_deepen_multigrid`** (plus helpers as needed) — MUST NOT SKIP; Linux + Windows dual-green (CI `39141930422` @ `5e05675`).

**Status: Closed / Done (2026-10-19)** — CI [`39141930422`](https://github.com/MasterLivens/S100/actions/runs/39141930422) @ `5e05675` dual-green; **`s102_deepen_multigrid` Passed Ubuntu+Win** (native `s100_tests` + HDF5 — **not** `viewer_*`; **no Win O1 (a)/(b)**). Eng **`0dd39f2`** already on tip — **docs close only**. Oracles MUST #1–6 **unchanged**. **O1 closed as Backlog** — `grid()` → `grids[0]` shim **stays** (still used heavily in `test_s102` single-grid / UNCERT / QBC; retiring now = big Eng test churn, hurts downstream Active cases). Leave cleanup tip / UNCERT Close path to migrate `grids[0]`/`sample_wgs84` later. Next: **`IHO-S102-UNCERTAINTY`**.

**Planner freeze (kept — rev2 unchanged):** Reuse case id / CTest name; MUST #1–6 (`grids.size() >= 2`; envelope `sample_wgs84` routing; N×CoverageFill; FourShades/SafetyContour; Day→Dusk token-stable + soft RGB; prior single-grid / overlay green); fixture `synthetic_multigrid_4326.h5` MUST NOT SKIP; WGS84 façade; deepen disjoint ≠ QBC-MG co-located (Non-goal).

**O1 closed as Backlog:** `grid()` temporary shim **kept** this slice. Do **not** retire here. Downstream UNCERT/QBC / single-grid tests still call `grid()`. Cleanup = Backlog (migrate callers → `grids[0]` / `sample_wgs84`; then remove shim). **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest.

## Slice close — 2026-10-19

**Status: Closed / Done** (formal docs close — **not** a loader / portray rewrite).

- Evidence: CI [39141930422](https://github.com/MasterLivens/S100/actions/runs/39141930422) tip `5e05675` — Ubuntu + Windows **`s102_deepen_multigrid` Passed** (MUST NOT SKIP).
- **O1=Backlog SUCCESS** — shim stays; retire deferred (test churn / downstream Active).
- Oracles stay as frozen rev2 MUST #1–6.
- Eng `0dd39f2` already on tip — docs close only; no `ci.yml` change.
- **Notes:** next Eng **`IHO-S102-UNCERTAINTY`**. **Backlog:** retire `CoverageDataset::grid()` shim after UNCERT/QBC callers migrate.
- **Non-goals this close:** inventing oracles; retiring `grid()` shim; loader/portray rewrite; reopening DISPLAY / UNCERT / QBC; Win (a)/(b) (N/A — non-`viewer_*`).

## Portray model (locked)

**N × CoverageFill instructions** — one `DrawingInstruction::Kind::Coverage` per loaded grid instance, in stable grid order.  
Each instruction carries a distinct `feature_id` (coverage feature instance id) and/or `coverage_attribute` / state so the viewer can attribute paint to a grid.  
**Not** a single instruction with an embedded grid-id list.

## Data model (sync `api-contracts.md`)

```cpp
struct CoverageDataset {
  // ...
  std::vector<CoverageGrid> grids;           // required; size >= 1 after successful open
  // CoverageGrid grid;                      // REMOVED as sole storage — keep temporary
  //                                       // shim `grid()` → grids[0] only for one release
  bool sample_wgs84(double lon, double lat, CoverageSample& out) const;
};
```

`CoverageDataset::sample_wgs84` routing:

1. Collect grids whose geographic envelope contains `(lon,lat)`.
2. If several: choose **smallest enclosing area**, then lowest index.
3. Else → false.

Projected CRS: handled inside `CoverageGrid::sample_wgs84` (EPSG:4326 + UTM 326xx/327xx today). Façade stays WGS84 — **no “projected deferred”** wording.

## CI fixture (MUST — no full SKIP)

Ship **`testdata/s102/synthetic_multigrid_4326.h5`** (gittracked via `.gitignore` `!` exception; see `testdata/s102/SOURCE.md`): two small EPSG:4326 regular grids with disjoint envelopes.  
MUST tests #1–3 below **must run on CI**, not SKIP.

## Palette / band oracle (locked)

S-102 PC 3.0.0 context parameters **`FourShades`** and **`SafetyContour`** (also ShallowContour/DeepContour when FourShades):

| Step | Assert |
|------|--------|
| `FourShades=false`, change `SafetyContour` | CoverageFill token / LookupEntry boundaries on DEPVS/DEPDW change (instruction text or parsed bands) |
| `FourShades=true` | Four depth tokens appear (DEPVS/DEPMS/DEPMD/DEPDW) per BathymetryCoverage.lua |
| Day → Dusk | Instruction token list **unchanged**; remapped RGB differs (soft) |

No hard-coded RGB in core.

## Tests (MUST)

| # | CTest focus |
|---|-------------|
| 1 | Synthetic `.h5`: `grids.size() >= 2` after open |
| 2 | `sample_wgs84` in grid-A-only vs grid-B-only returns that grid's depth |
| 3 | Portray emits **≥2** Coverage instructions; viewer/dump can attribute both |
| 4 | SafetyContour / FourShades band oracle above |
| 5 | Day→Dusk token-stable remap |
| 6 | Prior single-grid / UTM / overlay tests still green |

## Non-goals

S-104/S-111 time, S-98, EncDotNet pixel match. Uncertainty as its own layer → `docs/s102-uncertainty-contract.md` (`IHO-S102-UNCERTAINTY`). Reopening DISPLAY / TEXT / POINT / BRIDGE; rewriting multi-grid loader; converting deepen disjoint fixture to PS co-located multi-VD (QBC-MG); Win viewer_* `ci.yml` changes; platform SKIP; same-tip close of UNCERTAINTY / QBC; inventing new oracles.
