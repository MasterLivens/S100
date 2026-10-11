# Visual acceptance (S-101 portrayal)

| Level | Artifact | Gates CI? | Scope |
|-------|----------|-----------|-------|
| Smoke / regression | `testdata/golden/iho_0008.png` + `viewer_golden` | Yes | Day palette anchors, no magenta placeholders, ≤25% pixel drift |
| **Final acceptance** | `testdata/golden/s164-ref/AA5DBASE.png`, `AA5STNDR.png` | No (yet) | Symbol / layer look vs S-164 manual Display Base & Standard figures |

Provenance: `testdata/golden/s164-ref/SOURCE.md`. Current plan (S-102 CoverageFill, etc.) is not blocked by wiring automated comparison to these figures.
