# Timeline UI — S-104 / S-111 frame scrubber (rev 1)

Owner: SoftwareArchitecture (2026-10-17). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-TIMELINE-UI`** (**Active**).
CTest: **`timeline_ui`** — **MUST NOT SKIP**.

Frozen with: `docs/api-contracts.md`, `docs/s104-contract.md`, `docs/s111-contract.md`, `docs/testing/iho-ctest-case-ids.md`.

## Goal

Interactive + CLI control of **active frame / time** for S-104 water level and S-111 surface current. Core setters already exist — this slice wires **UI + list/step surfaces**; does **not** invent a second frame model.

## Non-goals

Auto-play animation loop; S-102 multi-grid as a time axis; linking S-104 and S-111 clocks into one master timeline (WARN follow-up); S-124.

## Core (reuse — locked)

| Product | Type | Setters | Frame clock |
|---------|------|---------|-------------|
| S-104 | `WaterLevelDataset` | `set_active_frame` / `set_active_time` | `frames[i].time_iso` |
| S-111 | `CurrentDataset` | same | `frames[i].time_iso` |

No new setters. OOR / nearest-time semantics unchanged from product contracts.

### Timeline list helper (name locked)

```cpp
struct TimelineEntry {
  std::size_t index{0};
  std::string time_iso;  // may be empty if dataset omitted time
  bool active{false};
};

/** Fill entries from wl.frames / cur.frames; mark active_frame. Returns false if frames empty. */
bool timeline_entries_water_level(const WaterLevelDataset& wl, std::vector<TimelineEntry>& out);
bool timeline_entries_current(const CurrentDataset& cur, std::vector<TimelineEntry>& out);
```

Header e.g. `include/s100/timeline.hpp`. Pure; no Qt.

### Step helpers (optional but if present, names locked)

```cpp
bool timeline_step_frame(WaterLevelDataset& wl, int delta);  // +1/-1; clamp; false if empty
bool timeline_step_frame(CurrentDataset& cur, int delta);
```

Clamp: stay at 0 or `size-1`; return true if `active_frame` changed.

## CLI (MUST)

| Flag | Behaviour |
|------|-----------|
| `--list-frames` | After open, print timeline for loaded S-104 and/or S-111 (whichever present). Format **one line per frame**: `product\tindex\ttime_iso\tactive` where `active` is `1` or `0`. Exit 0. |
| Existing `--s104-frame` / `--s104-time` / `--s111-frame` / `--s111-time` | Unchanged; apply **before** list/portray dump |

`--list-frames` with neither S-104 nor S-111 open → stderr message + exit **non-zero**.

## Viewer UI (MUST)

When `Dataset::water_level` or `Dataset::current` is present:

1. Show a **timeline strip** (slider or equivalent) bound to that product's `active_frame` (0 … N-1).
2. Show **active `time_iso`** text (or `frame i/N` if time empty).
3. Changing the scrubber → `set_active_frame` → **re-portray active frame only** → repaint (SCAMIN + draw-order still apply).
4. S-104 and S-111 get **independent** controls if both loaded (two strips or a product selector + one strip). No forced clock sync this slice.
5. Keyboard: Left/Right (or `[` / `]`) step ±1 frame on the focused product — MUST document which; CTest may cover via core `timeline_step_frame` only.

Headless / no-display CI: UI widgets need not be screenshot-tested; **core + CLI** carry MUST oracles. Soft pixel frame0 vs frame1 remains WARN (as s104-contract).

## Oracle — CTest `timeline_ui` (no SKIP)

Fixtures (gittracked):

- `testdata/s104/synthetic_waterlevel_4326.h5` (≥2 frames)
- `testdata/s111/synthetic_current_4326.h5` (≥2 frames)

| # | Check | Sev |
|---|-------|-----|
| 1 | `timeline_entries_water_level`: size==frames.size(); exactly one `active`; indices 0..n-1 | MUST |
| 2 | After `set_active_frame(1)`, entries mark index 1 active | MUST |
| 3 | Same for `timeline_entries_current` | MUST |
| 4 | `timeline_step_frame(+1)` from 0 → 1 on both products (fixture n≥2); from last → unchanged / false | MUST |
| 5 | CLI `--s104 <fix> --list-frames`: ≥2 lines, product `S-104`, one `active=1` | MUST |
| 6 | CLI `--s111 <fix> --list-frames`: same for `S-111` | MUST |
| 7 | CLI `--list-frames` alone (no product path) → non-zero exit | MUST |
| 8 | After `--s104-frame 1 --list-frames`, active index is 1 | MUST |

Viewer widget presence: WARN (manual / soft) unless Eng adds an accessible dump; do not SKIP MUST rows 1–8 for missing GUI.

## MUST / WARN summary

| Check | Sev |
|-------|-----|
| Core timeline_entries + step + CLI list-frames oracles | MUST |
| CTest `timeline_ui` no SKIP | MUST |
| Viewer scrubber when S-104/111 present; independent products | MUST |
| Soft pixel / auto-play / dual-product clock sync | WARN |

## Supersedes

`docs/s104-contract.md` Non-goal “Animation scrubbing” — scrubber UI is now this Active slice (auto-play still out).
