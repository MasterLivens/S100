# S-102 deepen (next slice)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/s102-contract.md` (open / bounds / sample_wgs84 / CoverageFill / ChartSession overlay already shipped).

Case id: **`IHO-S102-DEEPEN-MULTIGRID`**.

## Goals

1. **Multi-grid**: load and portray every regular-grid BathymetryCoverage instance in one S-102 dataset (not only the first), each with its own geographic envelope and sample domain.
2. **Palette refinement**: CoverageFill colouring stays token-based; deepen how bands map and how Day/Dusk/Night remaps without hard-coded RGB.

## Multi-grid

| Rule | Detail |
|------|--------|
| Model | `CoverageDataset` holds `std::vector<CoverageGrid> grids` (or equivalent named instances). Deprecate “single grid only” paths. |
| Open | Enumerate all grid feature instances from HDF5; skip non-regular / unsupported with a logged warning, not a hard fail if ≥1 grid loads. |
| `bounds()` | Union of all loaded grids' geographic envelopes (and still union with vector features when overlaid). |
| `sample_wgs84` | Query the grid whose geographic envelope contains the point (if several contain it, prefer smallest enclosing area, then stable instance order). Outside all → false. |
| Portrayal | One CoverageFill (or equivalent) instruction stream **per grid** (or one instruction with explicit grid id — pick one and document). Viewer paints all. |
| Session key | Unchanged: `S-102:<dataset-id>` one member; multi-grid is inside the member. |

Fixtures: prefer NOAA/CHS `.h5` that already has >1 grid if available; otherwise synthetic HDF5 or dual-file overlay test documenting limitation.

## Palette refinement

| Rule | Detail |
|------|--------|
| Tokens | Still from PC `CoverageColor` / LookupEntry → colour token names only. |
| Bands | Honour interval kinds already parsed (`ltSemiInterval`, `geLtInterval`, `geSemiInterval`); add test that Safety-style context changes (if S-102 PC exposes any) or band table changes alter tokens. |
| Day/Dusk/Night | Switching palette **must not** re-open HDF5 or re-`portray` if instructions are unchanged — only remap tokens→RGB (same as S-101). |
| No RGB in core | Viewer/palette XML only. |

## Tests (MUST)

1. Multi-grid open: `grids.size() >= 2` on chosen fixture **or** SKIP with explicit reason if no multi-grid fixture in CI.
2. `sample_wgs84` hits correct grid on a point inside grid A only vs B only.
3. Overlay paint: both grids contribute visible CoverageFill (instruction count or non-zero painted pixels per grid id dump).
4. Palette switch Dusk after Day: instruction token list identical; pixel RGB changes (soft assert).
5. Existing S-102 single-grid / UTM / overlay tests stay green.

## Non-goals

S-104/S-111 time axis, uncertainty visualization as a separate layer (may stay in sample struct), S-98 IO, pixel-match EncDotNet for S-102.
