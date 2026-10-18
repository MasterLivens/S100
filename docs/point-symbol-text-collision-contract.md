# Point-symbol text collision — rev1 · Active / Open

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Extends `docs/text-collision-contract.md`. Case id: **`IHO-S101-POINT-LABEL-COLLISION`** (**Active / Open**). CTest: **`viewer_point_label_collision`**.

**Status: Active / Open (2026-10-18)** — Planner freeze + **O1(b→a)**. Eng tip base: **`origin/main` @ `b02a4ac`**（≡ TEXT Closed `61a5b89`；可叠）。 Implementation already on tip (`ba9aa10` PointLabel classify / dump / CTest — **verify**, not greenfield). Ubuntu tip CI already Passed (`viewer_point_label_collision`); this slice = **docs close primarily** + Win policy; independent tip from TEXT. Do **not** edit PC Lua / `catalogues/`. No algorithm rewrite. Do **not** change TEXT cmake `--class Text`.

**Planner freeze (MUST):** fixture `101AA00DS0008.000` Day 1280×800 (no synth needed — off pairs>0); PointLabel = same-feature nearest Text within N=48 px; label-beside text AABB; shared `--no-text-collision` (default ON; OFF for Text **and** PointLabel; no second flag); greedy priority → unpaired Text before PointLabel → stable; dump `class=Text|PointLabel`; oracle **only** PointLabel∩PointLabel `pairs_off >= pairs_on` **and** `pairs_off > 0`; `point_glyphs_drawn(ON)==(OFF)`; Ubuntu CI: `viewer_point_label_collision` MUST NOT SKIP, Passed. Optional symbol-AABB-as-obstacle → **Backlog**, not this slice.

**O1 (b→a):** Try **(b) first** — Win CI narrowly run **only** `viewer_point_label_collision` (Eng `ci.yml`: keep `ctest -E "viewer_"` for the rest; TEXT's O1=(b) narrow-open of `viewer_text_collision` does **not** auto-cover this test). If hang/timeout → fall back to **(a)**: Ubuntu bind + contract Notes document Win CI `viewer_*` exclude (same family as golden/snapshot). **Forbidden:** platform SKIP inside the test itself. Docs close primarily; Win (b) is the only path that needs an Eng `ci.yml` one-liner.

**Close criteria (honest dual-green):**
- Ubuntu: `viewer_point_label_collision` MUST NOT SKIP, Passed (already on tip)
- Windows: (b) success → that test Passed on Win; (a) fallback → full matrix green is enough (Notes record exclude)
- Docs → Closed + CI Notes when done; if (b) needs Eng `ci.yml` narrow-open
- After Close next: **`IHO-S164-DISPLAY-COMPARE`** (`viewer_s164_display_compare`; BRIDGE #13 pixel defer). Then **`IHO-S102-DEEPEN-MULTIGRID`**.

**Eng notes:** verify `ba9aa10` on tip; cell `testdata/s101/101AA00DS0008.000`; shared flag already covers PointLabel; PL∩PL pairs + glyph survival oracles already wired. TEXT Closed already on main (`b02a4ac`); Eng may stack this Open + Win (b) `ci.yml` narrow-open. Do **not** edit `check_text_collision.cmake` / TEXT `--class Text`. Symbol-AABB obstacle = Backlog.

**Notes (Win CI):** Existing policy: Windows `ctest -E "viewer_"` then TEXT narrow-open only `^viewer_text_collision$`. O1 tries a narrow exception for **`viewer_point_label_collision`** first; on hang, document exclude and bind Close on Ubuntu Passed + Win matrix green.

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
- `docs/testing/iho-ctest-case-ids.md` registers `IHO-S101-POINT-LABEL-COLLISION` (main table Active/Open).

## Non-goals

SVG-embedded path text, Text∩PointLabel as hard CI metric, leader lines, S-102 deepen, symbol-AABB obstacle (Backlog), TEXT cmake `--class Text` change, in-test platform SKIP.
