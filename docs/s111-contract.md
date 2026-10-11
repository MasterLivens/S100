# S-111 surface currents — first slice (rev 2)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S111-OPEN-FRAME`**. CTest: **`s111_open_frame`**.

## Goal

HDF5 open → ≥2 frames → sample/portray **one active frame** as arrows → `ChartSession` overlay with S-101.

## Frozen sample & storage

```cpp
struct CurrentSample {
  float speed_ms{0};       // m/s, >= 0
  float direction_deg{0};  // from-direction, deg clockwise from true N, in [0, 360)
  bool valid{false};
};

struct CurrentFrame {
  std::string time_iso;    // UTC
  CoverageGrid speed;      // values = speed_ms
  CoverageGrid direction;  // values = direction_deg (same shape as speed)
};

struct CurrentDataset {
  ProductId product{ProductId::S111};
  std::filesystem::path path;
  std::vector<CurrentFrame> frames;
  std::size_t active_frame{0};

  Bounds bounds() const;
  bool set_active_frame(std::size_t index);          // false if index >= frames.size(); no change
  bool set_active_time(std::string_view iso_utc);    // nearest |Δt|; ties → lower index; empty frames → false
  bool sample_wgs84(double lon, double lat, CurrentSample& out) const;  // active frame only
};
```

- Façade is **speed + `direction_deg` only**. No `direction_or_v`, no u/v at API.
- If HDF5 stores u/v, convert to speed/direction **at load** into the two planes.
- `Dataset::current` only. Wrong product must **not** fill `current` → `LoadError` (same as S-104 water_level).

## Arrow DrawingInstruction (locked)

Add `DrawingInstruction::Kind::Arrow`.

| Field | Use |
|-------|-----|
| `kind` | `Arrow` |
| `geometries` | single Point anchor (WGS84) |
| `rotation` | = `direction_deg` (PortrayalCRS / GeographicCRS as documented in emit) |
| `scale_factor` | proportional to `speed_ms` (clamp in viewer) |
| `symbol_ref` | **PC id** `SCAROW01`…`SCAROW09` (band table in `pc-wire-contract`); interim `"Arrow"` superseded when PC wired |
| `fill_ref` | colour token (default `CHGRD` if no PC) |

Do not overload `Kind::Point` for currents.

## Subsample

Constant: **`kS111ArrowStride = 2`** (emit arrow every 2×2 cells on the active frame grid; edges inclusive). Named in code exactly `kS111ArrowStride`.

## Frame API

On `CurrentDataset` only; CLI `--s111` / `--s111-frame` / `--s111-time` wrappers. Session key `S-111:<stem>`.

## Fixture oracle (`testdata/s111/SOURCE.md`)

File: **`synthetic_current_4326.h5`** (gittracked). Layout pinned there:

- 4326, envelope ~0–0.4, 5×5, spacing 0.1
- Frame0 `2024-01-01T00:00:00Z`: speed 0.5 m/s, direction 90°
- Frame1 `2024-01-01T06:00:00Z`: speed 1.5 m/s, direction 180°
- Sample **lon=0.2, lat=0.2** must match those values

MUST tests use this file — **no SKIP**.

## MUST / WARN

| Check | Sev |
|-------|-----|
| Open; frames≥2; OOR `set_active_frame` false; `set_active_time` nearest | MUST |
| sample at (0.2,0.2) matches SOURCE oracle per frame | MUST |
| Wrong product → LoadError, `current` empty | MUST |
| Portray active frame: Arrow count follows stride; ≥1 Arrow | MUST |
| ChartSession S-101+S-111 | MUST |
| Soft pixel | WARN |
| Official S-111 FC/PC on disk + provenance | **MUST** via `docs/catalogue-complete-contract.md` |
| Portray→PC wiring (SCAROW + ColorProfiles) | **MUST** via `docs/pc-wire-contract.md` (XSLT engine WARN; no invented Lua) |

## Non-goals

Animation, S-104 sync UI, S-98, uncertainty ellipses.
