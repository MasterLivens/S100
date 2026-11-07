# S-101 Sec.4–10 load samples (rev 1) · Active / Open

Owner: SoftwareArchitecture (2026-11-07). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S101-LOAD-0004..0010`** (**Active / Open**). CTest: reuse **`s100_tests` / `test_load`** (extend `test_load_samples` / `test_load` — **do not** invent a second CTest name).

**Status: Active / Open (2026-11-07)** — Planner freeze after **DISP-REF-COMPARE Closed** tip `9ab5851` (Close CI [`40293243584`](https://github.com/MasterLivens/S100/actions/runs/40293243584) dual-green; Eng tip-truth `3ba1329` / Eng CI [`40292051452`](https://github.com/MasterLivens/S100/actions/runs/40292051452); Open `4852fc9`≡`581fe61` / Open CI [`40257119938`](https://github.com/MasterLivens/S100/actions/runs/40257119938)). Promoted from Backlog (case-ids former Backlog row). **Eng tip-truth TBD / needs Eng** after APPROVE — **not** docs-close-only. Open CI TBD after push.

**Planner freeze (rev 1 — Open nail):**
- Reuse single case id **`IHO-S101-LOAD-0004..0010`**; **forbidden** to split per-cell or invent a second case id.
- CTest **nailed:** reuse **`s100_tests` / `test_load`** (extend `tests/test_load.cpp` `test_load_samples` / related load cover). **Do not** invent a second `add_test` name.
- Goal: expand `testdata/s101` Sec.4–10 sample cells + MUST load cover — **do not** change engine `open_dataset` / `load_s101` / Dataset public API.
- **Tip-truth = needs Eng:** vendor five missing cells `101AA00DS0004.000` / `0005.000` / `0007.000` / `0009.000` / `0010.000` into `testdata/s101/` + update `README.md` / `SOURCE.txt` + **extend `test_load_samples` / `test_load`** so **each new cell MUST** open / `feature_count > 0` / bounds valid (**not** files-only MVP; current tip only asserts preferred `0008` smoke).
- **Cell set:** MUST add **0004 / 0005 / 0007 / 0009 / 0010**. Reuse **0003 / 0006 / 0008** already on tip (keep; do not re-close). **Exclude** 0001 / 0002 / 0011+ from this tip.
- **Hardcoded multi-cell loops OUT of this tip:** do **not** expand `test_layers_all_cells` / `test_aggregates` / `test_fc_primitives` / `test_portrayal` this slice — load surface only; portray/list expand = later tip if needed.
- **Win: no O1 a/b** — native `s100_tests`; do **not** touch `viewer_*` / `ci.yml`. Ubuntu+Win same path MUST NOT SKIP / no platform SKIP.
- Failure face = bare CHECK only — **do not** invent `LOAD4-*` / `LOADSAMPLES-*` prefixes.
- Hold: do **not** reopen DISP-REF-COMPARE / DISPLAY-COMPARE / EDITION-HARDEN / CANCEL-REISSUE / UPD-SEQ / UPD-CORRUPT / SIG family / FASC / BRIDGE / other Closed. `S102-GRID-SHIM-RETIRE` stays Backlog — **do not** insert ahead.
- After Closed next: **TBD** (architecture / registered Active — **do not** invent).
- Team stop line (ops): stop when weekly remaining ≤5% — do **not** invent usage numbers in this contract.

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
| Reuse `IHO-S101-LOAD-0008` / `0003` / `0006` → `test_load` | Already on tip — **≠** this range tip-truth |
| Tree cells `101AA00DS0003/0006/0008.000` | Keep; README currently lists only these three |
| Upstream [iho-ohi/S-101-Test-Datasets](https://github.com/iho-ohi/S-101-Test-Datasets) `S-101_Test_DataSets/cells/` | Source for missing five |
| This tip Eng | **TBD / new** after APPROVE — vendor five + README/SOURCE + extend `test_load_samples` / `test_load` |

## Win / CI policy

| Path | Win @ base `9ab5851` / this Open | Notes |
|------|----------------------------------|-------|
| `s100_tests` / `test_load` (this tip) | **no O1 a/b** | native; Ubuntu+Win same |
| `viewer_*` / golden / disp-ref / display_compare | untouched | do **not** pull into this tip |
| `.github/workflows/ci.yml` | **no change** | Win `viewer_*` exclude stays as-is |

## Docs sync

- Case id in `docs/testing/iho-ctest-case-ids.md` — **Active / Open** this tip (promoted from Backlog); flip Closed on docs close after Eng tip-truth + CI dual-green.
- `docs/architecture.md` **next** → this Active / Open; after Closed → **TBD**.

## Close criteria (honest — after Eng)

- Eng tip-truth vendors five cells + README/SOURCE + `test_load` cover for each new cell (MUST rows green).
- Ubuntu+Win `s100_tests` / `test_load` dual-green; **no** Win O1 a/b.
- Docs → Closed + CI Notes; contract display stays **rev 1** unless a later Arch Open.
- After Close next: **TBD**. Do **not** reopen DISP-REF / DISPLAY / EDITION / CANCEL / UPD / SIG / FASC / BRIDGE; shim Backlog.

## Non-goals

Reopen DISP-REF-COMPARE / DISPLAY-COMPARE / EDITION-HARDEN / CANCEL-REISSUE / UPD-SEQ / UPD-CORRUPT / SIG family / FASC / BRIDGE / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC / UNCERT / DEEPEN; change `open_dataset` / `load_s101` / Dataset public API; invent second CTest name / per-cell case ids / `LOAD4-*` / `LOADSAMPLES-*` failure prefixes / Closed-next tip name; expand hardcoded multi-cell portray/list loops this tip; vendor 0001/0002/0011+; touch `updates/` / `signatures/` / viewer golden·AA5·disp-ref oracles; insert `S102-GRID-SHIM-RETIRE` ahead; files-only MVP without load MUST cover; Win O1 a/b / `ci.yml` / `viewer_*` changes.
