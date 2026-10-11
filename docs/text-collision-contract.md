# Text collision (viewer layout) — next slice

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.

## Scope

Reduce overlapping chart text in `s100_viewer` **after** portrayal. Official PC Lua and `catalogues/` stay read-only.

## Rules

1. **Input** = `DrawingInstructions` with `Kind::Text` (and any text-bearing point symbols that carry a label field). Do not re-query Dataset for labels.
2. **Layout runs in the viewer** (or a Qt-free layout helper linked only by the viewer). Core portrayal must not depend on font metrics.
3. **Toggle** session/UI flag `text_collision_avoidance` (default **on**). Off → draw text at instruction anchors, bit-identical to pre-slice behaviour for Text placement.
4. **Algorithm (MVP)**: greedy by drawing priority (then stable instruction order). Each label gets an axis-aligned screen box; if it intersects an already-placed box (or optional marginal priority geometry), try a small offset set (N/E/S/W / slight diagonal); if none fit, **drop** the label (do not shrink font below PC size in MVP).
5. **Do not** invent new symbology, change colour tokens, or edit Lua. Symbol SVGs unchanged.
6. Day/Dusk/Night palette switch must not re-run layout unless DPI/zoom/viewport changed (cache key: viewport + avoidance flag + instruction list id).

## Test hooks

- CTest / offscreen: same cell, avoidance **on vs off** → when on, pairwise Text screen-box IoU below threshold **or** fewer overlapping pairs than off (document metric in test).
- Avoidance off: Text anchor pixels match golden within existing soft tolerance (or dedicated smoke).
- `catalogue_provenance` still green (no catalogue file edits).
- Optional: count of dropped labels exposed in CLI/viewer status for debugging (not a CI fail).

## Out of scope

Full ECDIS text group priority tables, leader lines, user-selectable text groups, EncDotNet pixel-equality.
