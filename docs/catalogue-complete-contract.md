# Official FC/PC completeness (next slice)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-CATALOGUE-COMPLETE`**. CTest: extend **`catalogue_provenance`** (keep name; add asserts).

## Goal

Pin **official** Feature Catalogues + Portrayal Catalogues for every product we already load (S-101…S-124), with SOURCE/commit/SHA and version pairing. `catalogue_provenance` stays green and grows coverage. No silent hand-edited catalogues.

## Already done (must remain)

| Product | FC | PC | Pairing |
|---------|----|----|---------|
| S-101 | 2.0.0 | 2.0.0 | already pinned |
| S-102 | 3.0.0 | 3.0.0 | already pinned |

Do not bump S-101/S-102 editions in this slice.

## Missing — MUST vendor this slice

| Product | Target editions (IHO in-force / matching product spec we claim) | Layout |
|---------|------------------------------------------------------------------|--------|
| S-104 | FC + PC matching **Ed.2.0.0** water level (pin exact filenames in SOURCE) | `catalogues/S-104/` + `SOURCE.md` |
| S-111 | FC + PC matching **Ed.2.0.0** surface current | `catalogues/S-111/` + `SOURCE.md` |
| S-124 | FC + PC matching **Ed.2.0.0** navigational warnings | `catalogues/S-124/` + `SOURCE.md` |

If an official combo is only published under a different patch label, pin that label and document why (same major.minor as product claim).

## Rules

1. **SOURCE.md** per product: upstream URL/repo, tag or commit, date fetched, sha256 of FC XML + PC `portrayal_catalogue.xml` (and note if whole PC tree hashed).
2. **Unsigned OK** for R&D (same as S-101); note “unsigned; replace with signed for type approval”.
3. **Read-only** in git after land — no drive-by Lua edits.
4. **Version pairing:** FC `versionNumber` == PC declared FC dependency / same edition string; CTest fails on mismatch (mirror S-101/S-102 checks).
5. **Loaders:** S-104/S-111/S-124 open paths may keep minimal portrayal if PC wiring is large; this slice’s MUST is **files on disk + provenance CTest**. Wiring portray to new PC is **WARN/follow-up** unless already trivial.
6. Windows `.gitattributes` already forces binary/LF under `catalogues/` — keep it.

## MUST / WARN

| Check | Sev |
|-------|-----|
| Three new SOURCE.md + FC/PC paths exist | MUST |
| sha256 matches SOURCE for each new FC + PC xml | MUST |
| FC/PC version pairing for S-104/111/124 | MUST |
| Existing S-101/S-102 provenance still green | MUST |
| Portray engines consume new PC | WARN (follow-up OK) |

## Non-goals

S-127/S-131, signed IHO packs for MED, S-98 IO catalogues, bumping S-101 to 2.1.0.
