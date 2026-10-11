# S-124 navigational warnings — first slice (rev 2)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S124-OPEN-LIST`** (Active). CTest: **`s124_open_list`**.

## Goal

Open S-124 GML → list warnings → one overlay frame on S-101. No NW management UI.

## Fixture (shipped)

**Already in repo:** `testdata/s124/synthetic_navwarning.gml` (gittracked). Eng parses this shape (element local-names below); schema-valid IHO packaging is WARN/follow-up.

| id | title | time_iso | geometry |
|----|-------|----------|----------|
| `NW.1` | `Synthetic Hazard Alpha` | `2024-01-01T00:00:00Z` | Point lon**=0.2** lat**=0.2** (`gml:pos` order **lon lat**) |
| `NW.2` | `Synthetic Area Bravo` | `2024-01-01T06:00:00Z` | Polygon ring **exact** `posList`: `0.0 0.0 0.4 0.0 0.4 0.4 0.0 0.4 0.0 0.0` (lon lat pairs) |

`warnings.size()` after open **MUST == 2** (same as `>=2` on this fixture).

## Frozen model

```cpp
struct NavWarning {
  std::string id;
  std::string title;
  std::string message;       // may be empty
  std::string time_iso;      // MUST parse from fixture when present (both NW have values)
  Bounds bounds;
  std::vector<Geometry> geometries;
  // severity / status: NOT in this slice → Non-goals
};

struct NavWarningDataset {
  ProductId product{ProductId::S124};
  std::filesystem::path path;
  std::vector<NavWarning> warnings;

  Bounds bounds() const;  // union of warning bounds
  const NavWarning* find(std::string_view id) const;
};

Dataset::nav_warnings;           // filled only for S-124
Dataset::bounds()                // MUST include nav_warnings.bounds() in the union
// No Feature mirroring this slice (no 1:1 Feature copies).
```

## Open / errors

| Input | Result |
|-------|--------|
| Fixture / valid S-124 GML with ≥1 NW | `product=S124`, `nav_warnings` filled |
| Empty file / whitespace only | `LoadErrorKind::Io` or `Invalid` (pick **Invalid**), `nav_warnings` empty |
| Well-formed XML but **zero** NavigationalWarning | `LoadErrorKind::Parse`, message notes zero warnings |
| Non-S-124 / wrong product | `LoadErrorKind::Invalid` or `Parse`; **must not** fill `nav_warnings` |

## Portray (all warnings in fixture)

For **each** warning, emit:

| Kind | Rule |
|------|------|
| `Point` / `Line` / `Area` | From `geometries` (NW.1→Point, NW.2→Area/Polygon) |
| `Text` | `title`; anchor = point itself, or polygon **centroid** = average of exterior ring vertices **excluding the repeated closing vertex** |

- `fill_ref` / line colour token default **`CHYLW`**.
- Fixture has 2 warnings → MUST see geometry DI count ≥2 and Text count ≥2 (one title each).

## CLI

**This slice:** `s100_cli` / `s100_viewer` support **`--s124 <path.gml>`** (and session open via path detect). No defer.

## Session

Key `S-124:<stem>`. Overlay with S-101.

## MUST / WARN

| Check | Sev |
|-------|-----|
| Open fixture; `warnings.size()==2`; ids/titles/`time_iso` match table | MUST |
| NW.2 ring vertices match exact posList | MUST |
| `Dataset::bounds()` intersects fixture envelope | MUST |
| Empty / zero-NW / wrong product errors as table | MUST |
| Portray: ≥2 geom + ≥2 Text; each warning represented | MUST |
| `--s124` opens fixture | MUST |
| ChartSession S-101+S-124 | MUST |
| Soft pixel / exchange-set catalog member | WARN |
| Official S-124 FC/PC on disk + provenance | **MUST** via `docs/catalogue-complete-contract.md` (supersedes defer). Portray→PC wiring WARN follow-up |

## Non-goals

severity/status fields, cancel/replace chains, Feature mirroring, in-force UI, encryption.
