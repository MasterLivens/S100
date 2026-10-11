# S-111 surface currents — first slice

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S111-OPEN-FRAME`**. CTest: **`s111_open_frame`**.

## Goal

HDF5 open → ≥2 time frames of surface current → sample/portray **one active frame** as **vector arrows** → overlay S-101 in `ChartSession`. No animation scrubbing.

## Frozen data model

```cpp
struct CurrentSample {
  float speed_ms{0};      // m/s
  float direction_deg{0}; // from-direction, meteorological, degrees clockwise from N [0,360)
  bool valid{false};
};

struct CurrentFrame {
  std::string time_iso;  // UTC
  // Regular grid of speed + direction (or u/v stored and converted at sample time)
  CoverageGrid speed;           // reuse grid layout; values = speed
  CoverageGrid direction_or_v;  // direction deg, or store u/v pair — see below
};

struct CurrentDataset {
  ProductId product{ProductId::S111};
  std::filesystem::path path;
  std::vector<CurrentFrame> frames;  // >= 1 after open
  std::size_t active_frame{0};

  Bounds bounds() const;
  bool set_active_frame(std::size_t index);
  bool set_active_time(std::string_view iso_utc);  // nearest
  bool sample_wgs84(double lon, double lat, CurrentSample& out) const;  // active frame
};

// Dataset::current  when product==S111 — not coverage, not water_level.
```

**Component storage (locked):** store **speed + direction** planes on each frame (direction in degrees). If HDF5 provides u/v, convert at load. Do not expose u/v at the façade this slice.

## Frame API ownership

Same pattern as S-104: setters/sample/bounds on **`CurrentDataset`**; CLI `--s111` / `--s111-frame` / `--s111-time` thin wrappers; session key **`S-111:<stem>`**.

## Open path

- `open_dataset` detects S-111 HDF5 → `Dataset::current`.
- Entry line: S-111 open path live after this slice; GML still Parse.
- HDF5 policy = S-102/S-104.

## Portray (arrows)

- Emit point/arrow instructions for a **subsampled** grid (fixed stride, e.g. every N cells, documented constant) at active frame.
- Arrow orientation from `direction_deg`; length/scale from `speed_ms` (clamp + legend constant in viewer).
- Colour via tokens if PC present; else neutral token (e.g. `CHGRD`) — no hard-coded RGB in core.
- Official S-111 FC/PC: vendor if practical this slice; else WARN and minimal arrows.

## Fixture (CI MUST)

- **`testdata/s111/synthetic_current_4326.h5`** gittracked (`!` exception + `SOURCE.md`).
- ≥2 frames, EPSG:4326; known sample point with different speed/dir per frame.
- Primary CTest MUST **not SKIP**.

## MUST / WARN

| Check | Severity |
|-------|----------|
| Open synthetic; `frames.size()>=2`; frame setters | **MUST** |
| `sample_wgs84` differs frame0 vs frame1 (speed and/or dir) | **MUST** |
| Portray emits ≥1 arrow instruction for active frame | **MUST** |
| ChartSession S-101 + S-111 | **MUST** |
| Soft pixel / arrow density aesthetics | **WARN** |
| Full S-111 PC/FC provenance | **WARN** if deferred |

## Non-goals

Particle animation, S-104 time sync UI, S-98 stacking, uncertainty ellipses.
