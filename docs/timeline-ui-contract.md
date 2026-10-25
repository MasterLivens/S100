# Timeline UI — S-104 / S-111 frame scrubber (rev 1) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-TIMELINE-UI`** (**Active / Open**). CTest: **`timeline_ui`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev 1** — Planner freeze (MUST below). S-104/S-111 timeline scrubber / `--list-frames` slice after closed **`IHO-PC-WIRE-111-124`**. Does **not** reopen PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; does **not** mix **`IHO-CATALOGUE-COMPLETE`**; does **not** jump **`S102-GRID-SHIM-RETIRE`**.

Builds on `docs/api-contracts.md` (Timeline UI) / `docs/s104-contract.md` / `docs/s111-contract.md` (both Closed; TIMELINE-UI separate) / `docs/pc-wire-contract.md` (Closed; scrubber → TIMELINE separate). Vendor / sha / pair / SOURCE.md → separate Active **`IHO-CATALOGUE-COMPLETE`** — **do not** close CATALOGUE this tip. Wiring → **PC-WIRE** already Closed — **do not** reopen. **S-124 does not enter timeline**.

Primary research: `/workspace/research/timeline-ui/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`20ca2ea`** (+Win **`0c54647`** / **`e6f7483`** / **`f7c3709`**).

**Status: Active / Open (2026-10-25)** — after **PC-WIRE Closed** (tip `7e7eb91` ≡ Arch `888c2ee`; tip Close CI [`40166375705`](https://github.com/MasterLivens/S100/actions/runs/40166375705) dual-green; Open-bind evidence CI [`40165684207`](https://github.com/MasterLivens/S100/actions/runs/40165684207) @ `9e7eae0`≡`5b27957`, O1=tip-truth/docs-close). Eng tip ancestors **`20ca2ea`** (+Win **`0c54647`** / **`e6f7483`** / **`f7c3709`**) already on tip — **not greenfield** (`timeline_entries_water_level` / `timeline_entries_current` + `timeline_step_frame` + CLI `--list-frames` oracles 1–8 + viewer independent scrubber; CTest no screenshot; widget → WARN; fixture MUST NOT SKIP; auto-play / dual-clock / soft pixel → WARN; **S-124 does not enter timeline**; vendor/sha → **CATALOGUE-COMPLETE** separate; wiring → **PC-WIRE** already Closed). Ubuntu+Windows already Passed — tip Close CI [`40166375705`](https://github.com/MasterLivens/S100/actions/runs/40166375705) `#1 timeline_ui` + Eng landing [`39047409533`](https://github.com/MasterLivens/S100/actions/runs/39047409533) @ `f7c3709` (native `s100_tests` + CLI spawn + HDF5, **not** `viewer_*` — **no Win O1 (a)/(b)**). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / CATALOGUE-COMPLETE. Do **not** invent case ids or next tip names; freeze tip-truth below. Do **not** reopen PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Do **not** mix CATALOGUE. Do **not** invent `TIMELINE-*` failure-face prefixes (keep existing bare CHECKs). Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). Contract display header stays **rev 1** (do **not** bump just for Open). After Closed next: **TBD** (architecture / registered Active only — do **not** invent).

**Planner freeze (MUST — from BRIEF §7 + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-TIMELINE-UI`** / CTest **`timeline_ui`**; **forbidden** to invent a second case/test name.
- MUST tip-truth = `timeline_entries_*` + `timeline_step_frame` + CLI `--list-frames` oracles 1–8 + viewer independent scrubber (CTest no screenshot; widget → WARN); fixture **MUST NOT SKIP**; auto-play / dual-clock / soft pixel → **WARN**; **S-124 does not enter timeline**; vendor/sha → **CATALOGUE-COMPLETE** separate; wiring → **PC-WIRE** already Closed (do **not** reopen).
- Do **not** invent a second frame model; reuse `set_active_frame` / `set_active_time`; OOR / nearest-time unchanged.
- Fixture `testdata/s104/synthetic_waterlevel_4326.h5` + `testdata/s111/synthetic_current_4326.h5` **MUST NOT SKIP** on CI (Linux+Win).
- Vendor / sha / pair / SOURCE.md → **`IHO-CATALOGUE-COMPLETE`** (separate tip; **do not** close this tip).
- Wiring → **`IHO-PC-WIRE-111-124`** already Closed — **do not** reopen.
- Eng tip already has **`20ca2ea`** (+Win **`0c54647`** / **`e6f7483`** / **`f7c3709`**); Ubuntu+Win (tip `40166375705` `#1` / Eng `39047409533`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog** (DEEPEN O1); **not** this tip.
- Failure face = existing bare CHECKs; do **not** invent `TIMELINE-*` prefixes.
- Ubuntu + Windows: `timeline_ui` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** mix CATALOGUE-COMPLETE.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*` for CTest; scrubber is viewer but CTest does not screenshot. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `timeline_ui` MUST NOT SKIP, Passed (already tip CI `40166375705` `#1` / Eng `39047409533`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng diff required
- After Close next: **TBD**

**Eng notes:** verify `20ca2ea` (+`0c54647`/`e6f7483`/`f7c3709`) on tip; fixtures `testdata/s104/synthetic_waterlevel_4326.h5` + `testdata/s111/synthetic_current_4326.h5` + SOURCE; driver `tests/test_timeline_ui.cpp`; cmake `timeline_ui`. PC-WIRE Closed already on this stack (`7e7eb91`). Do **not** reopen PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Auto-play / dual-clock / soft pixel **WARN-only** — do **not** harden. Vendor/sha → CATALOGUE separate (do **not** close CATALOGUE this tip). **S-124 does not enter timeline**.

**O-questions (BRIEF §3 Open 1–N — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `20ca2ea`+Win fixes already; no `ci.yml` / no large Eng) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` (scrubber viewer but CTest no screenshot) — **yes / freeze**.
3. Contract head → Open; keep display **rev 1** (do **not** bump just for Open) — **yes / freeze**.
4. case-ids Active → Open (Status stays Active family) — **yes / freeze**.
5. Notes ≠ PC-WIRE (already Closed; do **not** reopen); ≠ CATALOGUE-COMPLETE; **S-124 does not enter timeline**; do **not** reopen S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; shim-retire stays Backlog; auto-play / dual-clock / soft pixel stay WARN — **yes / freeze**.
6. Closed next → **TBD** (architecture / registered Active; do **not** invent) — **yes / freeze**.
7. No other cases this tip (do **not** Closed CATALOGUE / SESSION-PICK / SIG / POWERUP / etc.) — **yes / freeze**.
8. architecture.md flip **next** to this Open — **yes / freeze**.
9. `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog — **yes / freeze**.
10. Failure face = existing bare CHECKs; do **not** invent `TIMELINE-*` prefixes — **yes / freeze**.
11. Scrubber/list-frames = this tip; wiring = PC-WIRE Closed; vendor/sha → CATALOGUE; S-124 not in timeline — **yes / freeze**.

---

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
