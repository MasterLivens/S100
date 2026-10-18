# Bridge / composite feature rendering — first slice (rev 1)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S101-BRIDGE-RENDER`** (**Active**). CTest: **`bridge_render`** — **MUST NOT SKIP**.

Builds on `docs/association-contract.md` rev 2 (FASC/INAS merge + Host queries, `35b5b12`). Frozen with `docs/portrayal-contract.md`, `docs/display-priority-contract.md`, `docs/testing/iho-ctest-case-ids.md`.

## Principle (unchanged, re-stated)

Bridge symbology comes **only** from official S-101 PC 2.0.0 Lua (`Bridge.lua`, `SpanFixed.lua`, `SpanOpening.lua`, `PylonBridgeSupport.lua`). **No hand-written Bridge drawing in Qt or C++**, no local Lua edits. This slice proves the PC output reaches the screen correctly; it does not invent rules.

## Fixture (pinned, already gittracked)

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
6. **Clearance / name text** goes through the existing text-collision path; no special case.

## Oracle — CTest `bridge_render` (no SKIP)

Load `101AA00DS0006.000` with pinned S-101 FC+PC, default context (`RadarOverlay=false`):

| # | Check | Sev |
|---|-------|-----|
| 1 | Feature counts equal the fixture table (10 / 6 / 4 / 6) | MUST |
| 2 | `lua_errors == 0` over the whole cell, and none attributed to the four bridge types | MUST |
| 3 | Every `Bridge`/`SpanFixed`/`SpanOpening` feature has ≥1 DI; every non-Null DI of these types has `viewing_group == "12210"` and `drawing_priority == 24` | MUST |
| 4 | Every `Bridge` with Empty geometry has only `Kind::Null` DIs | MUST |
| 5 | Every `SpanFixed` with Curve/Surface geometry has ≥1 `Kind::Line` DI whose line style colour is `CHGRD` | MUST |
| 6 | Any DI with `symbol_ref == "BRIDGE01"` → file exists under PC `Symbols/` | MUST |
| 7 | Re-portray with `RadarOverlay=true`: same DIs flip `display_plane` to `OverRadar` | MUST |
| 8 | ≥1 `BRIDGE01` symbol on this fixture | **WARN** (depends on `openingBridge` data in 0006; if 0 add a synthetic Dataset case later) |
| 9 | Viewer snapshot of 0006 shows bridge lines (soft pixel) | WARN |

Eng records actual per-type DI counts in the test output so Tester can pin them as MUST in a later rev if wanted.

## Non-goals

Hand-drawn bridge glyphs; editing official Lua; cross-dataset associations; S-98; vertical-clearance alerts (Track B).
