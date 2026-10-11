# s100-chart

Cross-platform S-100 nautical chart system (Linux / Windows) — **C++17 + CMake + Qt6**.

## Scope (MVP)

- **P0**: S-101 ENC load (ISO 8211 `.000`), vector preview, LAND/DEPARE toggles, WGS84 pick (≤ 1e-5°)
- **P1**: Feature/Portrayal catalogues + Lua portrayal
- **P2**: update chain, query / validation

## Layout

| Target | Role |
|--------|------|
| `s100_core` | ISO 8211 loader, layers, pick |
| `s100_cli` | headless load smoke |
| `s100_viewer` | Qt6 map window |
| `s100_tests` | CTest (load / layers / pick) |

## Test data

IHO samples under `testdata/s101/*.000` (see `SOURCE.txt`). Fixtures: `testdata/s101/fixtures.toml`.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

## Run

```bash
./build/s100_cli testdata/s101/101AA00DS0008.000
./build/s100_viewer testdata/s101/101AA00DS0008.000
```
