# IEC 61174 requirements → modules → tests (PROVISIONAL)

**Status:** S-100-first (per MIN YE 2026-10-10: **ignore S-57 for now**). Target performance path treated as **MSC.530/Rev.1 + S-100 stack**; IEC 61174 Ed.5 preferred when citing tests.  
S-57 dual-fuel rows are **deferred / out of scope** until explicitly revived — not blocking P0–P3 product work.

Owner: SoftwareArchitecture. Standards index: Research&Planner (`standards-index.md`). Test split: SoftwareTester.

Official IEC shall-text is paywalled — rows below are **capability themes** from public MSC/IHO summaries mapped to our modules. Replace with clause IDs when Ed.4/Ed.5 PDFs are on freeze.

## Legend

| Auto | Lab | Meaning |
|------|-----|---------|
| A | — | Automatable in CTest / fixtures / S-164 scripted where we have data |
| P | — | Partial today (R&D hooks only) |
| — | L | Needs notified-body / lab (HMI, env, BAM, sensors) |
| — | — | Out of Track A until Track B funded |

## Matrix (dual-fuel draft)

| Theme (MSC / IEC 61174 family) | Target module(s) | Phase | Auto vs lab | Notes |
|--------------------------------|------------------|-------|-------------|-------|
| Load official ENC / S-100 cell | `s100_core` `open_dataset`, `iso8211` | P0 | A | S-101 `.000` smoke; S-164 later |
| Display chart / layers | `layer`, `viewer` | P0 | A/P | LAND/DEPARE; official portrayal → P1 |
| Pick / interrogation report | `pick`, future `pick_report` | P0→P1 | A | ≤1e-5°; full object report later |
| Coverage overlay (bathymetry) | `hdf5_product`, `coverage`/`sample` | P1 | A | S-102 |
| Water level / currents + time | `coverage`, time axis | P2 | A/P | S-104 / S-111 |
| Nav warnings (GML) | `products/gml` | P3 | A/P | S-124 |
| Exchange set + sequential updates | `exchange_set` | P1 | A | CATALOG.XML, `.001…` |
| Data protection (S-100 Part 15; S-63 deferred) | `data_mgmt` | Track B | — | Part 15 when needed; S-63 with S-57 only |
| Official portrayal / S-98 IO | `catalogue`, `portrayal` | P1→B | P/L | Lua + IO catalogues |
| Dual-fuel S-57 + S-101 | `products/s57` + S-52 PL | **Deferred** | — | **Out of scope for now** (MIN YE: 先不理会 S-57) |
| Own-ship sensors (61162) | `sensors` | Track B | L | GPS/gyro/log/AIS |
| Route plan / monitor | `route` | Track B | L/A | Anti-grounding claims need S-102/104/129 |
| Alerts + BAM (62923) | `alerts`/`bam` | Track B | L | |
| Presentation day/dusk/night (62288) | `viewer` + portrayal | P1→B | L/P | |
| Independent back-up arrangement | `backup` + ops docs | Track B | L | |
| Environmental / EMC (60945) | hardware | Track B | L | |
| S-64 / S-164 type-approval TDS | Tester harness | B | A→L | Automate smoke; lab owns formal scripts |
| Software identity / cyber | `integrity` | Track B | L | Class cyber (e.g. IACS UR E27) |

## Immediate Track A actions (no window decision needed)

1. Keep P0 hooks stable: `open` / `bounds` / layers / `pick` (see `docs/api-contracts.md`).
2. Real ISO 8211 → revalidate `bounds` + `fixtures.toml`.
3. P1: `sample` for S-102; do not flatten grids into `features`.

## Product decision log

| Date | Decision |
|------|----------|
| 2026-10-10 | MIN YE: **先不理会 S-57** — no S-57 loader, no S-63/S-52 dual-fuel work in current roadmap |

If dual-fuel is revived later: re-add S-57 + S-63 + S-52 Annex A:100 and Ed.4 lab path.

## Explicit non-claim

This matrix does **not** make `s100-chart` type-approved or SOLAS-suitable.
