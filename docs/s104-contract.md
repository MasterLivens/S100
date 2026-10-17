# S-104 water level — first slice (rev 2)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S104-OPEN-FRAME`**. CTest: **`s104_open_frame`**.

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
