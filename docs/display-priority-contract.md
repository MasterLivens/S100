# Display priority / draw order (rev 2)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-DISPLAY-PRIORITY`** (**Active**). Also closes backlog **`IHO-S101-DRAW-PRIORITY`**.
CTest: **`display_priority_order`** — **MUST NOT SKIP**.

Frozen with: `docs/api-contracts.md`, `docs/portrayal-contract.md`, `docs/testing/iho-ctest-case-ids.md`.

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
