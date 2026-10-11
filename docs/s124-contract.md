# S-124 navigational warnings — first slice

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S124-OPEN-LIST`**. CTest: **`s124_open_list`**.

## Goal

Open S-124 **GML** (standalone file or via exchange-set member) → parse warnings into a list → draw **one overlay frame** (geometries + labels) on S-101 in `ChartSession`. No NW management UI / cancel cascade.

## Frozen model

```cpp
struct NavWarning {
  std::string id;              // GML/feature identifier
  std::string title;           // short text for list
  std::string message;         // body (may be empty in MVP)
  std::string time_iso;        // issue/valid-from UTC if present
  Bounds bounds;               // WGS84 envelope of geometries
  std::vector<Geometry> geometries;  // points/lines/areas as parsed
};

struct NavWarningDataset {
  ProductId product{ProductId::S124};
  std::filesystem::path path;
  std::vector<NavWarning> warnings;  // size >= 1 after successful open of fixture

  Bounds bounds() const;  // union of warning bounds
  const NavWarning* find(std::string_view id) const;
};

// Dataset::nav_warnings  when product==S124 — not features vector reuse for NW body text
// Optional: also mirror lightweight Feature entries for portray host — if so, document 1:1 id map.
```

Pinned:

1. `open_dataset(.gml/.xml)` detecting S-124 fills `Dataset::nav_warnings` (`product=S124`).
2. Wrong/non-S-124 GML → `LoadError`; must **not** fill `nav_warnings`.
3. Exchange-set: S-124 members use key **`S-124:<stem>`** in the same `ChartSession`.
4. List API for CLI/tests: iterate `warnings` (id, title, time_iso, bounds).

## Portray (one frame)

- Emit drawing instructions for **all** warnings in the dataset this slice (no time scrubber): Area/Line/Point from `geometries` + `Kind::Text` title at anchor (centroid / first point).
- Colour token default `CHYLW` (or PC token if S-124 PC vendored); no RGB in core.
- Official S-124 FC/PC: vendor if practical; else WARN and minimal emit.

## Fixture (CI MUST)

- **`testdata/s124/synthetic_navwarning.gml`** gittracked (`.gitignore` allowlist + `SOURCE.md`).
- ≥2 warnings with distinct ids; known geometries; SAMPLE titles.
- Oracle in SOURCE: warning ids, titles, a lon/lat inside warning0 polygon/point.
- MUST tests **not SKIP**.

## MUST / WARN

| Check | Sev |
|-------|-----|
| Open synthetic; `warnings.size()>=2`; bounds valid | MUST |
| `find(id)` works; list titles non-empty | MUST |
| Wrong product / garbage XML → LoadError | MUST |
| Portray ≥1 geometry + text instruction | MUST |
| ChartSession S-101 + S-124 overlay | MUST |
| Exchange-set member open (if synthetic catalog shipped) | MUST if catalog in fixture dir; else WARN follow-up |
| Soft pixel / full PC | WARN |

## Non-goals

Cancel/replace chains, in-force filtering UI, S-98 highlight rules, encryption.
