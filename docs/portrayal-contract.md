# Portrayal contract (P1) — PC + Lua boundary

Owner: SoftwareArchitecture (2026-10-11). Sources: IHO S-101 PC **2.0.0** (`catalogues/S-101/PC_2.0.0`, pairs with FC 2.0.0), S-102 PC 3.0.0. Host interface per **S-100 Part 9A**; Lua runtime version per S-100 Part 13 (Eng to confirm edition and pin it).

## Pipeline

```
Dataset + FeatureCatalogue ──► HostAdapter (Part 9A Host* functions)
                                    │
PortrayalCatalogue + Context ──► PortrayalEngine (embedded Lua, official main.lua)
                                    │  HostPortrayalEmit
                                    ▼
                         DrawingInstructions (product-neutral)
                                    │
                         s100_viewer Renderer (Qt) + symbol/colour assets
```

## Modules

| Module | Lives in | Qt? | Responsibility |
|--------|----------|-----|----------------|
| `portrayal_catalogue` | `s100_core` | no | Parse `portrayal_catalogue.xml`: rules, context params + defaults, colour profiles, symbol / line style / area fill refs, viewing groups, display planes. Data only. |
| `host_adapter` | `s100_core` | no | Implements exactly the Part 9A `Host*` functions over `Dataset` + `FeatureCatalogue`. All rule fixes and quirks live here. |
| `portrayal_engine` | `s100_core` | no | Load official Lua unmodified, set context, run per dataset, collect `HostPortrayalEmit` output, parse into `DrawingInstructions`. |
| `renderer` | `s100_viewer` | yes | Draw instructions with symbols and palette (Day / Dusk / Night). No S-101 knowledge. |

## Frozen rules

1. **Official catalogue files are read-only.** Never edit Lua/XML under `catalogues/` (the `catalogue_provenance` hash test enforces this). Fixes go into `host_adapter`.
2. **Host surface = Part 9A, nothing invented.** Start with the set the S-101 PC 2.0.0 framework actually calls (see `S100Scripting.lua`): feature/information IDs and codes, simple/complex attributes, spatial lookup and associations, type-info queries, `HostPortrayalEmit`, `HostDebuggerEntry` (no-op). A missing function must fail loudly, not return nil silently.
3. **Output is product-neutral.** S-101 and S-102 engines both emit the same `DrawingInstructions` type, so the renderer and the later S-98 interoperability step work on one format.
4. **Core stays Qt-free.** `portrayal_*` and `host_adapter` link only Lua + the XML parser.
5. **Context parameters belong to the session** (mariner settings: SafetyContour, SafetyDepth, ShallowContour, DeepContour, FourShades, SimplifiedSymbols, PlainBoundaries, …), defaults from the PC. Changing one re-runs portrayal; cache key = (dataset, PC version, context hash).
6. **Layers get display info from portrayal**, not new keys: after a run, fill `LayerInfo` with viewing group, display plane, drawing priority (as reserved in `api-contracts.md`).

7. **Host function registry, per product.** Each product's host adapter registers its `Host*` functions in one registry (name → implementation) exposed for tests; the target list is `docs/testing/host-functions.txt`. Name variants share one implementation as aliases (e.g. `HostFeatureTypeGetCode` = `HostFeatureGetCode` for S-102). S-101 registry is backed by the feature list; S-102 by coverage data — same interface, separate registrations.

## DrawingInstructions (shape, Eng may refine field names)

```cpp
struct DrawingInstruction {
  std::uint64_t feature_id;
  std::string viewing_group;     // drives display category / toggles
  int drawing_priority{0};
  std::string display_plane;     // e.g. UnderRadar / OverRadar
  enum class Kind { Point, Line, Area, Text, Null } kind;
  std::string symbol_ref;        // Point
  std::string line_style_ref;    // Line
  std::string fill_ref;          // Area: colour token or pattern
  std::string text; /* + text style */
  double rotation{0};
  // augmented geometry (e.g. light sectors) as explicit geometry, not re-derived in renderer
};
using DrawingInstructions = std::vector<DrawingInstruction>;

bool portray(const Dataset&, const FeatureCatalogue&, const PortrayalCatalogue&,
             const PortrayalContext&, DrawingInstructions& out, std::string* err = nullptr);
```

## Test hooks (SoftwareTester)

- `portray` on all 5 cells: returns true, zero Lua errors, every feature yields ≥1 instruction (a `Null` instruction counts — it is the rule's explicit "draw nothing").
- Every `Host*` name referenced by the PC framework is implemented (static check over `Rules/*.lua`).
- Context change test: moving SafetyContour changes DepthArea fills.
- Golden snapshots per cell come after renderer, not before.

## Not in this step

S-98 interoperability ordering, S-102 overlay rules (engine reuse only), alerts (`AlertCatalog-S101.xml`), PC 2.1.0 upgrade (must move together with FC 2.1.0).
