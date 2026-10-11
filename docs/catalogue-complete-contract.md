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

**File MUST exist** at `catalogues/S-104|S-111|S-124/SOURCE.md`.

**Content SHOULD include** (Tester treats missing fields as BLOCK on review):

- Upstream URL / registry or git tag/commit  
- Fetch date  
- sha256 of every **required** file for that product  
- **unsigned** R&D note + upstream redistribution terms  
- Edition **2.0.0** claim  

S-104 SOURCE additionally MUST state PC is **officially N/A** for Ed.2.0.0 (PS §9 / GI Registry) and list **FC sha only** (no PC sha row).

## Portray

Minimal tokenized / engine portray for S-104 remains OK. S-111/S-124 **PC wiring** is Active: `docs/pc-wire-contract.md` (`IHO-PC-WIRE-111-124`); XSLT engine for S-111 remains WARN.

## Supersedes

Rev 2 requirement of S-104 PC path. Product contracts updated accordingly.


## Extend `tests/check_catalogues.cmake` (align existing style)

Keep current S-101/S-102 `expect_sha` + `versionNumber` / `productId…version=` pairing.

Add:

```cmake
# S-104 FC only; FATAL if PC_2.0.0/ appears
expect_sha("${C}/S-104/S-104_FC_2.0.0.xml" <from SOURCE>)
# read FC versionNumber — non-empty
if(EXISTS "${C}/S-104/PC_2.0.0")
  message(FATAL_ERROR "S-104 must not ship PC_2.0.0 (official N/A)")
endif()

# S-111 and S-124: expect_sha FC + PC xml; pair FC versionNumber == PC productId/version
```

No `SKIP` for missing required files — missing path fails SHA/read → FATAL.

## Non-goals

Forged S-104 PC, S-127/131, signed MED packs, S-101 2.1.0.
