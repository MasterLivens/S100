# S-104 water level — first slice (rev 2) (Closed)

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/api-contracts.md` (S-104) / `docs/catalogue-complete-contract.md` (S-104 FC only). Case id: **`IHO-S104-OPEN-FRAME`** (**Closed / Done** 2026-10-25).  
CTest name: **`s104_open_frame`** — MUST NOT SKIP; Linux + Windows dual-green (CI `40159993218` @ `28e0f2b`). Supersedes Open draft on tip `28e0f2b` (≡ Arch `06da68d`).

Primary research: `/workspace/research/s104-open-frame/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`2ee32c7`**. Scrubber / `--list-frames` / `timeline_entries` → separate Active **`IHO-TIMELINE-UI`** (`docs/timeline-ui-contract.md`).

**Status: Closed / Done (2026-10-25)** — CI [`40159993218`](https://github.com/MasterLivens/S100/actions/runs/40159993218) @ `28e0f2b` dual-green; **`s104_open_frame` Passed Ubuntu+Win** (native `s100_tests` + HDF5 — **not** `viewer_*`; **no Win O1 (a)/(b)**). Eng **`2ee32c7`** already on tip — **docs close only**. MUST tip-truth **unchanged** (open≥2 frames + frame API + sample 0.2/0.2 ≈1.0≠≈2.5 + CoverageFill + token change across frames + ChartSession S-101+S-104 + FC-only / no forge PC + soft pixel WARN-only; fixture MUST NOT SKIP). **O1=tip-truth / docs-close SUCCESS**. Failure face = bare CHECK tip-truth (do **not** invent `S104-*`). Do **not** reopen QBC-MG / UNCERT / DEEPEN; do **not** mix TIMELINE-UI / S-111; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog. Contract display stays **rev 2**. Next: **TBD**.

**Planner freeze (kept — rev 2 tip-truth):** Reuse case id **`IHO-S104-OPEN-FRAME`** / CTest **`s104_open_frame`**; MUST = open≥2 frames + frame API (`set_active_frame` / `set_active_time` / `sample_wgs84` / `bounds`) + sample 0.2/0.2 frame0≈**1.0** / frame1≈**2.5** (diff>0.5) + portray ≥1 CoverageFill + **token changes across frames** + ChartSession S-101+S-104 + FC-only (**do not forge PC**) + soft pixel **WARN-only**; data model `Dataset::water_level` / `WaterLevelDataset` (**forbid** reuse `Dataset::coverage`); fixture `synthetic_waterlevel_4326.h5` MUST NOT SKIP; scrubber → **`IHO-TIMELINE-UI`** (separate); Eng `2ee32c7` already; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog; failure face = bare CHECKs (do **not** invent `S104-*`); after Closed next **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen QBC-MG / UNCERT / DEEPEN; do **not** mix TIMELINE-UI / S-111.

**O1 closed as tip-truth / docs-close:** Eng `2ee32c7` already on tip (HDF5 open → `Dataset::water_level`; ≥2 frames; frame API; sample 0.2/0.2 → 1.0 / 2.5; portray CoverageFill + token change; ChartSession S-101+S-104; fixture `synthetic_waterlevel_4326.h5`; CTest **`s104_open_frame`**). Slice = Arch docs close only — not greenfield Eng. Do **not** invent case ids or next tip names; do **not** reopen QBC-MG / UNCERT / DEEPEN; do **not** forge PC; do **not** invent `S104-*` prefixes; do **not** retire `grid()` this tip. **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest; inventing oracles / next tip id; mixing TIMELINE-UI / S-111.

---

## Slice close — 2026-10-25

**Status: Closed / Done** (formal docs close — **not** a loader / portray rewrite).

- Evidence: CI [40159993218](https://github.com/MasterLivens/S100/actions/runs/40159993218) tip `28e0f2b` — Ubuntu + Windows **`s104_open_frame` Passed** (MUST NOT SKIP). Ancestor greens also Eng landing [`38126580110`](https://github.com/MasterLivens/S100/actions/runs/38126580110) @ `2ee32c7` + pre-Open tip [`40158058619`](https://github.com/MasterLivens/S100/actions/runs/40158058619) `#7`.
- **O1=tip-truth / docs-close SUCCESS** — Eng `2ee32c7` already on tip; Arch Open→Closed only; Planner BRIEF §7 answers frozen.
- MUST tip-truth unchanged (open≥2 frames + frame API + sample 0.2/0.2 ≈1.0≠≈2.5 + CoverageFill + token change + ChartSession S-101+S-104 + FC-only / no forge PC + soft pixel WARN-only; fixture MUST NOT SKIP).
- Eng `2ee32c7` already on tip — docs close only; no `ci.yml` change.
- Failure face stays bare CHECK tip-truth — do **not** invent `S104-*`.
- **Notes:** next **TBD** (architecture / registered Active — do **not** invent a next tip id; do **not** auto-open S-111). **Backlog:** `grid()` shim retire (`S102-GRID-SHIM-RETIRE`) — do not insert ahead. Contract display stays **rev 2**.
- Boundaries: independent; ≠ TIMELINE-UI; ≠ S-111; do **not** reopen QBC-MG / UNCERT / DEEPEN.
- **Non-goals this close:** inventing oracles / next tip id; forging PC; retiring `grid()` shim; loader/portray rewrite; reopening QBC-MG / UNCERT / DEEPEN; Win (a)/(b) (N/A — non-`viewer_*`); mixing TIMELINE-UI / S-111; inventing `S104-*` failure-face prefixes.

---

## Goal

HDF5 open → ≥2 time frames → sample/portray **one active frame** → overlay S-101 in `ChartSession`. Interactive scrubber: `docs/timeline-ui-contract.md` (separate Active).

## Frozen data model

```cpp
struct WaterLevelFrame {
  std::string time_iso;   // UTC
  CoverageGrid grid;      // one WL plane (reuse CoverageGrid layout/values-as-level)
};

struct WaterLevelDataset {
  ProductId product{ProductId::S104};
  std::filesystem::path path;
  std::vector<WaterLevelFrame> frames;  // size >= 1 after successful open
  std::size_t active_frame{0};
  // host/coverage features for portrayal if needed
  std::vector<CoverageFeature> features;

  Bounds bounds() const;  // geographic union of frames (usually shared envelope)
  bool set_active_frame(std::size_t index);           // false if out of range
  bool set_active_time(std::string_view iso_utc);     // nearest frame; always picks one if frames non-empty
  bool sample_wgs84(double lon, double lat, CoverageSample& out) const;  // active frame only
};

// Dataset integration (locked):
// Dataset::water_level  optional WaterLevelDataset  when product==S104
// Do NOT reuse Dataset::coverage for S-104.
```

`CoverageSample::depth` carries water level (metres) for this product.

## Frame API ownership

| API | Lives on | Notes |
|-----|----------|-------|
| `set_active_frame` / `set_active_time` / `sample_wgs84` / `bounds` | **`WaterLevelDataset`** | Core |
| Session overlay | `ChartSession` member `S-104:<stem>` holds `Dataset` with `water_level` filled | |
| CLI | `s100_viewer … --s104 <path.h5> [--s104-frame <i>\|--s104-time <iso>]` | Thin wrappers calling the core setters |

## Open path

- `open_dataset(path)` detects S-104 HDF5 and fills `Dataset::water_level` (`product=S104`).
- Wrong product / unreadable → `LoadError` (not silent Parse-as-unknown if file claims S-104).
- HDF5 link policy = S-102 (required when claiming S-104).

## Fixture (CI MUST)

- **`testdata/s104/synthetic_waterlevel_4326.h5`** gittracked (`.gitignore` `!` already set).
- ≥2 frames, EPSG:4326, known levels at a documented lon/lat (different per frame).
- MUST tests run on this file in CI — **no SKIP** of the primary open/sample/frame assertions.
- Real CHS/NOAA cells optional extras only.

## Portray MUST / WARN

| Check | Severity |
|-------|----------|
| Open synthetic; `frames.size()>=2`; `set_active_frame` / `set_active_time` behave | **MUST** |
| `sample_wgs84` at fixture point differs frame0 vs frame1 | **MUST** |
| Portray emits ≥1 water-level / Coverage-like instruction for active frame | **MUST** |
| ChartSession S-101 + S-104 loads | **MUST** |
| Instruction / band **token** change when switching frames (if tokens encode level bands) | **MUST** when using tokenized surface |
| Soft pixel diff frame0 vs frame1 snapshot | **WARN-only** |
| Official S-104 **FC** on disk + provenance | **MUST** via `docs/catalogue-complete-contract.md` (Ed.2.0.0 has **no** official PC — N/A, do not forge). Portray stays minimal tokens |

Day→Dusk: tokens stable, RGB remap only (same as S-102).

## Non-goals

Auto-play animation loop, S-111, S-98, uncertainty layer UI. Frame **scrubber UI** → `docs/timeline-ui-contract.md` (`IHO-TIMELINE-UI`).
