# ChartSession unified pick (rev 2) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-SESSION-PICK-UNIFIED`** (**Active / Open**). CTest: **`session_pick_unified`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev 2** — Planner freeze (MUST below). ChartSession unified pick / CLI `--session-pick` slice after closed **`IHO-CATALOGUE-COMPLETE`**. Does **not** reopen CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; does **not** jump **`S102-GRID-SHIM-RETIRE`**.

Builds on `docs/api-contracts.md` (Session pick) / `docs/s124-contract.md` (centroid same slice; S-124 open Closed — **do not** reopen) / `docs/s104-contract.md` / `docs/s111-contract.md` (both Closed). Vendor / sha / pair / SOURCE.md → **CATALOGUE** already Closed — **do not** reopen. Scrubber → **TIMELINE** already Closed — **do not** reopen. Wiring → **PC-WIRE** already Closed — **do not** reopen. **No** `Dataset::pick`; viewer dump = deferred; ECDIS pick-report / S-98 / time scrubbing = Non-goals.

Primary research: `/workspace/research/session-pick-unified/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`3c607b7`** (docs pin **`5affbb1`**).

**Status: Active / Open (2026-10-25)** — after **CATALOGUE Closed** (tip `d77216d` ≡ Arch `78979f1`; tip Close CI [`40171329297`](https://github.com/MasterLivens/S100/actions/runs/40171329297) dual-green; Open-bind evidence CI [`40170639650`](https://github.com/MasterLivens/S100/actions/runs/40170639650) @ `0ba61fb`≡`e2a3d43`, O1=tip-truth/docs-close). Eng tip ancestor **`3c607b7`** already on tip — **not greenfield** (`SessionPickHit`/`SessionPickResult` + `chart_session_pick` + hit order S-101→S-124→S-102→S-104→S-111 + `numeric_valid` + S-124 centroid exclude closing vertex + CLI `--session-pick` + oracles A–E; **No** `Dataset::pick`; viewer dump deferred; vendor/sha → **CATALOGUE** already Closed; scrubber → **TIMELINE** already Closed; wiring → **PC-WIRE** already Closed). Ubuntu+Windows already Passed — tip Close CI [`40171329297`](https://github.com/MasterLivens/S100/actions/runs/40171329297) `#10 session_pick_unified` + Eng landing [`38128761263`](https://github.com/MasterLivens/S100/actions/runs/38128761263) @ `3c607b7` (native `s100_tests` + multi-product fixtures — **not** `viewer_*` — **no Win O1 (a)/(b)**). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Do **not** invent case ids or next tip names; freeze tip-truth below. Do **not** reopen CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Do **not** invent `SESSIONPICK-*` failure-face prefixes (keep existing bare CHECKs). Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). Contract display header stays **rev 2** (do **not** bump just for Open). After Closed next: **TBD** (architecture / registered Active only — do **not** invent).

**Planner freeze (MUST — from BRIEF §7 + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-SESSION-PICK-UNIFIED`** / CTest **`session_pick_unified`**; **forbidden** to invent a second case/test name.
- MUST tip-truth = `chart_session_pick` → `true` + hits may be empty (incl. OOB/ocean); `false` only hard failure; hit order S-101 Feature → S-124 NavWarning → S-102 CoverageSample → S-104 WaterLevel → S-111 Current; `numeric_valid` per Kind table; active_frame = current only (no `set_active_frame` / `set_active_time` inside pick); oracles **A–E**; S-124 centroid = exterior ring average **excluding** repeated closing vertex; CLI `--session-pick <lon> <lat>`; fixtures **MUST NOT SKIP**; **No** `Dataset::pick`; viewer dump deferred; ECDIS / S-98 / time scrubbing = Non-goals.
- Eng tip already has **`3c607b7`** (docs pin **`5affbb1`**); Ubuntu+Win (tip `40171329297` `#10` / Eng `38128761263`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield.
- Vendor / sha → **`IHO-CATALOGUE-COMPLETE`** already Closed — **do not** reopen.
- Scrubber → **`IHO-TIMELINE-UI`** already Closed — **do not** reopen.
- Wiring → **`IHO-PC-WIRE-111-124`** already Closed — **do not** reopen.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog** (DEEPEN O1); **not** this tip.
- Failure face = existing bare CHECKs; do **not** invent `SESSIONPICK-*` prefixes.
- Ubuntu + Windows: `session_pick_unified` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`; native `s100_tests` + multi-product fixtures; viewer dump deferred. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `session_pick_unified` MUST NOT SKIP, Passed (already tip CI `40171329297` `#10` / Eng `38128761263`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng diff required
- After Close next: **TBD**

**Eng notes:** verify `3c607b7` on tip (docs pin `5affbb1`); fixtures `testdata/s101/101AA00DS0008.000` + `testdata/s124/synthetic_navwarning.gml` + `testdata/s102/synthetic_multigrid_4326.h5` + `testdata/s104/synthetic_waterlevel_4326.h5` + `testdata/s111/synthetic_current_4326.h5`; driver `tests/test_session_pick.cpp`; cmake `session_pick_unified`. CATALOGUE Closed already on this stack (`d77216d`). Do **not** reopen CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Viewer dump / ECDIS / S-98 / time scrubbing **Non-goals / deferred** — do **not** harden. Do **not** invent `Dataset::pick`. Do **not** change hit order / `numeric_valid` tip-truth.

**O-questions (BRIEF §3 Open 1–N — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `3c607b7` already; no `ci.yml` / no large Eng) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` — **yes / freeze**.
3. Contract head → Open; keep display **rev 2** (do **not** bump just for Open) — **yes / freeze**.
4. case-ids Active → Open (Status stays Active family) — **yes / freeze**.
5. Notes ≠ CATALOGUE-COMPLETE (already Closed; do **not** reopen); ≠ TIMELINE-UI (already Closed; do **not** reopen); ≠ PC-WIRE (already Closed; do **not** reopen); ≠ S-124 / S-111 / S-104 open; do **not** reopen QBC-MG / UNCERT / DEEPEN; shim-retire stays Backlog; viewer dump / ECDIS / S-98 stay Non-goals — **yes / freeze**.
6. Closed next → **TBD** (architecture / registered Active; do **not** invent) — **yes / freeze**.
7. No other cases this tip (do **not** Closed SCAMIN / DISPLAY-PRIORITY / SIG / POWERUP / etc.) — **yes / freeze**.
8. architecture.md flip **next** to this Open — **yes / freeze**.
9. `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog — **yes / freeze**.
10. Failure face = existing bare CHECKs; do **not** invent `SESSIONPICK-*` prefixes — **yes / freeze**.
11. unified pick/CLI = this tip; vendor/sha = CATALOGUE Closed; scrubber = TIMELINE Closed; wiring = PC-WIRE Closed — **yes / freeze**.

---

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
