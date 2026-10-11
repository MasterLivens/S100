# S-164 display compare automation (next slice)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/testing/visual-acceptance.md` and `testdata/golden/s164-ref/` (`AA5DBASE`, `AA5STNDR` + `SOURCE.md`).

Case id: **`IHO-S164-DISPLAY-COMPARE`**.

## Goal

Automate a **non-pixel-perfect** check that our viewer, for Display Base / Standard style settings, is in the same ballpark as the S-164 manual reference figures — layer presence + colour-token usage — plus an optional soft image warn. Does **not** replace `viewer_golden` smoke and does **not** gate CI red on pixel equality.

## Inputs

| Role | Artifact |
|------|----------|
| Reference | `testdata/golden/s164-ref/AA5DBASE.png`, `AA5STNDR.png` (already in repo) |
| Ours | offscreen `--snapshot` at documented viewport (see test doc; default 1280×800 Day) |
| Settings | Map viewer/session to Display **Base** vs **Standard** using PC viewing groups / display category already on `LayerInfo` where available; if category not filled yet, document the proxy flags used (layer visibility presets) in the test README |

Prefer the S-164 / IHO cell that matches the figures when present in testdata; otherwise run on `101AA00DS0008.000` and mark the compare as **heuristic** (still useful, not type-approval).

## Automation (MVP)

1. **Script** (e.g. `scripts/s164_display_compare.py` + thin CMake/CTest wrapper):
   - Load ref PNG + our snapshot.
   - Report: (a) soft pixel drift % (warn only), (b) presence of Day palette anchor tokens in our dump / instruction summary, (c) optional viewing-group / layer-on checklist vs a small YAML expected list for Base vs Standard.
2. **CTest** `s164_display_compare` default **OFF** or **warn-only** (exit 0 with annotations), same spirit as EncDotNet oracle. Tip CI stays green if figures diverge.
3. Wire case id into `docs/testing/iho-ctest-case-ids.md`.
4. Do not edit official PC/FC; do not require EncDotNet or NIWC for this slice (those remain separate oracles in visual-acceptance.md).

## Explicit non-goals

Pixel-identical to S-164 figures; Windows NIWC capture; point-symbol text collision; tightening `viewer_golden` threshold.

## Done when

- Script + docs landed; Tester can run one command and get Base/Standard reports.
- Optional CTest warn-only hooked.
- Architect + Tester ack; Eng may push after Tester 准推.
