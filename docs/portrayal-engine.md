# Portrayal engine — implementation notes (P1)

Owner: SoftwareEngineer. Binding contract: `docs/portrayal-contract.md` (this file only records
how it was implemented and where the contract was silent).

## Lua runtime

Lua **5.1.5**, vendored unmodified in `third_party/lua-5.1.5` (provenance + hashes in its
`SOURCE.md`, CTest `third_party_provenance`). Justification: S-100 Ed 5.2.0 Part 13 clause 13-2
("Scripts conforming to this part shall be implemented using version 5.1 of the Lua programming
language") and 13-7.1 ("The host must provide ... a Lua version 5.1 interpreter"); the S-101 PC
2.0.0 `Rules/main.lua` says the same. Built as C++ so `lua_error` unwinds C++ frames.

Sandbox: fresh `lua_State` per run; only base/package/table/string/math; `dofile`/`loadfile`/
`package.loadlib` removed, `package.cpath` empty, `package.path` = `<PC>/Rules/?.lua`.

## Files (contract module names)

| Module | Header | Source |
|--------|--------|--------|
| `portrayal_catalogue` | `include/s100/portrayal_catalogue.hpp` | `src/core/portrayal_catalogue.cpp` (pugixml 1.14, vendored) |
| `host_adapter` | `include/s100/host_adapter.hpp` | `src/core/host_adapter.cpp` |
| `portrayal_engine` | `include/s100/portrayal_engine.hpp` | `src/core/portrayal_engine.cpp` |
| DrawingInstructions | `include/s100/drawing_instructions.hpp` | `src/core/drawing_instructions.cpp` |

All in `s100_core` (links `s100_lua`, `s100_pugixml` privately; no Qt).

## Host function registry (contract rule 7)

```cpp
class HostRegistry {               // one per product host
  const std::string& product() const;          // "S-101" | "S-102"
  std::vector<std::string> names() const;      // all registered names, sorted (incl. aliases)
  bool has(const std::string& name) const;
  bool is_alias(const std::string& name) const;
  std::string target_of(const std::string& name) const;   // implementation behind an alias
  HostFunction function(const std::string& name) const;   // int(*)(lua_State*)
};
class HostAdapter { const HostRegistry& registry() const; ... };   // shared interface
class S101HostAdapter : public HostAdapter { S101HostAdapter(const Dataset&, const FeatureCatalogue&); };
class S102HostAdapter : public HostAdapter { S102HostAdapter(const CoverageDataset&, const FeatureCatalogue&); };
```

Query in a test: `s100::S101HostAdapter h(ds, fc); h.registry().has("HostGetSpatial");`
`s100::S102HostAdapter h2(cov, fc); h2.registry().target_of("HostFeatureTypeGetCode") == "HostFeatureGetCode"`.

The engine installs exactly `registry().names()` as Lua globals (adapter as upvalue). Any other
global starting with `Host` raises `Host function 'X' is not implemented by the <product> host`
(strict `_G` `__index`), so nothing silently reads as nil.

- S-101 (28): backed by `Dataset` features, topology (`Dataset::spatial`), information records.
- S-102 (29): backed by `CoverageDataset`; `HostFeatureTypeGetCode` is an alias of
  `HostFeatureGetCode`; `HostGetSimpleAttribute`/`HostGetComplexAttributeCount` alias the feature
  variants (coverage has no information instances); information/spatial lookups are real
  implementations over empty sets (unknown ID -> Lua error), associations return `{}`.
- `HostGetSimpleAttribute` / `HostGetComplexAttributeCount` are not Part 13 functions — they are
  parameter names inside `PortrayalAPI.lua` `LookupAttributeValue`. Registered because the tester
  list contains them; S-101 implements them as a dispatcher (ID `I…` = information, else feature).

### Host IDs

Feature: decimal `Feature::id`. Information: `I<IRID RCID>`. Spatial: `P|M|C|K|S<RCID>` for
point / multi point / curve / composite curve / surface (RCNM 110/115/120/125/130).

### Host quirks (all in `host_adapter.cpp`, catalogues untouched)

- Empty ATVL (unknown value) -> Part 13 unknown-attribute string.
- `HostInformationTypeGetSimpleAttribute` returns `{}` not nil when absent (framework applies `#`).
- FASC is stored on one side: the other side gets the inverse role from the FC association roles.
- `HostSpatialGetAssociatedFeatureIDs` includes indirect references (composite curves, rings).
- `HostSpatialGetAssociatedInformationIDs` returns `{}` (never nil).

## Data model additions (additive)

`Feature`: `record_id`, `spatial` (SPAS), `feature_associations` (FASC),
`information_associations` (INAS). `Dataset`: `information` (IRID), `spatial` (topology store).
`FeatureCatalogue`: attribute/information/feature bindings, simple/complex attributes, information
types, roles, associations (FC loader now uses pugixml). `LayerInfo`: `viewing_group`,
`drawing_priority`, `display_plane`, `display_category` (filled by `apply_portrayal_to_layers`).

## DrawingInstruction refinements

`Kind` gains `Coverage` (S-102 `CoverageFill`). Added `command` (exact drawing command), all
viewing groups, scale min/max, line style definitions, text style, transforms, spatial references,
augmented geometry (kept as explicit commands), time and alert state. `feature_id` is
`std::uint64_t` as in the contract. Unknown commands: counted in `PortrayalStats::unparsed`.
After the Lua run, `portray(Dataset)` calls `resolve_spatial_references` so each instruction's
`spatial_refs` become concrete `geometries` (Point / MultiPoint / LineString / Polygon, including
holes) from `Dataset::spatial`; the viewer paints those and does not walk topology.

## Errors

`PortrayalStats::lua_errors` = rule errors main.lua recovered from (its `Debug.Trace('Error: …')`)
+ fatal errors. Known on CHS data: 3 Obstructions with neither `valueOfSounding` nor
`defaultClearanceDepth` make PC 2.0.0 `OBSTRN07.lua` raise `error()`; main.lua uses Default
symbology. Not a host defect; listed exactly in `tests/test_portrayal.cpp` (open question).

## CLI

`s100_cli <cell> --portray [--pc <PC dir>] [--set SafetyContour=5]... [--errors] [--layers]`

## Viewer (DrawingInstructions renderer)

`s100_viewer` consumes `portray()` output (via `PortrayalSession`) and paints with the PC colour
profile (`ColorProfiles/colorProfile.xml` sRGB). DrawingInstructions keep colour **token names**
(e.g. `DEPVS`, `LANDA`); the viewer maps tokens → RGB through the active palette at paint time.
**Day / Dusk / Night** select which `<palette name="…">` table from the official profile is used —
switching palette does **not** re-run Lua/portray (same instruction list; SVG symbol cache is
rebuilt with the new token→RGB map). Default is Day. UI: toolbar combo; CLI: `--palette Day|Dusk|Night`.
Official catalogues are not modified (SVG CSS inject + presentation-attribute promotion at load,
same as before). Layer list / use-type visibility still apply; meta stays off by default. Status
bar shows feature count, instruction count, and `lua_errors`. Flags: `--fc`, `--pc`,
`--palette`, `--snapshot <png>` (use `QT_QPA_PLATFORM=offscreen`).

| Kind | What is drawn |
|------|----------------|
| Area `ColorFill` | Polygon fill from colour token + transparency (active palette) |
| Area `AreaFillReference` / `SymbolFill` | PC `AreaFills/<ref>.xml` **symbolFill** only: stamp `Symbols/<symbol>.svg` on the v1/v2 mm lattice (y-up → screen y-down), clipped to instruction/feature rings with odd-even fill. `GlobalGeometry` anchors the lattice at lon/lat (0,0); `LocalGeometry` at the outer-ring first vertex. Colour tokens via the same active-palette SVG remapping as Point symbols (CSS inject + presentation attrs). |
| Line `_simple_` | Width / colour / dash from inline `LineStyleDef` |
| Line named style | Colour + width from `LineStyles/<ref>.xml` (dashes simplified) |
| Point | PC `Symbols/<symbol_ref>.svg` via `QSvgRenderer` (Qt6::Svg). External CSS class tokens (`fCHBLK` / `sSNDG1` / …) remapped through the active Day/Dusk/Night colour profile into an embedded `<style>` **and** promoted to `fill=`/`stroke=` presentation attributes (Qt Svg Tiny does not apply stroke-via-CSS-class); renderers cached by `symbol_ref` (cache cleared on palette change). Scale: SVG user unit = mm × `scale_factor` × DPI; `LocalOffset` + `rotation` applied; pivot `(0,0)`. Missing file → tiny `CHBLK` dot |
| Text | Token colour + LocalOffset; font size from mm |

### AreaFill formats supported

S-101 PC 2.0.0 `AreaFills/` contains only `af:symbolFill` (25 files). Viewer loads that schema
(`areaCRS`, `symbol@reference`, `v1`/`v2`, optional `clipSymbols`). **Not painted:** `PixmapFill` /
`HatchFill` drawing commands (none emitted by PC 2.0.0 rules against this catalogue), and any future
`pixmapFill` / `hatchFill` AreaFills XML. Inline `SymbolFill` with the same named-ref payload as
`AreaFillReference` is accepted.

### SpatialReference clipping

When a drawing instruction carries `SpatialReference` id(s) (`P|M|C|K|S` + RCID, optional
`,reverse`), the engine resolves them against `Dataset::spatial` into `DrawingInstruction::geometries`
(one entry per ref: curve → LineString, surface → Polygon with exterior/interior rings for odd-even
holes, point/multipoint as-is; composite curves expanded). The viewer paints only those geometries
(ColorFill / AreaFillReference / Line / Point / Text). If `spatial_refs` is empty, behaviour is
unchanged (feature geometry). Unresolved refs yield an empty `geometries` list and nothing is drawn
for that instruction (no fall-back to the whole feature).

### Colour palettes (Day / Dusk / Night)

Official `ColorProfiles/colorProfile.xml` defines three sRGB palettes (`Day`, `Dusk`, `Night`) plus
matching `Symbols/{day,dusk,night}SvgStyle.css` (CSS is not loaded by QtSvg; the viewer remaps from
the XML palette). Token names in DrawingInstructions are palette-independent. Golden snapshot stays
Day for now (soft compare may expand per palette later).

Not yet (remaining gaps): augmented geometry, CoverageFill, PixmapFill/HatchFill. Sounding digit
composites are real SVG symbols now (each digit/`SOUNDG*` file); composite placement still follows
whatever PointInstructions the rules emit.
