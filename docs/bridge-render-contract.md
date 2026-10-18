# Bridge / composite feature rendering — FASC slice (rev 2.2 · association-contract rev 3)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S101-BRIDGE-RENDER`** (**Active**). CTest: **`fasc_bridge_render`** — **MUST NOT SKIP**; registered with no platform guard, must run on **Linux and Windows** CI.

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
| 12 | ≥ 1 `BRIDGE01` on this fixture | WARN (depends on `openingBridge` data) |
| 13 | Viewer snapshot shows bridge lines | WARN |

## Non-goals

Hand-drawn bridge glyphs; editing official Lua; cross-dataset associations; S-98; vertical-clearance alerts (Track B).

## Notes

- `bridge_render` may stay registered as an alias; CI gating and the case-id row use `fasc_bridge_render`.
- Rulings by SoftwareTester 2026-10-18 on `1ac028f`: orphan pylons WARN with pinned set; text DIs only need `12210` in `viewing_groups`; pylon surface prio 18 / UnderRadar.
