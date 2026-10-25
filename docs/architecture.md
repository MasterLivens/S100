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
| `catalogue` / `portrayal` | FC/PC XML + Lua; install-tree root `(prefix)/share/s100` (`IHO-S164-POWERUP-INSTALL-PREFIX` **Closed**); Bridge/FASC render (`IHO-S101-BRIDGE-RENDER` **Closed** 2026-10-18, #12 MUST); Text collision (`IHO-S101-TEXT-COLLISION` **Closed** 2026-10-18, CI `39123185463` @ `845c51c`, O1=b dual-green); PointLabel collision (`IHO-S101-POINT-LABEL-COLLISION` **Closed** 2026-10-18, CI `39129316141` @ `9a1531c`, O1=b dual-green); Display compare (`IHO-S164-DISPLAY-COMPARE` **Closed** 2026-10-18, CI `39133583148` @ `ba63833`, O1=a dual-green); S-102 deepen (`IHO-S102-DEEPEN-MULTIGRID` **Closed** 2026-10-19, CI `39141930422` @ `5e05675`, O1=shim Backlog, dual-green); S-102 uncertainty (`IHO-S102-UNCERTAINTY` **Closed** 2026-10-25, CI `40142462597` @ `13b7c3c`, O1=#1b tip-truth, dual-green); S-102 QoBC meta (`IHO-S102-QBC-META` **Closed** 2026-10-25, CI `40154401620` @ `670708f` / formal Close CI `40155189914` @ `305bc89`, O1=tip-truth/docs-close, dual-green); S-102 QoBC multigrid (`IHO-S102-QBC-MULTIGRID` **Closed** 2026-10-25, CI `40156013552` @ `30f3546` / formal Close on Open tip `30f3546`, O1=tip-truth/docs-close, dual-green); S-104 open-frame (`IHO-S104-OPEN-FRAME` **Closed** 2026-10-25, CI `40159993218` @ `28e0f2b` / formal Close on Open tip `28e0f2b`≡`06da68d`, O1=tip-truth/docs-close, dual-green); S-111 open-frame (`IHO-S111-OPEN-FRAME` **Closed** 2026-10-25, CI `40162377554` @ `d512e30` / formal Close on Open tip `d512e30`≡`1be2b1b`, O1=tip-truth/docs-close, dual-green); S-124 open-list (`IHO-S124-OPEN-LIST` **Closed** 2026-10-25, CI `40164164518` @ `f2dccb1` / formal Close on Open tip `f2dccb1`≡`647bd75`, O1=tip-truth/docs-close, dual-green); PC wire 111/124 (`IHO-PC-WIRE-111-124` **Closed** 2026-10-25, CI `40165684207` @ `9e7eae0` / formal Close on Open tip `9e7eae0`≡`5b27957`, O1=tip-truth/docs-close, dual-green); Timeline UI (`IHO-TIMELINE-UI` **Closed** 2026-10-25, CI `40167445485` @ `c29b8ba` / formal Close on Open tip `c29b8ba`≡`feab8a0`, O1=tip-truth/docs-close, dual-green); Catalogue complete (`IHO-CATALOGUE-COMPLETE` **Closed** 2026-10-25, tip Close CI `40171329297` @ `d77216d` ≡ Arch `78979f1` / Open-bind CI `40170639650` @ `0ba61fb`≡`e2a3d43`, O1=tip-truth/docs-close, dual-green); Session pick unified (`IHO-SESSION-PICK-UNIFIED` **Closed** 2026-10-25, tip Close CI `40172798123` @ `2041980` ≡ Arch `371fedf` / Open-bind CI `40172265305` @ `f8aac10`≡`8dbe072`, O1=tip-truth/docs-close, dual-green); SCAMIN filter (`IHO-SCAMIN-FILTER` **Closed** 2026-10-25, tip Close CI `40174178870` @ `0630604` ≡ Arch `d948c79` / Open-bind CI `40173570061` @ `8d6fdc2`≡`9a14fb5`, O1=tip-truth/docs-close, dual-green); Display priority (`IHO-DISPLAY-PRIORITY` **Closed** 2026-10-25, tip Close CI `40175675310` @ `c8a1104` ≡ Arch `e8d4b2b` / Open-bind CI `40175087823` @ `dbfa1b6`≡`e6afbf7`, O1=tip-truth/docs-close, dual-green); Install-prefix (`IHO-S164-POWERUP-INSTALL-PREFIX` **Closed** 2026-10-25, tip Close CI `40177052197` @ `f7e8418` ≡ Arch `3ad7cb3` / Open-bind CI `40176493323` @ `7b1e9b6`≡`2cfe932`, O1=tip-truth/docs-close, dual-green); Power-up self-check (`IHO-S164-POWERUP` **Closed** 2026-10-25, CI `40177947235` @ `2d2e259` / formal Close on Open tip `2d2e259`≡`c64f9a6`, O1=tip-truth/docs-close, dual-green); Producer-code match (`IHO-S164-SIG-PRODUCER` **Closed** 2026-10-25, CI `40179378032` @ `4431d13` / formal Close on Open tip `4431d13`≡`9aa7f34`, O1=tip-truth/docs-close, dual-green); CN MRN→OfficialClass (`IHO-S164-SIG-CN-SHAPE` **Closed** 2026-10-25, CI `40181021692` @ `7793a63` / formal Close on Open tip `7793a63`≡`c799da5`, O1=tip-truth/docs-close, dual-green); Aggregator exempt+WARN (`IHO-S164-SIG-AGGREGATOR` **Closed** 2026-10-25, CI `40182601972` @ `3387aa4` / formal Close on Open tip `3387aa4`≡`62a81e3`, O1=tip-truth/docs-close, dual-green); Baseline Part 15 crypto (`IHO-S164-SIG` **Closed** 2026-10-25, CI `40186786770` @ `0158aa0` / formal Close on Open tip `0158aa0`≡`2d05469`, O1=tip-truth/docs-close, dual-green); Update-sequence robustness (`IHO-S164-UPD-SEQ` **Closed** 2026-10-25, CI `40190690732` @ `c3bb372` / formal Close on Open tip `c3bb372`≡`fd853d6`, O1=tip-truth/docs-close, dual-green); Corrupt-input / strong-guarantee (`IHO-S164-UPD-CORRUPT` **Closed** 2026-10-25, CI `40193555523` @ `700d73d` / formal Close on Open tip `700d73d`≡`5e4ccb1`, O1=tip-truth/docs-close, dual-green); Cancel / re-issue / new-edition (`IHO-S164-CANCEL-REISSUE` **Closed** 2026-10-25, CI `40197755849` @ `80e20dd` / formal Close on Open tip `80e20dd`≡`e7c99fb`, O1=tip-truth/docs-close, dual-green) | P1; **next** **TBD** (architecture / backlog — no invented case) |
| `exchange_set` | CATALOG.XML, .001… updates; update-sequence (`IHO-S164-UPD-SEQ` **Closed** 2026-10-25); corrupt-input / strong-guarantee (`IHO-S164-UPD-CORRUPT` **Closed** 2026-10-25); cancel / re-issue / new-edition (`IHO-S164-CANCEL-REISSUE` **Closed** 2026-10-25, CI `40197755849` @ `80e20dd`≡`e7c99fb`, Eng `45c355e`, O1=tip-truth/docs-close, dual-green) | **done (P1)** |
| `security` | Part 15 verify (`s100_security`); OpenSSL 3; exchange-set Enforce path; CN-shape OfficialClass + producer-code + aggregator follow-ons | **`IHO-S164-SIG`** (**Closed** 2026-10-25, CI `40186786770` @ `0158aa0`≡`2d05469`, Eng `f31679d`, O1=tip-truth/docs-close, dual-green) + **`IHO-S164-SIG-CN-SHAPE`** (**Closed** 2026-10-25, CI `40181021692` @ `7793a63`≡`c799da5`, Eng `0c21b4e`, O1=tip-truth/docs-close, dual-green) + **`IHO-S164-SIG-PRODUCER`** (**Closed** 2026-10-25, CI `40179378032` @ `4431d13`≡`9aa7f34`, Eng `0a7fe5f`/`89e31cd`, O1=tip-truth/docs-close, dual-green) + **`IHO-S164-SIG-AGGREGATOR`** (**Closed** 2026-10-25, CI `40182601972` @ `3387aa4`≡`62a81e3`, Eng `29be187`, O1=tip-truth/docs-close, dual-green) |
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

Power-up self-check **Closed** (`IHO-S164-POWERUP` / `powerup_self_check`; Eng `980a10c`; O1=tip-truth/docs-close). Install-prefix follow-on **Closed** (`IHO-S164-POWERUP-INSTALL-PREFIX` / `powerup_install_prefix`; Eng `5d323b8`/`065887b`; O1=tip-truth/docs-close): after `cmake --install`, PU-2 + `S100_DEFAULT_*` resolve under `(prefix)/share/s100` (env `S100_CATALOGUE_ROOT` override); build-tree CTest kept — see `docs/powerup-install-prefix-contract.md`. Producer-code match **Closed** (`IHO-S164-SIG-PRODUCER` / `exchange_signature_producer`; Eng `0a7fe5f`/`89e31cd`; O1=tip-truth/docs-close; CI `40179378032` @ `4431d13`≡`9aa7f34`; **no Win a/b**; do **not** mix AGGREGATOR/base SIG; shim Backlog) — see `docs/part15-producer-code-contract.md`. CN MRN→OfficialClass **Closed** (`IHO-S164-SIG-CN-SHAPE` / `exchange_signature_cn_shape`; Eng `0c21b4e`; O1=tip-truth/docs-close; CI `40181021692` @ `7793a63`≡`c799da5`; **no Win a/b**; do **not** reopen PRODUCER/POWERUP/INSTALL-PREFIX/DISPLAY etc; do **not** mix base SIG; shim Backlog) — see `docs/part15-cn-shape-contract.md`. Aggregator permanent exempt+WARN **Closed** (`IHO-S164-SIG-AGGREGATOR` / `exchange_signature_aggregator`; Eng `29be187`; O1=tip-truth/docs-close; CI `40182601972` @ `3387aa4`≡`62a81e3`; **no Win a/b**; do **not** reopen CN-SHAPE/PRODUCER/POWERUP/INSTALL-PREFIX/DISPLAY etc; do **not** mix base SIG; shim Backlog) — see `docs/part15-aggregator-producer-contract.md`. Baseline Part 15 crypto **Closed** (`IHO-S164-SIG` / `exchange_signature`; Eng `f31679d`; O1=tip-truth/docs-close; CI `40186786770` @ `0158aa0`≡`2d05469` / formal Close tip `3197c64`≡`283728a` / Close CI `40187387296`; **no Win a/b**; do **not** reopen AGGREGATOR/CN-SHAPE/PRODUCER/POWERUP/INSTALL-PREFIX/DISPLAY etc; shim Backlog) — see `docs/part15-signature-contract.md`. Update-sequence robustness **Closed** (`IHO-S164-UPD-SEQ` / `exchange_update_robustness`; Eng `331b9b9`; O1=tip-truth/docs-close; CI `40190690732` @ `c3bb372`≡`fd853d6` / formal Close tip `0833930`≡`bb337ce` / Close CI `40191381492`; **no Win a/b**; do **not** reopen SIG/AGGREGATOR/CN-SHAPE/PRODUCER/POWERUP/INSTALL-PREFIX/DISPLAY etc; shim Backlog) — see `docs/update-robustness-contract.md`. Corrupt-input / strong-guarantee **Closed** (`IHO-S164-UPD-CORRUPT` / `exchange_update_robustness`; Eng `331b9b9`; O1=tip-truth/docs-close; CI `40193555523` @ `700d73d`≡`5e4ccb1` / formal Close tip `eb91435`≡`17eea30` / Close CI `40195685446`; **no Win a/b**; do **not** reopen UPD-SEQ/SIG/AGGREGATOR/CN-SHAPE/PRODUCER/POWERUP/INSTALL-PREFIX/DISPLAY etc; shim Backlog) — see `docs/update-robustness-contract.md`. Cancel / re-issue / new-edition **Closed** (`IHO-S164-CANCEL-REISSUE` / `exchange_cancel_reissue`; Eng `45c355e`; O1=tip-truth/docs-close; CI `40197755849` @ `80e20dd`≡`e7c99fb`; **no Win a/b**; do **not** reopen UPD-SEQ/UPD-CORRUPT/SIG/AGGREGATOR/CN-SHAPE/PRODUCER/POWERUP/INSTALL-PREFIX/DISPLAY etc; do **not** mix EDITION-HARDEN; shim Backlog; next **TBD**) — see `docs/cancel-reissue-contract.md`.

Product decision (2026-10-10): **ignore S-57 for now** — S-100-only product path. Do **not** start S-57 dual-fuel or sensor/BAM/route modules in P0.

Planned future top-level modules (not P0 work): `data_mgmt` (exchange + Part15/S-63), `portrayal` (official), `sensors` (IEC 61162), `route`, `alerts`/`bam`, `backup`, `integrity`.

## Out of scope (for now)

ECDIS type-approval claims, S-57/S-52 dual-fuel (deferred), Part 15 encryption/permits (signature verify is Active — see `docs/part15-signature-contract.md`).
