# Official FC/PC completeness (rev 2)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-CATALOGUE-COMPLETE`** (Active). CTest: **`catalogue_provenance`** (extend `tests/check_catalogues.cmake`).

## Goal

Vendor official FC+PC for S-104 / S-111 / S-124 with pinned paths, sha256, and edition pairing. Missing file / bad hash / edition mismatch → **`message(FATAL_ERROR)`** (no SKIP). Portray wiring to new PC remains WARN/follow-up; this slice does **not** require `open_dataset` LoadError when catalogues absent (files must be present in CI tree).

## Unchanged

| Product | Paths | Pairing |
|---------|-------|---------|
| S-101 | `S-101/S-101_FC_2.0.0.xml`, `S-101/PC_2.0.0/portrayal_catalogue.xml` | FC `versionNumber` == PC `productId="S-101" version="…"` |
| S-102 | `S-102/S-102_FC_3.0.0.xml`, `S-102/PC_3.0.0/portrayal_catalogue.xml` | same pattern for S-102 |

Do not bump these editions.

## MUST vendor — exact on-disk names

| Product | FC path | PC path |
|---------|---------|---------|
| S-104 | `catalogues/S-104/S-104_FC_2.0.0.xml` | `catalogues/S-104/PC_2.0.0/portrayal_catalogue.xml` |
| S-111 | `catalogues/S-111/S-111_FC_2.0.0.xml` | `catalogues/S-111/PC_2.0.0/portrayal_catalogue.xml` |
| S-124 | `catalogues/S-124/S-124_FC_2.0.0.xml` | `catalogues/S-124/PC_2.0.0/portrayal_catalogue.xml` |

Also ship full PC tree assets as published (Lua/symbols/…) under each `PC_2.0.0/`; provenance MUST hash at least the `portrayal_catalogue.xml` (and FC xml). Optional: tree digest in SOURCE.

## Pairing (align `check_catalogues.cmake`)

For each of S-104/111/124:

1. `expect_sha` on FC + PC xml vs SOURCE.md  
2. Read FC `<S100FC:versionNumber>` (or product-equivalent tag if schema differs — prefer S100FC)  
3. Read PC `productId="S-10X" version="…"`  
4. Require both non-empty and **equal** → else `FATAL_ERROR`  
5. Missing file → CMake/`file(SHA256)` failure → FATAL (not SKIP)

## SOURCE.md (SHOULD content, required file)

Each of `catalogues/S-104|S-111|S-124/SOURCE.md`:

- Upstream URL / IHO registry or GitHub tag/commit  
- Fetch date  
- sha256 of FC xml + PC `portrayal_catalogue.xml`  
- **unsigned** R&D note + link/terms of upstream redistribution  
- Edition 2.0.0 claim

## MUST / WARN

| Check | Sev |
|-------|-----|
| Paths exist; sha match; edition pair for 104/111/124 | MUST (`catalogue_provenance` FATAL on fail) |
| S-101/S-102 still green | MUST |
| Engines consume new PC / open_dataset requires catalogues | **WARN** follow-up |

## Supersedes

Product-slice WARN rows that said “official FC/PC may defer” in `docs/s104-contract.md`, `docs/s111-contract.md`, `docs/s124-contract.md` are **replaced by this MUST**. Minimal tokenized portray may continue until PC wiring lands.

## Non-goals

S-127/131, signed MED packs, S-101→2.1.0, S-98 IO catalogues.
