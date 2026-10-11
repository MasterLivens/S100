# SCAMIN / scale filter (rev 1)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-SCAMIN-FILTER`** (**Active**). Also closes backlog row **`IHO-S101-SCAMIN`**.
CTest: **`scamin_filter`** — **MUST NOT SKIP**.

Frozen with: `docs/api-contracts.md`, `docs/portrayal-contract.md` (visibility), `docs/testing/iho-ctest-case-ids.md`.

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
