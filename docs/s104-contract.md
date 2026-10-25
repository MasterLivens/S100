# S-104 water level — first slice (rev 2) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S104-OPEN-FRAME`** (**Active / Open**). CTest: **`s104_open_frame`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev 2** — Planner freeze (MUST below). First S-104 open/frame slice after closed **`IHO-S102-QBC-MULTIGRID`**. Does **not** reopen QBC-MG / UNCERT / DEEPEN; does **not** mix **`IHO-TIMELINE-UI`** / **`IHO-S111-OPEN-FRAME`**; does **not** jump **`S102-GRID-SHIM-RETIRE`**.

Builds on `docs/api-contracts.md` (S-104) / `docs/catalogue-complete-contract.md` (S-104 FC only). Scrubber / `--list-frames` / `timeline_entries` → separate Active **`IHO-TIMELINE-UI`** (`docs/timeline-ui-contract.md`).

Primary research: `/workspace/research/s104-open-frame/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`2ee32c7`**.

**Status: Active / Open (2026-10-25)** — after **QBC-MG Closed** (tip `596e4ea`; tip CI [`40158058619`](https://github.com/MasterLivens/S100/actions/runs/40158058619) dual-green; formal Close CI bind [`40156013552`](https://github.com/MasterLivens/S100/actions/runs/40156013552) @ `30f3546`, O1=tip-truth/docs-close). Eng tip ancestor **`2ee32c7`** already on tip — **not greenfield** (HDF5 open → `Dataset::water_level`; ≥2 frames; frame API; sample 0.2/0.2 → 1.0 / 2.5; portray CoverageFill + token change; ChartSession S-101+S-104; fixture `synthetic_waterlevel_4326.h5`; CTest **`s104_open_frame`**). Ubuntu+Windows already Passed — tip CI [`40158058619`](https://github.com/MasterLivens/S100/actions/runs/40158058619) `#7` + Eng landing [`38126580110`](https://github.com/MasterLivens/S100/actions/runs/38126580110) @ `2ee32c7` (native `s100_tests` + HDF5, **not** `viewer_*` — **no Win O1 (a)/(b)**). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from QBC-MG / UNCERT / DEEPEN / TIMELINE-UI / S-111. Do **not** invent case ids or next tip names; freeze tip-truth below. Do **not** reopen QBC-MG / UNCERT / DEEPEN. Do **not** forge S-104 PC (FC-only; official PC N/A). Do **not** invent `S104-*` failure-face prefixes (keep existing bare CHECKs). Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). Contract display header stays **rev 2** (content already has PC N/A — do **not** bump to rev 3 just for Open). After Closed next: **TBD** (architecture / registered Active only — do **not** invent).

**Planner freeze (MUST — from BRIEF §7 + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-S104-OPEN-FRAME`** / CTest **`s104_open_frame`**; **forbidden** to invent a second case/test name.
- MUST = open≥2 frames + frame API (`set_active_frame` / `set_active_time` / `sample_wgs84` / `bounds`) + sample 0.2/0.2 frame0≈**1.0** / frame1≈**2.5** (diff>0.5) + portray ≥1 CoverageFill + **token changes across frames** + ChartSession S-101+S-104 + FC-only (**do not forge PC**) + soft pixel **WARN-only**.
- Data model: `Dataset::water_level` / `WaterLevelDataset`; **forbid** reuse `Dataset::coverage` for S-104.
- Fixture `testdata/s104/synthetic_waterlevel_4326.h5` **MUST NOT SKIP** on CI (Linux+Win).
- Scrubber / `--list-frames` / `timeline_entries` → **`IHO-TIMELINE-UI`** (separate tip; **not** this case).
- Eng tip already has **`2ee32c7`**; Ubuntu+Win (tip `40158058619` `#7` / Eng `38126580110`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog** (DEEPEN O1); **not** this tip.
- Failure face = existing bare CHECKs; do **not** invent `S104-*` prefixes.
- Ubuntu + Windows: `s104_open_frame` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen QBC-MG / UNCERT / DEEPEN; do **not** mix TIMELINE-UI / S-111.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `s104_open_frame` MUST NOT SKIP, Passed (already tip CI `40158058619` `#7` / Eng `38126580110`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng diff required
- After Close next: **TBD**

**Eng notes:** verify `2ee32c7` on tip; fixture `testdata/s104/synthetic_waterlevel_4326.h5` + `SOURCE.md`; driver `tests/test_s104.cpp` `test_s104_open_frame`; cmake `s104_open_frame`. QBC-MG Closed already on this stack (`596e4ea`). Do **not** reopen QBC-MG / UNCERT / DEEPEN. Soft pixel frame0 vs frame1 **WARN-only** — do **not** harden. FC-only via catalogue-complete; **do not forge PC**.

**O-questions (BRIEF §7 Open 1–N — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `2ee32c7` already; no `ci.yml` / no large Eng) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` — **yes / freeze**.
3. Contract head → Open; keep display **rev 2** (content already PC N/A; do **not** bump to rev 3 just for Open) — **yes / freeze**.
4. case-ids Active → Open (Status stays Active family) — **yes / freeze**.
5. Notes ≠ TIMELINE-UI; ≠ S-111; do **not** reopen QBC-MG / UNCERT / DEEPEN; shim-retire stays Backlog — **yes / freeze**.
6. Closed next → **TBD** (architecture / registered Active; do **not** invent) — **yes / freeze**.
7. No other cases this tip (do **not** Closed TIMELINE / S-111 / SIG / POWERUP / etc.) — **yes / freeze**.
8. architecture.md flip **next** to this Open — **yes / freeze**.
9. `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog — **yes / freeze**.
10. Failure face = existing bare CHECKs; do **not** invent `S104-*` prefixes — **yes / freeze**.

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
