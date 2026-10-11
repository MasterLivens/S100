# S-104 water level — first slice

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Product: **S-104** (HDF5). Case id: **`IHO-S104-OPEN-FRAME`**. CTest: **`s104_open_frame`**.

## Goal

Read an S-104 HDF5 water-level dataset, pick **one time frame**, sample/portray it, and overlay on S-101 in the same `ChartSession`. Not a full time animation UI.

## Model

Reuse coverage-shaped types where practical; do **not** stuff water level into `ProductId::S102` grids.

```cpp
struct TimeFrame {
  std::string time_iso;          // UTC instant from dataset
  // values: same layout as one CoverageGrid depth plane, or dedicated WL grid
};

struct WaterLevelDataset {       // or CoverageDataset with product=S104 + frames
  ProductId product{ProductId::S104};
  std::filesystem::path path;
  Bounds geographic_bounds;      // WGS84
  std::vector<TimeFrame> frames; // size >= 1 after open
  std::size_t active_frame{0};   // default: first frame (or nearest to a requested time)
};
```

Pinned choices:

1. **`open_dataset(.h5)`** dispatches S-104 by product identity in the file (same façade as S-102). Mis-detect → `LoadError`.
2. **First slice = single active frame.** API: `set_active_frame(index)` / `set_active_time(iso)` (nearest). Portray/sample use **only** `active_frame`.
3. **Sample façade** `sample_wgs84(lon,lat)` → water level (metres) at active frame; CRS rules same as S-102 (WGS84 out; convert inside core).
4. **ChartSession** key `S-104:<stem>`; overlay with S-101 like S-102 (`--s104` CLI ok).
5. **Portrayal**: emit Coverage-like / water-level instructions via S-104 PC when present; if PC not yet vendored, emit a minimal tokenized surface (document tokens) and gate full PC to a follow-up — **prefer** vendoring official S-104 PC + FC with SOURCE/hash like S-102 if size allows this slice.
6. **HDF5** same link policy as S-102 (required when claiming S-104; CI reproducible).

## Fixture

- Prefer a small public CHS/NOAA S-104 cell if license-ok and downloadable; else **`testdata/s104/synthetic_waterlevel_4326.h5`** gittracked (`!` gitignore exception + `SOURCE.md`), ≥2 time frames, EPSG:4326, so tests can switch frames.
- NOAA/CHS large binaries stay gitignored.

## Tests (MUST)

1. Open synthetic (or fetched) file: `frames.size() >= 2`, bounds valid.
2. `sample_wgs84` at a known cell differs between frame 0 and frame 1.
3. Portray/snapshot with active frame 0 vs 1 produces different pixel/token evidence (soft ok).
4. ChartSession loads S-101 + S-104 together.
5. Existing S-102 tests untouched/green.

## Non-goals

Scrubbing animation, uncertainty UI, S-111 currents, S-98 stacking rules, Track B.
