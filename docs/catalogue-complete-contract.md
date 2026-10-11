# Official FC/PC completeness (rev 3)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-CATALOGUE-COMPLETE`** (Active). CTest: **`catalogue_provenance`**.

## Goal

Vendor official catalogues for S-104 / S-111 / S-124 with pinned paths and sha256. **Never invent a Portrayal Catalogue.** S-104 Ed.2.0.0 officially has **no PC** (IHO GI Registry / Product Spec §9).

## Unchanged

S-101 FC+PC 2.0.0 and S-102 FC+PC 3.0.0 stay pinned; do not bump.

## MUST vendor — on-disk names

| Product | FC (MUST) | PC |
|---------|-----------|-----|
| S-104 | `catalogues/S-104/S-104_FC_2.0.0.xml` | **Official N/A** — do **not** create `PC_2.0.0/`. SOURCE.md states N/A + PS cite. `catalogue_provenance` **skips PC sha/pairing** for S-104 only (STATUS/WARN log ok; not FATAL). |
| S-111 | `catalogues/S-111/S-111_FC_2.0.0.xml` | `catalogues/S-111/PC_2.0.0/portrayal_catalogue.xml` (+ tree) **MUST** |
| S-124 | `catalogues/S-124/S-124_FC_2.0.0.xml` | `catalogues/S-124/PC_2.0.0/portrayal_catalogue.xml` (+ tree) **MUST** |

## Provenance FATAL rules

| Check | S-104 | S-111 / S-124 |
|-------|-------|----------------|
| FC file missing / sha mismatch | FATAL | FATAL |
| FC versionNumber present | FATAL if empty | FATAL if empty |
| PC file missing / sha / FC↔PC version pair (`productId` + `version` == FC `versionNumber`) | **N/A — do not FATAL** | FATAL |
| Unexpected S-104 `PC_2.0.0/` present | FATAL (forged PC) | — |

S-101/S-102 checks unchanged.

## SOURCE.md

Each product folder: upstream URL/commit, date, sha256 of required files, **unsigned** note + redistribution terms.  
S-104 SOURCE MUST quote that PC is not defined for Ed.2.0.0 and list FC sha only.

## Portray

Minimal tokenized / engine portray for S-104 remains OK. Wiring S-111/S-124 loaders to new PC is WARN follow-up.

## Supersedes

Rev 2 requirement of S-104 PC path. Product contracts updated accordingly.

## Non-goals

Forged S-104 PC, S-127/131, signed MED packs, S-101 2.1.0.
