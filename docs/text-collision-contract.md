# Text collision (viewer layout) — rev1 (Closed)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S101-TEXT-COLLISION`** (**Closed / Done** 2026-10-18). CTest: **`viewer_text_collision`** — MUST NOT SKIP; Linux + Windows dual-green (CI `39123185463` @ `845c51c`).
**Status: Closed / Done (2026-10-18)** — CI [`39123185463`](https://github.com/MasterLivens/S100/actions/runs/39123185463) @ `845c51c` dual-green `viewer_text_collision`; **O1=(b)** Win narrow-open Passed (no fallback to a); oracles unchanged (`pairs_off >= pairs_on` and `pairs_off > 0`; unpaired `Kind::Text` only). Docs close only — **no algorithm rewrite this tip**. Next: `IHO-S101-POINT-LABEL-COLLISION` (shared `--no-text-collision` already; separate case/CTest).

**Planner freeze (kept):** fixture `101AA00DS0008.000` Day 1280×800; default collision ON; `--no-text-collision` OFF; unpaired `Kind::Text` only; no PC Lua. Point-companion → `IHO-S101-POINT-LABEL-COLLISION` / `docs/point-symbol-text-collision-contract.md`.

**O1 closed as (b):** Win CI narrowly runs **only** `viewer_text_collision` (rest of `viewer_*` still `-E "viewer_"`); Ubuntu + Windows both Passed — no (a) Ubuntu-bind / Notes-exclude fallback.

## Slice close — 2026-10-18

**Status: Closed / Done** (formal docs close — **not** an algorithm rewrite).

- Evidence: CI [39123185463](https://github.com/MasterLivens/S100/actions/runs/39123185463) tip `845c51c` — Linux+Windows **`viewer_text_collision` Passed** (not Not Run / not Skip).
- **O1=(b) SUCCESS** — Win narrow-open Passed; no fallback to (a).
- Oracles stay as frozen MUST (pairwise Text boxes; soft golden with `--no-text-collision` on golden/palette paths).
- **Notes:** next Eng **`IHO-S101-POINT-LABEL-COLLISION`** — shared `--no-text-collision` already covers PointLabel on tip; separate case id / CTest (`viewer_point_label_collision`); gap-check later.
- **Non-goals this close:** PC Lua / `catalogues/` edits; algorithm rewrite; PointLabel oracle / glyph-survival counts (next slice).

## Scope

Reduce overlapping chart text in `s100_viewer` **after** portrayal. Official PC Lua and `catalogues/` stay read-only.

**This slice covers unpaired `Kind::Text` only.** Point-companion Text labels → `docs/point-symbol-text-collision-contract.md`. No SVG text parsing.

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

Point-companion labels and SVG-embedded text → see `docs/point-symbol-text-collision-contract.md`. Full ECDIS text-group tables, leader lines, EncDotNet pixel-equality.
