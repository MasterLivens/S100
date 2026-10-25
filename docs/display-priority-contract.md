# Display priority / draw order (rev 2) (Closed)

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/api-contracts.md` / `docs/portrayal-contract.md` / `docs/testing/iho-ctest-case-ids.md` / `docs/scamin-contract.md` (Closed; scale filter → SCAMIN separate) / `docs/session-pick-contract.md` (Closed; unified pick → SESSION-PICK separate) / `docs/catalogue-complete-contract.md` (Closed; vendor/sha → CATALOGUE separate) / `docs/timeline-ui-contract.md` (Closed; scrubber → TIMELINE separate) / `docs/pc-wire-contract.md` (Closed; wiring → PC-WIRE separate). Case id: **`IHO-DISPLAY-PRIORITY`** (**Closed / Done** 2026-10-25). Also closes backlog row **`IHO-S101-DRAW-PRIORITY`** (do **not** invent new ids).  
CTest name: **`display_priority_order`** — MUST NOT SKIP; Linux + Windows dual-green (CI `40175087823` @ `dbfa1b6`). Supersedes Open draft on tip `dbfa1b6` (≡ Arch `e6afbf7`).

Primary research: `/workspace/research/display-priority/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`c7a7deb`** (docs prep **`de8082a`** / **`322cd96`**). Scale filter → **SCAMIN** already Closed — **do not** reopen. Unified pick → **SESSION-PICK** already Closed — **do not** reopen. Vendor / sha → **CATALOGUE** already Closed — **do not** reopen. Scrubber → **TIMELINE** already Closed — **do not** reopen. Wiring → **PC-WIRE** already Closed — **do not** reopen. Soft pixel / official PC Lua / S-98 / `--dump-draw-order` / z-fighting beyond plane+priority / per-layer UI reorder = Non-goals / WARN.

**Status: Closed / Done (2026-10-25)** — CI [`40175087823`](https://github.com/MasterLivens/S100/actions/runs/40175087823) @ `dbfa1b6`≡`e6afbf7` dual-green; **`display_priority_order` Passed Ubuntu+Win** (native `s100_tests` + synthetic DI — **not** `viewer_*`; **no Win O1 (a)/(b)**). Eng **`c7a7deb`** already on tip — **docs close only**. MUST tip-truth **unchanged** (`display_plane_rank` / `draw_order_less` / `sort_instructions_for_draw` + plane rank rules + nullptr oracle `P2,P5,P4,P1,P0,P3` + inverted `P0,P3,P4,P2,P5,P1` + DEF `DrawingPriority`/`DisplayPlane` + viewer core comparator; SCAMIN filter **before** sort; fixtures **MUST NOT SKIP**). **O1=tip-truth / docs-close SUCCESS**. Failure face = existing bare CHECKs tip-truth (do **not** invent `DISPLAYPRI-*`). Do **not** reopen SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog. Contract display stays **rev 2**. Next: **TBD**.

**Planner freeze (kept — rev 2 tip-truth):** Reuse case id **`IHO-DISPLAY-PRIORITY`** / CTest **`display_priority_order`**; Close also closes backlog **`IHO-S101-DRAW-PRIORITY`** (do **not** invent new ids); MUST tip-truth = `display_plane_rank(plane, pc_planes)` + `draw_order_less(a, b, pc_planes)` + `sort_instructions_for_draw(ios, pc_planes)`; plane rank (1) `pc_planes` hit → map value (2) else `OverRadar`→1 else 0; comparator rank→priority→stable; nullptr oracle ids `P2,P5,P4,P1,P0,P3`; inverted `{{UnderRadar,10},{OverRadar,0}}` → `P0,P3,P4,P2,P5,P1`; DEF `DrawingPriority:7;DisplayPlane:OverRadar`; viewer paint uses core comparator; SCAMIN filter **before** sort; fixtures **MUST NOT SKIP** (synthetic DI); official PC Lua / soft pixel / S-98 / `--dump-draw-order` / z-fighting beyond / per-layer UI = Non-goals / WARN; scale filter → **SCAMIN** already Closed (do **not** reopen); unified pick → **SESSION-PICK** already Closed (do **not** reopen); vendor/sha → **CATALOGUE** already Closed (do **not** reopen); scrubber → **TIMELINE** already Closed (do **not** reopen); wiring → **PC-WIRE** already Closed (do **not** reopen); Eng `c7a7deb` already (docs prep `de8082a`/`322cd96`); `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog; failure face = bare CHECK (do **not** invent `DISPLAYPRI-*` prefixes / new CTest); after Closed next **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.

**O1 closed as tip-truth / docs-close:** Eng `c7a7deb` already on tip (`display_plane_rank` / `draw_order_less` / `sort_instructions_for_draw` + viewer paint core comparator + CTest `display_priority_order` nullptr/inverted oracles + DEF parse; docs prep `de8082a`/`322cd96`; CTest **`display_priority_order`**). Slice = Arch docs close only — not greenfield Eng. Do **not** invent case ids or next tip names; do **not** reopen SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** invent `DISPLAYPRI-*` prefixes; do **not** retire `grid()` this tip; do **not** harden official PC Lua / soft pixel / S-98 / `--dump-draw-order`. **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest; inventing oracles / next tip id; inventing `DISPLAYPRI-*` failure-face prefixes.

---

## Slice close — 2026-10-25

**Status: Closed / Done** (formal docs close — **not** a draw-order / Eng rewrite).

- Evidence: CI [40175087823](https://github.com/MasterLivens/S100/actions/runs/40175087823) tip `dbfa1b6`≡`e6afbf7` — Ubuntu + Windows **`display_priority_order` Passed** (MUST NOT SKIP). Ancestor greens also Eng landing [`38132741413`](https://github.com/MasterLivens/S100/actions/runs/38132741413) @ `c7a7deb` + prior tip `#12 display_priority_order` Passed (SCAMIN Close CI [`40174178870`](https://github.com/MasterLivens/S100/actions/runs/40174178870) @ `0630604`).
- **O1=tip-truth / docs-close SUCCESS** — Eng `c7a7deb` already on tip; Arch Open→Closed only; Planner BRIEF §7 answers frozen.
- MUST tip-truth unchanged (`display_plane_rank` / `draw_order_less` / `sort_instructions_for_draw` + nullptr/inverted oracles + DEF DrawingPriority/DisplayPlane + viewer core comparator + fixtures **MUST NOT SKIP**).
- Eng `c7a7deb` already on tip — docs close only; no `ci.yml` change; **no Win (a)/(b)**; no large Eng.
- Failure face stays bare CHECK tip-truth — do **not** invent `DISPLAYPRI-*`.
- **Notes:** next **TBD** (architecture / registered Active — do **not** invent a next tip id). **Backlog:** `grid()` shim retire (`S102-GRID-SHIM-RETIRE`) — do not insert ahead. Contract display stays **rev 2**. Also closes backlog **`IHO-S101-DRAW-PRIORITY`**. Scale filter → SCAMIN already Closed (do **not** reopen); unified pick → SESSION-PICK already Closed (do **not** reopen); vendor/sha → CATALOGUE already Closed (do **not** reopen); scrubber → TIMELINE already Closed (do **not** reopen); wiring → PC-WIRE already Closed (do **not** reopen).
- Boundaries: independent; ≠ SCAMIN (already Closed); ≠ SESSION-PICK (already Closed); ≠ CATALOGUE (already Closed); ≠ TIMELINE (already Closed); ≠ PC-WIRE (already Closed); do **not** reopen SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.
- **Non-goals this close:** inventing oracles / next tip id; retiring `grid()` shim; draw-order/Eng rewrite; reopening SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; Win (a)/(b) (N/A — non-`viewer_*`); inventing `DISPLAYPRI-*` failure-face prefixes; hardening official PC Lua / soft pixel / S-98 / `--dump-draw-order`.

## Goal

Paint DrawingInstructions in a **deterministic order**: display plane first, then `drawing_priority` (DEF `DrawingPriority`). CTest pins the order.

## Fields (already on DI)

| Field | Source | Notes |
|-------|--------|-------|
| `display_plane` | DEF `DisplayPlane:` | e.g. `UnderRadar`, `OverRadar`; empty → treat as UnderRadar rank |
| `drawing_priority` | DEF `DrawingPriority:` | `int`; default `0` |

## Plane rank (locked)

```cpp
/** Lower rank paints first (underneath). */
int display_plane_rank(std::string_view plane,
                       const std::map<std::string, int>* pc_planes /*nullable*/);
```

Rules:

1. If `pc_planes` non-null and `plane` is a key → use that int (PortrayalCatalogue `displayPlanes` map).
2. Else fallback: `OverRadar` → **1**; anything else (incl. empty / `UnderRadar`) → **0**.

## Sort comparator (name locked)

```cpp
/** True if a must paint before b (underneath). Stable on ties. */
bool draw_order_less(const DrawingInstruction& a, const DrawingInstruction& b,
                     const std::map<std::string, int>* pc_planes);
```

1. `display_plane_rank(a) < display_plane_rank(b)` → a before b.
2. Else if ranks equal: `a.drawing_priority < b.drawing_priority` → a before b.
3. Else equal → **false** (caller uses `std::stable_sort` so input order preserved).

## Batch helper (name locked)

```cpp
/** stable_sort ios by draw_order_less. */
void sort_instructions_for_draw(DrawingInstructions& ios,
                                const std::map<std::string, int>* pc_planes = nullptr);
```

Also accept pointer-vector overload used by viewer if Eng prefers — but **at least** the value-vector API above MUST exist in `include/s100/` (e.g. `draw_order.hpp`).

## Where applied (MUST)

| Surface | Hook |
|---------|------|
| Core | `display_plane_rank` / `draw_order_less` / `sort_instructions_for_draw` |
| Viewer | Paint path **MUST** call the core helper (or identical comparator). No divergent ad-hoc sort. |
| CLI | Optional: `--dump-draw-order` prints feature_id/kind/plane/priority in sorted order after portray (helps oracle). WARN if omitted this slice if CTest covers core only. |

SCAMIN filter runs **before** sort when both apply (filter → sort → paint).

## Oracle — CTest `display_priority_order` (no SKIP)

Synthetic DIs (no cell required):

| id | `display_plane` | `drawing_priority` |
|----|-----------------|--------------------|
| P0 | `OverRadar` | 0 |
| P1 | `UnderRadar` | 5 |
| P2 | `UnderRadar` | 1 |
| P3 | `OverRadar` | 2 |
| P4 | `` (empty) | 3 |
| P5 | `UnderRadar` | 1 |

With `pc_planes == nullptr` (fallback ranks):

**Expected sorted order (ids):** `P2`, `P5`, `P4`, `P1`, `P0`, `P3`

Reason: Under+empty share fallback rank 0 before Over (rank 1). Within rank 0 by `drawing_priority`: P2(1), P5(1 stable), P4(3), P1(5); then Over: P0(0), P3(2).

Also MUST:

1. DEF parse: string with `DrawingPriority:7;DisplayPlane:OverRadar` → fields 7 / `OverRadar`.
2. With `pc_planes = {{"UnderRadar", 10}, {"OverRadar", 0}}` (inverted PC ranks): Over=0, empty missing→fallback **0** (same rank as Over), Under=10. Full pin: **`P0`, `P3`, `P4`, `P2`, `P5`, `P1`** (rank0 by prio: P0(0), P3(2), P4(3); then Under: P2(1), P5(1), P1(5)).
3. Viewer (or unit linking viewer comparator) uses core `draw_order_less` — test via core only is enough if viewer calls `sort_instructions_for_draw` / shared comparator (Eng documents one call site).

## MUST / WARN

| Check | Sev |
|-------|-----|
| Core API names + oracle orders above | MUST |
| CTest `display_priority_order` no SKIP | MUST |
| Viewer paint uses core sort (no fork) | MUST |
| Soft pixel if golden depends on order | WARN |
| `--dump-draw-order` CLI | WARN |

## Non-goals

Changing Lua PC; z-fighting beyond plane+priority; per-layer UI reorder; S-98.

## Supersedes

Backlog defer on `IHO-S101-DRAW-PRIORITY`.
