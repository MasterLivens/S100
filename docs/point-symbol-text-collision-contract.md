# Point-symbol text collision (next slice)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Extends `docs/text-collision-contract.md` (Kind::Text already shipped). Case id: **`IHO-S101-POINT-LABEL-COLLISION`**.

## Scope

Avoidance for **labels attached to point symbols**, not `Kind::Text` alone.

Sources of point labels (MVP — pick what instructions already expose; do not invent fields in Lua):

1. Point `DrawingInstruction`s that carry a non-empty text/label string alongside `symbol_ref`, **or**
2. A documented companion Text instruction sharing the same `feature_id` and anchor (if the engine already emits that pattern).

If neither exists for a feature, **skip** (no SVG font parsing in MVP). Parsing text out of Symbols/*.svg is **out of scope**.

## Rules

1. Same viewer-side layout pass as Text collision; one shared placer, two input classes: `Text` + `PointLabel`.
2. Default **ON** with the same master flag (`text_collision_avoidance` / `--no-text-collision`). No second CLI flag in MVP.
3. Greedy order: drawing priority, then Text before PointLabel at equal priority (Text wins ties), then stable instruction order.
4. PointLabel box = symbol screen AABB **inflated** by label metrics (or label-only box beside the symbol if the instruction separates them). On conflict: offset set then **drop label only** — never drop the symbol glyph.
5. Official Lua / catalogues / SVGs read-only. Soft golden / palette / snapshot captures keep `--no-text-collision`.
6. Do not require bit-identical pixels.

## Tests

Fixture: `101AA00DS0008.000`, Day, 1280×800.

| # | Assertion |
|---|-----------|
| 1 | `--no-text-collision` still forces OFF for both Text and PointLabel. |
| 2 | Oracle: pairwise overlaps among **PointLabel boxes** (and optionally Text∩PointLabel): `pairs_off >= pairs_on` and `pairs_off > 0` if the fixture has point labels; if zero point labels on 0008, use a tiny synthetic instruction list in CTest instead and document it. |
| 3 | Symbols remain visible when labels drop (count of point symbols drawn ON == OFF). |
| 4 | Existing `viewer_text_collision` (Kind::Text) stays green. |
| 5 | Case id in `iho-ctest-case-ids.md`. |

## Non-goals

SVG-embedded path text, full ECDIS text groups, leader lines, S-102 deepen.
