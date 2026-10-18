# s100-chart architecture (v0.2)

Cross-platform S-100 chart system: **C++17 + CMake + Qt6**, Linux / Windows.
Owner: **SoftwareArchitecture**. Scope/plan: Research&Planner. Implementation: SoftwareEngineer. Acceptance: SoftwareTester.

## Goals

- Load and display multiple S-100 products in one viewer.
- Keep product codecs behind a common façade so UI and tests stay stable.
- Ship P0 on S-101; leave hooks for HDF5 (S-102/104/111) and GML (S-124).
- Keep **Track A** (R&D engine) separate from **Track B** (ECDIS / class approval) — never claim SOLAS use until Track B is intentional.

## Layered modules

```
┌─────────────────────────────────────────────┐
│  s100_viewer (Qt6)  │  s100_cli (headless)  │  apps
├─────────────────────────────────────────────┤
│  ChartSession / LayerStack / PickController │  app services (thin)
├─────────────────────────────────────────────┤
│  s100_core                                  │
│   ├─ open_dataset / ProductId               │  façade
│   ├─ products/s101  (ISO 8211)              │  P0
│   ├─ products/hdf5  (S-102/104/111)         │  P1–P2 stubs → real
│   ├─ products/gml   (S-124)                 │  P3
│   ├─ catalogue (FC/PC) + portrayal (Lua)    │  P1
│   ├─ exchange_set (CATALOG.XML, updates)    │  P1
│   ├─ security (Part 15 signatures)          │  Active
│   ├─ layer / pick / crs / time              │  shared
│   └─ types (Geometry, Feature, Bounds)      │  shared
└─────────────────────────────────────────────┘
```

| Module | Responsibility | Status |
|--------|----------------|--------|
| `product` | `ProductId`, product_name | done |
| `dataset` | `Dataset`, `open_dataset`, `load_s101` | P0 stub→real ISO 8211 |
| `hdf5_product` | `load_s102` / `CoverageDataset` / sample | S-102 first slice |
| `layer` | LAND/DEPARE visibility | P0 |
| `pick` | screen → WGS84 (≤1e-5°) | P0 |
| `iso8211` | binary cell decode + spatial | **next (Eng)** |
| `catalogue` / `portrayal` | FC/PC XML + Lua | P1 |
| `exchange_set` | CATALOG.XML, .001… updates | **done (P1)** |
| `security` | Part 15 verify (`s100_security`); OpenSSL 3; exchange-set Enforce path | **Active (`IHO-S164-SIG`)** |
| `coverage` | grid `bounds` / `sample_wgs84` (EPSG:4326) | S-102 first slice |
| `viewer` | Qt map, toggles, status pick | P0 shell |

## Frozen public test hooks (do not break)

Stable surface for CTest / `fixtures.toml`. Apps and tests call these only — never ISO 8211 or HDF5 internals.

| Hook | Signature intent | Owner of tests |
|------|------------------|----------------|
| `open` | `open_dataset(path) → Dataset \| LoadError` | P0 smoke |
| `bounds` | lon/lat box of loaded product (add on `Dataset` when geometry is real) | fit view / fixtures |
| `feature_count` / layer ids | via `Dataset::stats()` + `LayerSet` | LAND/DEPARE |
| `sample` | `sample(lon, lat[, t])` for coverage products | P1+ S-102 |
| `pick` | `pick_wgs84(...)` → WGS84 ≤1e-5° | P0; control points after real geom |

### Coding rules (contract)

1. Apps call `open_dataset` / session APIs only — never ISO 8211 or HDF5 internals.
2. New products add a loader under `products/` and register in `open_dataset`; do not fork `Dataset` per UI.
3. CRS is WGS84 lon/lat at the façade for P0; projected views stay in the viewer.
4. Vector products fill `features` + `layers`. Coverage products (S-102+) must **not** explode grids into `Feature` lists — use `coverage`/`sample` path; keep `Dataset.product` + path metadata shared.
5. No navigation certification claim; test data is development-only.

## Product roadmap vs modules

| Phase | Products | Modules to land |
|-------|----------|-----------------|
| P0 | S-101 | real ISO 8211, layers, pick, Qt preview, CI |
| P1 | S-101 portrayal + exchange; **S-102** | catalogue, Lua, HDF5 grid render + `sample` |
| P2 | **S-104**, **S-111** | time axis, arrow/coverage symbology |
| P3 | **S-124** (+ optional S-127/S-131) | GML, multi-product time-aligned pick |

## Track B — certification (architecture only for now)

See `docs/certification/`. Next architect deliverable: `docs/certification/iec61174-requirements-matrix.md` (shall → module → auto vs lab test).

Product decision (2026-10-10): **ignore S-57 for now** — S-100-only product path. Do **not** start S-57 dual-fuel or sensor/BAM/route modules in P0.

Planned future top-level modules (not P0 work): `data_mgmt` (exchange + Part15/S-63), `portrayal` (official), `sensors` (IEC 61162), `route`, `alerts`/`bam`, `backup`, `integrity`.

## Out of scope (for now)

ECDIS type-approval claims, S-57/S-52 dual-fuel (deferred), Part 15 encryption/permits (signature verify is Active — see `docs/part15-signature-contract.md`).
