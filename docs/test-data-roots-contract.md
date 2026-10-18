# Test data roots: one CHS root, absolute fixture paths (rev 1)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Tag: **`TEST-DATA-ROOTS`** — test infrastructure, **not an IHO case id** and **no CTest of its own**. Deliberately **not** listed in `docs/testing/iho-ctest-case-ids.md` (that table only carries `IHO-*` ids with a CTest landing point). Docs-only contract; **no engine code, no assertion, no fixture byte, no CMake, no CI change**.

## Findings in the code (e3c0b2d)

CHS data (`testdata/chs/`, gitignored, testing-only licence) exists only in the shared clone. Each test finds it its own way, so in a worktree everything below prints “skip”/“not exercised” even when `S100_CHS_ROOT` is set:

| File:line | Today | Problem |
|-----------|-------|---------|
| `test_layers_all_cells.cpp:26` | `S100_SOURCE_DIR/testdata/chs/COMBINED/S100_ROOT/S-101` | ignores env |
| `test_portrayal.cpp:65` (`cells()`, used by `portrayal.portrayal_all_cells`) | same | ignores env |
| `test_fc_primitives.cpp:47` | same | ignores env |
| `test_soundings.cpp:17–18` | **relative** `testdata/chs`, then `CMAKE_SOURCE_DIR_STR/testdata/chs`; walks all `.000` whose path string contains `S-101` | ignores env; cwd-dependent; string match on path |
| `test_edition_hardening.cpp:443–456` (S-57 block) | `S100_SOURCE_DIR/testdata/chs/COMBINED/ENC_ROOT`, **first** `.000` of a recursive walk (order unspecified; `CA54GN2A.000` in the baseline) | ignores env; non-deterministic pick |
| `test_s102.cpp:31–33` (`find_s102_cell`), `:210` (multi-product series) | `S100_SOURCE_DIR/testdata/chs/COMBINED/S100_ROOT/...` | ignores env |
| `test_exchange_set.cpp:60–62` (lone `.001` refusal) | source-dir path | ignores env |
| `test_exchange_set.cpp:74–75`, `test_chs_bounds.cpp:20–21` | `getenv("S100_CHS_ROOT")` = **`.../COMBINED/S100_ROOT`**; set-but-missing → silent skip | second meaning of the variable; silent skip |
| `test_fixtures_pick.cpp:19–20`, `test_aggregates.cpp:18–19`, `test_load.cpp:18–19`, `test_attributes.cpp:16–17` | `S100_TESTDATA` else **relative** `testdata/s101` | `build/s100_tests` with no env: `test_fixtures_pick` CHECK fails (then `test_aggregates` would); `test_load`/`test_attributes` silently skip. Passes under CTest only because `CMakeLists.txt` sets absolute `S100_TESTDATA` on the `s100_tests` entry |

Not affected: `test_update_robustness.cpp:40`, `test_cancel_reissue.cpp:41`, `test_edition_hardening.cpp:50` (`s101_dir()` is already absolute from `S100_SOURCE_DIR`); all other tests use `S100_SOURCE_DIR`/`CMAKE_SOURCE_DIR_STR` absolute paths. Absent CHS is a **pass with a message, never a CTest SKIP** (no `SKIP_RETURN_CODE` anywhere); that stays.

## Decisions

- **R1 One header-only helper** `tests/support/test_data_roots.hpp` (namespace `s100test`, `#error` without `S100_SOURCE_DIR`; both `s100_tests` and `s100_portrayal_tests` already define it, and `#include "support/test_data_roots.hpp"` resolves relative to `tests/`, so **no CMake change**):
  ```cpp
  const ChsRoot& chs_root();      // resolve once (below); FAIL + std::exit(1) on a bad root
  fs::path chs_s100_root();       // <root>/COMBINED/S100_ROOT
  fs::path chs_s101_dir();        // <root>/COMBINED/S100_ROOT/S-101
  fs::path chs_enc_root();        // <root>/COMBINED/ENC_ROOT
  bool chs_exercised(const char* tag); // prints the line below, returns true iff a root is resolved
  fs::path fixture_root();        // S100_TESTDATA if set, else S100_SOURCE_DIR/testdata/s101 (absolute)
  ```
- **R2 Resolution order (CHS).** `S100_CHS_ROOT` **set and non-empty** → it is the root; if it is missing or lacks the layout → **FAIL**, never fall back. **Unset (or empty)** → `S100_SOURCE_DIR/testdata/chs` if that directory exists. Else → **not exercised**, test passes. A resolved root (env **or** default) is strict: every layout entry must exist, otherwise FAIL listing the missing paths (a half-copied CHS tree is a bug, not optional data). Relative env values are made absolute against the cwd once and printed absolute.
- **R3 Every CHS site prints what it used** (stdout, one line per call site): `chs: <tag> using "<abs root>" (S100_CHS_ROOT|default)` or `chs: <tag> not exercised (S100_CHS_ROOT unset, "<src>/testdata/chs" absent)`. Tags: `soundings`, `chs_bounds`, `fc_primitives`, `layers_all_cells`, `exchange_set.upd001`, `exchange_set.chs`, `s102.open`, `s102.series`, `portrayal_all_cells`, `edition_hardening.s57`. Old `skip chs…`/`skip CHS cells…`/`skip soundings…` texts are replaced by it.
- **R4 Pinned files, not walks, where a test needs one file.** The S-57 check uses `chs_enc_root()/CA/CA54GN2A/2/0/CA54GN2A.000` (the file the baseline exercised). Cell-set walks (`layers_all_cells`, `fc_primitives`, `portrayal`, `soundings`) walk `chs_s101_dir()` only, by `extension() == ".000"`; this is the same set today’s `soundings` string match finds (the two `101CA…` cells).
- **R5 Fixtures from an absolute path.** `fixture_root()` replaces the four relative sites. `S100_TESTDATA` set and non-empty is used verbatim (as today); if `<it>/101AA00DS0008.000` is missing → FAIL (same rule as R2). The existing per-test skip branches stay in the code (no assertion edits) but are unreachable with committed fixtures. `s101_dir()` in the update tests is **not** migrated (CI exports a relative `S100_TESTDATA` to every CTest entry; only the `s100_tests` entry overrides it absolute).

## Named changes (tell everyone)

- **NC1 `S100_CHS_ROOT` now has one meaning: the `testdata/chs` level** (the directory that contains `COMBINED/`). Before, `test_exchange_set`/`test_chs_bounds` read it as `.../COMBINED/S100_ROOT`. Anyone with `S100_CHS_ROOT=.../COMBINED/S100_ROOT` must change it to its grandparent. The helper detects the old value (`<env>/CATALOG.XML` exists and `<env>/COMBINED` does not) and FAILs with that hint; it does not auto-correct.
- **NC2 Set-but-bad `S100_CHS_ROOT` FAILs** (was: silent skip in `test_exchange_set`/`test_chs_bounds`, ignored elsewhere).
- **D1 Deviation (frozen file): `tests/test_edition_hardening.cpp`**, S-57 block lines 443–456 only (comment, `enc` path → `chs_exercised("edition_hardening.s57")` + R4 pinned path, not-exercised message). Lines 457–461 (`run_cli`, both `ECHECK`s) and everything else byte-identical. `tests/test_update_robustness.cpp`, `tests/test_cancel_reissue.cpp`: **empty diff**.

## Expected layout under `S100_CHS_ROOT` (matches the shared clone and `testdata/s102/SOURCE.md`)

```
<S100_CHS_ROOT>/                                   e.g. /workspace/s100-chart/testdata/chs
  COMBINED/S100_ROOT/CATALOG.XML                   exchange_set.chs, s102.series
  COMBINED/S100_ROOT/S-101/DATASET_FILES/CA00/101CA00P468N0712W/2/101CA00P468N0712W.000  (+ .001: exchange_set.upd001)
  COMBINED/S100_ROOT/S-101/DATASET_FILES/CA00/101CA00T460N0720W/6/101CA00T460N0720W.000
  COMBINED/S100_ROOT/S-102/DATASET_FILES/CA00/102CA0064682N07116W/16/102CA0064682N07116W.h5
  COMBINED/ENC_ROOT/CA/CA54GN2A/2/0/CA54GN2A.000   edition_hardening.s57 (S-57; sibling of S100_ROOT, not under it)
```

These six paths are the layout check (one table in the helper). S-101 cells live under `COMBINED/S100_ROOT/S-101/`; S-57 under `COMBINED/ENC_ROOT/`. Other CHS content (`RTZ`, `S124`, `S129`, zip, PDFs) is ignored.

## Windows

Read the variable with `_wgetenv` on `_WIN32` (non-ASCII paths), `std::getenv` elsewhere; empty counts as unset on both (cmd `set X=` deletes). Build paths only with `fs::path` `operator/` — no `S100_SOURCE_DIR "/…"` literal concatenation, no `path.string().find(…)`; compare by `fs::exists`/`extension()`, never by string. Printed paths use native separators; Tester greps the `chs: <tag> using` prefix, not separators. MSVC is multi-config: the binary is `build\Release\s100_tests.exe`.

## Oracles (Tester)

| # | Check | Sev |
|---|-------|-----|
| a | Worktree build, `S100_CHS_ROOT=/workspace/s100-chart/testdata/chs ctest --test-dir build`: 34/34 pass; `chs: <tag> using "/workspace/s100-chart/testdata/chs" (S100_CHS_ROOT)` for all ten tags; and the baseline lines from the shared clone (`build/Testing/Temporary/LastTest.log`, 2026-10-18 13:18 UTC+8): `soundings ok` ×2, `chs bounds … ok` ×2, `fc primitives checked "101CA…"` ×2, `layers "101CA…"` ×2, `open_dataset(.001) refused`, `exchange chs ok cell=S-101:101CA00P468N0712W … final_ed.upd=2.1`, `S-102 open "102CA0064682N07116W.h5"`, `CHS exchange S-102 member … loaded`, `S-57 CA54GN2A.000 exit=1` (in `s100_tests` and `exchange_edition_hardening`); `portrayal.portrayal_all_cells` portrays both CHS cells. Zero `not exercised` / `skip chs` / `skip CHS` / `skip soundings` lines | MUST |
| b | No CHS anywhere (worktree, `S100_CHS_ROOT` unset; Linux + Windows CI): every tag prints `not exercised`, all pass; `ctest -N` total **unchanged** per platform (34 on Linux at e3c0b2d); same pass/fail set as before; no CI file change | MUST |
| c | `S100_TESTDATA` unset: `cd build && ./s100_tests` (Windows `build\Release\s100_tests.exe`, run from `build\` and `build\Release\`) exits 0 with `fixture nw_corner/se_corner/center ok` and `all tests passed`; same from an unrelated cwd (e.g. `/tmp`) | MUST |
| d | Full CTest green with and without CHS, both platforms | MUST |
| e | Negative: `S100_CHS_ROOT=/nonexistent` → `s100_tests`, `exchange_edition_hardening`, `portrayal.portrayal_all_cells` FAIL naming the variable, also in the shared clone where the default exists (no fallback); `S100_CHS_ROOT=<chs>/COMBINED/S100_ROOT` → FAIL with the NC1 hint; a copy of the tree missing `CA54GN2A.000` → FAIL listing it | MUST |
| f | Grep checks below: first hits **only** `tests/support/test_data_roots.hpp` (today 18 lines in 12 files), second is **empty** (today 5: the relative `testdata/…` sites) | MUST |
| g | `git diff e3c0b2d -- tests/test_update_robustness.cpp tests/test_cancel_reissue.cpp tests/test_pick.cpp src include testdata CMakeLists.txt .github` empty; `test_edition_hardening.cpp` diff only inside D1; no `CHECK`/`ECHECK`/`UCHECK` line added, removed or edited in any file (keep local variable names such as `root`, `chs`, `cell`, `enc`→its replacement only inside D1, so `CHECK` lines stay byte-identical) | MUST |

```sh
rg -n 'S100_CHS_ROOT|S100_TESTDATA|testdata/chs|COMBINED|ENC_ROOT' tests --glob '*.cpp' --glob '*.hpp'
rg -n '"testdata' tests --glob '*.cpp' | rg -v '(S100_SOURCE_DIR|CMAKE_SOURCE_DIR_STR)\)? ?/|\bsrc / |kSrc / '
```

## Tests touched (exact)

New `tests/support/test_data_roots.hpp`; edited `test_layers_all_cells.cpp`, `test_portrayal.cpp` (`cells()` only), `test_soundings.cpp`, `test_fc_primitives.cpp`, `test_s102.cpp` (two CHS paths only; NOAA fallback unchanged), `test_exchange_set.cpp`, `test_chs_bounds.cpp`, `test_edition_hardening.cpp` (D1), `test_fixtures_pick.cpp`, `test_aggregates.cpp`, `test_load.cpp`, `test_attributes.cpp`.

## Out of scope

- CHS data in CI, CTest `SKIP_RETURN_CODE`, new or renamed CTest entries.
- `scripts/render_encdotnet_oracle.sh`, `scripts/dual_capture_oracle.sh` (hard-coded `testdata/chs/...`), `tools/*.py`, `testdata/*/SOURCE.md`.
- The relative `S100_TESTDATA: testdata/s101` in `.github/workflows/ci.yml`; `s101_dir()` in the update tests; the NOAA S-102 sample lookup; SHOM data.
- Non-ASCII `S100_SOURCE_DIR` on Windows (narrow compile-time literal, pre-existing).
