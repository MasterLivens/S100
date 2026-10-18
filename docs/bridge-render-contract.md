# Bridge / composite feature rendering — FASC slice (rev 2.3 · association-contract rev 3)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S101-BRIDGE-RENDER`** (**Active** — reuse; do **not** invent a new case name). CTest: **`fasc_bridge_render`** — **MUST NOT SKIP**; registered with no platform guard; **Linux + Windows** dual-green.
**Status: Closed / Done (2026-10-18)** — CI `39117072306` @ `065887b` dual-green `fasc_bridge_render`; oracles 1–11 stay MUST; **#12 BRIDGE01≥1** promoted WARN→MUST. Docs close only — Eng tiny tip if #12 assert not yet hard; next slice `IHO-S101-TEXT-COLLISION`.

Builds on `docs/association-contract.md` rev 2 (FASC/INAS merge + Host queries, `35b5b12`). Frozen with `docs/portrayal-contract.md`, `docs/display-priority-contract.md`, `docs/testing/iho-ctest-case-ids.md`.

## Principle (unchanged, re-stated)

Bridge symbology comes **only** from official S-101 PC 2.0.0 Lua (`Bridge.lua`, `SpanFixed.lua`, `SpanOpening.lua`, `PylonBridgeSupport.lua`). **No hand-written Bridge drawing in Qt or C++**, no local Lua edits. This slice proves the PC output reaches the screen correctly; it does not invent rules.

## Fixture (pinned, already gittracked)

Path is fixed: `testdata/s101/101AA00DS0006.000` (committed IHO sample; CHS cells are not in the repo, so they are not used). **File missing or load fails → FAIL, never SKIP.**


`testdata/s101/101AA00DS0006.000` — measured on `f7c3709` via `s100_cli --layers` / `--portray --errors`:

| feature_type | count |
|--------------|-------|
| `Bridge` | 10 |
| `SpanFixed` | 6 |
| `SpanOpening` | 4 |
| `PylonBridgeSupport` | 6 |

Cell-wide portray: `features=113 emitted=113 lua_errors=0 features_without_instructions=0`.

## What the PC emits (from the official rules)

| Feature | PrimitiveType | Expected DI shape |
|---------|---------------|-------------------|
| `Bridge` | None (aggregate, no SPAS) | `NullInstruction`, `viewing_group 12210`, `drawing_priority 24` |
| `Bridge` | Curve / Surface | `LineInstruction` `_simple_` solid 1.6 `CHGRD`; + `PointInstruction:BRIDGE01` when `openingBridge == true`; optional name Text |
| `SpanFixed` | Curve / Surface | `LineInstruction` `_simple_` `CHGRD` (1.6 curve / 1.28 surface); clearance Text when present |
| `SpanOpening` | Curve / Surface | per official rule (line + symbol); clearance Text when present |
| all above | — | `viewing_group 12210`, `drawing_priority 24`, plane `UnderRadar` (or `OverRadar` when `RadarOverlay=true`) |

## Engine / viewer hooks (MUST)

1. **No new portray entry point.** Use existing `PortrayalSession::portray` on the S-101 Dataset; association Host functions from `association-contract` must be what Span/Pylon rules call.
2. **Null for aggregates:** a `Bridge` with no geometry yields only Null DI(s) — viewer draws nothing for it and does not error.
3. **Line style:** `_simple_` line styles from DEF (`SimpleLineStyle`) reach the painter with width + `CHGRD` token; palette remap Day/Dusk/Night like other lines.
4. **Symbol:** `BRIDGE01` resolves to `catalogues/S-101/PC_2.0.0/Symbols/BRIDGE01.svg` via the existing SymbolCache path; a missing file is an error, not a silent drop.
5. **Order:** Bridge DIs go through `filter_instructions_by_scale` → `sort_instructions_for_draw` like everything else (priority 24 paints above lower-priority area fills).
6. **Viewing-group switch (new, pure):** `include/s100/viewing_group_filter.hpp` —
   `void filter_instructions_by_viewing_group(DrawingInstructions& ios, const std::set<std::string>& hidden_groups);`
   removes every DI with **any** entry of `viewing_groups` in `hidden_groups` (so `11,12210` text hides with its bridge), keeps order of the rest. Pipeline: VG filter → SCAMIN filter → sort → paint. The viewer layer toggle calls it; no FASC-specific code in Qt.
7. **Clearance / name text** goes through the existing text-collision path; no special case.

## Oracle — CTest `fasc_bridge_render` (no SKIP)

Load the fixture with pinned S-101 FC+PC, default context (`RadarOverlay=false`). **No exact DI counts are pinned** (they change when official Lua is upgraded); only “> 0” and set relations.

Demoting check 3 to WARN is allowed **only** with `s100_cli` output attached proving 0006 lacks that parent link.

**Bridge graph G** (computed in the test from data, then printed to the log as TSV `bridge_id\tchild_id\tchild_type\tassociation\trole`): for each feature with `feature_type == "Bridge"`, resolve every FASC `rcid` to a feature via `record_id`; skip dangling. `B` = bridge ids in G, `C` = all resolved child ids.

| # | Check | Sev |
|---|-------|-----|
| 1 | Fixture exists and loads; feature counts equal the fixture table (10 / 6 / 4 / 6) | MUST |
| 2 | `lua_errors == 0` for the whole cell | MUST |
| 3 | `|B| ≥ 1` and `|C| ≥ 1`. FASC lives on the Bridge side only (`BridgeAggregation` / role `theComponent`; reverse lookup role `theCollection` must give the same graph). **Every** `SpanFixed`/`SpanOpening` has exactly a `Bridge` parent; every `PylonBridgeSupport` that has a FASC link has a `Bridge` parent — Host query confirms | MUST |
| 3w | Orphan `PylonBridgeSupport` (no FASC on either side, evidence `s100_cli --fasc`): the orphan set must **equal exactly** `{33, 43, 47, 69}` — an extra orphan, or a parented pylon (45, 46) losing its parent, is **FAIL**. Being orphan itself is reported as WARN; orphans still obey checks 5/11 | MUST (set) / WARN (orphan) |
| 4 | For each bridge in B, the C++ backing of `HostFeatureGetAssociatedFeatureIDs(bridge, association, "")` returns exactly that bridge's children in G (set compare, per association code) | MUST |
| 5 | Every `Bridge`/`SpanFixed`/`SpanOpening`/`PylonBridgeSupport` feature has DI count > 0. Every non-Null DI of these types has `12210` in `viewing_groups`. `drawing_priority == 24`, except `PylonBridgeSupport` Surface DIs `== 18` and plane `UnderRadar` in both RadarOverlay states (official `PylonBridgeSupport.lua:20`) | MUST |
| 6 | Every `Bridge` with Empty geometry has only `Kind::Null` DIs | MUST |
| 7 | **ON** (`hidden_groups = {}`): every id in B has ≥ 1 DI and every id in C has ≥ 1 non-Null DI — bridge and children come out **together** | MUST |
| 8 | **OFF** (`hidden_groups = {"12210"}`, matched against any entry of `viewing_groups`): no DI remains for any id in B ∪ C — filtered **together**; and the multiset of feature ids of DIs without `12210` in `viewing_groups` is identical to ON (nothing else affected) | MUST |
| 9 | Non-vacuous: ≥ 1 `SpanFixed` with Curve/Surface geometry exists; every such one has ≥ 1 `Kind::Line` DI with a line style colour `CHGRD` | MUST |
| 10 | Any DI with `symbol_ref == "BRIDGE01"` → file exists under PC `Symbols/` | MUST |
| 11 | Non-vacuous: B ∪ C has ≥ 1 non-Null DI. Re-portray with `RadarOverlay=true`: total DI count of B ∪ C unchanged, and the 12210 DIs of B ∪ C that were `UnderRadar` become `OverRadar` (PylonBridgeSupport priority‑18 surface branch stays UnderRadar per official rule) | MUST |
| 12a | Test log prints actual DI count per feature type (pinned as MUST in a later rev) | MUST (print) |
| 12 | ≥ 1 `BRIDGE01` on this fixture (openingBridge present on 0006) | **MUST** (promoted 2026-10-18; was WARN) |
| 13 | Viewer snapshot / pixel compare | **Deferred** — not CTest MUST; → `viewer_*` / DISPLAY-COMPARE |


## IHO-S101-BRIDGE-RENDER slice (2026-10-18) — MUST / Open / Non-goals

Planner freeze for the **next** Eng slice on top of rev 2.2. Case stays **`IHO-S101-BRIDGE-RENDER`** (Active). Research BRIEF TBD: `/workspace/research/fasc-bridge/` (**Open** — Planner fills; Architect does not invent BRIEF body here).

### MUST

1. **Prove on fixture 0006** (`testdata/s101/101AA00DS0006.000`, gittracked IHO sample): Bridge / SpanFixed / SpanOpening / PylonBridgeSupport **FASC graph** + Host query (`HostFeatureGetAssociatedFeatureIDs`) + PC Lua DI arrival — contract **`bridge-render-contract` rev 2.2 oracles 1–11** (table above). Missing fixture / load fail → **FAIL**, never SKIP.
2. **CI / local actually run** `s100_portrayal_tests bridge_render` (CTest names **`fasc_bridge_render`** + alias **`bridge_render`**). Recently observed **Not Run** / **missing portrayal binary** — this slice **MUST** close that gap to **always-green** on Linux + Windows (see § CI gap below).
3. CTest **`fasc_bridge_render` MUST NOT SKIP**; no platform `#ifdef` skip; no `SKIP_RETURN_VALUE`.

### CI gap — MUST close (exact wiring)

| Item | Location / fact |
|------|-----------------|
| Executable target | `s100_portrayal_tests` — `CMakeLists.txt` `add_executable(s100_portrayal_tests tests/test_portrayal.cpp tests/test_bridge_render.cpp)` (~L178); links `s100_core` + `s100_lua`; **not** `EXCLUDE_FROM_ALL` |
| Case entry | `tests/test_bridge_render.cpp` → `void bridge_render()`; dispatched from `tests/test_portrayal.cpp` map key `"bridge_render"` |
| CTest (contract) | `add_test(NAME fasc_bridge_render COMMAND s100_portrayal_tests bridge_render)` (~L187) |
| CTest (alias) | `add_test(NAME bridge_render COMMAND s100_portrayal_tests bridge_render)` (~L188) |
| Sibling portrayal cases | `foreach(...); add_test(NAME portrayal.${c} COMMAND s100_portrayal_tests ${c})` (~L182–184) — same binary |
| Platform / SKIP guards on these tests | **None** in CMake (no `SKIP_RETURN_VALUE`, no OS `if`) |
| Test-source SKIP | **None** — missing fixture/catalogues → `FAIL` (`BR_CHECK` / exit 1) |
| CI invoke | `.github/workflows/ci.yml`: `cmake --build build --config Release` then full `ctest` (Linux) / `ctest -E "viewer_"` (Windows). **`fasc_bridge_render` is not excluded** by `-E viewer_` |
| Failure mode to close | If `s100_portrayal_tests` is **not built** (selective `--target s100_tests`, stale build tree, or multi-config path miss), CTest reports **`***Not Run`** / unable to find executable — **not** a red Fail. That hides the gate. |

**Close criteria (Eng):** default `cmake --build` (CI shape) **always** produces `s100_portrayal_tests`; `ctest -R '^(fasc_bridge_render|bridge_render)$'` **runs and passes** on Linux + Windows; a missing binary must **not** silently Not-Run the gate (prefer build-all / explicit `--target s100_portrayal_tests` in CI, and/or `REQUIRED_FILES` / `$<TARGET_FILE:s100_portrayal_tests>` hardening — Eng choice). Bump stale CMake comment still saying “rev 2.1” when touching that block.

### Open (Planner)

| # | Topic | Notes |
|---|-------|-------|
| **O1** | BRIEF body | Fill `/workspace/research/fasc-bridge/` (directory exists empty). Architect leaves Open. |
| **O2** | Oracle 1–11 ambiguities | Any Tester/Eng clarification on rev 2.2 rows (esp. **3** / **3w** orphan set, **5** pylon surface 18, **8** VG multiset, **11** RadarOverlay plane flip) — record here after BRIEF; do not silently change MUST sev. |
| **O3** | Fixture 0006 path | Confirm pinned path remains `testdata/s101/101AA00DS0006.000` (counts 10 / 6 / 4 / 6). CHS cells stay out of CI. |

### Non-goals (this slice)

- **No** official S-101 PC Lua edits (`Bridge.lua` / `Span*.lua` / `PylonBridgeSupport.lua`).
- **No** hand-drawn bridge symbols in Qt/C++.
- **Do not reopen** `docs/association-contract.md` (FASC/INAS Host merge) or **`IHO-S164-POWERUP-INSTALL-PREFIX`** / install-prefix layout.
- Cross-dataset associations; S-98; vertical-clearance alerts (Track B); pinning exact DI counts (12a stays print-only).

## Non-goals (baseline, unchanged)

Hand-drawn bridge glyphs; editing official Lua; cross-dataset associations; S-98; vertical-clearance alerts (Track B).

## Notes

- `bridge_render` may stay registered as an alias; CI gating and the case-id row use `fasc_bridge_render`.
- Rulings by SoftwareTester 2026-10-18 on `1ac028f`: orphan pylons WARN with pinned set; text DIs only need `12210` in `viewing_groups`; pylon surface prio 18 / UnderRadar.
- **rev 2.3 (2026-10-18):** Active slice activation — CI Not Run / missing `s100_portrayal_tests` is MUST-to-close; BRIEF Open at `/workspace/research/fasc-bridge/`.

---

## Slice close — 2026-10-18 (Planner BRIEF `/workspace/research/fasc-bridge/BRIEF.md`)

**Status: Closed / Done** (formal docs close — **not** a re-implement of Host/FASC/bridge render).

- Evidence: CI [39117072306](https://github.com/MasterLivens/S100/actions/runs/39117072306) tip `065887b` — Linux+Windows **`fasc_bridge_render` Passed** (not Not Run / not Skip).
- Oracles **1–11** remain MUST as rev2.2; **#12 `BRIDGE01 ≥ 1`** promoted **WARN → MUST** (fixture 0006 has `openingBridge`). Eng: if assert not yet hard-failing, tiny tip only.
- **#12a** DI count: print-only — **no** numeric MUST this close.
- **#13** pixel / DISPLAY-COMPARE: **not** CTest MUST — defer `viewer_*` / DISPLAY-COMPARE.
- Viewer VG12210 UI switch: **Backlog**.
- Other structure–equipment FASC: **Backlog** (separate case).
- **Next Eng after this close:** `IHO-S101-TEXT-COLLISION` (`viewer_text_collision`); POINT-LABEL follows.

**Non-goals this close:** official Lua edits; hand-drawn bridge symbols; reopen association-contract / install-prefix; new feature stack beyond #12 assert if needed.

