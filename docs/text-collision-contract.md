# Text collision (viewer layout) — rev 2 · Open

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S101-TEXT-COLLISION`** (**Active / Open**). CTest: **`viewer_text_collision`** — **MUST NOT SKIP** (Planner freeze); Linux + Windows.

**Status: Active / Open (2026-10-18)** — Planner freeze applied. Eng tip base: **`origin/main` @ `9977a34`**. Implementation already on tip (`50ab9ba` viewer layout + CLI/oracle; `ba9aa10` PointLabel follow-on). **Not greenfield** — Eng verify / polish only; Tester dual-green then Close. Do **not** edit PC Lua / `catalogues/`.

**Planner freeze (MUST):** fixture `101AA00DS0008.000` Day 1280×800; default collision ON; `--no-text-collision` OFF; oracle `pairs_off >= pairs_on` **and** `pairs_off > 0`; CTest **MUST NOT SKIP**; Linux+Win; **unpaired `Kind::Text` only**; no PC Lua. Point-companion → `IHO-S101-POINT-LABEL-COLLISION` / `docs/point-symbol-text-collision-contract.md` (separate).

**Eng notes:** contract + CTest name above; cell path **`testdata/s101/101AA00DS0008.000`** (existence-only; not under `testdata/fixtures/catalogues`). Golden/palette paths already pass `--no-text-collision`. Optional tip: count oracle with `--class Text` so pairs are unpaired-Text only (PointLabel has its own CTest).

**O1 (ask Planner if blocking Close):** Windows CI currently runs `ctest -E "viewer_"` (Qt offscreen hang). Planner freeze says Linux+Win MUST NOT SKIP. Resolve: (a) accept Linux CI as bind + document Win exclude, or (b) Eng/CI tip to run `viewer_text_collision` on Win without hang. Do not silently reopen freeze.

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
