# ChartSession unified pick (rev 2)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-SESSION-PICK-UNIFIED`** (Active). CTest: **`session_pick_unified`**.

## Goal

Geographic pick on a **ChartSession** → one `SessionPickResult` across loaded members. Screen/`Dataset`-only pick APIs are out of this slice (existing `pick_wgs84` may feed lon/lat into this API).

## API

```cpp
struct SessionPickHit {
  enum class Kind { Feature, CoverageSample, WaterLevel, Current, NavWarning };
  Kind kind;
  std::string member_key;
  ProductId product{ProductId::Unknown};
  std::uint64_t feature_id{0};   // S-101 feature id; S-124: 0 (use attributes)
  std::string feature_type;      // S-101 FC code; S-124: NW id string also in attributes["id"]
  std::string label;
  std::map<std::string, std::string> attributes;
  float depth_or_level{0};
  float speed_ms{0};
  float direction_deg{0};
  bool numeric_valid{false};     // true iff depth_or_level / speed / direction fields are meaningful
};

struct SessionPickResult {
  double lon{0}, lat{0};
  std::vector<SessionPickHit> hits;  // empty = no hits, still success
};

// true  = call completed (hits may be empty — including OOB / ocean)
// false = hard failure only (null session, corrupt member pointer); fill err
bool chart_session_pick(const ChartSession& session, double lon, double lat,
                        SessionPickResult& out, std::string* err = nullptr);
```

**No** `Dataset::pick` in this slice. OOB coordinates → `true` + empty hits (not an error).

## `numeric_valid`

| Kind | `numeric_valid` | Numbers |
|------|-----------------|---------|
| Feature / NavWarning | false | ignore numeric fields |
| CoverageSample / WaterLevel | true iff sample succeeded | `depth_or_level` |
| Current | true iff sample succeeded | `speed_ms`, `direction_deg` |

## Active frame

Use each member’s **current** `active_frame` (S-104/S-111). Pick **must not** call `set_active_frame` / `set_active_time`. S-102 has no frame — sample grids as already defined.

## S-124 hit mapping

| Field | Value |
|-------|-------|
| `kind` | `NavWarning` |
| `feature_id` | `0` |
| `feature_type` | NW `id` (e.g. `NW.1`) |
| `label` | `title` |
| `attributes["id"]` | id |
| `attributes["title"]` | title |
| `attributes["time_iso"]` | time_iso |

## Hit order (stable)

1. S-101 `Feature` (ascending feature_id)  
2. S-124 `NavWarning` (fixture order / id ascending)  
3. S-102 `CoverageSample`  
4. S-104 `WaterLevel`  
5. S-111 `Current`  

## Geometry tolerances

- Point (S-101 / NW.1): distance ≤ `1e-4` deg  
- Polygon contains (S-101 / NW.2): standard contains test  
- Line: ≤ `1e-4` deg (if exercised)  
No px→deg conversion in this contract.

## S-124 centroid (MUST this slice)

When emitting NW Text (and for pick anchors if needed): centroid = average of exterior ring vertices **excluding the repeated closing vertex**. Update `docs/s124-contract.md` the same way — not deferred.

## CLI

**MUST:** `s100_cli --session-pick <lon> <lat>` after members loaded (prints hits).  
**Viewer dump:** **deferred** (not this slice).

## CTest oracles (separate picks — envelopes don’t overlap)

| # | Session contents | Pick | Expect |
|---|------------------|------|--------|
| A | S-101 `101AA00DS0008.000` only | known land/feature point inside 0008 (from existing fixtures / LAND) | ≥1 `Feature` hit |
| B | S-124 synthetic only | `(0.2, 0.2)` | `NavWarning` for `NW.1` and `NW.2` (point + containing poly) |
| C | S-102 + S-104 + S-111 synthetics | point inside 0–0.4 oracle cells | CoverageSample + WaterLevel + Current; `numeric_valid==true` |
| D | full stack optional | `(0.2,0.2)` on synthetics-only session | kinds ⊆ {NW,102,104,111}; **no** S-101 Feature |
| E | any | empty ocean e.g. `(180,0)` with only synthetics loaded | `true`, `hits.empty()` |

MUST not SKIP when fixtures present (all gittracked).

## Non-goals

ECDIS pick-report layout, S-98, viewer pick dump, Dataset-level pick API, time scrubbing.
