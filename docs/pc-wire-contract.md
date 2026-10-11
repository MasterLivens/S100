# Wire official S-111 / S-124 Portrayal Catalogues (next slice)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-PC-WIRE-111-124`**. CTest: **`pc_wire_111_124`** (plus extend existing portray tests).

## Goal

Engines/viewer **consume** the vendored catalogues from `catalogue-complete` — not only hash them.

| Product | Catalogue tech | This slice |
|---------|----------------|------------|
| S-124 | **Lua** Part 9 (like S-101) | **MUST** run official PC via existing `portray` + HostAdapter |
| S-111 | **XSLT** rules (`main.xsl` / `SurfaceCurrent.xsl`) | **MUST** load PC + paint with PC **ColorProfiles + Symbols** (e.g. `SCAROW*`); XSLT rule engine = **WARN follow-up** (do not fake Lua rules) |
| S-104 | No official PC | unchanged — keep token/heuristic; do not invent PC |

## S-124 (MUST)

1. `load_feature_catalogue(catalogues/S-124/S-124_FC_2.0.0.xml)` + `load_portrayal_catalogue(catalogues/S-124/PC_2.0.0)`.
2. `portray(nav_warnings → Dataset view or dedicated entry)` through Lua Host; emit instructions from official rules for fixture warnings.
3. Viewer draws those instructions (CHYLW fallback only if Lua emits null — prefer PC output).
4. CTest: open synthetic GML + FC/PC → `lua_errors` within allowlist (document); ≥1 non-Null DI per warning **or** explicit Null from rule.
5. Official Lua/XML under `catalogues/` remain read-only.

## S-111 (MUST / WARN split)

**MUST**

1. Load FC + PC 2.0.0 from pinned paths.
2. Arrow rendering uses PC symbol SVG refs + Day/Dusk/Night from PC `ColorProfiles` (remap like S-101).
3. Default stride still `kS111ArrowStride` unless PC defines density — document choice in SOURCE note if overridden.
4. CTest: session with synthetic current → arrows reference PC symbol ids present on disk; palette tokens resolve.

**WARN / follow-up**

- Full XSLT portrayal pipeline (Part 9 XSLT host). Until then, keep `Kind::Arrow` generation in core as today, but **assets/colours from PC**.

## S-104

No change. Explicitly out of wiring.

## CLI / session

Default paths: if `--pc` / `--fc` omitted, resolve from `catalogues/S-XXX/` by product. Document in `--help`.

## MUST / WARN summary

| Check | Sev |
|-------|-----|
| S-124 Lua portray path green on synthetic | MUST |
| S-111 PC load + symbol/palette from PC | MUST |
| S-111 XSLT rule execution | WARN |
| S-104 unchanged / no PC dir | MUST |
| `catalogue_provenance` still FATAL-clean | MUST |

## Non-goals

Forged S-104 PC, S-98, rewriting S-111 XSLT into Lua.
