# SCAMIN / scale filter (rev 1) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-SCAMIN-FILTER`** (**Active / Open**). Also closes backlog row **`IHO-S101-SCAMIN`** per this contract (do **not** invent new ids).
CTest: **`scamin_filter`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev 1** — Planner freeze (MUST below). SCAMIN / scale filter / `visible_at_scale` slice after closed **`IHO-SESSION-PICK-UNIFIED`**. Does **not** reopen SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; does **not** jump **`S102-GRID-SHIM-RETIRE`**.

Frozen with: `docs/api-contracts.md`, `docs/portrayal-contract.md` (visibility), `docs/testing/iho-ctest-case-ids.md`. Unified pick → **SESSION-PICK** already Closed — **do not** reopen. Vendor / sha → **CATALOGUE** already Closed — **do not** reopen. Scrubber → **TIMELINE** already Closed — **do not** reopen. Wiring → **PC-WIRE** already Closed — **do not** reopen. Soft pixel / official PC Lua / S-98 dual fuel / ISO 8211 SCAMIN writer / animation = Non-goals / WARN.

Primary research: `/workspace/research/scamin-filter/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`3541836`** (docs prep **`6e2adac`** / **`b443a13`**).

**Status: Active / Open (2026-10-25)** — after **SESSION-PICK Closed** (tip `2041980` ≡ Arch `371fedf`; tip Close CI [`40172798123`](https://github.com/MasterLivens/S100/actions/runs/40172798123) dual-green; Open-bind evidence CI [`40172265305`](https://github.com/MasterLivens/S100/actions/runs/40172265305) @ `f8aac10`≡`8dbe072`, O1=tip-truth/docs-close). Eng tip ancestor **`3541836`** already on tip — **not greenfield** (`visible_at_scale` / `filter_instructions_by_scale` + viewer/CLI `--view-scale`/`--ignore-scamin` + CTest `scamin_filter` oracles A–H + batch + DEF `ScaleMinimum` parse; **no** Win a/b chain). Ubuntu+Windows already Passed — tip Close CI [`40172798123`](https://github.com/MasterLivens/S100/actions/runs/40172798123) `#11 scamin_filter` + Eng landing [`38131881865`](https://github.com/MasterLivens/S100/actions/runs/38131881865) @ `3541836` (native `s100_tests` + synthetic DI — **not** `viewer_*` — **no Win O1 (a)/(b)**). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Do **not** invent case ids or next tip names; freeze tip-truth below. Do **not** reopen SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Do **not** invent `SCAMIN-*` failure-face prefixes (keep existing bare CHECKs). Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). Contract display header stays **rev 1** (do **not** bump just for Open). After Closed next: **TBD** (architecture / registered Active only — do **not** invent).

**Planner freeze (MUST — from BRIEF §7 + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-SCAMIN-FILTER`** / CTest **`scamin_filter`**; **forbidden** to invent a second case/test name; Close also closes backlog **`IHO-S101-SCAMIN`** (do **not** invent new ids).
- MUST tip-truth = `visible_at_scale(di, view_scale_denom, ignore_scale_minimum)` + `filter_instructions_by_scale` (stable relative order); rules (1) `view_scale_denom <= 0` → false; (2) `!ignore && scale_minimum > 0 && view > scale_minimum` → hide; (3) `scale_maximum > 0 && view < scale_maximum` → hide; (4) else visible; `ignore_scale_minimum` skips SCAMIN only (SCAMAX still applies); oracles **A–H**; batch [A,B,C]@50000/false → [A]; @50000/ignore → [A,B,C]; DEF `ScaleMinimum:22000` → field == 22000; CLI/viewer `--view-scale` / `--ignore-scamin`; fixtures **MUST NOT SKIP** (synthetic DI); official PC Lua / soft pixel / S-98 dual fuel / ISO 8211 writer / animation = Non-goals / WARN.
- Eng tip already has **`3541836`** (docs prep **`6e2adac`** / **`b443a13`**); Ubuntu+Win (tip `40172798123` `#11` / Eng `38131881865`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield.
- Unified pick → **`IHO-SESSION-PICK-UNIFIED`** already Closed — **do not** reopen.
- Vendor / sha → **`IHO-CATALOGUE-COMPLETE`** already Closed — **do not** reopen.
- Scrubber → **`IHO-TIMELINE-UI`** already Closed — **do not** reopen.
- Wiring → **`IHO-PC-WIRE-111-124`** already Closed — **do not** reopen.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog** (DEEPEN O1); **not** this tip.
- Failure face = existing bare CHECKs; do **not** invent `SCAMIN-*` prefixes.
- Ubuntu + Windows: `scamin_filter` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`; native `s100_tests` + synthetic DI; no SKIP. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `scamin_filter` MUST NOT SKIP, Passed (already tip CI `40172798123` `#11` / Eng `38131881865`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng diff required
- Also closes backlog row **`IHO-S101-SCAMIN`**
- After Close next: **TBD**

**Eng notes:** verify `3541836` on tip (docs prep `6e2adac`/`b443a13`); API `include/s100/scale_filter.hpp` + `src/core/scale_filter.cpp`; driver `tests/test_scamin_filter.cpp`; cmake `scamin_filter`; synthetic DI (no cell); DEF string parse. SESSION-PICK Closed already on this stack (`2041980`). Do **not** reopen SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Official PC Lua / soft pixel / S-98 dual fuel / ISO 8211 writer / animation **Non-goals / WARN** — do **not** harden. Do **not** invent `SCAMIN-*` failure prefixes. Do **not** change tip-truth rule order / oracles A–H.

**O-questions (BRIEF §3 Open 1–N — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `3541836` already; no `ci.yml` / no large Eng) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` — **yes / freeze**.
3. Contract head → Open; keep display **rev 1** (do **not** bump just for Open) — **yes / freeze**.
4. case-ids Active → Open (Status stays Active family) — **yes / freeze**.
5. Notes ≠ SESSION-PICK-UNIFIED (already Closed; do **not** reopen); ≠ CATALOGUE-COMPLETE (already Closed; do **not** reopen); ≠ TIMELINE-UI (already Closed; do **not** reopen); ≠ PC-WIRE (already Closed; do **not** reopen); ≠ S-124 / S-111 / S-104 open; do **not** reopen QBC-MG / UNCERT / DEEPEN; shim-retire stays Backlog; official Lua / soft pixel / S-98 stay Non-goals / WARN — **yes / freeze**.
6. Closed next → **TBD** (architecture / registered Active; do **not** invent) — **yes / freeze**.
7. No other cases this tip (do **not** Closed DISPLAY-PRIORITY / SIG / POWERUP / etc.) — **yes / freeze**.
8. architecture.md flip **next** to this Open — **yes / freeze**.
9. `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog — **yes / freeze**.
10. Failure face = existing bare CHECKs; do **not** invent `SCAMIN-*` prefixes — **yes / freeze**.
11. scale filter/CLI = this tip; unified pick = SESSION-PICK Closed; vendor/sha = CATALOGUE Closed; scrubber = TIMELINE Closed; wiring = PC-WIRE Closed — **yes / freeze**.

---

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
