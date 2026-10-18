# Text collision (viewer layout) — rev1 · Active / Open

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S101-TEXT-COLLISION`** (**Active / Open**). CTest: **`viewer_text_collision`**.

**Status: Active / Open (2026-10-18)** — Planner freeze + **O1(b→a)**. Eng tip base: **`origin/main` @ `9977a34`**. Stack Arch docs onto **`5fa4b98`**. Implementation already on tip (`50ab9ba` viewer layout + CLI/oracle; `ba9aa10` PointLabel follow-on — **not this slice**). **Not greenfield** — Eng verify / polish only; optional oracle `--class Text` (not blocking Close); Tester dual-green then Close. Do **not** edit PC Lua / `catalogues/`. No algorithm rewrite.

**Planner freeze (MUST):** fixture `101AA00DS0008.000` Day 1280×800; default collision ON; `--no-text-collision` OFF; oracle `pairs_off >= pairs_on` **and** `pairs_off > 0`; **Ubuntu CI: `viewer_text_collision` MUST NOT SKIP, Passed**; **unpaired `Kind::Text` only**; no PC Lua. Point-companion → `IHO-S101-POINT-LABEL-COLLISION` / `docs/point-symbol-text-collision-contract.md` (separate tip / separate Closed).

**O1 (closed as b→a):** Try **(b) first** — Win CI narrowly run **only** `viewer_text_collision` (Eng `ci.yml` tip: keep `ctest -E "viewer_"` for the rest of the viewer family; Qt offscreen hang policy unchanged for golden/snapshot/etc.). If hang/timeout → fall back to **(a)**: Ubuntu bind + contract Notes document Win CI `viewer_*` exclude (same family as golden/snapshot). **Forbidden:** platform SKIP inside the test itself; local Win run optional. Do **not** choose pure (a)-only without trying (b); whole-viewer Win headless Qt remains a separate future slice.

**Close criteria (honest dual-green):**
- Ubuntu: `viewer_text_collision` MUST NOT SKIP, Passed
- Windows: (b) success → that test Passed on Win; (a) fallback → full matrix green is enough (Notes record `viewer_*` exclude)
- Docs → Closed + CI Notes when done; if (b) needs Eng `ci.yml` narrow-open
- After Close next: **`IHO-S101-POINT-LABEL-COLLISION`**

**Eng notes:** contract + CTest name above; cell path **`testdata/s101/101AA00DS0008.000`** (existence-only; not under `testdata/fixtures/catalogues`). Golden/palette/snapshot paths already pass `--no-text-collision`. Optional tip: count oracle with `--class Text` so pairs are unpaired-Text only (POINT-LABEL has its own CTest — **next slice**). Shared `--no-text-collision` already covers PointLabel on tip; PointLabel oracle / `viewer_point_label_collision` / glyph-survival counts **do not** belong to this Closed.

**Notes (Win CI):** Existing policy in `.github/workflows/ci.yml`: Windows `ctest -E "viewer_"` (Qt offscreen/show+grab hang). O1 tries a narrow exception for this one CTest first; on hang, document exclude like golden/snapshot and bind Close on Ubuntu Passed + Win matrix green.

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
