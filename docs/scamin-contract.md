# SCAMIN / scale filter (rev 1) (Closed)

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/api-contracts.md` / `docs/portrayal-contract.md` (visibility) / `docs/testing/iho-ctest-case-ids.md` / `docs/session-pick-contract.md` (Closed; unified pick → SESSION-PICK separate) / `docs/catalogue-complete-contract.md` (Closed; vendor/sha → CATALOGUE separate) / `docs/timeline-ui-contract.md` (Closed; scrubber → TIMELINE separate) / `docs/pc-wire-contract.md` (Closed; wiring → PC-WIRE separate). Case id: **`IHO-SCAMIN-FILTER`** (**Closed / Done** 2026-10-25). Also closes backlog row **`IHO-S101-SCAMIN`** (do **not** invent new ids).  
CTest name: **`scamin_filter`** — MUST NOT SKIP; Linux + Windows dual-green (CI `40173570061` @ `8d6fdc2`). Supersedes Open draft on tip `8d6fdc2` (≡ Arch `9a14fb5`).

Primary research: `/workspace/research/scamin-filter/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`3541836`** (docs prep **`6e2adac`** / **`b443a13`**). Unified pick → **SESSION-PICK** already Closed — **do not** reopen. Vendor / sha → **CATALOGUE** already Closed — **do not** reopen. Scrubber → **TIMELINE** already Closed — **do not** reopen. Wiring → **PC-WIRE** already Closed — **do not** reopen. Soft pixel / official PC Lua / S-98 dual fuel / ISO 8211 SCAMIN writer / animation = Non-goals / WARN.

**Status: Closed / Done (2026-10-25)** — CI [`40173570061`](https://github.com/MasterLivens/S100/actions/runs/40173570061) @ `8d6fdc2`≡`9a14fb5` dual-green; **`scamin_filter` Passed Ubuntu+Win** (native `s100_tests` + synthetic DI — **not** `viewer_*`; **no Win O1 (a)/(b)**). Eng **`3541836`** already on tip — **docs close only**. MUST tip-truth **unchanged** (`visible_at_scale` / `filter_instructions_by_scale` + rules 1–4 + `ignore_scale_minimum` skips SCAMIN only + oracles **A–H** + batch [A,B,C]@50k + DEF `ScaleMinimum:22000` + CLI/viewer `--view-scale`/`--ignore-scamin`; fixtures **MUST NOT SKIP**). **O1=tip-truth / docs-close SUCCESS**. Failure face = existing bare CHECKs tip-truth (do **not** invent `SCAMIN-*`). Do **not** reopen SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog. Contract display stays **rev 1**. Next: **TBD**.

**Planner freeze (kept — rev 1 tip-truth):** Reuse case id **`IHO-SCAMIN-FILTER`** / CTest **`scamin_filter`**; Close also closes backlog **`IHO-S101-SCAMIN`** (do **not** invent new ids); MUST tip-truth = `visible_at_scale(di, view_scale_denom, ignore_scale_minimum)` + `filter_instructions_by_scale` (stable relative order); rules (1) `view_scale_denom <= 0` → false; (2) `!ignore && scale_minimum > 0 && view > scale_minimum` → hide; (3) `scale_maximum > 0 && view < scale_maximum` → hide; (4) else visible; `ignore_scale_minimum` skips SCAMIN only (SCAMAX still applies); oracles **A–H**; batch [A,B,C]@50000/false → [A]; @50000/ignore → [A,B,C]; DEF `ScaleMinimum:22000` → field == 22000; CLI/viewer `--view-scale` / `--ignore-scamin`; fixtures **MUST NOT SKIP** (synthetic DI); official PC Lua / soft pixel / S-98 dual fuel / ISO 8211 writer / animation = Non-goals / WARN; unified pick → **SESSION-PICK** already Closed (do **not** reopen); vendor/sha → **CATALOGUE** already Closed (do **not** reopen); scrubber → **TIMELINE** already Closed (do **not** reopen); wiring → **PC-WIRE** already Closed (do **not** reopen); Eng `3541836` already (docs prep `6e2adac`/`b443a13`); `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog; failure face = bare CHECK (do **not** invent `SCAMIN-*` prefixes / new CTest); after Closed next **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.

**O1 closed as tip-truth / docs-close:** Eng `3541836` already on tip (`visible_at_scale` / `filter_instructions_by_scale` + viewer/CLI `--view-scale`/`--ignore-scamin` + CTest `scamin_filter` oracles A–H + batch + DEF ScaleMinimum parse; docs prep `6e2adac`/`b443a13`; CTest **`scamin_filter`**). Slice = Arch docs close only — not greenfield Eng. Do **not** invent case ids or next tip names; do **not** reopen SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** invent `SCAMIN-*` prefixes; do **not** retire `grid()` this tip; do **not** harden official PC Lua / soft pixel / S-98 dual fuel / ISO 8211 writer / animation. **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest; inventing oracles / next tip id; inventing `SCAMIN-*` failure-face prefixes.

---

## Slice close — 2026-10-25

**Status: Closed / Done** (formal docs close — **not** a scale-filter / Eng rewrite).

- Evidence: CI [40173570061](https://github.com/MasterLivens/S100/actions/runs/40173570061) tip `8d6fdc2`≡`9a14fb5` — Ubuntu + Windows **`scamin_filter` Passed** (MUST NOT SKIP). Ancestor greens also Eng landing [`38131881865`](https://github.com/MasterLivens/S100/actions/runs/38131881865) @ `3541836` + prior tip `#11 scamin_filter` Passed (SESSION-PICK Close CI [`40172798123`](https://github.com/MasterLivens/S100/actions/runs/40172798123) @ `2041980`).
- **O1=tip-truth / docs-close SUCCESS** — Eng `3541836` already on tip; Arch Open→Closed only; Planner BRIEF §7 answers frozen.
- MUST tip-truth unchanged (`visible_at_scale` / `filter_instructions_by_scale` + oracles A–H + batch + DEF ScaleMinimum + CLI/viewer `--view-scale`/`--ignore-scamin` + fixtures **MUST NOT SKIP**).
- Eng `3541836` already on tip — docs close only; no `ci.yml` change; **no Win (a)/(b)**; no large Eng.
- Failure face stays bare CHECK tip-truth — do **not** invent `SCAMIN-*`.
- **Notes:** next **TBD** (architecture / registered Active — do **not** invent a next tip id). **Backlog:** `grid()` shim retire (`S102-GRID-SHIM-RETIRE`) — do not insert ahead. Contract display stays **rev 1**. Also closes backlog **`IHO-S101-SCAMIN`**. Unified pick → SESSION-PICK already Closed (do **not** reopen); vendor/sha → CATALOGUE already Closed (do **not** reopen); scrubber → TIMELINE already Closed (do **not** reopen); wiring → PC-WIRE already Closed (do **not** reopen).
- Boundaries: independent; ≠ SESSION-PICK (already Closed); ≠ CATALOGUE (already Closed); ≠ TIMELINE (already Closed); ≠ PC-WIRE (already Closed); do **not** reopen SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.
- **Non-goals this close:** inventing oracles / next tip id; retiring `grid()` shim; scale-filter/Eng rewrite; reopening SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; Win (a)/(b) (N/A — non-`viewer_*`); inventing `SCAMIN-*` failure-face prefixes; hardening official PC Lua / soft pixel / S-98 dual fuel / ISO 8211 writer / animation.

## Goal

Show/hide DrawingInstructions by **view scale denominator** using DI `scale_minimum` / `scale_maximum` (from DEF `ScaleMinimum` / `ScaleMaximum`). Toggle filter **on/off** for tests and ops.

## Definitions (locked)

| Term | Meaning |
|------|---------|
| `view_scale_denom` | Integer **N** for display scale **1:N**. Larger N = more zoomed out. |
| `scale_minimum` (SCAMIN) | DI field; **0 = none**. Smallest intended viewing scale (denominator). |
| `scale_maximum` (SCAMAX) | DI field; **0 = none**. Largest intended viewing scale (denominator). |
| `ignore_scale_minimum` | Bool; maps PC context **`IgnoreScaleMinimum`** (`true`/`false`). When true, **skip** SCAMIN (`scale_minimum`) checks only; SCAMAX still applies. |

## Visibility predicate (name locked)

```cpp
/** Returns false → caller must not paint this DI. */
bool visible_at_scale(const DrawingInstruction& di, long view_scale_denom,
                      bool ignore_scale_minimum);
```

Rules (evaluate in order):

1. If `view_scale_denom <= 0` → treat as **invalid**; `visible_at_scale` returns **false** (and CLI/viewer MUST reject ≤0 with error — do not paint).
2. If `!ignore_scale_minimum` && `di.scale_minimum > 0` && `view_scale_denom > di.scale_minimum` → **hidden** (zoomed out past SCAMIN).
3. If `di.scale_maximum > 0` && `view_scale_denom < di.scale_maximum` → **hidden** (zoomed in past SCAMAX).
4. Else → **visible**.

`scale_minimum == 0` and `scale_maximum == 0` → always visible (subject to other filters outside this contract).

## Batch helper (name locked)

```cpp
/** Erase or compact so only visible_at_scale==true remain; stable relative order. */
void filter_instructions_by_scale(DrawingInstructions& ios, long view_scale_denom,
                                  bool ignore_scale_minimum);
```

Pure function on DI list — no I/O. Portray engine may still emit ScaleMinimum into DI as today; **this slice does not re-run Lua**.

## Where applied (MUST)

| Surface | Hook |
|---------|------|
| Core | `visible_at_scale` / `filter_instructions_by_scale` in `s100` (header under `include/s100/`, e.g. `scale_filter.hpp`) |
| Viewer | Before paint: filter session DI by current `view_scale_denom` |
| CLI | Optional dump path filters when `--view-scale` set |

Default: filter **ON** (`ignore_scale_minimum == false`) when a positive view scale is known.

## View scale source (MUST)

| Surface | How `view_scale_denom` is set |
|---------|-------------------------------|
| Viewer | Computed from current view (metres-per-pixel → 1:N) **or** CLI override `--view-scale <N>` when launching headless/snapshot. Document formula in SOURCE comment once; tests may pin override only. |
| CLI | `--view-scale <N>` (required for scale-aware dump); omit → no scale filter in CLI dump |
| Context | `--ignore-scamin` sets `ignore_scale_minimum=true` (also honour PortrayalContext `IgnoreScaleMinimum` when portray session sets it) |

## Oracle — CTest `scamin_filter` (no SKIP, no cell required)

Build **synthetic** `DrawingInstructions` in the test (do not SKIP if sample cell lacks SCAMIN):

| id | `scale_minimum` | `scale_maximum` | `view_scale_denom` | `ignore_scale_minimum` | expect `visible_at_scale` |
|----|-----------------|-----------------|--------------------|------------------------|---------------------------|
| A | 0 | 0 | 12000 | false | **true** |
| B | 22000 | 0 | 12000 | false | **true** (`12k ≤ 22k`) |
| C | 22000 | 0 | 50000 | false | **false** (`50k > 22k`) |
| D | 22000 | 0 | 50000 | **true** | **true** (SCAMIN ignored) |
| E | 0 | 10000 | 5000 | false | **false** (`5k < 10k` SCAMAX) |
| F | 0 | 10000 | 12000 | false | **true** |
| G | 22000 | 10000 | 15000 | false | **true** (inside band) |
| H | any | any | **0** or **-1** | false | **false** |

Also MUST:

1. `filter_instructions_by_scale` on a list of {B,C,D} at `view=50000`, `ignore=false` → remaining ids = {B} only if B was included… use explicit: input [A,B,C] at 50000/false → output [A] only (B visible? B has scamin 22000, 50000>22000 → hide B; A always; C hide) → **[A]**.
2. Same list at 50000/`ignore=true` → **[A,B,C]** (C's SCAMIN ignored; none have SCAMAX).
3. Pin: DEF parse still fills `scale_minimum` — one instruction string containing `ScaleMinimum:22000` → field == 22000 (reuse parser; no SKIP).

## MUST / WARN

| Check | Sev |
|-------|-----|
| Predicate + filter API names above | MUST |
| Oracle rows A–H + filter batch pins | MUST |
| CTest `scamin_filter` no SKIP | MUST |
| Viewer applies filter when view scale known / `--view-scale` | MUST |
| `--ignore-scamin` / `IgnoreScaleMinimum` bypasses SCAMIN only | MUST |
| Soft pixel golden update if snapshot scale changes | WARN |
| Per-feature attribute → DI without Lua emit | WARN (Lua already emits ScaleMinimum today) |

## Non-goals

Changing official PC Lua; S-98 dual fuel; SCAMIN encoding into ISO 8211 writers; animation.

## Supersedes

Backlog defer on `IHO-S101-SCAMIN` — this contract is the Active implementation slice.
