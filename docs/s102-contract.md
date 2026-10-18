# S-102 overlay contract (P1)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Pairs with FC/PC **3.0.0** under `catalogues/S-102/` (provenance + hash pairing like S-101).

## Decisions (freeze)

1. **`sample_wgs84` is WGS84-only at the façade.** Callers pass lon/lat; `s100_core` converts projected grids internally (EPSG:326xx/327xx UTM today; other EPSG → false until extended). Viewer/CLI never see a projected CRS.
2. **CoverageFill uses symbolic colour tokens**, same Day/Dusk/Night palette path as S-101. Depth-band → token mapping comes from PC/product rules; no hard-coded RGB in core or viewer.
3. **HDF5 is required for S-102 builds that claim the product.** Linux: system `libhdf5`. Windows CI: reproducible install (choco or vcpkg); if configure cannot find HDF5, **fail configure** for that job — do not stub `open` to succeed. Unit tests without fixture files may `SKIP`.
4. **One `ChartSession`.** Member key `S-102:<dataset-id>` (stem or catalogue id). `Dataset.features` stays empty for coverage; geometry/values via `bounds`/`sample` + CoverageFill instructions. Overlay view Datasets attach `coverage` onto the S-101 member; `Dataset::bounds()` is the **union** of feature and coverage geographic envelopes.
5. **Same `DrawingInstructions`.** Viewer paints CoverageFill from `CoverageColor`/`LookupEntry` tokens (Day/Dusk/Night); engine may emit instructions before the renderer paints cells. Open `.h5` alone or overlay with `s100_viewer <S-101.000> --s102 <path.h5>`.

## Out of first slice (see deepen / uncertainty)

Multi-grid / palette refinement → `docs/s102-deepen-contract.md`.
Uncertainty as its own CoverageFill layer → `docs/s102-uncertainty-contract.md` (`IHO-S102-UNCERTAINTY`).
S-98 interoperability, S-104/S-111 time axis.
