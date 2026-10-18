# S-102 bathymetry uncertainty layer (rev1)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S102-UNCERTAINTY`** (**Active**). CTest: **`s102_uncertainty`**.
Status: **rev1** — Planner P1–P5 resolved (clause table + decisions below). Supersedes draft `696cad3`.

Builds on `docs/s102-contract.md` / `docs/s102-deepen-contract.md`. Unlocks deepen Non-goal “uncertainty as its own layer” for this case only; does **not** reopen OPEN / OVERLAY / DEEPEN.

Primary research: `/workspace/research/s102-uncertainty/BRIEF.md` + `SOURCES.md` (S-102 Ed **3.0.0**, S-100 **5.2.0**).

---

## Tester focus (three named decisions — unmistakable)

| # | Named decision | Oracle / MUST |
|---|---------------|---------------|
| **T1** | **Depth-only compound** (PS §10) | Loader **MUST** open a BathymetryCoverage whose `values` compound has **only** `depth` (uncertainty column omitted **and** Group_F `uncertainty` entry omitted) → `has_uncertainty == false` at sample. May broadcast a constant from `minimumUncertainty==maximumUncertainty` when that constant is **not** the fill value (`1e6`). **Synthetic fixture B** + SOURCE.md oracle required. Current loader assumes compound always has `uncertainty` and is **brittle** — Eng must fix. |
| **T2** | **VG `13031` = project-private vendor extension** | Uncertainty CoverageFill uses viewing group **`13031`** (NOT a PS/PC clause). Official PC only has **`13030`**. Product may hide `13031` by default. Toggle/`set_hidden_viewing_groups` on **`13031` only** — **MUST NOT** hide or strip depth DIs on **`13030`**. Do **not** reuse `13030` for the uncertainty toggle (would hide depth when unc is off). |
| **T3** | **Non-goals + symbolic bands** | QBC U32 meta + §10 min==max broadcast → **taken by `IHO-S102-QBC-META`** (`docs/s102-qbc-meta-contract.md`; Non-goals here; do **not** treat id raster as continuous unc pseudocolor). EncDotNet pixel compare + S-98 worst-case safety contour / uncertainty buffer → Non-goals. Colour bands: **symbolic colour tokens only**, no hard-coded RGB; Day→Dusk tokens stable (RGB remap only). |

---

## P1 — S-102 Ed 3.0.0 clause table (locked)

| Topic | Clause | Requirement (short) |
|-------|--------|---------------------|
| Optional unc beside depth; same compound grid; fill | **§4** | Uncertainty optional beside depth in same compound grid; fill = **`1e6`**; QoBC id fill = **`0`** |
| Definition | **§1** | Uncertainty is **1-dimensional**, **2-sigma / 95%** |
| Per-grid vertical | **§6** | Per-grid-point **vertical** uncertainty |
| Omit when all equal | **§10** | If all cells equal, entire uncertainty column **may be omitted** + Group_F entry omitted; compound has **1 or 2** members |
| QoBC | **§10** | Multiplicity **0..1**; if present `dataCodingFormat` **must be 9**; `featureAttributeTable` required |
| Portrayal | **§9** | **No** portrayal rules for uncertainty/QoBC — points to PC |
| Encoding Guide | Annex | `depth` multiplicity **1**; `uncertainty` **0..1**; QoBC only **`iD`** |
| S-100 Part 8 | **§8-4.3** | Uncertainty is **application schema**, not a Part 8 fixed field |
| Official PC 3.0.0 | catalogues | Only VG **`13030`**; `BathymetryCoverage.lua` → `CoverageFill:depth`; `QualityOfBathymetryCoverage.lua` → **`NullInstruction`** (also 13030); S-164 Ed **1.0.0** has **no** uncertainty display golden |

---

## Findings (code + samples — locked naming)

| What | Actual name / path | Notes |
|------|--------------------|-------|
| Vertical uncertainty (metres) | HDF5 compound member **`uncertainty`** in `/BathymetryCoverage/BathymetryCoverage.NN/Group_001/values` beside **`depth`** | Already read in `hdf5_product.cpp` (`DepthPair::{depth,uncertainty}` → `CoverageGrid::uncertainty`) — **but** today assumes compound always has unc (fails depth-only; see **T1**) |
| Group_F codes | `/Group_F/BathymetryCoverage` rows `code=depth` and optionally `code=uncertainty`; `fillValue` string **`1000000`**, uom metres | Matches `CoverageGrid::fill_uncertainty` default `1.0e6f` |
| Depth-only compound (§10) | Compound **one** member (`depth` only); Group_F omits uncertainty row; typically `minimumUncertainty==maximumUncertainty` | Loader MUST support — **T1** / synthetic **B** |
| `QualityOfBathymetryCoverage` group | Optional `/QualityOfBathymetryCoverage/.../values` = **U32** indices into `featureAttributeTable` | **Not** the metres uncertainty channel. **Taken by `IHO-S102-QBC-META`**. NOAA `102US004AK5HQ262267.h5` has it; CHS may lack it |
| FC | `BathymetryCoverage` binds `depth` (1) + optional `uncertainty` (0..1); meta `QualityOfBathymetryCoverage` (`iD`) | `catalogues/S-102/S-102_FC_3.0.0.xml` |
| Official PC 3.0.0 | `BathymetryCoverage.lua` → VG **`13030`**, `CoverageFill:depth`; QoBC.lua → VG **`13030`**, **`NullInstruction`** | No official unc CoverageFill; no ShowUncertainty context |

**This slice’s “uncertainty layer”** = load/sample the BathymetryCoverage compound field **`uncertainty`** (metres), including depth-only omit, and optionally portray it as a vendor-extension CoverageFill on VG **`13031`**. QBC U32 / `featureAttributeTable` **and** §10 min==max broadcast election are **taken by `IHO-S102-QBC-META`**.

---

## Decisions

- **U1 Dual-channel sample (no side effects).** `CoverageSample` already carries `depth`/`has_depth` and `uncertainty`/`has_uncertainty`. `sample_wgs84` / `sample_cell` fill both from the same cell; return value stays “cell has depth” (existing). Sampling must not mutate grids, session hidden-VG set, or portrayal output. Toggle of the uncertainty layer must not change subsequent `sample_*` results.
- **U2 Missing-uncertainty sentinel.** Treat as missing when `is_fill_uncertainty(v)` (existing: non-finite, ≈ `fill_uncertainty`, or `|v| >= 1e5`). Then `has_uncertainty == false` and `uncertainty` is unspecified. Default fill **`1.0e6f`** (= Group_F `1000000`). Do not invent a second sentinel API.
- **U2b Depth-only compound (MUST).** Per PS §10: when the uncertainty column is omitted and the Group_F uncertainty entry is omitted, loader **MUST** succeed. **rev1 landing:** Eng did not elect broadcast → fixture **B** samples `has_uncertainty == false`. **Broadcast election + fixture-B oracle flip** moved to **`IHO-S102-QBC-META`** (`BCAST-*` surface; when min==max and not fill → broadcast, `has_uncertainty == true`). Unusable min/max still → `has_uncertainty == false`.
- **U3 Independent CoverageFill + vendor-extension VG `13031` (P2).** For each loaded bathymetry grid that has an uncertainty channel to portray, portrayal emits **two** `Kind::Coverage` instructions in stable order: (1) depth — existing PC path, `coverage_attribute == "depth"`, VG **`13030`**; (2) uncertainty — `coverage_attribute == "uncertainty"`, ViewingGroup **`13031`**, DrawingPriority **3**, DisplayPlane **UnderRadar**. **`13031` is a project-private vendor extension — NOT a PS/PC clause.** Official PC 3.0.0 only lists `13030`. Product **may hide `13031` by default**. Viewer paints uncertainty bands from `CoverageGrid::uncertainty` when `coverage_attribute == "uncertainty"`. Hide/show via existing `filter_instructions_by_viewing_group` / `set_hidden_viewing_groups` on **`13031` only** — depth DIs on **`13030` MUST remain**. **Do NOT use `13030` for the uncertainty toggle** (hiding it would hide depth). When the grid is depth-only (`has_uncertainty` never true / no unc channel loaded), emit **no** uncertainty CoverageFill (depth-only DI count unchanged).
- **U4 Emit path + band table (P4).** Core/host emits the uncertainty CoverageFill after (or beside) the depth PC path — same pattern as S-104’s minimal tokenized CoverageFill. **Do not** edit the shipped `QualityOfBathymetryCoverage.lua` NullInstruction as the contract of record; do not invent a replacement official Lua file in-catalogue. **rev1 band table:** fixed band table in core/host as **vendor extension**; **symbolic colour tokens only** (no hard-coded RGB); Day→Dusk: tokens stable, RGB remap only. Replace later if official PC adds rules. Document bands in SOURCE.md.
- **U5 Synthetic fixtures mandatory (A + B).** Both gittracked under `testdata/s102/` (`.gitignore` `!` exceptions + SOURCE.md oracles). CTest **`s102_uncertainty` MUST NOT SKIP** on CI (Linux + Windows). Deepen `synthetic_multigrid_4326.h5` stays deepen-only; do not overload it as the uncertainty oracle.
- **U6 Real sample via TEST-DATA-ROOTS rules.** Prefer NOAA `testdata/s102/102US004AK5HQ262267.h5` when present (gitignored; has **real per-cell unc**, e.g. cell unc **40.34**, and also QoBC U32 — **this slice asserts only the former**). Else CHS S-102 cell via `chs_exercised` / `chs_s100_root()` (`docs/test-data-roots-contract.md`). **Absent** → print not-exercised / skip-message and **pass** (not CTest SKIP). **`S100_CHS_ROOT` set-but-bad** → **FAIL** (no silent fallback). Real path asserts load + dual-channel sample shape; positive `has_uncertainty` oracle only when the file actually carries non-fill unc (NOAA yes; CHS cell today is all-fill unc). Do **not** assert QoBC load/portray here.
- **U7 ChartSession / overlay.** No new session key. Uncertainty is an instruction/VG concern on the existing `S-102:<stem>` member. Prior deepen N× depth CoverageFill behaviour unchanged when VG `13031` is hidden.

### Planner resolutions (P1–P5) — closed

| # | Resolution |
|---|------------|
| **P1** | Clause table above (S-102 Ed 3.0.0 §1/§4/§6/§9/§10 + Encoding Guide + Part 8 §8-4.3 + official PC 3.0.0 / S-164 Ed1.0.0). |
| **P2** | Keep project-private VG **`13031`** as vendor extension — **not** a PS/PC clause. Product may hide 13031 by default. **Do not** use 13030 for unc toggle. |
| **P3** | QBC U32 → **`IHO-S102-QBC-META`** (Active; `docs/s102-qbc-meta-contract.md`); stays Non-goal **here**; do not treat id raster as continuous uncertainty pseudocolor. |
| **P4** | rev1 uses core/host fixed band table as vendor extension; symbolic colour tokens only; Day→Dusk tokens stable; replace later if PC adds official rules. |
| **P5** | EncDotNet / S-98 worst-case safety contour + uncertainty buffer stay Non-goals. |

---

## API (additive notes — sync `api-contracts.md`)

No new public types required. Existing:

```cpp
struct CoverageSample {
  float depth{0};
  float uncertainty{0};
  bool has_depth{false};
  bool has_uncertainty{false};
};
// CoverageGrid::uncertainty / fill_uncertainty / is_fill_uncertainty — already present
```

Portrayal contract addition: uncertainty Coverage DI must set `coverage_attribute = "uncertainty"` and `viewing_group = "13031"` (**vendor extension**). Depth DI unchanged (`"depth"` / `"13030"`). Depth-only compound → no uncertainty DI; samples report `has_uncertainty == false`.

---

## Fixtures + oracle (`testdata/s102/SOURCE.md`)

| Fixture | File | Purpose / oracle |
|---------|------|------------------|
| **A** (dual-channel) | `synthetic_uncertainty_4326.h5` (tracked) | CRS EPSG:4326, single `BathymetryCoverage.01`, compound `depth`+`uncertainty`. **Point A:** both channels — known depth **and** `has_uncertainty` with pinned metres. **Point B:** `has_depth` && **`has_uncertainty == false`** (cell unc = `1e6`). Envelope small, preferably disjoint from deepen multigrid. |
| **B** (depth-only omit — **T1**) | `synthetic_uncertainty_depth_only_4326.h5` (tracked) | Compound **depth only**; Group_F **no** uncertainty row; `minimumUncertainty==maximumUncertainty==0.75`. **UNCERTAINTY rev1:** `has_uncertainty == false` (no broadcast). **QBC-META elects broadcast** → oracle flips there (`BCAST-*`); see `docs/s102-qbc-meta-contract.md`. |
| Real NOAA | `102US004AK5HQ262267.h5` (gitignored) | Assert **per-cell unc metres** only (e.g. unc **40.34**); QoBC U32 present but **not** asserted this slice. |

---

## Oracles — CTest `s102_uncertainty` (Tester MUST)

| # | Assert | Sev |
|---|--------|-----|
| 1 | Open fixture **A**; `grids.size()>=1`; Point A: `has_depth` + `has_uncertainty` + values match SOURCE; Point B: `has_depth` && !`has_uncertainty` | MUST |
| **1b** | Open fixture **B** (depth-only); load succeeds; documented sample → **`has_uncertainty == false`** (or documented broadcast oracle); **no** uncertainty CoverageFill required | **MUST (T1)** |
| 2 | Portray (fixture A) ≥1 depth Coverage (`coverage_attribute=="depth"`, VG `13030`) **and** ≥1 uncertainty Coverage (`"uncertainty"`, VG `13031`) | MUST |
| 3 | Hide VG **`13031`** → uncertainty DIs filtered out; **depth DIs on `13030` remain**; re-sample Point A unchanged (no side effects) | **MUST (T2)** |
| 4 | Show VG `13031` again → uncertainty DIs present; depth still on `13030` | MUST |
| 5 | Dual-platform (Linux + Windows CI) smoke: at least **#1/#1b sample** + **#3/#4 layer toggle** | MUST |
| 6 | Real NOAA/CHS path: absent → not exercised, pass; set-but-bad `S100_CHS_ROOT` → FAIL; when NOAA present → sample finds ≥1 cell with `has_uncertainty` (per-cell metres; **not** QoBC) | MUST |
| 7 | Prior `s102_deepen_multigrid` / overlay / open-sample still green | MUST |

Soft pixel / screenshot: **WARN-only**. EncDotNet pixel compare: **out** (Non-goals).

---

## Non-goals

- EncDotNet pixel compare; S-98 / IMO **worst-case safety contour + uncertainty buffer** (interop beyond S-102 PC) — **P5**.
- Producer-code matching from SIG backlog.
- Reopening OPEN / OVERLAY / DEEPEN cases or rewriting deepen fixtures.
- Loading / portraying `/QualityOfBathymetryCoverage` U32 index grid or `featureAttributeTable` as continuous uncertainty pseudocolor — **taken by `IHO-S102-QBC-META`** (`docs/s102-qbc-meta-contract.md`) (**P3**). §10 min==max constant broadcast election also moved there (separate **`BCAST-*`** oracle surface).
- Editing shipped official `QualityOfBathymetryCoverage.lua` NullInstruction as the long-term PC source of truth; forging a “standard” ShowUncertainty in official PC.
- S-104/S-111 time; animation; sun illumination / hillshade (PS §9).
- Hard-coded RGB in uncertainty band table (tokens only — **P4** / **T3**).

---

## Open

*(cleared — P1–P5 resolved above; residual research caveats live in BRIEF §6 and do not block Eng.)*

Follow-up **taken**: **`IHO-S102-QBC-META`** — QBC U32 + `featureAttributeTable` load/pick **and** §10 min==max broadcast election (`docs/s102-qbc-meta-contract.md`). Fixture B broadcast oracle flips there (`BCAST-*`); do not conflate with QBC CHECKs.

---

## Implement sketch (Eng)

1. Fixture **A** + fixture **B** generators + SOURCE.md oracles + `.gitignore` `!` exceptions.
2. **Fix loader** for depth-only compound (§10): omit unc member + omit Group_F row → `has_uncertainty == false`; optional broadcast from min==max when not fill. Harden fill sentinel only if tests require.
3. Emit uncertainty CoverageFill (VG **`13031` vendor extension**) beside depth (`13030`); viewer branch on `coverage_attribute`; default-hide `13031` OK; never bind unc toggle to `13030`.
4. Band table: symbolic tokens only; document in SOURCE.md.
5. CTest `s102_uncertainty` wired Linux+Win (#1, **#1b**, #2–7); real-sample branch per U6 (unc metres only).
6. No push from Architect; Eng lands code on eng branch after Tester sign-off plan.
