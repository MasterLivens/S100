# S-101 Sec.4–10 load samples (rev 1) (Closed — IHO-S101-LOAD-0004..0010)

Owner: SoftwareArchitecture (2026-11-07). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S101-LOAD-0004..0010`** (**Closed / Done** 2026-11-07). CTest: reuse **`s100_tests` / `test_load`** (extend `test_load_samples` / `test_load` — **do not** invent a second CTest name). Supersedes Open draft on tip `414da29` (≡ `18903b0`; Open CI [`42367661814`](https://github.com/MasterLivens/S100/actions/runs/42367661814)).

**Status: Closed / Done (2026-11-07)** — Eng tip-truth **`66b0c70`**; Eng CI [`42368360920`](https://github.com/MasterLivens/S100/actions/runs/42368360920) dual-green (Ubuntu+Win; `s100_tests` Passed; **no Win O1 a/b**); Open `414da29`≡`18903b0` / Open CI [`42367661814`](https://github.com/MasterLivens/S100/actions/runs/42367661814); base before Open was DISP-REF Closed `9ab5851` (Close CI [`40293243584`](https://github.com/MasterLivens/S100/actions/runs/40293243584)). Tip-truth: vendor cells `0004/0005/0007/0009/0010` + README/SOURCE; `test_load_samples` MUST per new cell; reuse `s100_tests`/`test_load`; **no** engine API change; portray/layers loops **OUT**; bare CHECK; `S102-GRID-SHIM-RETIRE` stays Backlog. Contract display stays **rev 1**. Next: **TBD**.

**Planner freeze (kept — rev 1 tip-truth):** Reuse single case id **`IHO-S101-LOAD-0004..0010`**; CTest reuse **`s100_tests` / `test_load`**; expand `testdata/s101` Sec.4–10 samples + MUST load cover; **do not** change `open_dataset` / `load_s101` / Dataset public API; Eng tip-truth `66b0c70` landed (vendor five + README/SOURCE + `test_load_samples` MUST); Reuse 0003/0006/0008 kept; exclude 0001/0002/0011+; hardcoded multi-cell portray/list loops **OUT**; **no Win O1 a/b**; failure face = bare CHECK (do **not** invent `LOAD4-*` / `LOADSAMPLES-*`); do **not** reopen DISP-REF/DISPLAY/EDITION/CANCEL/UPD/SIG/FASC/BRIDGE; shim Backlog; after Closed next **TBD** (do **not** invent). Team stop line (ops): stop when weekly remaining ≤5% — do **not** invent usage numbers in this contract.

---

## Slice close — IHO-S101-LOAD-0004..0010 2026-11-07

**Status: Closed / Done** (formal docs close after Eng tip-truth — **not** a DISP-REF reopen / engine API change).

- Evidence: Eng tip-truth `66b0c70`; Eng CI [42368360920](https://github.com/MasterLivens/S100/actions/runs/42368360920) — Ubuntu+Win dual-green; `s100_tests` Passed; **no Win O1 a/b**. Open tip `414da29`≡`18903b0` / Open CI [42367661814](https://github.com/MasterLivens/S100/actions/runs/42367661814) dual-green. Base before Open: DISP-REF Closed `9ab5851`.
- Tip-truth landed: vendor `101AA00DS0004/0005/0007/0009/0010.000` + README/SOURCE; `test_load_samples` MUST open / `feature_count > 0` / bounds valid per new cell; 0008 smoke kept; reuse `s100_tests`/`test_load`.
- Engine `open_dataset` / `load_s101` / Dataset public API unchanged. Portray/layers multi-cell loops untouched (**OUT**).
- Failure face stays bare CHECK — do **not** invent `LOAD4-*` / `LOADSAMPLES-*`.
- **Notes:** next **TBD** (architecture / registered Active — do **not** invent). **Backlog:** `grid()` shim retire (`S102-GRID-SHIM-RETIRE`) — do not insert ahead. Contract display stays **rev 1**. DISP-REF / DISPLAY / EDITION / CANCEL / UPD / SIG / FASC / BRIDGE already Closed (do **not** reopen).
- Boundaries: ≠ DISP-REF-COMPARE (already Closed — **do not** reopen); ≠ engine load API shape change; ≠ files-only MVP without load MUST cover.
- **Non-goals this close:** inventing next tip id / `LOAD4-*` prefixes; reopen DISP-REF/DISPLAY/EDITION/CANCEL/UPD/SIG/FASC/BRIDGE; change `open_dataset`/`load_s101`; expand hardcoded multi-cell portray/list loops; vendor 0001/0002/0011+; insert `S102-GRID-SHIM-RETIRE` ahead; Win O1 a/b / `ci.yml` / `viewer_*` changes.

---

## MUST (hard fail, exit ≠ 0)

| Check | Severity | Mechanism |
|-------|----------|-----------|
| Five cells present under `testdata/s101/`: `101AA00DS0004.000` / `0005.000` / `0007.000` / `0009.000` / `0010.000` (from public IHO TDS) | **MUST** | path existence / Eng vendor |
| `testdata/s101/README.md` + `SOURCE.txt` list new cells (keep "Test only — not for navigation"; align SOURCE path style) | **MUST** | docs / provenance |
| `test_load` / `test_load_samples` (under `s100_tests`) opens **each** of 0004/0005/0007/0009/0010; `feature_count > 0`; bounds valid | **MUST** | CTest / bare CHECK |
| Reuse LOAD-0003 / 0006 / 0008 load assertions still green (do not regress preferred 0008 smoke) | **MUST** | existing `test_load` |
| Engine `open_dataset` / `load_s101` / Dataset public API unchanged | **MUST** | review / no API diff |
| Ubuntu + Win native `s100_tests` dual-green; **no** Win O1 a/b; fixture MUST NOT SKIP | **MUST** | CI |

## Out of this tip

| Item | Notes |
|------|-------|
| `test_layers_all_cells` / `test_aggregates` / `test_fc_primitives` / `test_portrayal` multi-cell loops | **OUT** — later tip if needed |
| Cells 0001 / 0002 / 0011+ | **OUT** — beyond Sec.4–10 this slice |
| `updates/` / `signatures/` / viewer golden·AA5·disp-ref | **OUT** — Closed tip fixtures; do not mix |
| Engine load API shape change | **Forbidden** |

## Predecessor / Reuse vs tip-truth

| Item | Role |
|------|------|
| Reuse `IHO-S101-LOAD-0008` / `0003` / `0006` → `test_load` | Already on tip — **≠** this range tip-truth alone |
| Tree cells `101AA00DS0003/0006/0008.000` | Keep; README/SOURCE now also list five new cells |
| Upstream [iho-ohi/S-101-Test-Datasets](https://github.com/iho-ohi/S-101-Test-Datasets) `S-101_Test_DataSets/cells/` | Source for the five |
| Eng tip-truth `66b0c70` / Eng CI `42368360920` | Landed vendor five + README/SOURCE + `test_load_samples` MUST — **Closed** this tip |

## Win / CI policy

| Path | Win @ Closed Eng tip `66b0c70` | Notes |
|------|--------------------------------|-------|
| `s100_tests` / `test_load` (this tip) | **no O1 a/b** | native; Ubuntu+Win same |
| `viewer_*` / golden / disp-ref / display_compare | untouched | do **not** pull into this tip |
| `.github/workflows/ci.yml` | **no change** | Win `viewer_*` exclude stays as-is |

## Docs sync

- Case id in `docs/testing/iho-ctest-case-ids.md` — **Closed / Done** this tip.
- `docs/architecture.md` **next** → **TBD**.

## Non-goals

Reopen DISP-REF-COMPARE / DISPLAY-COMPARE / EDITION-HARDEN / CANCEL-REISSUE / UPD-SEQ / UPD-CORRUPT / SIG family / FASC / BRIDGE / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC / UNCERT / DEEPEN; change `open_dataset` / `load_s101` / Dataset public API; invent second CTest name / per-cell case ids / `LOAD4-*` / `LOADSAMPLES-*` failure prefixes / Closed-next tip name; expand hardcoded multi-cell portray/list loops this tip; vendor 0001/0002/0011+; touch `updates/` / `signatures/` / viewer golden·AA5·disp-ref oracles; insert `S102-GRID-SHIM-RETIRE` ahead; files-only MVP without load MUST cover; Win O1 a/b / `ci.yml` / `viewer_*` changes.
