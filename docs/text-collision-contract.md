# Text collision (viewer layout) — next slice (rev 2)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S101-TEXT-COLLISION`** (also list in `docs/testing/iho-ctest-case-ids.md` when implementing).

## Scope

Reduce overlapping chart text in `s100_viewer` **after** portrayal. Official PC Lua and `catalogues/` stay read-only.

**MVP covers only `DrawingInstruction::Kind::Text`.** Labels baked into point-symbol SVGs / non-Text kinds are a later slice.

## Rules

1. **Input** = Text instructions only. Do not re-query Dataset for labels.
2. **Layout runs in the viewer** (or a Qt-free helper used only by the viewer). Core portrayal must not depend on font metrics.
3. **Default ON.** CLI / capture flag:
   - `--no-text-collision` → force **OFF**
   - Optional `--dump-text-boxes` → write screen AABB list (for oracle / debug)
4. **Algorithm (MVP)**: greedy by drawing priority (then stable instruction order). Each Text label → axis-aligned screen box; on intersection with an already-placed Text box, try a small offset set (N/E/S/W / slight diagonal); if none fit, **drop** the label (do not shrink font in MVP).
5. **Do not** invent symbology, change colour tokens, or edit Lua / SVGs.
6. Palette switch does not re-layout unless viewport/DPI/zoom or the avoidance flag changed.

## Test hooks (pinned)

Fixture: `101AA00DS0008.000`, Day, 1280×800 offscreen.

| # | Assertion |
|---|-----------|
| 1 | CLI: default avoidance ON; `--no-text-collision` turns OFF; `--dump-text-boxes` available for oracle. |
| 2 | **Primary oracle:** count pairwise overlapping Text screen-boxes. Assert `pairs_off >= pairs_on` **and** `pairs_off > 0` on the fixture (proves the cell actually has collisions when off). |
| 3 | Avoidance **OFF** paths used by `viewer_golden` / palette captures **must** pass `--no-text-collision` until an intentional ON golden refresh. |
| 4 | Do **not** require bit-identical pixels; use existing soft golden tolerance. |
| 5 | MVP tests only `Kind::Text`. |
| 6 | Case id `IHO-S101-TEXT-COLLISION` wired in docs/CTest naming. |

Dropped-label count may show in CLI/status for debug; not a CI failure by itself.

## Out of scope

Point-symbol / SVG-embedded text, full ECDIS text-group tables, leader lines, EncDotNet pixel-equality, S-164 auto compare (after this slice signs off).
