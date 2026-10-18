# Cancellation · re-issue · new edition (rev 2)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S164-CANCEL-REISSUE`** (**Active**). CTest: **`exchange_cancel_reissue`** — **MUST NOT SKIP**, no platform guard, Linux + Windows CI. Fixture reproducibility stays under `update_fixtures_reproducible`.

Builds on `docs/update-robustness-contract.md` rev 1.4 (strong guarantee, `UpdateRejectCode`, `UpdateReport`, `dataset_digest()`, session keeps last good state). Same module boundary: all decisions in `s100_core`; CLI/viewer only display.

## Sources (Research&Planner, 2026-10-18, primary texts in `/workspace/research/`)

| Topic | Clause |
|-------|--------|
| Cancel = update file, **Edition number 0**, dataset "must be removed from the system" | S-101 PS 2.0.0 §11.3.3 |
| Cancel encoding: DSID only, PROF="2", **DSED="0"** | S-101 PS 2.0.0 B-7.1.1 |
| Cancel file name = next update sequence number | S-101 PS 2.0.0 §11.3.2 |
| Re-issue: same ED, UPD = last applied update, file is `.000` | S-101 PS 2.0.0 §11.3.1, §11.3.3 |
| New edition: ED + 1, UPD 0, same file name | S-101 PS 2.0.0 §11.3.3 |
| Fileless cancel (catalogue purpose = cancelled) | S-100 5.2.0 §17-4.4.1 (**Non-goal** here) |
| ECDIS behaviour: new edition replaces old + all its updates; re-issue replaces original + applied updates; cancel removes and reports the cell | S-98 2.0.0 §20.5 (HSSC draft numbering) |

**Project decisions (spec is silent; D1–D3 by Research&Planner, D4–D5 by SoftwareArchitecture on SoftwareTester review):** D1 cancelled cell is not drawn, is named in the report, and every later update is rejected. D2 after a re-issue at UPD *n*, updates with UPD ≤ *n* are skipped as already applied (counted, not an error). D3 an update with ED > installed ED is rejected with "newer edition exists, dataset not up to date"; when the new-edition base arrives, leftover old-edition updates are reported as removable. **D4** a cancel file is **not** sequence-checked: it is executed even if it arrives out of order or with earlier updates missing, because removing a withdrawn cell is the safe direction. **D5** every update (cancel included) must name its target: DSID DSNM stem (file name without extension) must equal the dataset’s DSNM stem, else `DatasetNameMismatch` — a wrong-cell cancel would wipe the wrong chart. A successful update **never overwrites** `ds.dataset_name` (today `s101_update.cpp` ~802 copies the update DSNM over it; remove that).

## Prerequisite fix (MUST) — DSED without a dot

`parse_dsid_field` (`src/core/iso8211.cpp` ~249) currently requires `ED.UPD`. It MUST also accept a bare integer: `"0"` → ED 0 (cancel), `"9"` → ED 9, UPD 0. `""`, `"9.x"`, `"x"`, `"9."`, `".1"` → not ok → `BadStructure`. Existing `ED.UPD` parsing unchanged. `DatasetIdentification` also exposes `profile` (PROF) — already read, currently discarded.

## API (additive)

```cpp
// exchange_set.hpp
enum class UpdateRejectCode { /* existing 8 + None */ ...,
  DatasetCancelled,     // any update to a dataset that is already cancelled (D1)
  NewerEditionPending,  // update ED > installed ED (D3); existing EditionMismatch stays for ED < installed
  DatasetNameMismatch   // update/cancel DSNM stem != dataset DSNM stem (D5)
};
struct UpdateReport { /* existing */ ...
  int skipped{0};                                  // D2
  bool cancelled{false};                           // a cancel update was applied
  std::vector<std::filesystem::path> superseded;   // old-edition updates not applied because a newer base is loaded (D3)
  std::string notice;                              // "cancelled: <DSNM>" / "newer edition N pending" / "superseded updates can be removed"
};
// dataset.hpp — Dataset
bool cancelled{false};
```

`update_reject_code_name` covers the two new values. Existing callers compile unchanged.

## Behaviour (MUST)

1. **Check order** (replaces rev 1.4 order): Truncated → BadStructure → **DatasetNameMismatch** → **DatasetCancelled** (target already cancelled) → **cancel detection** (DSED ED == 0) → NotUpdateFile → CatalogueMismatch → **NewerEditionPending** (ED >) → EditionMismatch (ED <) → OutOfSequence → records. Cancel detection sits **before** `NotUpdateFile`, so a cancel file (UPD parses as 0) is never mistaken for a base. A real base has ED ≥ 1, so `notupd` (0003.000 passed as update) still reaches `NotUpdateFile` exactly as in rev 1.4.
2. **Cancel.** A cancel file must have PROF = "2" and contain only DSID (otherwise → `BadStructure`). Applying it: `ds.cancelled = true`; features, information records and `SpatialStore` cleared; ED/UPD unchanged; returns `true`, code `None`; `report.cancelled = true`, `notice = "cancelled: <DSNM>"`. Not sequence-checked (D4).
3. **After cancel**, every further update → `DatasetCancelled`, strong guarantee as before.
4. **Re-issue.** Base `.000` with DSID UPD *n* > 0. `open_dataset_with_updates` / `open_cell_series`: updates of the same ED with UPD ≤ *n* are **skipped** (`report.skipped`), not applied, not errors; the next applied update must be *n* + 1. Outside that window duplicates stay `OutOfSequence`.
5. **New edition.** `group_cell_series` picks the highest-ED base for a cell; updates of a lower ED in the same exchange set are **not applied** and are listed in `report.superseded` with `notice = "superseded updates can be removed"`. `apply_s101_update` with ED < installed stays `EditionMismatch`.
6. **Newer edition pending.** Update ED > installed ED → `NewerEditionPending`, dataset unchanged, message says edition *N* exists and the dataset is not up to date.
7. **Session.** `ChartSession` keeps a cancelled member (so it can be reported): `find(key)` returns it with `cancelled == true` and 0 features; `last_update_report(key).cancelled == true`. Re-opening an exchange set whose base for a key is a re-issue or new edition **replaces** the member wholesale (no merge with the old state). A fresh base for a previously cancelled name replaces it and clears `cancelled` (S-164 cancel-and-replace).
8. **CLI / viewer (thin).** `s100_cli --with-updates` adds `skipped=N cancelled=<0|1> superseded=N` to its report line; cancel → exit 0; `NewerEditionPending` → non-zero. Viewer does not draw a cancelled dataset and shows `CANCELLED: <cell>` in the status bar (WARN, no automation).

## Fixtures (synthetic, `tools/gen_update_fixtures.py`, same seed, committed + sha256 in `fixtures.toml`)

Base `101AA00DS0003.000` (ED 9). Each scenario in its own directory under `testdata/s101/updates/` because re-issue / new edition reuse the base file name.

| Dir | Content | Expected |
|-----|---------|----------|
| `nodot/` | base with DSED `"9"` + valid `001` | loads ED 9 UPD 0; 001 applies (prerequisite fix positive control) |
| `cancel/` | `001` valid, `002` cancel (DSID only, DSED `"0"`), `003` valid-looking | 001 applied; 002 → `cancelled`, 0 features; 003 → `DatasetCancelled` |
| `cancel_bad/` | cancel file that also carries a FRID | `BadStructure`, dataset unchanged |
| `cancel_wrong_name/` | ED 9 base 0003 + cancel file whose DSNM is `101AA00DS0006` | `DatasetNameMismatch`, digest unchanged, **not** cancelled |
| `cancel_ooo/` | base + `001` + cancel named `003` (`002` never arrives) | cancel executes: `cancelled`, 0 features (D4) |
| `wrong_name/` | valid-looking `001` whose DSNM is `101AA00DS0006` | `DatasetNameMismatch`, digest unchanged |
| `reissue/` | base DSED `"9.2"` whose records are the **materialized** state of base + 001 + 002 (generator writes it from the same ops), and `001`,`002`,`003` | `skipped=2`, `applied=1`, UPD 3; 003's change visible |
| `newed/` | base DSED `"10.0"` + leftover ED 9 `001` | ED 10 UPD 0; `superseded` = [001]; `applied=0` |
| `pending/` | ED 9 base + `001` with DSED `"10.1"` | `NewerEditionPending`, digest unchanged |
| `session_replace/` | `A/`: ED 9 base + 001,002 · `B/`: re-issue 9.2 base + 003 · `X/`: ED 9 base + 001 + cancel 002 · `C/`: fresh ED 10 base, same name as X | oracle 8 (A→B), oracle 9 (X→C) |

## Oracle — CTest `exchange_cancel_reissue` (no SKIP)

| # | Check | Sev |
|---|-------|-----|
| 0 | Positive controls first: `nodot` loads (ED 9, UPD 0) and its 001 applies; a plain `ED.UPD` base still parses as before. Unit asserts on `parse_dsid_field`: `"0"`→ED 0, `"9"`→9/0, `"4.20"`→4/20 ok; `""`, `"9.x"`, `"x"`, `"9."`, `".1"` → not ok (→ `BadStructure` when loaded) | MUST |
| 1 | Every fixture exists, sha256 matches; reproducibility covered by `update_fixtures_reproducible` (now includes these files) | MUST |
| 2 | `cancel`: 002 returns `true`, `cancelled`, features/information/spatial all empty, `report.notice` contains DSNM; 003 → exactly `DatasetCancelled`, `dataset_digest` unchanged | MUST |
| 3 | `cancel_bad` → exactly `BadStructure`; `cancel_wrong_name` and `wrong_name` → exactly `DatasetNameMismatch`, not cancelled; digest unchanged in all three. `cancel_ooo` → cancelled (D4) | MUST |
| 4 | `reissue`: `skipped == 2`, `applied == 1`, UPD 3, **and** `dataset_digest(reissue + 003) == dataset_digest(base + 001 + 002 + 003)` (ED/UPD both 9/3) | MUST |
| 5 | `newed`: ED 10 UPD 0, `superseded == {001}`, `applied == 0`, notice set, **and** `dataset_digest` equals a standalone `open_dataset` of the ED 10 `.000` | MUST |
| 6 | `pending` → exactly `NewerEditionPending`, digest unchanged, message names edition 10 | MUST |
| 7 | Every new code (`DatasetCancelled`, `NewerEditionPending`, `DatasetNameMismatch`) has ≥ 1 fixture; `update_reject_code_name` returns their names | MUST |
| 8 | Session: load A → member UPD 2; open B → member digest == fresh load of B (re-issue + 003), not a merge with A | MUST |
| 9 | Session: cancelled member is still `find`-able with `cancelled == true`, report says cancelled; after opening `X/` the member is cancelled; opening `C/` replaces it with `cancelled == false`, ED 10 | MUST |
| 10 | No crash / exception on any fixture; rev 1.4 tests (`exchange_update_robustness`) still pass and `tests/test_update_robustness.cpp` is **not modified** (Tester checks `git diff`). If D5 or the new order would change a rev 1.4 expectation, Eng stops and reports instead of editing that test | MUST |
| 11 | Viewer status bar `CANCELLED:` text | WARN |

## Non-goals

- **Fileless cancel** via catalogue `purpose` = cancelled (S-100 §17-4.4.1). Placeholder: `ExchangeDatasetEntry::purpose` is already parsed; a later slice sets `UpdateReport::cancelled` from it in `group_cell_series` without new API.
- Issue-date check when a cancelled name is reused (S-101 §11.3.3 last sentences).
- Signatures, permits, S-164 power-up, S-102+ updates.
