# ChartSession unified pick — next slice

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-SESSION-PICK-UNIFIED`**. CTest: **`session_pick_unified`**.

## Goal

One pick at WGS84 `(lon,lat)` (and optional screen px via existing `pick_wgs84`) returns a **single report** covering loaded session members: S-101 features, S-102/S-104/S-111 samples, S-124 warnings. No per-product CLI only.

## Frozen API

```cpp
struct SessionPickHit {
  enum class Kind { Feature, CoverageSample, WaterLevel, Current, NavWarning };
  Kind kind;
  std::string member_key;      // ChartSession key, e.g. S-101:0008
  ProductId product{ProductId::Unknown};
  std::uint64_t feature_id{0}; // S-101 / NW as applicable
  std::string feature_type;    // FC code or "BathymetryCoverage" / NW id
  std::string label;           // human one-liner
  // optional payloads (only one populated by kind):
  std::map<std::string, std::string> attributes; // S-101 attrs or NW fields
  float depth_or_level{0};     // S-102 depth or S-104 level
  float speed_ms{0};
  float direction_deg{0};
  bool numeric_valid{false};
};

struct SessionPickResult {
  double lon{0}, lat{0};
  std::vector<SessionPickHit> hits;  // may be empty
};

/** Pick in geographic space. Uses each member's active frame where applicable. */
bool chart_session_pick(const ChartSession& session, double lon, double lat,
                        SessionPickResult& out, std::string* err = nullptr);
```

Screen pick: convert with existing `pick_wgs84` then call `chart_session_pick`.

## Hit rules (MUST)

| Product | Include when | Hit content |
|---------|--------------|-------------|
| S-101 | Point-in-polygon / on-line tolerance / point radius (document px→deg at view scale; tests use geo tolerances below) | feature_id, feature_type, top-level attributes (cap 32) |
| S-102 | `sample_wgs84` success on any grid | depth, member_key |
| S-104 | active-frame `sample_wgs84` success | level as `depth_or_level` |
| S-111 | active-frame sample success | speed_ms, direction_deg |
| S-124 | point in warning geometry (point radius 1e-4°; polygon contains) | NW id, title, time_iso |

**Order:** S-101 features (stable id) → S-124 → S-102 → S-104 → S-111. Stable within kind.

## Tolerances (tests)

- Point feature / NW point: ≤ `1e-4` deg
- Line: ≤ `1e-4` deg perpendicular
- Polygon: winding contains (centroid of NW.2 / known S-101 poly)

## CLI

`s100_cli --session-pick <lon> <lat>` after loading members (or viewer status line). Thin wrapper.

## Fixture / CTest

Reuse existing synthetics + IHO 0008:

1. Session with S-101 `0008` + S-102 synthetic multigrid + S-104 synthetic + S-111 synthetic + S-124 synthetic.
2. Pick known points from each SOURCE oracle → assert expected kinds present.
3. Pick ocean empty → `hits.empty()`.
4. MUST not SKIP when synthetics present (all are gittracked).

## Non-goals

Full ECDIS pick-report formatting, S-98 highlight, time-aligned scrubbing UI (use each product’s current `active_frame` only), centroid WARN fix (separate one-liner ok in same PR if tiny).

## Centroid note

S-124 Text anchor: average ring vertices **excluding** the repeated closing vertex (closes WARN from `7cdba01`).
