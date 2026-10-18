# Point-symbol text collision — rev1 (Closed)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Extends `docs/text-collision-contract.md`. Case id: **`IHO-S101-POINT-LABEL-COLLISION`** (**Closed / Done** 2026-10-18). CTest: **`viewer_point_label_collision`** — MUST NOT SKIP; Linux + Windows dual-green (CI `39129316141` @ `9a1531c`).
**Status: Closed / Done (2026-10-18)** — CI [`39129316141`](https://github.com/MasterLivens/S100/actions/runs/39129316141) @ `9a1531c` dual-green `viewer_point_label_collision`; **O1=(b)** Win narrow-open Passed (no fallback to a); oracles unchanged (PointLabel∩PointLabel `pairs_off >= pairs_on` and `pairs_off > 0`; `point_glyphs_drawn(ON)==(OFF)`). Docs close only — **no algorithm rewrite this tip**. Next: `IHO-S164-DISPLAY-COMPARE` (`viewer_s164_display_compare`; BRIDGE #13 pixel defer). Then `IHO-S102-DEEPEN-MULTIGRID`.

**Planner freeze (kept):** fixture `101AA00DS0008.000` Day 1280×800 (off pairs>0); PointLabel = same-feature nearest Text within N=48 px; label-beside text AABB; shared `--no-text-collision` (default ON; OFF for Text **and** PointLabel; no second flag); greedy priority → unpaired Text before PointLabel → stable; dump `class=Text|PointLabel`; oracle **only** PointLabel∩PointLabel; `point_glyphs_drawn(ON)==(OFF)`; Ubuntu CI MUST NOT SKIP Passed. Optional symbol-AABB-as-obstacle → **Backlog**. Do **not** edit PC Lua / `catalogues/`. Do **not** change TEXT cmake `--class Text`.

**O1 closed as (b):** Win CI narrowly runs **only** `viewer_point_label_collision` (in addition to TEXT's narrow-open; rest of `viewer_*` still `-E "viewer_"`); Ubuntu + Windows both Passed — no (a) Ubuntu-bind / Notes-exclude fallback.

## Slice close — 2026-10-18

**Status: Closed / Done** (formal docs close — **not** an algorithm rewrite).

- Evidence: CI [39129316141](https://github.com/MasterLivens/S100/actions/runs/39129316141) tip `9a1531c` — Linux+Windows **`viewer_point_label_collision` Passed** (not Not Run / not Skip).
- **O1=(b) SUCCESS** — Win narrow-open Passed; no fallback to (a).
- Oracles stay as frozen MUST (PL∩PL pairs; glyph survival; shared `--no-text-collision`).
- **Notes:** next Eng **`IHO-S164-DISPLAY-COMPARE`** — BRIDGE #13 pixel defer lands there; then **`IHO-S102-DEEPEN-MULTIGRID`**.
- **Non-goals this close:** PC Lua / `catalogues/` edits; algorithm rewrite; symbol-AABB obstacle (Backlog); TEXT cmake `--class Text` change; DISPLAY-COMPARE oracles.

## 1. PointLabel identity (pinned)

A **PointLabel** is a `Kind::Text` instruction that is the **companion** of a `Kind::Point` instruction from the **same** `portray` output:

```
same feature_id
AND Text instruction appears in the instruction list associated with that point
AND pairing rule: nearest Text with that feature_id whose anchor lies within N screen pixels of the Point anchor
   (N = 48 px at 1280×800 reference; document constant in code)
```

- There is **no** `Point.text` field today — do not invent one and do not parse Symbols/*.svg.
- Unpaired Text stays class `Text` (existing collision path).
- Unpaired Point has **no** PointLabel (symbol only).

## 2. Box model (pick one — locked)

**Label-beside box only:** PointLabel's collision box is the **text AABB** placed at the Text instruction anchor (after any offset attempts).  
The Point symbol uses a separate **symbol AABB** that participates only as an obstacle for *other* labels (**Backlog** — optional MVP; **not this slice**).  
**Do not** inflate the symbol AABB to include the label.

On failure to place: **drop the Text (PointLabel) instruction** only; Point glyph always draws.

## 3. Shared flag

Same master switch as Text collision: default ON; `--no-text-collision` forces OFF for Text **and** PointLabel. No second flag.

Greedy order: drawing priority → at equal priority **unpaired Text before PointLabel** → stable list order.

## 4. Dump / oracle

`--dump-text-boxes` lines include `class=Text|PointLabel` (and feature_id, box).

**Overlap oracle counts only PointLabel–PointLabel pairs** (not Text∩PointLabel in the primary assert).  
Assert `pairs_off >= pairs_on` and `pairs_off > 0`.

Fixture `101AA00DS0008.000` already yields PointLabel pairs when OFF (no synth required for this slice).

## 5. Symbol survival (pinned metric)

```
point_glyphs_drawn(ON) == point_glyphs_drawn(OFF)
```

Count = number of `Kind::Point` instructions actually submitted to the symbol painter (not dropped). Labels may drop; this count must not.

## 6. Cross-links

- `docs/text-collision-contract.md` (**Closed**) — unpaired `Kind::Text`; shared flag; separate tip.
- `docs/testing/iho-ctest-case-ids.md` registers `IHO-S101-POINT-LABEL-COLLISION` (main table **Closed / Done**).
- Next: `docs/s164-display-compare-contract.md` (**Active / Open** after this close).

## Non-goals

SVG-embedded path text, Text∩PointLabel as hard CI metric, leader lines, S-102 deepen, symbol-AABB obstacle (Backlog), TEXT cmake `--class Text` change, in-test platform SKIP, algorithm rewrite this tip.
