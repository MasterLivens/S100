# Update-sequence robustness (rev 1.4) · UPD-CORRUPT Active / Open (UPD-SEQ Closed)

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S164-UPD-CORRUPT`** (**Active / Open**). Sibling **`IHO-S164-UPD-SEQ`** already **Closed** (same contract / same CTest — **do not** reopen). CTest: **`exchange_update_robustness`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev 1.4** — Planner freeze (MUST below). Corrupt-input / strong-guarantee slice after closed **`IHO-S164-UPD-SEQ`**. Does **not** reopen UPD-SEQ / SIG / AGGREGATOR / CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE; does **not** mix/jump Closed **CANCEL-REISSUE / EDITION-HARDEN**; does **not** jump **`S102-GRID-SHIM-RETIRE`**. Does **not** expand tip-truth oracles / change `UpdateRejectCode` enum / change check order / weaken copy-and-swap strong guarantee.

Frozen with: `docs/api-contracts.md` (update robustness), `docs/part15-signature-contract.md` (SIG Closed — **do not** reopen), `docs/cancel-reissue-contract.md` / `docs/edition-hardening-contract.md` (CANCEL / EDITION separate Active — **do not** mix/Closed), `docs/testing/iho-ctest-case-ids.md`. Extends `docs/exchange-set.md`. S-101 only; S-102+ updates out of scope.

Primary research: `/workspace/research/upd-corrupt/BRIEF.md` (2026-10-25 tip-truth refresh). Former Eng gap **already fixed** @ Eng **`331b9b9`** (feat UpdateRejectCode/UpdateReport + check order + fixtures incl. **`corrupt/`** + CTest; parent **`a3eb691`** copy-and-swap strong guarantee; tip ancestors **`0b97c33`/`3c68439`** / **`a2acaa6`**). **Same Eng SHA covers SEQ+CORRUPT fixtures** — not a second Eng landing.

**rev1.4 field freeze (kept):** `UpdateRejectCode` single code; `UpdateReport`; `apply_s101_update` strong guarantee (copy-and-swap — **never half-apply** on Truncated/BadStructure/BadInstruction/MissingTarget/NotUpdateFile); stop-at-first; check order Truncated→BadStructure→NotUpdateFile→CatalogueMismatch→EditionMismatch→OutOfSequence→record; corrupt fixtures `testdata/s101/updates/corrupt/{trunc,leader,ruin9,missing,late_fail}.001` + `notupd/`; `dataset_digest` rollback oracles; session last-good; viewer sibling via core; oracles 0–11 tip-truth (CORRUPT face = Closed object this tip; SEQ fixtures stay green — **do not** reopen UPD-SEQ); MUST NOT SKIP; Linux+Win. CANCEL / EDITION → separate — **do not** mix/Closed.

**Status: Active / Open (2026-10-25)** — after **UPD-SEQ Closed** (tip `0833930` ≡ Arch `bb337ce`; tip Close CI [`40191381492`](https://github.com/MasterLivens/S100/actions/runs/40191381492) dual-green; Open-bind evidence CI [`40190690732`](https://github.com/MasterLivens/S100/actions/runs/40190690732) @ `c3bb372`≡`fd853d6`, CTest `#14 exchange_update_robustness` Passed, O1=tip-truth/docs-close). Eng tip ancestors **`a3eb691`→`331b9b9`** already on tip — **not greenfield**. Eng green CI [`39094178764`](https://github.com/MasterLivens/S100/actions/runs/39094178764) @ `331b9b9` (native `s100_tests` + update/corrupt fixtures — **not** `viewer_*` — **no Win O1 (a)/(b)**). Ubuntu+Windows already Passed — tip Close CI [`40191381492`](https://github.com/MasterLivens/S100/actions/runs/40191381492) `#14` + Open-bind [`40190690732`](https://github.com/MasterLivens/S100/actions/runs/40190690732) `#14` + Eng landing [`39094178764`](https://github.com/MasterLivens/S100/actions/runs/39094178764). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from UPD-SEQ / SIG / AGGREGATOR / CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE / CANCEL-REISSUE / EDITION-HARDEN. Do **not** invent case ids or next tip names; freeze tip-truth below (**keep** oracles 0–11 — **do not** expand). Do **not** reopen UPD-SEQ / SIG / AGGREGATOR / CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE. Do **not** mix/jump Closed **CANCEL-REISSUE / EDITION-HARDEN**. Do **not** invent `UPDCORR-*` failure-face / skip prefixes (failure face = bare CHECK + enum tip-truth). Do **not** retire `grid()` shim this tip. Contract display header stays **rev 1.4**. After Closed next: **TBD** (architecture / registered Active only — do **not** invent).

**Planner freeze (MUST — from BRIEF + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-S164-UPD-CORRUPT`** / CTest **`exchange_update_robustness`**; **forbidden** to invent a second case/test name.
- MUST tip-truth = UpdateRejectCode single-code / UpdateReport / check order / strong guarantee (never half-apply) / corrupt fixtures + oracles 0–11 (CORRUPT face) + CTest **MUST NOT SKIP**; Linux+Win.
- Eng tip already has **`331b9b9`** (parent `a3eb691`; same SHA as SEQ); Ubuntu+Win (tip `40191381492` `#14` / Open-bind `40190690732` `#14` / Eng `39094178764`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield. **Keep** tip-truth oracles — **do not** expand / change enum / change check order / weaken strong guarantee / reopen UPD-SEQ.
- Update-sequence → **`IHO-S164-UPD-SEQ`** already Closed — **do not** reopen.
- SIG / AGGREGATOR / CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE already Closed — **do not** reopen.
- Cancel / re-issue → **`IHO-S164-CANCEL-REISSUE`** separate Active — **do not** mix / Closed this tip.
- Edition harden → **`IHO-S164-EDITION-HARDEN`** separate Active — **do not** mix / Closed this tip.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog**; **not** this tip.
- Failure face = bare CHECK + `UpdateRejectCode` tip-truth; do **not** invent `UPDCORR-*` prefixes.
- Ubuntu + Windows: `exchange_update_robustness` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen UPD-SEQ / SIG / AGGREGATOR / CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE; do **not** mix/jump Closed CANCEL-REISSUE / EDITION-HARDEN.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`; native `s100_tests` + update/corrupt fixtures; no SKIP. **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `exchange_update_robustness` MUST NOT SKIP, Passed (already tip CI `40191381492` `#14` / Open-bind `40190690732` `#14` / Eng `39094178764`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; **only** UPD-CORRUPT case-ids row → Closed; UPD-SEQ stays Closed; no Eng `ci.yml` / large Eng diff required
- After Close next: **TBD**

**Eng notes:** verify `331b9b9` (parent `a3eb691`) on tip; corrupt fixtures under `testdata/s101/updates/corrupt/` + `notupd/`; driver `tests/test_update_robustness.cpp`; cmake `exchange_update_robustness`; `dataset_digest` rollback. UPD-SEQ Closed already on this stack (`0833930`). Do **not** reopen UPD-SEQ / SIG / AGGREGATOR / CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY etc. Do **not** mix/Closed CANCEL-REISSUE / EDITION-HARDEN. Do **not** invent `UPDCORR-*` failure prefixes. Do **not** change tip-truth oracles / enum / check order / weaken strong guarantee.

**O-questions (BRIEF — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `331b9b9` already; no `ci.yml` / no large Eng) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green — **yes / freeze**.
3. Keep tip-truth oracles 0–11 (CORRUPT face Closed object; SEQ fixtures stay green; do **not** reopen UPD-SEQ) — **yes / freeze**.
4. CANCEL-REISSUE / EDITION-HARDEN stay Active — **yes / freeze**; do **not** mix/Closed this tip.
5. `grid()` / shim Backlog — **yes / freeze**.
6. Failure face = bare CHECK + enum — **yes / freeze**; do **not** invent `UPDCORR-*`.
7. After Closed next = **TBD** — **yes / freeze**; do **not** invent.

**UPD-SEQ Closed (kept — do not reopen):** tip `0833930`≡`bb337ce`; Open CI `40190690732` @ `c3bb372`≡`fd853d6`; Close CI `40191381492` dual-green; O1 tip-truth/docs-close SUCCESS.

---

## Module boundary

All logic stays in `s100_core` (`src/core/s101_update.cpp`, `src/core/exchange_set.cpp`). Viewer and CLI only **show** the result (report + warning); they never decide sequencing. `ChartSession` calls the same API.

## API changes (additive, source-compatible)

```cpp
// include/s100/exchange_set.hpp
enum class UpdateRejectCode {
  None,
  EditionMismatch,   // update ED != dataset ED
  OutOfSequence,     // update UPD != dataset UPD + 1 (covers duplicate, gap, reversed)
  NotUpdateFile,     // DSID says base / wrong product / not S-101
  Truncated,         // file shorter than leader/directory claims
  BadStructure,      // ISO 8211 leader / directory / field parse error
  BadInstruction,    // unknown RUIN / FAUI / IUIN / ATIN value
  MissingTarget,     // Delete/Modify of a record id that does not exist
  CatalogueMismatch  // CATALOG.XML ED/UPD disagrees with DSID of the file
};

struct UpdateReport {
  int applied{0};                          // updates applied successfully, in order
  std::filesystem::path rejected;          // first rejected file, empty if none
  UpdateRejectCode code{UpdateRejectCode::None};
  std::string message;                     // human text, includes file name + ED.UPD
};

bool apply_s101_update(Dataset& ds, const std::filesystem::path& update_path,
                       LoadError* err = nullptr, UpdateRejectCode* code = nullptr);
bool open_dataset_with_updates(const std::filesystem::path& base,
                               const std::vector<std::filesystem::path>& updates, Dataset& out,
                               LoadError* err = nullptr, UpdateReport* report = nullptr);
bool open_cell_series(const ExchangeCellSeries& series, Dataset& out, LoadError* err = nullptr,
                      UpdateReport* report = nullptr);
```

**Where the code appears:** `apply_s101_update(..., UpdateRejectCode* code)` sets it on every `false` return (and `None` on success); `open_dataset_with_updates` / `open_cell_series` copy it into `UpdateReport::code`. `LoadError` is unchanged (message only). Every existing `return false` in `s101_update.cpp` / update path in `exchange_set.cpp` MUST map to exactly one code; the message string is for humans and is not asserted beyond rule 6.

Existing callers that pass no report keep compiling and behaving the same on the happy path.

## Behaviour (MUST)

1. **Strong guarantee per update.** `apply_s101_update` returning `false` leaves `ds` **bit-for-bit equal** to its state before the call (features, attributes, FASC/INAS lists, information records, `SpatialStore`, ED/UPD). Implementation: apply to a working copy, swap on success. The current in-place partial path (erase/push before a later `return false`) is a defect this slice removes.
2. **Stop at first rejection.** `open_dataset_with_updates` / `open_cell_series` return `false` at the first rejected update; `out` holds **base + the applied prefix** (last good state, usable for display); `report` names the file and code. Later files are not attempted.
3. **Check order (fixes which code wins):** file size vs leader → `Truncated`; 8211 parse → `BadStructure`; DSID is a base/non-S-101 → `NotUpdateFile`; catalogue vs DSID → `CatalogueMismatch`; ED → `EditionMismatch`; UPD → `OutOfSequence`; then records (`BadInstruction`, `MissingTarget`).
3a. **List order is the apply order.** `open_dataset_with_updates` applies `updates` in the order given (no auto-sort); `open_cell_series` passes them in catalogue ED/UPD order; `discover_update_files` returns extension order. A wrong order is therefore rejected, never silently fixed.
3b. **Sequencing by DSID**, not file name: ED must equal; UPD must be current+1. Duplicate (UPD == current), gap (UPD > current+1) and reversed order all → `OutOfSequence`. If CATALOG.XML gives ED/UPD for the file and it disagrees with DSID → `CatalogueMismatch`.
4. **Corrupt input never crashes, never half-applies.** Truncated / bad leader / bad directory → `Truncated` / `BadStructure`; unknown instruction values → `BadInstruction`; Delete/Modify of a non-existent FRID/IRID/spatial RCID → `MissingTarget`. ASan/UBSan clean on Linux CI for these fixtures (WARN if sanitizer job not available yet).

## Fixtures

Official S-164 update packages: use if Research&Planner can pull them (add as extra Reuse rows later). This slice does **not** wait for them.

The repo has **no** `.001` files, so the positive control is synthetic too. Fixtures live under `testdata/s101/updates/`, made by **`tools/gen_update_fixtures.py`** (Python 3 stdlib only, fixed seed, deterministic). Either (a) commit the bytes + sha256 in `testdata/s101/fixtures.toml`, or (b) generate them in a CTest fixture step (`FIXTURES_SETUP`) before the test — Eng picks one; both platforms. Base: `101AA00DS0003.000` (ED 9 UPD 0). Missing fixture → **FAIL**, never SKIP.

| Fixture | Content | Expected |
|---------|---------|----------|
| `seq/…001`, `…002`, `…003` | valid chain: insert 1 feature; modify its attribute; delete one original feature | all applied, `applied=3`, UPD=3 |
| `seq` reversed order list (002 before 001) | — | `OutOfSequence` on 002, `applied=0` |
| `seq` with 001 twice | — | second 001 `OutOfSequence`, `applied=1` |
| `seq` 001 + 003 (gap) | — | `OutOfSequence` on 003, `applied=1` |
| `bad_ed/…001` (ED 8) | — | `EditionMismatch` |
| `corrupt/trunc.001` | valid 001 cut in half (leader record length > bytes on disk) | `Truncated` |
| `notupd/101AA00DS0003.000` | the base cell itself passed as an update | `NotUpdateFile` |
| `corrupt/leader.001` | leader bytes garbled | `BadStructure` |
| `corrupt/ruin9.001` | RUIN = 9 on a feature | `BadInstruction` |
| `corrupt/missing.001` | Delete of non-existent FRID | `MissingTarget` |
| `corrupt/late_fail.001` | 3 valid RUIN ops then a `MissingTarget` | rejected, dataset unchanged (proves rule 1) |
| `catalog/CATALOG.XML` listing 001 as UPD 2 | — | `CatalogueMismatch` |

## Oracle — CTest `exchange_update_robustness` (no SKIP)

| # | Check | Sev |
|---|-------|-----|
| 0 | **Positive control first:** valid `001` applies (`true`, code `None`, UPD 1). If it fails, the test FAILs before any reject case | MUST |
| 1 | Every fixture exists; sha256 equals `fixtures.toml`; running `tools/gen_update_fixtures.py` twice yields byte-identical output (checked in CTest or a sibling `update_fixtures_reproducible` test, both platforms) | MUST |
| 2 | Each reject row returns `false` **and exactly** its single expected `UpdateRejectCode` (every enum value except `None` has ≥ 1 fixture) and `applied` — asserting only "failed" is not enough. One MUST per case: gap n+2, duplicate n, reversed order, edition mismatch, truncated, bad 8211 leader, missing target (plus not-update-file, bad instruction, catalogue mismatch) | MUST |
| 3 | Valid chain: inserted feature present after 001; its attribute = patched value after 002; deleted feature absent after 003; feature count = base + 1 − 1 | MUST |
| 4 | **Rollback:** one shared helper `dataset_digest(const Dataset&)` (tests/support), compared before vs after **every** reject case. It hashes, in canonical order: ED/UPD; feature count; per feature sorted by id → id, type, attributes (code + value, nested, sorted), geometry **coordinates**, FASC/INAS (rcid, assoc, role, attributes); information records sorted by id with attribute values; `SpatialStore` records sorted by (rcnm, rcid) with **coordinates**. Any difference = FAIL | MUST |
| 5 | `late_fail`: none of its 3 valid ops visible after rejection | MUST |
| 6 | `report.message` contains the rejected file name and its DSID ED.UPD; when DSID is unreadable (`Truncated`, `BadStructure`) it shows `?.?` | MUST |
| 9 | No crash / no exception escapes on any corrupt fixture | MUST |
| 10 | Sanitizer run clean | WARN |
| 11 | Session keeps last good state on reject (see Session section) | MUST |

## Session (MUST) / CLI / viewer (thin)

- `ChartSession::load_member`: on a rejected update it **keeps** the last good state (base + applied prefix) as the member, stores the `UpdateReport` (`last_update_report(key)`), and returns `false`. It must not drop the cell. Oracle 11 (MUST): after loading a series whose 2nd update is rejected, `find(key) != nullptr`, its UPD = 1, and `last_update_report(key).code` is the expected code.

- `s100_cli <base.000> --with-updates` prints `updates applied=N rejected=<file> code=<Code>`; exit code non-zero when an update was rejected.
- Viewer: opening a base `.000` **also applies sibling updates** found by `discover_update_files` (same rule as `s100_cli`, `docs/exchange-set.md`), always through `open_dataset_with_updates` — the viewer never sequences on its own. Rationale: showing an un-updated base while updates sit next to it is the less safe default. On rejection: draw last good state + status-bar `WARNING: update rejected (file, code=…)`. Base-only cells behave as before (golden unchanged). Status-bar text = WARN (no automation yet). Snapshot = WARN.

## Non-goals

**`IHO-S164-CANCEL-REISSUE` is not in this slice** (now `docs/cancel-reissue-contract.md`): cancellation (DSID ED 0), re-issue (base UPD n > 0, skip ≤ n), new edition replacing a series, `Dataset::cancelled`, `UpdateReport::skipped/cancelled`, `DatasetCancelled` code. The enum/report are additive so that slice extends them without breaking this one.

Also out: 
Signatures / CRL (`IHO-S164-SIG`, Track B), S-164 power-up, permits, S-102+ updates, cross-cell consistency.
