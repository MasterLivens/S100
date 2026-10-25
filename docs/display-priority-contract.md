# Display priority / draw order (rev 2) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-DISPLAY-PRIORITY`** (**Active / Open**). Also closes backlog row **`IHO-S101-DRAW-PRIORITY`** per this contract (do **not** invent new ids).
CTest: **`display_priority_order`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev 2** — Planner freeze (MUST below). Display priority / draw order / `display_plane_rank` slice after closed **`IHO-SCAMIN-FILTER`**. Does **not** reopen SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; does **not** jump **`S102-GRID-SHIM-RETIRE`**.

Frozen with: `docs/api-contracts.md`, `docs/portrayal-contract.md`, `docs/testing/iho-ctest-case-ids.md`. Scale filter → **SCAMIN** already Closed — **do not** reopen. Unified pick → **SESSION-PICK** already Closed — **do not** reopen. Vendor / sha → **CATALOGUE** already Closed — **do not** reopen. Scrubber → **TIMELINE** already Closed — **do not** reopen. Wiring → **PC-WIRE** already Closed — **do not** reopen. Soft pixel / official PC Lua / S-98 / `--dump-draw-order` / z-fighting beyond plane+priority / per-layer UI reorder = Non-goals / WARN.

Primary research: `/workspace/research/display-priority/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`c7a7deb`** (docs prep **`de8082a`** / **`322cd96`**).

**Status: Active / Open (2026-10-25)** — after **SCAMIN Closed** (tip `0630604` ≡ Arch `d948c79`; tip Close CI [`40174178870`](https://github.com/MasterLivens/S100/actions/runs/40174178870) dual-green; Open-bind evidence CI [`40173570061`](https://github.com/MasterLivens/S100/actions/runs/40173570061) @ `8d6fdc2`≡`9a14fb5`, O1=tip-truth/docs-close). Eng tip ancestor **`c7a7deb`** already on tip — **not greenfield** (`display_plane_rank` / `draw_order_less` / `sort_instructions_for_draw` + viewer paint core comparator after SCAMIN filter + CTest `display_priority_order` nullptr/inverted oracles + DEF `DrawingPriority`/`DisplayPlane` parse; **no** Win a/b chain). Ubuntu+Windows already Passed — tip Close CI [`40174178870`](https://github.com/MasterLivens/S100/actions/runs/40174178870) `#12 display_priority_order` + Eng landing [`38132741413`](https://github.com/MasterLivens/S100/actions/runs/38132741413) @ `c7a7deb` (native `s100_tests` + synthetic DI — **not** `viewer_*` — **no Win O1 (a)/(b)**). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Do **not** invent case ids or next tip names; freeze tip-truth below. Do **not** reopen SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Do **not** invent `DISPLAYPRI-*` failure-face prefixes (keep existing bare CHECKs). Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). Contract display header stays **rev 2** (do **not** bump just for Open). After Closed next: **TBD** (architecture / registered Active only — do **not** invent).

**Planner freeze (MUST — from BRIEF §7 + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-DISPLAY-PRIORITY`** / CTest **`display_priority_order`**; **forbidden** to invent a second case/test name; Close also closes backlog **`IHO-S101-DRAW-PRIORITY`** (do **not** invent new ids).
- MUST tip-truth = `display_plane_rank(plane, pc_planes)` + `draw_order_less(a, b, pc_planes)` + `sort_instructions_for_draw(ios, pc_planes)`; plane rank (1) `pc_planes` hit → map value (2) else `OverRadar`→1 else 0; comparator rank→priority→stable; nullptr oracle ids `P2,P5,P4,P1,P0,P3`; inverted `{{UnderRadar,10},{OverRadar,0}}` → `P0,P3,P4,P2,P5,P1`; DEF `DrawingPriority:7;DisplayPlane:OverRadar`; viewer paint uses core comparator; SCAMIN filter **before** sort; fixtures **MUST NOT SKIP** (synthetic DI); official PC Lua / soft pixel / S-98 / `--dump-draw-order` / z-fighting beyond / per-layer UI = Non-goals / WARN.
- Eng tip already has **`c7a7deb`** (docs prep **`de8082a`** / **`322cd96`**); Ubuntu+Win (tip `40174178870` `#12` / Eng `38132741413`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield.
- Scale filter → **`IHO-SCAMIN-FILTER`** already Closed — **do not** reopen.
- Unified pick → **`IHO-SESSION-PICK-UNIFIED`** already Closed — **do not** reopen.
- Vendor / sha → **`IHO-CATALOGUE-COMPLETE`** already Closed — **do not** reopen.
- Scrubber → **`IHO-TIMELINE-UI`** already Closed — **do not** reopen.
- Wiring → **`IHO-PC-WIRE-111-124`** already Closed — **do not** reopen.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog** (DEEPEN O1); **not** this tip.
- Failure face = existing bare CHECKs; do **not** invent `DISPLAYPRI-*` prefixes.
- Ubuntu + Windows: `display_priority_order` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`; native `s100_tests` + synthetic DI; no SKIP. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `display_priority_order` MUST NOT SKIP, Passed (already tip CI `40174178870` `#12` / Eng `38132741413`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng diff required
- Also closes backlog row **`IHO-S101-DRAW-PRIORITY`**
- After Close next: **TBD**

**Eng notes:** verify `c7a7deb` on tip (docs prep `de8082a`/`322cd96`); API `include/s100/draw_order.hpp` + `src/core/draw_order.cpp`; driver `tests/test_display_priority_order.cpp`; cmake `display_priority_order`; synthetic DI (no cell); DEF string parse. SCAMIN Closed already on this stack (`0630604`). Do **not** reopen SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Official PC Lua / soft pixel / S-98 / `--dump-draw-order` **Non-goals / WARN** — do **not** harden. Do **not** invent `DISPLAYPRI-*` failure prefixes. Do **not** change tip-truth plane-rank rules / nullptr / inverted oracles.

**O-questions (BRIEF §3 Open 1–N — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `c7a7deb` already; no `ci.yml` / no large Eng) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` — **yes / freeze**.
3. Contract head → Open; keep display **rev 2** (do **not** bump just for Open) — **yes / freeze**.
4. case-ids Active → Open (Status stays Active family) — **yes / freeze**.
5. Notes ≠ SCAMIN-FILTER (already Closed; do **not** reopen); ≠ SESSION-PICK-UNIFIED (already Closed; do **not** reopen); ≠ CATALOGUE-COMPLETE (already Closed; do **not** reopen); ≠ TIMELINE-UI (already Closed; do **not** reopen); ≠ PC-WIRE (already Closed; do **not** reopen); ≠ S-124 / S-111 / S-104 open; do **not** reopen QBC-MG / UNCERT / DEEPEN; shim-retire stays Backlog; official Lua / soft pixel / S-98 / `--dump-draw-order` stay Non-goals / WARN — **yes / freeze**.
6. Closed next → **TBD** (architecture / registered Active; do **not** invent) — **yes / freeze**.
7. No other cases this tip (do **not** Closed SIG / POWERUP / etc.) — **yes / freeze**.
8. architecture.md flip **next** to this Open — **yes / freeze**.
9. `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog — **yes / freeze**.
10. Failure face = existing bare CHECKs; do **not** invent `DISPLAYPRI-*` prefixes — **yes / freeze**.
11. draw order/sort = this tip; scale filter = SCAMIN Closed; unified pick = SESSION-PICK Closed; vendor/sha = CATALOGUE Closed; scrubber = TIMELINE Closed; wiring = PC-WIRE Closed — **yes / freeze**.

---

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
