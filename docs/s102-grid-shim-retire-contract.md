# S-102 `CoverageDataset::grid()` shim retire (rev 1) (Active / Open — S102-GRID-SHIM-RETIRE)

Owner: SoftwareArchitecture (2026-11-07). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`S102-GRID-SHIM-RETIRE`** (**Active / Open** 2026-11-07). CTest: reuse **`s100_tests` / `s102_deepen_multigrid` / `s102_uncertainty` / `s102_qbc_meta` / `s102_qbc_multigrid`** (+ `tests/test_s102.cpp` single-grid path) — **do not** invent a second CTest name / `SHIM-*` / `GRIDRETIRE-*` failure-face prefixes.

**Status: Active / Open (2026-11-07)** — base **`1d81fc5`** (IHO-S101-LOAD-0004..0010 Closed / Close CI [`42368822431`](https://github.com/MasterLivens/S100/actions/runs/42368822431) dual-green). Promoted from Backlog: DEEPEN/UNCERT/QBC-META/QBC-MG all Closed → **eligible** (no longer “insert ahead of Active S-102”). **needs Eng** (not docs-close-only): migrate callers off `CoverageDataset::grid()` → delete header shim → sync api-contracts / deepen. Contract display stays **rev 1**. After Closed next: **TBD** (do **not** invent).

**Planner freeze (kept — rev 1):** Reuse case id **`S102-GRID-SHIM-RETIRE`**; CTest default reuse existing **`s100_tests` / `s102_*`** (+ `test_s102`); **needs Eng** migrate `tests/test_s102.cpp` (~30×) + `src/viewer/main.cpp` (2× status string) then delete `include/s100/coverage.hpp` `grid()` / `grid() const`; sync `docs/api-contracts.md` + `docs/s102-deepen-contract.md`; keep DEEPEN/UNCERT/QBC tip-truth oracles (semantics/values) — access path only; **no Win O1 a/b** (native; do **not** touch `viewer_*` / `ci.yml`); failure face = bare CHECK / existing `QBC-*` / `BCAST-*` / `QBC-MG-*` (do **not** invent `SHIM-*` / `GRIDRETIRE-*`); do **not** reopen DEEPEN/UNCERT/QBC-*/LOAD/DISP-REF/DISPLAY/EDITION/CANCEL/UPD/SIG/FASC/BRIDGE/…; after Closed next **TBD**. Team stop line (ops): weekly remaining ≤5% — do **not** invent usage numbers here.

---

## Arch Open decisions (frozen this tip)

| # | Decision |
|---|----------|
| 1 | **needs Eng = yes** — migrate **tests + viewer status string**, then delete shim. Core (`hdf5_product` / `portrayal_engine`) already on `grids[i]` / dataset `sample_wgs84` — **not** tip-truth alone. |
| 2 | **Migration map:** (a) field/property access (`nrows`/`ncols`/`origin_*`/`spacing_*`/`depth`/`uncertainty`/`qbc_ids`/`horizontal_crs`/`sample_cell`/…) → **`grids[0].…`**; (b) geo sample that used `coverage->grid().sample_wgs84` → prefer **`coverage->sample_wgs84`** (dataset envelope router); single-grid fixtures may use `grids[0].sample_wgs84` only when asserting grid-local behaviour; (c) delete any assert whose sole purpose was `grid() ≡ grids[0]`; (d) **do not** touch S-104 `fr->grid` / WaterLevel frame fields. |
| 3 | **CTest:** reuse existing suite — **no** new `add_test` name. Dual-green Ubuntu+Win on native `s100_tests` / `s102_*` after Eng. |
| 4 | **Proof shim gone:** compile-clear (zero `CoverageDataset::grid()` callers) + remove “temporary shim … one release only” from api-contracts. No new static_assert / negative CTest required. |
| 5 | **Contract:** this short file + sync api-contracts / deepen Notes (see Docs sync). |
| 6 | **api-contracts soft:** Coverage QBC “Active” wording → **Closed** pointer (do **not** reopen QBC tip-truth). |
| 7 | **Win:** **no O1 a/b**; viewer status-string edits only — **no** new `viewer_*` CTest. |
| 8 | **case-ids:** Backlog → **Active / Open** this tip → Closed on docs-close. |
| 9 | **Closed next:** **TBD** — do **not** invent. |
| 10 | **Failure face:** bare CHECK / existing oracle labels only. |

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
| UNCERT / QBC-META / QBC-MG Closed | callers still use `.grid()` — migrate here |
| Core `hdf5_product` / `portrayal_engine` | already `grids[i]` — Reuse, not this tip’s Eng body alone |
| LOAD Closed `1d81fc5` / Close CI `42368822431` | Open base |

## Win / CI policy

| Path | Win | Notes |
|------|-----|-------|
| `s100_tests` / `s102_*` / `test_s102` | **no O1 a/b** | native; Ubuntu+Win same |
| `viewer_*` / golden / disp-ref / display_compare | untouched | do **not** pull into this tip |
| `.github/workflows/ci.yml` | **no change** | Win `viewer_*` exclude stays |

## Docs sync

- Case id in `docs/testing/iho-ctest-case-ids.md` — **Active / Open** this tip (removed from Backlog table).
- `docs/architecture.md` **next** → **Active / Open** `S102-GRID-SHIM-RETIRE`.
- On Closed: api-contracts + deepen Backlog retire → Done; architecture next → **TBD**.

## Non-goals

Reopen DEEPEN / UNCERT / QBC-META / QBC-MG / S-104 / S-111 / S-124 / LOAD-0004..0010 / DISP-REF / DISPLAY / EDITION / CANCEL / UPD-* / SIG* / FASC / BRIDGE / POWERUP* / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE; change QBC bind / portray / Uncertainty VG semantics; invent second CTest / `SHIM-*` / `GRIDRETIRE-*` / Closed-next tip name; touch S-104 `fr->grid`; new `viewer_*` CTest / Win O1 a/b / `ci.yml` change.
