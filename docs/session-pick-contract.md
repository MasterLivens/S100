# ChartSession unified pick (rev 2) (Closed)

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/api-contracts.md` (Session pick) / `docs/s124-contract.md` (centroid same slice; S-124 open Closed — **do not** reopen) / `docs/s104-contract.md` / `docs/s111-contract.md` (both Closed) / `docs/catalogue-complete-contract.md` (Closed; vendor/sha → CATALOGUE separate) / `docs/timeline-ui-contract.md` (Closed; scrubber → TIMELINE separate) / `docs/pc-wire-contract.md` (Closed; wiring → PC-WIRE separate). Case id: **`IHO-SESSION-PICK-UNIFIED`** (**Closed / Done** 2026-10-25).  
CTest name: **`session_pick_unified`** — MUST NOT SKIP; Linux + Windows dual-green (CI `40172265305` @ `f8aac10`). Supersedes Open draft on tip `f8aac10` (≡ Arch `8dbe072`).

Primary research: `/workspace/research/session-pick-unified/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`3c607b7`** (docs pin **`5affbb1`**). Vendor / sha → **CATALOGUE** already Closed — **do not** reopen. Scrubber → **TIMELINE** already Closed — **do not** reopen. Wiring → **PC-WIRE** already Closed — **do not** reopen. **No** `Dataset::pick`; viewer dump = deferred; ECDIS pick-report / S-98 / time scrubbing = Non-goals.

**Status: Closed / Done (2026-10-25)** — CI [`40172265305`](https://github.com/MasterLivens/S100/actions/runs/40172265305) @ `f8aac10`≡`8dbe072` dual-green; **`session_pick_unified` Passed Ubuntu+Win** (native `s100_tests` + multi-product fixtures — **not** `viewer_*`; **no Win O1 (a)/(b)**). Eng **`3c607b7`** already on tip — **docs close only**. MUST tip-truth **unchanged** (`chart_session_pick` → `true` + hits may be empty; hit order S-101 Feature → S-124 NavWarning → S-102 CoverageSample → S-104 WaterLevel → S-111 Current; `numeric_valid` per Kind; active_frame = current only; oracles **A–E**; S-124 centroid exclude closing vertex; CLI `--session-pick`; fixtures **MUST NOT SKIP**; **No** `Dataset::pick`; viewer dump deferred). **O1=tip-truth / docs-close SUCCESS**. Failure face = existing bare CHECKs tip-truth (do **not** invent `SESSIONPICK-*`). Do **not** reopen CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog. Contract display stays **rev 2**. Next: **TBD**.

**Planner freeze (kept — rev 2 tip-truth):** Reuse case id **`IHO-SESSION-PICK-UNIFIED`** / CTest **`session_pick_unified`**; MUST tip-truth = `chart_session_pick` → `true` + hits may be empty (incl. OOB/ocean); `false` only hard failure; hit order S-101→S-124→S-102→S-104→S-111; `numeric_valid` per Kind table; active_frame = current only (no `set_active_frame` / `set_active_time` inside pick); oracles **A–E**; S-124 centroid = exterior ring average **excluding** repeated closing vertex; CLI `--session-pick <lon> <lat>`; fixtures **MUST NOT SKIP**; **No** `Dataset::pick`; viewer dump deferred; ECDIS / S-98 / time scrubbing = Non-goals; vendor/sha → **CATALOGUE** already Closed (do **not** reopen); scrubber → **TIMELINE** already Closed (do **not** reopen); wiring → **PC-WIRE** already Closed (do **not** reopen); Eng `3c607b7` already (docs pin `5affbb1`); `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog; failure face = bare CHECK (do **not** invent `SESSIONPICK-*` prefixes / new CTest); after Closed next **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.

**O1 closed as tip-truth / docs-close:** Eng `3c607b7` already on tip (`SessionPickHit`/`SessionPickResult` + `chart_session_pick` + hit order + `numeric_valid` + S-124 centroid + CLI `--session-pick` + oracles A–E; docs pin `5affbb1`; CTest **`session_pick_unified`**). Slice = Arch docs close only — not greenfield Eng. Do **not** invent case ids or next tip names; do **not** reopen CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** invent `SESSIONPICK-*` prefixes; do **not** retire `grid()` this tip; do **not** invent `Dataset::pick`; do **not** harden viewer dump / ECDIS / S-98 / time scrubbing. **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest; inventing oracles / next tip id; inventing `SESSIONPICK-*` failure-face prefixes.

---

## Slice close — 2026-10-25

**Status: Closed / Done** (formal docs close — **not** a pick / Eng rewrite).

- Evidence: CI [40172265305](https://github.com/MasterLivens/S100/actions/runs/40172265305) tip `f8aac10`≡`8dbe072` — Ubuntu + Windows **`session_pick_unified` Passed** (MUST NOT SKIP). Ancestor greens also Eng landing [`38128761263`](https://github.com/MasterLivens/S100/actions/runs/38128761263) @ `3c607b7` + prior tip `#10 session_pick_unified` Passed (CATALOGUE Close CI [`40171329297`](https://github.com/MasterLivens/S100/actions/runs/40171329297) @ `d77216d`).
- **O1=tip-truth / docs-close SUCCESS** — Eng `3c607b7` already on tip; Arch Open→Closed only; Planner BRIEF §7 answers frozen.
- MUST tip-truth unchanged (`chart_session_pick` + hit order + `numeric_valid` + oracles A–E + CLI `--session-pick` + S-124 centroid + fixtures **MUST NOT SKIP**; **No** `Dataset::pick`; viewer dump deferred).
- Eng `3c607b7` already on tip — docs close only; no `ci.yml` change; **no Win (a)/(b)**; no large Eng.
- Failure face stays bare CHECK tip-truth — do **not** invent `SESSIONPICK-*`.
- **Notes:** next **TBD** (architecture / registered Active — do **not** invent a next tip id). **Backlog:** `grid()` shim retire (`S102-GRID-SHIM-RETIRE`) — do not insert ahead. Contract display stays **rev 2**. Vendor/sha → CATALOGUE already Closed (do **not** reopen); scrubber → TIMELINE already Closed (do **not** reopen); wiring → PC-WIRE already Closed (do **not** reopen).
- Boundaries: independent; ≠ CATALOGUE (already Closed); ≠ TIMELINE (already Closed); ≠ PC-WIRE (already Closed); do **not** reopen CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.
- **Non-goals this close:** inventing oracles / next tip id; retiring `grid()` shim; pick/Eng rewrite; reopening CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; Win (a)/(b) (N/A — non-`viewer_*`); inventing `SESSIONPICK-*` failure-face prefixes; inventing `Dataset::pick`; hardening viewer dump / ECDIS / S-98 / time scrubbing.

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
