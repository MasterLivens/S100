# Hook → CTest map (P0)

Owner: SoftwareTester. Contracts: `docs/api-contracts.md`.

| Hook | Contract | CTest / check | Status |
|------|----------|---------------|--------|
| `open` | `open_dataset(path)` | `tests/test_load.cpp` — load `101AA00DS0008.000` <5s, `feature_count>0`, `layers>=2` | green (stub geom) |
| `layers` | LAND/DEPARE + visibility | `tests/test_layers.cpp` | green |
| `pick` | `pick_wgs84` ≤1e-5° | `tests/test_pick.cpp` + `test_fixtures_pick.cpp` vs `0008` bounds | green (real cell envelope) |
| `bounds` | `Dataset::bounds()` WGS84 envelope | `test_load` + fixtures expect for `0008` | green (real decode) |
| `sample` | coverage only | P1 S-102 | not started |

## Fixtures

- Path: `testdata/s101/fixtures.toml`
- After real spatial decode: control points keyed to `bounds()` + known cell corners; pick tol `1e-5` deg.

## Out of P0 automation

IEC 61174 full matrix → hand/lab items tracked in `docs/certification/` (Architect matrix). This table is R&D smoke only — not navigation/SOLAS evidence.
