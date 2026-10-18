# S-102 deepen (rev 2) · Active / Open

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/s102-contract.md`. Case id: **`IHO-S102-DEEPEN-MULTIGRID`** (**Active / Open**).  
CTest name: **`s102_deepen_multigrid`** (plus helpers as needed).

**Status: Active / Open (2026-10-18)** — after **DISPLAY-COMPARE Closed** (CI `39133583148` @ `ba63833`, O1=a). Eng tip ancestor **`0dd39f2`** already on tip — **not greenfield** (multi-grid `CoverageDataset` / synthetic fixture / CTest MUST #1–6). Ubuntu+Windows CI already Passed (`s102_deepen_multigrid` is **native `s100_tests` + HDF5**, **not** `viewer_*` — **no Win O1 (a)/(b)**). This slice = **docs close primarily**. Independent tip from DISPLAY / UNCERTAINTY / QBC. Do **not** invent oracles; rev2 MUST #1–6 unchanged. No loader / portray rewrite. Do **not** reopen DISPLAY / UNCERT / QBC. After Closed next: **`IHO-S102-UNCERTAINTY`**.

**Planner freeze (MUST — from rev2, unchanged):**
- Reuse case id / CTest name; **forbidden** to invent a second case/test name.
- MUST #1–6: `grids.size() >= 2`; envelope `sample_wgs84` routing (smallest area → low index); N×CoverageFill with distinct `feature_id`; FourShades/SafetyContour band oracle; Day→Dusk token-stable + soft RGB; prior single-grid / overlay still green.
- Fixture `testdata/s102/synthetic_multigrid_4326.h5` (disjoint envelopes) **MUST NOT SKIP** on CI (Linux+Win).
- WGS84 façade; N×CoverageFill model (not a single instruction with embedded grid-id list).
- Keep deepen disjoint fixture ≠ QBC-MG PS co-located multi-VD (Non-goal).
- Ubuntu + Windows: `s102_deepen_multigrid` MUST NOT SKIP, Passed (already on tip ancestors).
- After Closed next: **`IHO-S102-UNCERTAINTY`**.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `s102_deepen_multigrid` MUST NOT SKIP, Passed (already on tip ancestors; re-check on tip after DISPLAY Close)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` change required
- After Close next: **`IHO-S102-UNCERTAINTY`**

**Eng notes:** verify `0dd39f2` on tip; fixture `testdata/s102/synthetic_multigrid_4326.h5` + `SOURCE.md`; driver `tests/test_s102.cpp` `test_s102_deepen_multigrid`; cmake `s102_deepen_multigrid`. DISPLAY Closed already on this stack. Day→Dusk soft RGB stays soft — do **not** harden. True AA5 / DISPLAY oracles out of scope.

**O-questions (real gaps only — Planner):**
1. **`grid()` temporary shim** — contract + api-contracts allow `grid()` → `grids[0]` for one release. Does this slice **retire the shim**, or leave as **Backlog** (downstream UNCERT/QBC may still call `grid()`)? **O1 for Planner** — do not pin Eng remove/keep until decided.
2. Docs close primarily (no `ci.yml`) — **recommend yes**.
3. Disjoint fixture / Day→Dusk soft — **keep** this slice.
4. Closed next → UNCERTAINTY (Eng `b2d17de` already on main; separate tip) — **recommend yes**.

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
