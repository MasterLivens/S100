# Public API contracts (P0 freeze)

Owner: SoftwareArchitecture. Implement: SoftwareEngineer. Test: SoftwareTester.

## Entry

- `bool open_dataset(path, Dataset&, LoadError*)` — only public load entry for apps/tests.
- `bool load_s101(...)` — S-101 helper; prefer `open_dataset` in new code.
- HDF5 / GML paths return `LoadErrorKind::Parse` until P1/P3 loaders land.

## Dataset (vector / S-101)

- `product`, `path`, `features`, `layers`, `stats()` (`feature_count`, `layer_count`).
- `Bounds bounds() const` → `{west,south,east,north}` WGS84 (`valid` flag). Present now from feature envelope; re-validate when ISO 8211 spatial is real. Tester keys fixtures on this.

## Layers / pick

- Layers: `LAND`, `DEPARE`, `Other` + `LayerVisibility`.
- `pick_wgs84(px,py,w,h, west,south,east,north)` → WGS84; tolerance check ≤ `1e-5` deg.
- Control points: `testdata/s101/fixtures.toml` after real ISO 8211 spatial records.

## Coverage (P1 — do not implement in P0)

- `sample(lon, lat[, t])` on coverage products; grids must not be flattened into `features`.

## Stability

Breaking these hooks needs Architect + Tester ack in the room before merge.
