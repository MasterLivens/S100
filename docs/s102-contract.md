# S-102 overlay contract (P1)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Pairs with FC/PC **3.0.0** under `catalogues/S-102/` (provenance + hash pairing like S-101).

## Decisions (freeze)

1. **`sample` is WGS84-only at the façade.** Callers pass lon/lat; if the HDF5 grid is projected, `s100_core` converts internally. Viewer/CLI never see a projected CRS. Add `sample_wgs84` (or document `sample` as WGS84) before claiming non-4326 cells.
2. **CoverageFill uses symbolic colour tokens**, same Day/Dusk/Night palette path as S-101. Depth-band → token mapping comes from PC/product rules; no hard-coded RGB in core or viewer.
3. **HDF5 is required for S-102 builds that claim the product.** Linux: system `libhdf5`. Windows CI: reproducible install (choco or vcpkg); if configure cannot find HDF5, **fail configure** for that job — do not stub `open` to succeed. Unit tests without fixture files may `SKIP`.
4. **One `ChartSession`.** Member key `S-102:<dataset-id>` (stem or catalogue id). `Dataset.features` stays empty for coverage; geometry/values via `bounds`/`sample` + CoverageFill instructions.
5. **Same `DrawingInstructions`.** Viewer paints CoverageFill from `CoverageColor`/`LookupEntry` tokens (Day/Dusk/Night); engine may emit instructions before the renderer paints cells. Open `.h5` alone or overlay with `s100_viewer <S-101.000> --s102 <path.h5>`.

## Out of this slice

Text collision, S-98 interoperability ordering, S-104/S-111 time axis, FASC/INAS element merge.
