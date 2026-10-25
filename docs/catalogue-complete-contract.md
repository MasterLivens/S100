# Official FC/PC completeness (rev 3) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-CATALOGUE-COMPLETE`** (**Active / Open**). CTest: **`catalogue_provenance`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev 3** — Planner freeze (MUST below). Official FC/PC vendor + sha/pair + SOURCE.md slice after closed **`IHO-TIMELINE-UI`**. Does **not** reopen TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; does **not** jump **`S102-GRID-SHIM-RETIRE`**.

Builds on `docs/api-contracts.md` (Catalogues) / `docs/pc-wire-contract.md` (Closed; vendor/sha → CATALOGUE separate) / `docs/s104-contract.md` / `docs/s111-contract.md` / `docs/s124-contract.md` (all Closed; FC/PC on disk cross). Wiring → **PC-WIRE** already Closed — **do not** reopen. Scrubber → **TIMELINE** already Closed — **do not** reopen. **Never invent S-104 PC**.

Primary research: `/workspace/research/catalogue-complete/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`161ef68`**.

**Status: Active / Open (2026-10-25)** — after **TIMELINE Closed** (tip `3154629` ≡ Arch `f8cfd5b`; tip Close CI [`40169217917`](https://github.com/MasterLivens/S100/actions/runs/40169217917) dual-green; Open-bind evidence CI [`40167445485`](https://github.com/MasterLivens/S100/actions/runs/40167445485) @ `c29b8ba`≡`feab8a0`, O1=tip-truth/docs-close). Eng tip ancestor **`161ef68`** already on tip — **not greenfield** (S-104 FC-only + **FATAL if `PC_2.0.0/`** + S-111/S-124 FC+PC sha+pair FATAL + SOURCE.md×3 + S-101/S-102 unchanged + no SKIP; wiring → **PC-WIRE** already Closed; scrubber → **TIMELINE** already Closed). Ubuntu+Windows already Passed — tip Close CI [`40169217917`](https://github.com/MasterLivens/S100/actions/runs/40169217917) `#26 catalogue_provenance` + Eng landing [`38129619461`](https://github.com/MasterLivens/S100/actions/runs/38129619461) @ `161ef68` (cmake `-P` file/sha/pair — **not** `viewer_*` — **no Win O1 (a)/(b)**). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng / not re-vendor. Independent tip from TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Do **not** invent case ids or next tip names; freeze tip-truth below. Do **not** reopen TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Do **not** forge S-104 PC. Do **not** invent `CATALOGUE-*` failure-face prefixes (keep cmake `FATAL_ERROR`). Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). Contract display header stays **rev 3** (do **not** bump just for Open). After Closed next: **TBD** (architecture / registered Active only — do **not** invent). Portray「PC wiring is Active」→ note on Close that PC-WIRE already Closed (do **not** reopen).

**Planner freeze (MUST — from BRIEF §7 + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-CATALOGUE-COMPLETE`** / CTest **`catalogue_provenance`**; **forbidden** to invent a second case/test name.
- MUST tip-truth = S-104 FC-only (`catalogues/S-104/S-104_FC_2.0.0.xml` sha + versionNumber; **FATAL if `PC_2.0.0/`**; skip PC sha/pairing) + S-111/S-124 FC+PC sha + FC↔PC version pair FATAL + SOURCE.md MUST exist (S-104 declares PC N/A) + S-101/S-102 unchanged + **MUST NOT SKIP**; never invent S-104 PC; S-111 PC = TWCWG XSLT 2.0.0 (**not** Lua 2.0.1); wiring → **PC-WIRE** already Closed (do **not** reopen); scrubber → **TIMELINE** already Closed (do **not** reopen); XSLT host → **WARN**.
- Eng tip already has **`161ef68`**; Ubuntu+Win (tip `40169217917` `#26` / Eng `38129619461`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield / not re-vendor.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog** (DEEPEN O1); **not** this tip.
- Failure face = cmake `FATAL_ERROR` (sha / pair / forged S-104 PC); do **not** invent `CATALOGUE-*` prefixes / new CTest names.
- Ubuntu + Windows: `catalogue_provenance` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`; cmake `-P`. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `catalogue_provenance` MUST NOT SKIP, Passed (already tip CI `40169217917` `#26` / Eng `38129619461`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng / re-vendor required
- Portray「PC wiring is Active」→ PC-WIRE **Closed** (cite Closed tip; do **not** reopen)
- After Close next: **TBD**

**Eng notes:** verify `161ef68` on tip; vendor trees `catalogues/S-104|S-111|S-124/` + `SOURCE.md`×3; driver `tests/check_catalogues.cmake`; cmake `catalogue_provenance`. TIMELINE Closed already on this stack (`3154629`). Do **not** reopen TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. XSLT host **WARN-only** — do **not** harden. Do **not** forge S-104 PC. Do **not** bump S-101/S-102 pins. Do **not** re-vendor.

**O-questions (BRIEF §3 Open 1–N — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `161ef68` already; no `ci.yml` / no large Eng / no re-vendor) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` (cmake `-P`) — **yes / freeze**.
3. Contract head → Open; keep display **rev 3** (do **not** bump just for Open); Portray「PC wiring Active」收口 on Close → PC-WIRE Closed — **yes / freeze**.
4. case-ids Active → Open (Status stays Active family) — **yes / freeze**.
5. Notes ≠ TIMELINE-UI (already Closed; do **not** reopen); ≠ PC-WIRE (already Closed; do **not** reopen); ≠ S-124 / S-111 / S-104 open; do **not** reopen QBC-MG / UNCERT / DEEPEN; shim-retire stays Backlog; XSLT host stays WARN; no forge S-104 PC — **yes / freeze**.
6. Closed next → **TBD** (architecture / registered Active; do **not** invent) — **yes / freeze**.
7. No other cases this tip (do **not** Closed SESSION-PICK / SIG / POWERUP / etc.) — **yes / freeze**.
8. architecture.md flip **next** to this Open — **yes / freeze**.
9. `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog — **yes / freeze**.
10. Failure face = cmake `FATAL_ERROR`; do **not** invent `CATALOGUE-*` prefixes / new CTest — **yes / freeze**.
11. vendor/sha/SOURCE = this tip; wiring = PC-WIRE Closed; scrubber = TIMELINE Closed; product open = each Closed tip — **yes / freeze**.

---

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
