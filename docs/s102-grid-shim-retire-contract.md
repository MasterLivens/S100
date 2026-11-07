# S-102 `CoverageDataset::grid()` shim retire (rev 1) (Closed — S102-GRID-SHIM-RETIRE)

Owner: SoftwareArchitecture (2026-11-07). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`S102-GRID-SHIM-RETIRE`** (**Closed / Done** 2026-11-07). CTest: reuse **`s100_tests` / `s102_deepen_multigrid` / `s102_uncertainty` / `s102_qbc_meta` / `s102_qbc_multigrid`** (+ `tests/test_s102.cpp` single-grid path) — **do not** invent a second CTest name / `SHIM-*` / `GRIDRETIRE-*` failure-face prefixes. Supersedes Open on tip `ec97b03` (Open CI [`42371854738`](https://github.com/MasterLivens/S100/actions/runs/42371854738)).

**Status: Closed / Done (2026-11-07)** — Eng tip-truth **`14e27bc`**; Eng CI [`42372491173`](https://github.com/MasterLivens/S100/actions/runs/42372491173) dual-green (Ubuntu+Win; `s100_tests` / `s102_*` Passed; **no Win O1 a/b**); Open `ec97b03` / Open CI [`42371854738`](https://github.com/MasterLivens/S100/actions/runs/42371854738); base before Open was LOAD Closed `1d81fc5` (Close CI [`42368822431`](https://github.com/MasterLivens/S100/actions/runs/42368822431)). Tip-truth: migrate `test_s102` + viewer status off `CoverageDataset::grid()` → `grids[0]` / dataset `sample_wgs84`; delete `coverage.hpp` shim; sync api-contracts / deepen; reuse `s100_tests`/`s102_*`; **no** DEEPEN/UNCERT/QBC oracle reopen; bare CHECK / existing `QBC-*`/`BCAST-*`/`QBC-MG-*`. Contract display stays **rev 1**. Next: **TBD**.

**Planner freeze (kept — rev 1 tip-truth):** Reuse case id **`S102-GRID-SHIM-RETIRE`**; CTest reuse **`s100_tests` / `s102_*`** (+ `test_s102`); Eng tip-truth `14e27bc` landed (migrate callers + delete shim + sync docs); keep DEEPEN/UNCERT/QBC tip-truth oracles (semantics/values) — access path only; **no Win O1 a/b**; failure face = bare CHECK / existing labels (do **not** invent `SHIM-*` / `GRIDRETIRE-*`); do **not** reopen DEEPEN/UNCERT/QBC-*/LOAD/DISP-REF/DISPLAY/EDITION/CANCEL/UPD/SIG/FASC/BRIDGE/…; after Closed next **TBD** (do **not** invent). Team stop line (ops): weekly remaining ≤5% — do **not** invent usage numbers here.

---

## Slice close — S102-GRID-SHIM-RETIRE 2026-11-07

**Status: Closed / Done** (formal docs close after Eng tip-truth — **not** a DEEPEN/UNCERT/QBC reopen / loader rewrite).

- Evidence: Eng tip-truth `14e27bc`; Eng CI [42372491173](https://github.com/MasterLivens/S100/actions/runs/42372491173) — Ubuntu+Win dual-green; `s100_tests` / `s102_*` Passed; **no Win O1 a/b**. Open tip `ec97b03` / Open CI [42371854738](https://github.com/MasterLivens/S100/actions/runs/42371854738) dual-green. Base before Open: LOAD Closed `1d81fc5`.
- Tip-truth landed: `tests/test_s102.cpp` + `src/viewer/main.cpp` status string → `grids[0]` / `CoverageDataset::sample_wgs84`; `include/s100/coverage.hpp` `grid()` / `grid() const` **deleted**; api-contracts / deepen soft-synced; tree call sites of `CoverageDataset::grid()` cleared.
- DEEPEN / UNCERT / QBC-META / QBC-MG tip-truth oracle **semantics/values** unchanged (access path only). Core already on `grids[i]` — Reuse.
- Failure face stays bare CHECK / existing `QBC-*` / `BCAST-*` / `QBC-MG-*` — do **not** invent `SHIM-*` / `GRIDRETIRE-*`.
- **Notes:** next **TBD** (architecture / registered Active — do **not** invent). Contract display stays **rev 1**. DEEPEN / UNCERT / QBC-* / LOAD / DISP-REF / DISPLAY / EDITION / CANCEL / UPD / SIG / FASC / BRIDGE already Closed (do **not** reopen).
- Boundaries: ≠ DEEPEN/UNCERT/QBC tip-truth reopen; ≠ QBC O1 hybrid / forbid `grids[0]` hard-bind policy change; ≠ S-104 `fr->grid`; ≠ new `viewer_*` CTest / Win O1 a/b / `ci.yml`.
- **Non-goals this close:** inventing next tip id / `SHIM-*` / `GRIDRETIRE-*` prefixes; reopen DEEPEN/UNCERT/QBC-*/LOAD/DISP-REF/DISPLAY/EDITION/CANCEL/UPD/SIG/FASC/BRIDGE; change QBC bind / portray / Uncertainty VG semantics; touch S-104 `fr->grid`; Win O1 a/b / `ci.yml` / `viewer_*` changes.

---

## MUST (hard fail, exit ≠ 0)

| Check | Severity | Mechanism |
|-------|----------|-----------|
| Zero call sites of `CoverageDataset::grid()` / `grid() const` in tree (tests + viewer + tools + core) | **MUST** | Eng migrate + compile |
| `include/s100/coverage.hpp` no longer declares the two `grid()` overloads | **MUST** | header diff |
| `docs/api-contracts.md` drops “temporary shim `grid()` → `grids[0]` allowed one release only” | **MUST** | docs |
| `docs/s102-deepen-contract.md` Backlog retire sentence points to this tip Closed (or Active→Closed Notes) | **MUST** | docs |
| DEEPEN / UNCERT / QBC-META / QBC-MG tip-truth oracle **semantics/values** unchanged (access path only) | **MUST** | review / CTest |
| Ubuntu + Win native `s100_tests` / `s102_*` dual-green; fixture MUST NOT SKIP; **no** Win O1 a/b | **MUST** | CI |
| No new CTest name / `SHIM-*` / `GRIDRETIRE-*` prefixes | **MUST** | review |

## Out of this tip

| Item | Notes |
|------|-------|
| Reopen DEEPEN / UNCERT / QBC-* tip-truth oracles or loader bind / portray N×CoverageFill | **Forbidden** |
| QBC O1 hybrid / forbid `grids[0]` hard-bind policy change | already Closed — leave |
| S-104/111/124 models; S-104 frame `.grid` field | **OUT** |
| New `viewer_*` CTest / Win O1=(a) / `ci.yml` change | **OUT** |
| Invent next tip id after Closed | **OUT** |

## Predecessor / Reuse vs tip-truth

| Item | Role |
|------|------|
| DEEPEN Closed `5e05675` / CI `39141930422` | O1=shim Backlog origin |
| UNCERT / QBC-META / QBC-MG Closed | callers migrated here |
| Core `hdf5_product` / `portrayal_engine` | already `grids[i]` — Reuse |
| LOAD Closed `1d81fc5` / Close CI `42368822431` | Open base |
| Eng tip-truth `14e27bc` / Eng CI `42372491173` | Landed migrate + delete shim — **Closed** this tip |

## Win / CI policy

| Path | Win @ Closed Eng tip `14e27bc` | Notes |
|------|--------------------------------|-------|
| `s100_tests` / `s102_*` / `test_s102` | **no O1 a/b** | native; Ubuntu+Win same |
| `viewer_*` / golden / disp-ref / display_compare | untouched | do **not** pull into this tip |
| `.github/workflows/ci.yml` | **no change** | Win `viewer_*` exclude stays |

## Docs sync

- Case id in `docs/testing/iho-ctest-case-ids.md` — **Closed / Done** this tip.
- `docs/architecture.md` **next** → **TBD**.
- Soft sync: `docs/api-contracts.md` + `docs/s102-deepen-contract.md` retire → **Closed / Done**.

## Non-goals

Reopen DEEPEN / UNCERT / QBC-META / QBC-MG / S-104 / S-111 / S-124 / LOAD-0004..0010 / DISP-REF / DISPLAY / EDITION / CANCEL / UPD-* / SIG* / FASC / BRIDGE / POWERUP* / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE; change QBC bind / portray / Uncertainty VG semantics; invent second CTest / `SHIM-*` / `GRIDRETIRE-*` / Closed-next tip name; touch S-104 `fr->grid`; new `viewer_*` CTest / Win O1 a/b / `ci.yml` change.
