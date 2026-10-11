# S-101 Feature Catalogue

| Field | Value |
|-------|-------|
| File | `S-101_FC_2.0.0.xml` |
| Product / version | S-101, FC `versionNumber` 2.0.0, `versionDate` 2024-10-16 |
| Source | IHO official repo [iho-ohi/S-101-Documentation-and-FC](https://github.com/iho-ohi/S-101-Documentation-and-FC), path `S-101FC/FeatureCatalogue.xml` (branch `main`, fetched 2026-10-11) |
| SHA-256 | `2743d0689d8a2130f9b809554591d794f01cd9953f222f065ac891b590bd3cfc` |
| Matches data | IHO TDS cells (`INT.IHO.S-101.2.0.0`) and CHS cells (`INT.IHO.S-101.2.0`) |

Unsigned copy. For type approval, replace with the IHO-signed exchange-set catalogue of the
same edition and record its signature here. FerriteS100's copy is reference only.

## S-101 Portrayal Catalogue 2.0.0 (`PC_2.0.0/`)

- Source: https://github.com/iho-ohi/S-101_Portrayal-Catalogue, tag `v2.0.0` (commit b20cef5234b787bebb497e1f0c852142357f3757), directory `PortrayalCatalog/` copied unchanged.
- `portrayal_catalogue.xml`: productId="S-101" version="2.0.0", schema S100PortrayalCatalog/5.2; sha256 a6165e950084dbb4ee6f75144a467f398809d57d8548d8d10ae80d715ac37962
- Tree hash (sha256 of sorted per-file sha256 list): 61100a9dd86459cf2800e3d1250540ba11896d8a28372f2bbc06d0c54a6f54e5; 1023 files, 215 Lua rule files.
- Pairing: the PC README says "2.0.0 - use with FC 2.0.0", which matches `S-101_FC_2.0.0.xml` above.
- Not used: `main` (8ebc687, 2026-10-09) is PC 2.1.0 "use with FC 2.1.0 (July 2026)", but the official FC repo (iho-ohi/S-101-Documentation-and-FC, d0dcb87) still publishes FC 2.0.0. Moving to 2.1.0 means upgrading FC and PC together.
- Fetched 2026-10-11, unsigned (no IHO digital signature checked).
