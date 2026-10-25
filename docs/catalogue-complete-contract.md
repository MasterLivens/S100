# Official FC/PC completeness (rev 3) (Closed)

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/api-contracts.md` (Catalogues) / `docs/pc-wire-contract.md` (Closed; vendor/sha → CATALOGUE separate) / `docs/s104-contract.md` / `docs/s111-contract.md` / `docs/s124-contract.md` (all Closed; FC/PC on disk cross) / `docs/timeline-ui-contract.md` (Closed; scrubber → TIMELINE separate). Case id: **`IHO-CATALOGUE-COMPLETE`** (**Closed / Done** 2026-10-25).  
CTest name: **`catalogue_provenance`** — MUST NOT SKIP; Linux + Windows dual-green (CI `40170639650` @ `0ba61fb`). Supersedes Open draft on tip `0ba61fb` (≡ Arch `e2a3d43`).

Primary research: `/workspace/research/catalogue-complete/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`161ef68`**. Wiring → **PC-WIRE** already Closed — **do not** reopen. Scrubber → **TIMELINE** already Closed — **do not** reopen. **Never invent S-104 PC**. XSLT host → **WARN** (do **not** harden).

**Status: Closed / Done (2026-10-25)** — CI [`40170639650`](https://github.com/MasterLivens/S100/actions/runs/40170639650) @ `0ba61fb`≡`e2a3d43` dual-green; **`catalogue_provenance` Passed Ubuntu+Win** (cmake `-P` file/sha/pair — **not** `viewer_*`; **no Win O1 (a)/(b)**). Eng **`161ef68`** already on tip — **docs close only**. MUST tip-truth **unchanged** (S-104 FC-only / **FATAL if `PC_2.0.0/`** / skip PC sha+pair; S-111/S-124 FC+PC sha + FC↔PC version pair FATAL; SOURCE.md×3; S-101/S-102 unchanged; **MUST NOT SKIP**). **O1=tip-truth / docs-close SUCCESS**. Failure face = cmake `FATAL_ERROR` tip-truth (do **not** invent `CATALOGUE-*`). Do **not** reopen TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog. Contract display stays **rev 3**. Next: **TBD**.

**Planner freeze (kept — rev 3 tip-truth):** Reuse case id **`IHO-CATALOGUE-COMPLETE`** / CTest **`catalogue_provenance`**; MUST tip-truth = S-104 FC-only (`catalogues/S-104/S-104_FC_2.0.0.xml` sha + versionNumber; **FATAL if `PC_2.0.0/`**; skip PC sha/pairing) + S-111/S-124 FC+PC sha + FC↔PC version pair FATAL + SOURCE.md MUST exist (S-104 declares PC N/A) + S-101/S-102 unchanged + **MUST NOT SKIP**; never invent S-104 PC; S-111 PC = TWCWG XSLT 2.0.0 (**not** Lua 2.0.1); wiring → **PC-WIRE** already Closed (do **not** reopen); scrubber → **TIMELINE** already Closed (do **not** reopen); XSLT host → **WARN**; Eng `161ef68` already; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog; failure face = cmake `FATAL_ERROR` (do **not** invent `CATALOGUE-*` prefixes / new CTest); after Closed next **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.

**O1 closed as tip-truth / docs-close:** Eng `161ef68` already on tip (S-104 FC-only + FATAL if `PC_2.0.0/` + S-111/S-124 FC+PC sha+pair FATAL + SOURCE.md×3 + S-101/S-102 unchanged + no SKIP; cmake `-P` `tests/check_catalogues.cmake`; CTest **`catalogue_provenance`**). Slice = Arch docs close only — not greenfield Eng / not re-vendor. Do **not** invent case ids or next tip names; do **not** reopen TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** invent `CATALOGUE-*` prefixes; do **not** retire `grid()` this tip; do **not** forge S-104 PC; do **not** harden XSLT host; do **not** bump S-101/S-102 pins. **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest; inventing oracles / next tip id; forging S-104 PC; re-vendor; inventing `CATALOGUE-*` failure-face prefixes.

---

## Slice close — 2026-10-25

**Status: Closed / Done** (formal docs close — **not** a catalogue / vendor rewrite).

- Evidence: CI [40170639650](https://github.com/MasterLivens/S100/actions/runs/40170639650) tip `0ba61fb`≡`e2a3d43` — Ubuntu + Windows **`catalogue_provenance` Passed** (MUST NOT SKIP). Ancestor greens also Eng landing [`38129619461`](https://github.com/MasterLivens/S100/actions/runs/38129619461) @ `161ef68` + prior tip `#26 catalogue_provenance` Passed.
- **O1=tip-truth / docs-close SUCCESS** — Eng `161ef68` already on tip; Arch Open→Closed only; Planner BRIEF §7 answers frozen.
- MUST tip-truth unchanged (S-104 FC-only / **FATAL if `PC_2.0.0/`** / skip PC sha+pair; S-111/S-124 FC+PC sha + FC↔PC version pair FATAL; SOURCE.md×3; S-101/S-102 unchanged; **MUST NOT SKIP**).
- Eng `161ef68` already on tip — docs close only; no `ci.yml` change; **no Win (a)/(b)**; no large Eng; no re-vendor.
- Failure face stays cmake `FATAL_ERROR` tip-truth — do **not** invent `CATALOGUE-*`.
- **Notes:** next **TBD** (architecture / registered Active — do **not** invent a next tip id). **Backlog:** `grid()` shim retire (`S102-GRID-SHIM-RETIRE`) — do not insert ahead. Contract display stays **rev 3**. Wiring → PC-WIRE already Closed (do **not** reopen); scrubber → TIMELINE already Closed (do **not** reopen); XSLT host stays WARN; no forge S-104 PC.
- Boundaries: independent; ≠ PC-WIRE (already Closed); ≠ TIMELINE (already Closed); do **not** reopen TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.
- **Non-goals this close:** inventing oracles / next tip id; retiring `grid()` shim; catalogue/vendor rewrite; reopening TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; Win (a)/(b) (N/A — non-`viewer_*`); inventing `CATALOGUE-*` failure-face prefixes; forging S-104 PC; hardening XSLT host; bumping S-101/S-102 pins; re-vendor.

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

Minimal tokenized / engine portray for S-104 remains OK. S-111/S-124 **PC wiring** already **Closed** via `docs/pc-wire-contract.md` (`IHO-PC-WIRE-111-124`) — do **not** reopen. Scrubber already **Closed** via `docs/timeline-ui-contract.md` (`IHO-TIMELINE-UI`) — do **not** reopen. XSLT engine for S-111 remains WARN.

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
