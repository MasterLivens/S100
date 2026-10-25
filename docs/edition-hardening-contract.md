# Edition hardening: old-edition updates · strict base DSED (rev 1) · EDITION-HARDEN Active / Open (after CANCEL-REISSUE Closed)

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S164-EDITION-HARDEN`** (**Active / Open**). Sibling **`IHO-S164-CANCEL-REISSUE`** already **Closed** (separate contract / separate CTest `exchange_cancel_reissue` — **do not** reopen). CTest: **`exchange_edition_hardening`** — **MUST NOT SKIP**, Linux + Windows CI. Fixture reproducibility stays under `update_fixtures_reproducible`.
Status: **rev 1** tip-truth (header stays rev 1 — **do not** open Eng for bump). Edition-harden slice after closed **`IHO-S164-CANCEL-REISSUE`**. Does **not** reopen CANCEL-REISSUE / UPD-SEQ / UPD-CORRUPT / SIG / AGGREGATOR / CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE; does **not** jump **`S102-GRID-SHIM-RETIRE`**. Does **not** expand tip-truth oracles / change enum / change check order / invent `EDITION-*` / `HARDEN-*` failure-face prefixes.

Frozen with: `docs/api-contracts.md` (edition hardening segment), `docs/cancel-reissue-contract.md` (CANCEL Closed — **do not** reopen), `docs/update-robustness-contract.md` (UPD-SEQ + UPD-CORRUPT Closed — **do not** reopen), `docs/testing/iho-ctest-case-ids.md`. Cleanup slice on top of cancel-reissue rev 2.1 and update-robustness rev 1.4. Same module boundary: all decisions in `s100_core`; CLI/viewer only display. Strong guarantee unchanged: **a rejected update or a rejected base load leaves the caller’s state unchanged**. S-101 only; fileless cancel / delete-superseded-from-disk / issue-date / S-57 / signatures / permits / power-up / S-102+ updates out of scope (Non-goals).

Primary research: `/workspace/research/edition-harden/BRIEF.md` (2026-10-25 tip-truth refresh). Former Eng gap **already fixed** @ Eng **`e3c0b2d`** (feat edition hardening rev1 + fixtures `testdata/s101/updates/edition/{old_left,mixed,nocat,base_dsed_*,base_no_dsid,base_bad_cat,…}` + CTest; tip ancestor docs `4aca743` rev1). **CANCEL Eng `45c355e` / tip `af07602` already Closed — do not reopen.**

**rev 1 field freeze (kept):** E1–E6 — series-path 0<ED<base → superseded; standalone `apply_s101_update` stays `EditionMismatch`; ED> → `NewerEditionPending`; single helper `classify_update_edition`; strict base DSED (missing DSID / unparseable DSED / ED0 → reject `Invalid`/`BadStructure`); DSED `"0"` base rejected; `[[edition_fixture]]`; oracles 0–11 MUST + 12–13 WARN; rev1.4 + cancel-reissue assertions unchanged; MUST NOT SKIP; Linux+Win. CANCEL / UPD-SEQ / UPD-CORRUPT → **Closed** — **do not** reopen.

**Status: Active / Open (2026-10-25)** — after **CANCEL-REISSUE Closed** (tip `af07602`; tip Close CI [`40198622771`](https://github.com/MasterLivens/S100/actions/runs/40198622771) dual-green; Open-bind evidence CI [`40197755849`](https://github.com/MasterLivens/S100/actions/runs/40197755849) @ `80e20dd`≡`e7c99fb`, CTest `#16 exchange_edition_hardening` Passed Ubuntu+Win, O1=tip-truth/docs-close). Eng tip ancestor **`e3c0b2d`** already on tip — **not greenfield**. Eng green CI [`39099520427`](https://github.com/MasterLivens/S100/actions/runs/39099520427) @ `e3c0b2d` (native `s100_tests` + edition fixtures — **not** `viewer_*` — **no Win O1 (a)/(b)**). Ubuntu+Windows already Passed — tip Close CI [`40198622771`](https://github.com/MasterLivens/S100/actions/runs/40198622771) `#16` + CANCEL Open-bind [`40197755849`](https://github.com/MasterLivens/S100/actions/runs/40197755849) `#16` + Eng landing [`39099520427`](https://github.com/MasterLivens/S100/actions/runs/39099520427). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from CANCEL-REISSUE / UPD-SEQ / UPD-CORRUPT / SIG / AGGREGATOR / CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE. Do **not** invent case ids or next tip names; freeze tip-truth below (**keep** oracles 0–11 MUST / 12–13 WARN — **do not** expand). Do **not** reopen CANCEL-REISSUE / UPD-SEQ / UPD-CORRUPT / SIG / AGGREGATOR / CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE. Do **not** invent `EDITION-*` / `HARDEN-*` failure-face / skip prefixes (failure face = bare CHECK + enum tip-truth / existing SSE/`sse_map`). Do **not** retire `grid()` shim this tip. Contract display header stays **rev 1**. After Closed next: **TBD** (architecture / registered Active only — do **not** invent).

**Planner freeze (MUST — from BRIEF + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-S164-EDITION-HARDEN`** / CTest **`exchange_edition_hardening`**; **forbidden** to invent a second case/test name.
- MUST tip-truth = E1–E6 / `classify_update_edition` / `[[edition_fixture]]` / oracles 0–11 MUST + 12–13 WARN + CTest **MUST NOT SKIP**; Linux+Win.
- Eng tip already has **`e3c0b2d`**; Ubuntu+Win (tip `40198622771` `#16` / CANCEL Open-bind `40197755849` `#16` / Eng `39099520427`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield. **Keep** tip-truth oracles — **do not** expand / change enum / change check order / reopen CANCEL/UPD-SEQ/UPD-CORRUPT.
- Cancel / re-issue → **`IHO-S164-CANCEL-REISSUE`** already Closed — **do not** reopen.
- Update-sequence → **`IHO-S164-UPD-SEQ`** already Closed — **do not** reopen.
- Corrupt-input → **`IHO-S164-UPD-CORRUPT`** already Closed — **do not** reopen.
- SIG / AGGREGATOR / CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE already Closed — **do not** reopen.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog**; **not** this tip.
- Failure face = bare CHECK + `UpdateRejectCode` / `LoadErrorKind` tip-truth (or existing SSE/`sse_map`); do **not** invent `EDITION-*` / `HARDEN-*` prefixes.
- Ubuntu + Windows: `exchange_edition_hardening` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen CANCEL-REISSUE / UPD-SEQ / UPD-CORRUPT / SIG / AGGREGATOR / CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`; native `s100_tests` + edition fixtures; no SKIP. **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `exchange_edition_hardening` MUST NOT SKIP, Passed (already tip CI `40198622771` `#16` / CANCEL Open-bind `40197755849` `#16` / Eng `39099520427`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; **only** EDITION-HARDEN case-ids row → Closed; CANCEL/UPD-SEQ/UPD-CORRUPT stay Closed; no Eng `ci.yml` / large Eng diff required
- After Close next: **TBD**

**Eng notes:** verify `e3c0b2d` on tip; fixtures under `testdata/s101/updates/edition/{old_left,mixed,nocat,base_dsed_empty,base_dsed_ed_x,base_dsed_x,base_dsed_ed_dot,base_dsed_dot_upd,base_dsed_zero,base_no_dsid,base_bad_cat}/`; driver `tests/test_edition_hardening.cpp`; cmake `exchange_edition_hardening`; CLI superseded line (WARN oracle 12); viewer `NOTICE: … superseded` (WARN oracle 13). CANCEL/UPD-SEQ/UPD-CORRUPT Closed already on this stack (`af07602` / `eb91435` / `0833930`). Do **not** reopen CANCEL / UPD-SEQ / UPD-CORRUPT / SIG / AGGREGATOR / CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY etc. Do **not** invent `EDITION-*` / `HARDEN-*` failure prefixes. Do **not** change tip-truth oracles / enum / check order / `test_update_robustness.cpp` / `test_cancel_reissue.cpp`.

**O-questions (BRIEF — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `e3c0b2d` already; no `ci.yml` / no large Eng) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green — **yes / freeze**.
3. Keep tip-truth oracles 0–11 MUST / 12–13 WARN (edition harden face Closed object; do **not** reopen CANCEL/UPD-SEQ/UPD-CORRUPT) — **yes / freeze**.
4. CANCEL / UPD-SEQ / UPD-CORRUPT stay Closed — **yes / freeze**; do **not** reopen.
5. `grid()` / shim Backlog — **yes / freeze**.
6. Failure face = bare CHECK + enum (or existing SSE/`sse_map`) — **yes / freeze**; do **not** invent `EDITION-*` / `HARDEN-*`.
7. After Closed next = **TBD** — **yes / freeze**; do **not** invent.

**CANCEL / UPD-SEQ / UPD-CORRUPT Closed (kept — do not reopen):** CANCEL tip `af07602`; Open `80e20dd`≡`e7c99fb`; Close CI `40198622771` dual-green; Open-bind `40197755849`; UPD-CORRUPT tip `eb91435`≡`17eea30`; UPD-SEQ tip `0833930`≡`bb337ce`; O1 tip-truth/docs-close SUCCESS.

Cleanup slice on top of `docs/cancel-reissue-contract.md` rev 2.1 (CANCEL **Closed** — **do not** reopen) and `docs/update-robustness-contract.md` rev 1.4 (UPD-SEQ + UPD-CORRUPT **Closed** — **do not** reopen). Same module boundary: all decisions in `s100_core`; CLI/viewer only display. Strong guarantee unchanged: **a rejected update or a rejected base load leaves the caller’s state unchanged** (`Dataset` bit-for-bit, session map untouched).

---

## Scope

1. **One rule for old-edition updates** on every series path (`open_dataset_with_updates`, `open_cell_series`, `ChartSession`, viewer, CLI): update ED < base ED → **superseded** (skipped, listed, not an error).
2. **Strict base DSED**: a base `.000` whose DSID DSED does not parse, whose DSID is missing, or whose edition is 0 is **rejected**, never loaded as ED 0.

## Findings in the code (45c355e)

- **Exchange-set path** catches old editions twice: `group_cell_series` (`exchange_set.cpp` ~282, catalogue `editionNumber` < base catalogue ED) and `open_cell_series` (~384–401, a second ad-hoc DSID pre-filter when the catalogue has no ED). Both fill `UpdateReport::superseded`.
- **`open_dataset_with_updates`** has no such step: the update reaches `apply_in_place` step 9 (`s101_update.cpp` ~720) → `EditionMismatch`, series stops.
- **Viewer uses `open_dataset_with_updates`**, not `ChartSession`: `src/viewer/main.cpp` ~1531–1540 calls `discover_update_files(p)` then `open_dataset_with_updates(p, updates, …)` for a `.000`. Its `ChartSession` use (~1712, ~1758, ~1795, ~1832) only `insert`s the already-opened S-101 dataset for S-102/104/111/124 overlays. So today an ED 10 base next to a leftover ED 9 `.001` shows `WARNING: update rejected (…, code=EditionMismatch)`. `s100_cli <cell.000>` (`src/cli/main.cpp` ~222–226) uses the same path and exits non-zero.
- **Base DSED**: `load_iso8211_file` (`iso8211.cpp` ~497–508) does `out = Dataset{}` then reads DSID, and **ignores `id.ok == false`** (and a missing DSID): the cell loads with ED 0 / UPD 0. Since cancel-reissue, ED 0 means “cancelled”. `parse_dsid_field` itself is already strict (rev 2.1 prerequisite fix); only the base loader drops the result.
- **Base-load error surface today**: `load_s101` / `open_dataset` → `false` + `LoadError` (`Invalid` for DSID problems, cf. `read_s101_dsid`); `open_dataset_with_updates` / `open_cell_series` → `classify_base_failure` → `UpdateReport{rejected = base, code = BadStructure | Truncated}`, message `base rejected: …`; `ChartSession::load_member` inserts no member when `report.rejected == base`; viewer/CLI show the load error.

## Decisions

- **E1 Superseded in series context.** On any series path, an update with 0 < ED < base ED is not applied, does not stop the series, is listed in `report.superseded`, and sets `notice` “superseded updates can be removed”. `code` stays `None`, `rejected` stays empty, return value unaffected.
- **E2 Standalone `apply_s101_update` stays `EditionMismatch`** for 0 < ED < installed ED. A single call has no series and no “base” to be superseded by; the caller asked to apply this exact file, so refusing it is correct. This keeps rev 1.4’s `bad_ed` assertion and cancel-reissue oracle 5’s last line unchanged.
- **E3 Exact edition rule** (series and standalone): ED == 0 → cancel (cancel-reissue #2); ED > installed → **`NewerEditionPending`** (unchanged, always, both contexts); ED == installed → continue to sequencing; 0 < ED < installed → **superseded** (series) / **`EditionMismatch`** (standalone). Consequence: `EditionMismatch` is now produced **only** by standalone `apply_s101_update` with a lagging edition; no series path can return it. Catalogue-vs-DSID disagreement stays `CatalogueMismatch`.
- **E4 One helper, so the paths cannot drift.** A single pure function decides the relation; `group_cell_series` (catalogue numbers) and `apply_in_place` (DSID numbers) both call it. The ad-hoc DSID pre-filter in `open_cell_series` is **removed**; the in-apply step covers it for both `open_cell_series` and `open_dataset_with_updates` (they share `detail::open_s101_with_updates`).
- **E5 Strict base DSED.** A base cell is rejected when DSID is missing, DSED fails `parse_dsid_field` (same accept/reject rules as updates: bare integer and `ED.UPD` OK; `""`, `"9.x"`, `"x"`, `"9."`, `".1"` fail), **or DSED edition is 0**. Error surface = the existing one (no new enum value): `LoadErrorKind::Invalid` from `load_s101`/`open_dataset`; `UpdateRejectCode::BadStructure` with `rejected == base` from the update paths.
- **E6 Base with DSED `"0"` is rejected.** S-101 PS 2.0.0 §11.3.2/§11.3.3 and B-7.1.1 define ED 0 only for a cancellation **update file** (DSID only, next sequence number), never for a `.000`. Loading it would create a dataset with ED 0 that is neither cancelled (`cancelled == false`) nor updatable (every real update would hit `NewerEditionPending`) — an impossible state. Treating it as “cancel” instead would let a mis-named file silently wipe a cell, which D4/D5 deliberately guard. Fileless cancel is a separate non-goal.

## API (additive)

```cpp
// exchange_set.hpp
enum class UpdateEditionRelation { Cancel, Current, Superseded, NewerPending };
/** E3/E4: the single edition rule. update_ed == 0 -> Cancel; > installed -> NewerPending;
 *  == installed -> Current; 0 < update_ed < installed -> Superseded. Pure, no I/O. */
UpdateEditionRelation classify_update_edition(int installed_ed, int update_ed);
```

- No new `UpdateRejectCode`; no new `UpdateReport` field. `UpdateReport::superseded` is now filled by `open_dataset_with_updates` too (doc comment updated: “old-edition updates skipped on any series path”).
- `UpdateRejectCode::EditionMismatch` comment becomes “standalone `apply_s101_update`: 0 < update ED < installed ED”.
- Internal: `ApplyOptions` gains `bool series{false}`; `ApplyOutcome` gains `bool superseded{false}`. `apply_s101_update` passes `series = false`; `detail::open_s101_with_updates` passes `true`.
- `load_s101` / `open_dataset(.000)`: signature unchanged; new `false` cases per E5/E6. Message names the file and contains `DSED` (or `no DSID`). `out` is **not touched** on any `false` return of `load_s101` (decode into a local, move on success — today `out = Dataset{}` runs first; fix that).

## Check order

**Updates** (cancel-reissue #1, unchanged except step 9): Truncated → BadStructure → DatasetNameMismatch → DatasetCancelled → cancel detection (ED 0) → NotUpdateFile → CatalogueMismatch → **edition: `classify_update_edition(ds.edition_number, ED)`** — `NewerPending` → `NewerEditionPending`; `Superseded` → series: **skip, `outcome.superseded = true`, dataset unchanged** / standalone: `EditionMismatch` → OutOfSequence (re-issue skip window first) → records.

So every earlier code still wins over “superseded”: a leftover old-edition file that is truncated, corrupt, names another cell, targets a cancelled dataset, or disagrees with its catalogue entry is rejected with that code, as today. Superseded files are skipped in list order; they do not advance UPD and do not affect the sequence check of later files.

`group_cell_series` keeps its catalogue-level pre-classification (it must pick the highest-ED base before any file is read), now via `classify_update_edition(base_cat_ed, upd_cat_ed)`; only `has_edition_meta` entries are pre-classified there. `report.superseded` = catalogue pre-classified ∪ in-apply superseded, sorted by path, filled also when the series later stops on a rejection.

**Base load** (`load_iso8211_file`, before any feature decode): ISO 8211 parse (existing `Parse` errors) → **DSID present** → **DSED parses** → **ED ≥ 1** → features (existing “no features decoded”). `open_cell_series`’ base catalogue check (`CatalogueMismatch`) is unchanged and still runs first when the base DSID is readable.

**Session / viewer / CLI (thin).** `ChartSession::load_member`: unchanged code; via `open_cell_series` it now gets E1 for catalogues without update ED; a base rejected per E5/E6 → `false`, no member inserted, any member already in the map for that key untouched, `last_update_report(key).code == BadStructure`. Viewer: unchanged path; on success with `!rep.superseded.empty()` the status bar appends `NOTICE: <N> superseded update(s) can be removed` (WARN); a rejected base shows the existing load-error text. CLI: `--with-updates` line already prints `superseded=N`; the viewer scenario now exits 0.

## Call paths: what changes and what stays

| Path / situation | rev 2.1 (today) | rev 1 (this slice) |
|------------------|-----------------|--------------------|
| `open_dataset_with_updates`, update 0 < ED < base ED | `EditionMismatch`, series stops | **superseded**, series continues, `true` |
| Viewer sibling auto-apply (`discover_update_files` → `open_dataset_with_updates`) | status-bar `update rejected … EditionMismatch` | **draws base (+ later same-ED updates), NOTICE superseded** |
| `s100_cli <cell.000>` (same path) | exit ≠ 0 | **exit 0**, `superseded=N` |
| `open_cell_series` / `ChartSession`, update without catalogue ED, DSID ED < base | superseded (ad-hoc pre-filter) | superseded (shared in-apply step) — **same result** |
| `group_cell_series`, catalogue ED < base catalogue ED | superseded | superseded (via helper) — **same result** |
| Standalone `apply_s101_update`, 0 < ED < installed | `EditionMismatch` | **unchanged `EditionMismatch`** (E2) |
| Any path, ED > installed | `NewerEditionPending` | **unchanged** |
| Any path, ED 0 update | cancel (#2) | **unchanged** |
| Base `.000` with bad / missing DSED or DSID | loads as ED 0 | **rejected**: `Invalid` / `BadStructure` |
| Base `.000` with DSED `"0"` | loads as ED 0, not cancelled | **rejected** (E6) |

## Existing assertions (Tester checks each; none may be edited)

Every existing assertion is **unchanged**. The only rev 1.4 fixture pinning `EditionMismatch` (`bad_ed`) goes through standalone `apply_s101_update` (E2). No existing test asserts `EditionMismatch` through `open_dataset_with_updates`, `open_cell_series` or a session, and every committed S-101 base parses with ED ≥ 1 (`101AA00DS0003/0006/0008.000`: `9.0`/`10.0`/`8.0`; `nodot` `9`; `newed` `10.0`; `reissue` `9.2`; `notupd` `9.0`).

| File:line | Case / fixture | Expectation | After this slice |
|-----------|----------------|-------------|------------------|
| `test_update_robustness.cpp:170` | root base `load_s101` | ED 9 UPD 0 | unchanged |
| `test_update_robustness.cpp:212` | `seq` 001–003 via `open_dataset_with_updates` | applied 3 | unchanged (all ED 9) |
| `test_update_robustness.cpp:222–224` | reversed / duplicate / gap via `open_dataset_with_updates` | `OutOfSequence` | unchanged (ED == base) |
| `test_update_robustness.cpp:226–227` | gap / duplicate via `apply_s101_update` | `OutOfSequence` | unchanged |
| `test_update_robustness.cpp:228–229` | **`bad_ed/…001` (ED 8) via standalone `apply_s101_update`** | **`EditionMismatch`, “8.1”** | **unchanged (E2)** |
| `test_update_robustness.cpp:230–241` | notupd / trunc / leader / ruin9 / missing / late_fail | own codes | unchanged (earlier steps or ED 9) |
| `test_update_robustness.cpp:269` | `catalog/` via `open_cell_series` | `CatalogueMismatch` | unchanged (step 7 precedes edition step; ED 9) |
| `test_update_robustness.cpp:290` | `session/` `load_member` | `MissingTarget`, UPD 1 | unchanged |
| `test_update_robustness.cpp:316` | `testdata/exchange_set` success | code `None` | unchanged (bases 8.0 / 9.0) |
| `test_update_robustness.cpp:324` | every code seen | incl. `EditionMismatch` | unchanged (seen via 228) |
| `test_cancel_reissue.cpp:185` | `chain()` helper | `true` | unchanged |
| `test_cancel_reissue.cpp:197, 208` | `nodot` `"9"`, root base | load | unchanged |
| `test_cancel_reissue.cpp:213` | `dsed_bad/*` updates | `BadStructure` | unchanged |
| `test_cancel_reissue.cpp:238, 252, 270` | cancel / cancel_ooo | cancel semantics | unchanged |
| `test_cancel_reissue.cpp:285, 303` | reissue | skipped 2 / `OutOfSequence` | unchanged |
| `test_cancel_reissue.cpp:318–328` | `newed` via `open_cell_series` | superseded {001}, applied 0 | unchanged (catalogue pre-classification kept) |
| `test_cancel_reissue.cpp:330` | **`newed/001` on ED 10 via standalone `apply_s101_update`** | **`EditionMismatch`** | **unchanged (E2)** |
| `test_cancel_reissue.cpp:339` | `pending` via `open_dataset_with_updates` | `NewerEditionPending` | unchanged |
| `test_cancel_reissue.cpp:363–396` | session_replace A/B/X/C | replace / cancel | unchanged (no lagging ED) |
| `test_exchange_set.cpp:99–110` | CHS series; re-apply `.001` | `true`; msg contains `ED/UPD` | unchanged (same ED → `OutOfSequence`); CHS data optional |

**Deviation from “do not modify tests”: none required.** If Eng finds any of the above would change, Eng stops and reports to Architect + Tester instead of editing the test. The only test-support file this slice touches is `tests/check_update_fixtures.py` (accept the new table, below); `tests/test_update_robustness.cpp` and `tests/test_cancel_reissue.cpp` stay byte-identical (Tester checks `git diff`).

Doc note (not a test change): rev 1.4 rule 3b “ED must equal” and cancel-reissue #5 last sentence now describe the **standalone** call; series paths follow E1.

## Fixtures (synthetic, `tools/gen_update_fixtures.py`, seed 20261018)

All new files under `testdata/s101/updates/edition/<case>/`, pinned in a **new table `[[edition_fixture]]`** in `fixtures.toml` (generator maps first path component `edition` → that table; `[[update_fixture]]` count stays as rev 1.4 asserts). `tests/check_update_fixtures.py` adds `[[edition_fixture]]` to its manifest set, so `update_fixtures_reproducible` covers them: two generator runs byte-identical, sha256 == manifest, committed bytes == regenerated. Missing fixture → FAIL. Each base is the root `101AA00DS0003.000` records with only DSID changed.

| Case dir | Content | Expected |
|----------|---------|----------|
| `base_dsed_empty/`, `base_dsed_ed_x/`, `base_dsed_x/`, `base_dsed_ed_dot/`, `base_dsed_dot_upd/` | `101AA00DS0003.000` with DSED `""`, `"9.x"`, `"x"`, `"9."`, `".1"` | rejected (E5) |
| `base_dsed_zero/` | `.000` with DSED `"0"`, PROF `"1"` | rejected (E6) |
| `base_no_dsid/` | `.000` with the DSID record removed | rejected (E5) |
| `base_bad_cat/CATALOG.XML` | lists `../base_dsed_x/101AA00DS0003.000` (no ED meta) + `../../seq/…001` | session: base rejected |
| `old_left/` | base DSED `"10.0"` + ED 9 `.001` (`"9.1"`) + ED 9 `.002` (`"9.2"`) | superseded {001, 002}, applied 0 |
| `mixed/` | base `"10.0"` + `.001` DSED `"10.1"` (ops of `seq/001`) + leftover `.002` `"9.2"` | applied 1, UPD 1, superseded {002} |
| `nocat/CATALOG.XML` | lists `../old_left/*`; base has `editionNumber` 10, updates **omit** `editionNumber` | in-apply superseded {001, 002} |

Reused read-only: `newed/` (ED 10 base + ED 9 `001`, the exact viewer scenario), root base, `nodot/`, `seq/`, `bad_ed/`, `wrong_name/`, `dsed_bad/x.001`, `pending/`.

## Oracle — CTest `exchange_edition_hardening` (no SKIP)

| # | Check | Sev |
|---|-------|-----|
| 0 | Positive controls first: root base and `nodot` load (9/0); `seq` 001–003 via `open_dataset_with_updates` applied 3. Unit table for `classify_update_edition`: (9,0)→Cancel, (9,9)→Current, (9,10)→NewerPending, (10,9)→Superseded, (10,1)→Superseded | MUST |
| 1 | Every `[[edition_fixture]]` exists, sha256 matches; `update_fixtures_reproducible` passes with them included | MUST |
| 2 | **Viewer scenario**: `open_dataset_with_updates(newed/000, discover_update_files(newed/000))` → `true`, code `None`, `rejected` empty, `applied == 0`, `superseded == {newed/001}`, notice contains “superseded updates can be removed”, digest == `open_dataset(newed/000)`. Same for `old_left` with superseded {001, 002} | MUST |
| 3 | `mixed`: `true`, `applied == 1`, ED 10 UPD 1, `superseded == {002}`, digest == base + `.001` applied alone | MUST |
| 4 | **Parity**: for `newed` (catalogue) and `nocat`, `open_cell_series` and `open_dataset_with_updates` on the same files give equal digest, `applied`, `code`, `superseded` set. `ChartSession` on `nocat`: `load_member` `true`, member ED 10 UPD 0, `last_update_report.superseded.size() == 2`, code `None` | MUST |
| 5 | Earlier codes win over superseded: `open_dataset_with_updates(newed/000, {wrong_name/101AA00DS0006.001})` → exactly `DatasetNameMismatch`; `(newed/000, {dsed_bad/x.001})` → exactly `BadStructure`; `superseded` empty, `out` digest == ED 10 base in both | MUST |
| 6 | Standalone stays (E2): `apply_s101_update(ED 10 ds, newed/001)` and `apply_s101_update(root base, bad_ed/001)` → exactly `EditionMismatch`, digest unchanged; `(root base, pending/001)` → `NewerEditionPending` | MUST |
| 7 | Each `base_dsed_*` and `base_no_dsid`: `open_dataset` → `false`, `LoadErrorKind::Invalid`, message names the file and contains `DSED` (or `no DSID`); a pre-filled `out` keeps its digest. `open_dataset_with_updates(base, {seq/001})` → `false`, code exactly `BadStructure`, `rejected == base`, `applied == 0` | MUST |
| 8 | `base_dsed_zero` (E6): same as 7, **and** never returns `true` with `edition_number == 0` or `cancelled == true` | MUST |
| 9 | Session base reject: `base_bad_cat` → `load_member` `false`, `find(key) == nullptr`, `last_update_report(key).code == BadStructure` | MUST |
| 10 | Invariant scan: every committed `testdata/s101/**/*.000` that `open_dataset` accepts has `edition_number ≥ 1` | MUST |
| 11 | No crash / exception on any fixture; `exchange_update_robustness` and `exchange_cancel_reissue` pass; their test files are not modified (`git diff`); every row of the assertions table holds | MUST |
| 12 | `s100_cli newed/101AA00DS0003.000 --with-updates` prints `code=None … superseded=1`, exit 0 | WARN |
| 13 | Viewer status bar `NOTICE: 1 superseded update(s) can be removed` for `newed/` | WARN |

## Out of scope

- Fileless cancel via catalogue `purpose` (S-100 §17-4.4.1) — still the cancel-reissue non-goal.
- Deleting superseded files from disk (report only; the user/ECDIS operator removes them).
- Issue-date checks; S-57 cells (`ENC_ROOT/*.000` have binary DSID and are not S-101 — after E5 they fail with the DSED message instead of loading as ED 0, which is correct but not a supported path).
- Changing `discover_update_files` (extension order, stop at first gap) or `group_cell_series` base selection.
- Signatures, permits, S-164 power-up, S-102+ updates.
