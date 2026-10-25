# Timeline UI — S-104 / S-111 frame scrubber (rev 1) (Closed)

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/api-contracts.md` (Timeline UI) / `docs/s104-contract.md` / `docs/s111-contract.md` (both Closed; TIMELINE-UI separate) / `docs/pc-wire-contract.md` (Closed; scrubber → TIMELINE separate). Case id: **`IHO-TIMELINE-UI`** (**Closed / Done** 2026-10-25).  
CTest name: **`timeline_ui`** — MUST NOT SKIP; Linux + Windows dual-green (CI `40167445485` @ `c29b8ba`). Supersedes Open draft on tip `c29b8ba` (≡ Arch `feab8a0`).

Primary research: `/workspace/research/timeline-ui/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`20ca2ea`** (+Win **`0c54647`** / **`e6f7483`** / **`f7c3709`**). Vendor / sha / pair / SOURCE.md → separate Active **`IHO-CATALOGUE-COMPLETE`** — **do not** close CATALOGUE this tip. Wiring → **PC-WIRE** already Closed — **do not** reopen. **S-124 does not enter timeline**. Auto-play / dual-clock / soft pixel / widget → **WARN** (do **not** harden).

**Status: Closed / Done (2026-10-25)** — CI [`40167445485`](https://github.com/MasterLivens/S100/actions/runs/40167445485) @ `c29b8ba`≡`feab8a0` dual-green; **`timeline_ui` Passed Ubuntu+Win** (native `s100_tests` + CLI spawn + HDF5 — **not** `viewer_*`; **no Win O1 (a)/(b)**). Eng **`20ca2ea`** (+Win **`0c54647`** / **`e6f7483`** / **`f7c3709`**) already on tip — **docs close only**. MUST tip-truth **unchanged** (`timeline_entries_*` + `timeline_step_frame` + CLI `--list-frames` oracles 1–8 + viewer independent scrubber; CTest no screenshot; widget → WARN; fixture MUST NOT SKIP; auto-play / dual-clock / soft pixel → **WARN**; **S-124 does not enter timeline**). **O1=tip-truth / docs-close SUCCESS**. Failure face = bare CHECK tip-truth (do **not** invent `TIMELINE-*`). Do **not** reopen PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** mix CATALOGUE-COMPLETE; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog. Contract display stays **rev 1**. Next: **TBD**.

**Planner freeze (kept — rev 1 tip-truth):** Reuse case id **`IHO-TIMELINE-UI`** / CTest **`timeline_ui`**; MUST tip-truth = `timeline_entries_*` + `timeline_step_frame` + CLI `--list-frames` oracles 1–8 + viewer independent scrubber (CTest no screenshot; widget → WARN); fixture MUST NOT SKIP; auto-play / dual-clock / soft pixel → **WARN**; **S-124 does not enter timeline**; vendor/sha → **CATALOGUE-COMPLETE** separate; wiring → **PC-WIRE** already Closed (do **not** reopen); do **not** invent a second frame model; reuse `set_active_frame` / `set_active_time`; Eng `20ca2ea` (+Win `0c54647`/`e6f7483`/`f7c3709`) already; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog; failure face = bare CHECKs (do **not** invent `TIMELINE-*`); after Closed next **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** mix CATALOGUE-COMPLETE.

**O1 closed as tip-truth / docs-close:** Eng `20ca2ea` (+Win `0c54647`/`e6f7483`/`f7c3709`) already on tip (`timeline_entries_water_level` / `timeline_entries_current` + `timeline_step_frame` + CLI `--list-frames` oracles 1–8 + viewer independent scrubber; fixtures `synthetic_waterlevel_4326.h5` + `synthetic_current_4326.h5`; CTest **`timeline_ui`**). Slice = Arch docs close only — not greenfield Eng. Do **not** invent case ids or next tip names; do **not** reopen PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** invent `TIMELINE-*` prefixes; do **not** retire `grid()` this tip; do **not** close CATALOGUE; do **not** harden auto-play / dual-clock / soft pixel / widget; **S-124 does not enter timeline**. **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest; inventing oracles / next tip id; mixing CATALOGUE; inventing a second frame model.

---

## Slice close — 2026-10-25

**Status: Closed / Done** (formal docs close — **not** a timeline / scrubber rewrite).

- Evidence: CI [40167445485](https://github.com/MasterLivens/S100/actions/runs/40167445485) tip `c29b8ba`≡`feab8a0` — Ubuntu + Windows **`timeline_ui` Passed** (MUST NOT SKIP). Ancestor greens also Eng landing [`39047409533`](https://github.com/MasterLivens/S100/actions/runs/39047409533) @ `f7c3709` + prior tip `#1 timeline_ui` Passed.
- **O1=tip-truth / docs-close SUCCESS** — Eng `20ca2ea`+Win fixes already on tip; Arch Open→Closed only; Planner BRIEF §7 answers frozen.
- MUST tip-truth unchanged (`timeline_entries_*` + `timeline_step_frame` + CLI `--list-frames` oracles 1–8 + viewer independent scrubber; CTest no screenshot; widget → WARN; fixture MUST NOT SKIP; auto-play / dual-clock / soft pixel → **WARN**; **S-124 does not enter timeline**).
- Eng `20ca2ea`+`0c54647`/`e6f7483`/`f7c3709` already on tip — docs close only; no `ci.yml` change; **no Win (a)/(b)**.
- Failure face stays bare CHECK tip-truth — do **not** invent `TIMELINE-*`.
- **Notes:** next **TBD** (architecture / registered Active — do **not** invent a next tip id). **Backlog:** `grid()` shim retire (`S102-GRID-SHIM-RETIRE`) — do not insert ahead. Contract display stays **rev 1**. Auto-play / dual-clock / soft pixel / widget stay WARN; vendor/sha → CATALOGUE-COMPLETE stays separate Active (do **not** close); wiring → PC-WIRE already Closed (do **not** reopen); **S-124 does not enter timeline**.
- Boundaries: independent; ≠ PC-WIRE (already Closed); ≠ CATALOGUE-COMPLETE; do **not** reopen PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.
- **Non-goals this close:** inventing oracles / next tip id; closing CATALOGUE; retiring `grid()` shim; timeline/scrubber rewrite; reopening PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; Win (a)/(b) (N/A — non-`viewer_*`); inventing `TIMELINE-*` failure-face prefixes; hardening auto-play / dual-clock / soft pixel / widget; pulling S-124 into timeline; inventing a second frame model.

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

`docs/s104-contract.md` Non-goal “Animation scrubbing” — scrubber UI is now this Closed slice (auto-play still out).
