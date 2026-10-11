# Point-symbol text collision (rev 2)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Extends `docs/text-collision-contract.md`. Case id: **`IHO-S101-POINT-LABEL-COLLISION`**.

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
The Point symbol uses a separate **symbol AABB** that participates only as an obstacle for *other* labels (optional MVP: symbol boxes block Text/PointLabel).  
**Do not** inflate the symbol AABB to include the label.

On failure to place: **drop the Text (PointLabel) instruction** only; Point glyph always draws.

## 3. Shared flag

Same master switch as Text collision: default ON; `--no-text-collision` forces OFF for Text **and** PointLabel. No second flag.

Greedy order: drawing priority → at equal priority **unpaired Text before PointLabel** → stable list order.

## 4. Dump / oracle

`--dump-text-boxes` lines include `class=Text|PointLabel` (and feature_id, box).

**Overlap oracle counts only PointLabel–PointLabel pairs** (not Text∩PointLabel in the primary assert).  
Assert `pairs_off >= pairs_on` and `pairs_off > 0`.

If `101AA00DS0008.000` yields zero PointLabels under the pairing rule, ship a **named synthetic fixture** `testdata/s101/point_label_collision_synth` (or C++-built instruction list in CTest named `point_label_collision_synth`) that produces ≥1 PointLabel pair when OFF.

## 5. Symbol survival (pinned metric)

```
point_glyphs_drawn(ON) == point_glyphs_drawn(OFF)
```

Count = number of `Kind::Point` instructions actually submitted to the symbol painter (not dropped). Labels may drop; this count must not.

## 6. Cross-links

- `docs/text-collision-contract.md` Out of scope → points here for point-companion labels / SVG text.
- `docs/testing/iho-ctest-case-ids.md` registers `IHO-S101-POINT-LABEL-COLLISION`.

## Non-goals

SVG-embedded path text, Text∩PointLabel as hard CI metric, leader lines, S-102 deepen.
