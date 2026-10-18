# S-102 bathymetry uncertainty layer (rev1 draft pending Planner clauses)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S102-UNCERTAINTY`** (**Active**). CTest: **`s102_uncertainty`**.
Status: **rev1 draft pending Planner clauses** — IHO PS clause numbers are not filled here; see **Open (Planner)**. Do not invent clause refs.

Builds on `docs/s102-contract.md` / `docs/s102-deepen-contract.md`. Unlocks deepen Non-goal “uncertainty as its own layer” for this case only; does **not** reopen OPEN / OVERLAY / DEEPEN.

## Findings (code + samples — locked naming)

| What | Actual name / path | Notes |
|------|--------------------|-------|
| Vertical uncertainty (metres) | HDF5 compound member **`uncertainty`** in `/BathymetryCoverage/BathymetryCoverage.NN/Group_001/values` beside **`depth`** | Already read in `hdf5_product.cpp` (`DepthPair::{depth,uncertainty}` → `CoverageGrid::uncertainty`) |
| Group_F codes | `/Group_F/BathymetryCoverage` rows `code=depth` and `code=uncertainty`; `fillValue` string **`1000000`**, uom metres | Matches `CoverageGrid::fill_uncertainty` default `1.0e6f` |
| `QualityOfBathymetryCoverage` group | Optional `/QualityOfBathymetryCoverage/QualityOfBathymetryCoverage.NN/Group_001/values` = **U32** indices into `/QualityOfBathymetryCoverage/featureAttributeTable` | **Not** the metres uncertainty channel. NOAA `102US004AK5HQ262267.h5` has it; CHS `102CA0064682N07116W.h5` does **not** |
| FC | `BathymetryCoverage` binds `depth` (1) + optional `uncertainty` (0..1); separate meta type `QualityOfBathymetryCoverage` (`iD`) | `catalogues/S-102/S-102_FC_3.0.0.xml` |
| Official PC 3.0.0 | `BathymetryCoverage.lua` → VG **`13030`**, `CoverageFill:depth`; `QualityOfBathymetryCoverage.lua` → VG **`13030`**, **`NullInstruction`** | Same VG → cannot toggle quality vs depth via official rules alone |

**This slice’s “uncertainty layer”** = portray the BathymetryCoverage compound field **`uncertainty`** (metres) as its own CoverageFill, independently toggleable from depth. The optional QBC U32 / `featureAttributeTable` meta coverage is **out of this slice** (Open).

## Decisions

- **U1 Dual-channel sample (no side effects).** `CoverageSample` already carries `depth`/`has_depth` and `uncertainty`/`has_uncertainty`. `sample_wgs84` / `sample_cell` fill both from the same cell; return value stays “cell has depth” (existing). Sampling must not mutate grids, session hidden-VG set, or portrayal output. Toggle of the uncertainty layer must not change subsequent `sample_*` results.
- **U2 Missing-uncertainty sentinel.** Treat as missing when `is_fill_uncertainty(v)` (existing: non-finite, ≈ `fill_uncertainty`, or `|v| >= 1e5`). Then `has_uncertainty == false` and `uncertainty` is unspecified. Default fill **`1.0e6f`** (= Group_F `1000000`). Do not invent a second sentinel API.
- **U3 Independent CoverageFill + dedicated viewing group.** For each loaded bathymetry grid, portrayal emits **two** `Kind::Coverage` instructions in stable order: (1) depth — existing PC path, `coverage_attribute == "depth"`, VG **`13030`**; (2) uncertainty — `coverage_attribute == "uncertainty"`, ViewingGroup **`13031`**, DrawingPriority **3**, DisplayPlane **UnderRadar**. Viewer paints uncertainty bands from `CoverageGrid::uncertainty` when `coverage_attribute == "uncertainty"`. Hide/show via existing `filter_instructions_by_viewing_group` / `set_hidden_viewing_groups` on **`13031` only** — depth DIs stay. Interim VG **`13031`** (official PC 3.0.0 only lists `13030`; Quality rule is NullInstruction) — Planner may replace with a PS-assigned id (Open).
- **U4 Emit path for uncertainty DI.** Core/host emits the uncertainty CoverageFill after (or beside) the depth PC path — same pattern as S-104’s minimal tokenized CoverageFill. **Do not** edit the shipped `QualityOfBathymetryCoverage.lua` NullInstruction as the contract of record; do not invent a replacement official Lua file in-catalogue. Tokenized uncertainty bands: symbolic colour tokens only (no hard-coded RGB); a small fixed band table in core/host is OK for this slice (document bands in SOURCE.md). Day→Dusk: tokens stable, RGB remap only.
- **U5 Synthetic fixture mandatory.** Ship gittracked **`testdata/s102/synthetic_uncertainty_4326.h5`** (`.gitignore` `!` exception + SOURCE.md oracle). CTest **`s102_uncertainty` MUST NOT SKIP** on CI (Linux + Windows). Deepen `synthetic_multigrid_4326.h5` stays deepen-only (may keep incidental unc=1); do not overload it as the uncertainty oracle.
- **U6 Real sample via TEST-DATA-ROOTS rules.** Prefer NOAA `testdata/s102/102US004AK5HQ262267.h5` when present (gitignored; has real unc metres). Else CHS S-102 cell via `chs_exercised` / `chs_s100_root()` (`docs/test-data-roots-contract.md`). **Absent** → print not-exercised / skip-message and **pass** (not CTest SKIP). **`S100_CHS_ROOT` set-but-bad** → **FAIL** (no silent fallback). Real path asserts load + dual-channel sample shape; positive `has_uncertainty` oracle only when the file actually carries non-fill unc (NOAA yes; CHS cell today is all-fill unc).
- **U7 ChartSession / overlay.** No new session key. Uncertainty is an instruction/VG concern on the existing `S-102:<stem>` member. Prior deepen N× depth CoverageFill behaviour unchanged when VG `13031` is hidden.

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

Portrayal contract addition: uncertainty Coverage DI must set `coverage_attribute = "uncertainty"` and `viewing_group = "13031"` (until Planner replaces `13031`). Depth DI unchanged (`"depth"` / `"13030"`).

## Fixture oracle (`testdata/s102/SOURCE.md`)

| Item | Value |
|------|-------|
| File | `synthetic_uncertainty_4326.h5` (tracked) |
| CRS | EPSG:4326, single `BathymetryCoverage.01` |
| Compound | `depth` + `uncertainty` (same layout as production) |
| Point A (both channels) | documented lon/lat → known depth **and** `has_uncertainty` with pinned metres |
| Point B (depth, unc fill) | documented lon/lat → `has_depth`, **`has_uncertainty == false`** (cell unc = `1e6`) |
| Envelope | small, disjoint from deepen multigrid envelopes preferred |

## Oracles — CTest `s102_uncertainty` (Tester MUST)

| # | Assert | Sev |
|---|--------|-----|
| 1 | Open synthetic; `grids.size()>=1`; Point A: `has_depth` + `has_uncertainty` + values match SOURCE; Point B: `has_depth` && !`has_uncertainty` | MUST |
| 2 | Portray ≥1 depth Coverage (`coverage_attribute=="depth"`, VG `13030`) **and** ≥1 uncertainty Coverage (`"uncertainty"`, VG `13031`) | MUST |
| 3 | Hide VG `13031` → uncertainty DIs filtered out; depth DIs remain; re-sample Point A unchanged (no side effects) | MUST |
| 4 | Show VG `13031` again → uncertainty DIs present | MUST |
| 5 | Dual-platform (Linux + Windows CI) smoke: at least **#1 sample** + **#3/#4 layer toggle** | MUST |
| 6 | Real NOAA/CHS path: absent → not exercised, pass; set-but-bad `S100_CHS_ROOT` → FAIL; when NOAA present → sample finds ≥1 cell with `has_uncertainty` | MUST |
| 7 | Prior `s102_deepen_multigrid` / overlay / open-sample still green | MUST |

Soft pixel / screenshot: **WARN-only**. EncDotNet pixel compare: **out** (Non-goals).

## Non-goals

- EncDotNet pixel compare; S-98 interoperability mega-items.
- Producer-code matching from SIG backlog.
- Reopening OPEN / OVERLAY / DEEPEN cases or rewriting deepen fixtures.
- Loading / portraying `/QualityOfBathymetryCoverage` U32 index grid or `featureAttributeTable` (meta survey / ZOC records).
- Editing shipped official `QualityOfBathymetryCoverage.lua` NullInstruction as the long-term PC source of truth.
- S-104/S-111 time; animation.

## Open (Planner)

Fill before Eng implements against PS text; Architect will rev the draft when clause table lands (research tree: `/workspace/research/s102-uncertainty/`).

| # | Topic | Architect placeholder |
|---|-------|------------------------|
| P1 | S-102 Ed 3.0.0 PS clause refs for BathymetryCoverage `uncertainty` attribute / Group_F / fill | *(empty — do not invent)* |
| P2 | Official portrayal / viewing-group assignment for an uncertainty surface (replace interim **`13031`** if PS/PC assigns one) | Interim `13031` until replaced |
| P3 | Whether QBC U32 + `featureAttributeTable` is a follow-up case id or stays deferred | Deferred this slice |
| P4 | Uncertainty colour-band table vs future official PC tokens | Core/host fixed bands OK for rev1 |
| P5 | Any S-98 / interop note that would force EncDotNet compare | None — Non-goal |

## Implement sketch (Eng)

1. Fixture generator + SOURCE.md oracle + `.gitignore` `!`.
2. Ensure load already populates `uncertainty`; harden fill sentinel only if tests require.
3. Emit uncertainty CoverageFill (VG `13031`) beside depth; viewer branch on `coverage_attribute`.
4. CTest `s102_uncertainty` wired Linux+Win; real-sample branch per U6.
5. No push from Architect; Eng lands code on eng branch after Tester sign-off plan.
