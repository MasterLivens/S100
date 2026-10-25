# S-124 navigational warnings — first slice (rev 2) (Closed)

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/api-contracts.md` (S-124) / `docs/catalogue-complete-contract.md` (S-124 **FC+PC** MUST — ≠ S-104 FC-only). Case id: **`IHO-S124-OPEN-LIST`** (**Closed / Done** 2026-10-25).  
CTest name: **`s124_open_list`** — MUST NOT SKIP; Linux + Windows dual-green (CI `40164164518` @ `f2dccb1`). Supersedes Open draft on tip `f2dccb1` (≡ Arch `647bd75`).

Primary research: `/workspace/research/s124-open-list/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`7cdba01`**. Scrubber / `--list-frames` / `timeline_entries` → separate Active **`IHO-TIMELINE-UI`** (`docs/timeline-ui-contract.md`) — **S-124 does not enter TIMELINE-UI**. PC wiring (`S124HostAdapter` / `portray_nav_warnings_pc` / no invented Lua) → separate Active **`IHO-PC-WIRE-111-124`** (`docs/pc-wire-contract.md`) — **do not** close PC-WIRE this tip.

**Status: Closed / Done (2026-10-25)** — CI [`40164164518`](https://github.com/MasterLivens/S100/actions/runs/40164164518) @ `f2dccb1` dual-green; **`s124_open_list` Passed Ubuntu+Win** (native `s100_tests` + GML — **not** `viewer_*`; **no Win O1 (a)/(b)**). Eng **`7cdba01`** already on tip — **docs close only**. MUST tip-truth **unchanged** (open → `warnings.size()==2` + NW.1 Point **0.2/0.2** + NW.2 exact ring + bounds envelope + empty→Invalid / zero-NW→Parse / wrong product 不填 + portray ≥2 geom+≥2 Text **CHYLW** + ChartSession S-101+S-124 + `--s124` + **FC+PC on disk** + soft/exchange WARN-only; fixture MUST NOT SKIP). **O1=tip-truth / docs-close SUCCESS**. Failure face = bare CHECK tip-truth (do **not** invent `S124-*`). Do **not** reopen S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** mix TIMELINE-UI / PC-WIRE / CATALOGUE-COMPLETE; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog. Contract display stays **rev 2**. Next: **TBD**.

**Planner freeze (kept — rev 2 tip-truth):** Reuse case id **`IHO-S124-OPEN-LIST`** / CTest **`s124_open_list`**; MUST = open → `product==S124` + `nav_warnings` + **`warnings.size()==2`** + NW.1 Point lon**=0.2** lat**=0.2** + NW.2 ring exact posList `0.0 0.0 0.4 0.0 0.4 0.4 0.0 0.4 0.0 0.0` + `Dataset::bounds()` envelope + empty→Invalid / zero-NW→Parse / wrong product 不填 `nav_warnings` + portray ≥2 geom + ≥2 Text **CHYLW** + ChartSession S-101+S-124 + **`--s124`** + **FC+PC on disk** (catalogue-complete; ≠ S-104 FC-only) + soft pixel / exchange-set **WARN-only**; data model `Dataset::nav_warnings` / `NavWarningDataset` / `NavWarning` (**forbid** Feature mirroring; **forbid** reuse `Dataset::coverage` / `water_level` / `current`); fixture `synthetic_navwarning.gml` MUST NOT SKIP; scrubber → **`IHO-TIMELINE-UI`** (separate; **S-124 does not enter TIMELINE-UI**); PC wiring → **`IHO-PC-WIRE-111-124`** (separate; **do not** close; open-list keeps heuristic `portray_nav_warnings` tip-truth); Eng `7cdba01` already; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog; failure face = bare CHECKs (do **not** invent `S124-*`); after Closed next **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** mix TIMELINE-UI / PC-WIRE / CATALOGUE-COMPLETE.

**O1 closed as tip-truth / docs-close:** Eng `7cdba01` already on tip (GML open → `Dataset::nav_warnings`; `warnings.size()==2`; NW.1 Point 0.2/0.2 + NW.2 exact ring; bounds envelope; empty→Invalid / zero-NW→Parse / wrong product 不填; portray ≥2 geom + ≥2 Text CHYLW; ChartSession S-101+S-124; CLI/viewer `--s124`; fixture `synthetic_navwarning.gml`; CTest **`s124_open_list`**). Slice = Arch docs close only — not greenfield Eng. Do **not** invent case ids or next tip names; do **not** reopen S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** write FC-only; do **not** invent `S124-*` prefixes; do **not** retire `grid()` this tip; do **not** close PC-WIRE / CATALOGUE / TIMELINE. **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest; inventing oracles / next tip id; mixing TIMELINE-UI / PC-WIRE / CATALOGUE.

---

## Slice close — 2026-10-25

**Status: Closed / Done** (formal docs close — **not** a loader / portray rewrite).

- Evidence: CI [40164164518](https://github.com/MasterLivens/S100/actions/runs/40164164518) tip `f2dccb1` — Ubuntu + Windows **`s124_open_list` Passed** (MUST NOT SKIP). Ancestor greens also Eng landing [`38128109542`](https://github.com/MasterLivens/S100/actions/runs/38128109542) @ `7cdba01` + pre-Open tip [`40162996134`](https://github.com/MasterLivens/S100/actions/runs/40162996134) `#9`.
- **O1=tip-truth / docs-close SUCCESS** — Eng `7cdba01` already on tip; Arch Open→Closed only; Planner BRIEF §7 answers frozen.
- MUST tip-truth unchanged (open → `warnings.size()==2` + NW.1 Point **0.2/0.2** + NW.2 exact ring + bounds envelope + empty→Invalid / zero-NW→Parse / wrong product 不填 + portray ≥2 geom+≥2 Text **CHYLW** + ChartSession S-101+S-124 + `--s124` + **FC+PC on disk** + soft/exchange WARN-only; fixture MUST NOT SKIP).
- Eng `7cdba01` already on tip — docs close only; no `ci.yml` change.
- Failure face stays bare CHECK tip-truth — do **not** invent `S124-*`.
- **Notes:** next **TBD** (architecture / registered Active — do **not** invent a next tip id). **Backlog:** `grid()` shim retire (`S102-GRID-SHIM-RETIRE`) — do not insert ahead. Contract display stays **rev 2**. FC+PC on disk kept; PC-WIRE stays separate Active (do **not** close). S-124 not in TIMELINE-UI.
- Boundaries: independent; ≠ TIMELINE-UI; ≠ PC-WIRE / CATALOGUE-COMPLETE; do **not** reopen S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.
- **Non-goals this close:** inventing oracles / next tip id; writing FC-only; closing PC-WIRE / CATALOGUE / TIMELINE; retiring `grid()` shim; loader/portray rewrite; reopening S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; Win (a)/(b) (N/A — non-`viewer_*`); inventing `S124-*` failure-face prefixes; hardening open-list to must-run PC path.

---

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

**Heuristic (pre-wire):** For **each** warning, emit:

**After PC wire:** prefer `portray_nav_warnings_pc` per `docs/pc-wire-contract.md`; heuristic only if PC path fails.

For **each** warning (heuristic), emit:

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
| Official S-124 FC/PC on disk + provenance | **MUST** via `docs/catalogue-complete-contract.md` |
| Portray→PC wiring (Lua Host) | **MUST** via `docs/pc-wire-contract.md` (`portray_nav_warnings_pc` / `S124HostAdapter`) |

## Non-goals

severity/status fields, cancel/replace chains, Feature mirroring, in-force UI, encryption.
