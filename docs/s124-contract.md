# S-124 navigational warnings — first slice (rev 2) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S124-OPEN-LIST`** (**Active / Open**). CTest: **`s124_open_list`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev 2** — Planner freeze (MUST below). First S-124 open/list slice after closed **`IHO-S111-OPEN-FRAME`**. Does **not** reopen S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; does **not** mix **`IHO-TIMELINE-UI`** / **`IHO-PC-WIRE-111-124`** / **`IHO-CATALOGUE-COMPLETE`**; does **not** jump **`S102-GRID-SHIM-RETIRE`**.

Builds on `docs/api-contracts.md` (S-124) / `docs/catalogue-complete-contract.md` (S-124 **FC+PC** MUST — ≠ S-104 FC-only). Scrubber / `--list-frames` / `timeline_entries` → separate Active **`IHO-TIMELINE-UI`** (`docs/timeline-ui-contract.md`) — **S-124 does not enter TIMELINE-UI**. PC wiring (`S124HostAdapter` / `portray_nav_warnings_pc` / no invented Lua) → separate Active **`IHO-PC-WIRE-111-124`** (`docs/pc-wire-contract.md`) — **do not** close PC-WIRE this tip.

Primary research: `/workspace/research/s124-open-list/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`7cdba01`**.

**Status: Active / Open (2026-10-25)** — after **S-111 Closed** (tip `2d2a499` ≡ Arch `6450046`; tip CI [`40162996134`](https://github.com/MasterLivens/S100/actions/runs/40162996134) dual-green; formal Close CI bind [`40162377554`](https://github.com/MasterLivens/S100/actions/runs/40162377554) @ `d512e30`≡`1be2b1b`, O1=tip-truth/docs-close). Eng tip ancestor **`7cdba01`** already on tip — **not greenfield** (GML open → `Dataset::nav_warnings`; `warnings.size()==2`; NW.1 Point **0.2/0.2** + NW.2 exact ring; `Dataset::bounds()` envelope; empty→Invalid / zero-NW→Parse / wrong product 不填; portray ≥2 geom + ≥2 Text **CHYLW**; ChartSession S-101+S-124; CLI/viewer **`--s124`**; fixture `testdata/s124/synthetic_navwarning.gml`; CTest **`s124_open_list`**). Ubuntu+Windows already Passed — tip CI [`40162996134`](https://github.com/MasterLivens/S100/actions/runs/40162996134) `#9` + Eng landing [`38128109542`](https://github.com/MasterLivens/S100/actions/runs/38128109542) @ `7cdba01` (native `s100_tests` + GML, **not** `viewer_*` — **no Win O1 (a)/(b)**). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / TIMELINE-UI / PC-WIRE / CATALOGUE-COMPLETE. Do **not** invent case ids or next tip names; freeze tip-truth below. Do **not** reopen S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Do **not** write this tip as S-104-style FC-only — **FC+PC on disk** (catalogue-complete); PC **wiring** stays PC-WIRE separate. Do **not** invent `S124-*` failure-face prefixes (keep existing bare CHECKs). Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). Contract display header stays **rev 2** (do **not** bump just for Open). After Closed next: **TBD** (architecture / registered Active only — do **not** invent).

**Planner freeze (MUST — from BRIEF §7 + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-S124-OPEN-LIST`** / CTest **`s124_open_list`**; **forbidden** to invent a second case/test name.
- MUST = open → `product==S124` + `nav_warnings` + **`warnings.size()==2`** + NW.1 Point lon**=0.2** lat**=0.2** + NW.2 ring exact posList `0.0 0.0 0.4 0.0 0.4 0.4 0.0 0.4 0.0 0.0` + `Dataset::bounds()` envelope + empty→Invalid / zero-NW→Parse / wrong product 不填 `nav_warnings` + portray ≥2 geom + ≥2 Text **CHYLW** + ChartSession S-101+S-124 + **`--s124`** + **FC+PC on disk** (catalogue-complete; ≠ S-104 FC-only) + soft pixel / exchange-set **WARN-only**.
- Data model: `Dataset::nav_warnings` / `NavWarningDataset` / `NavWarning`; **forbid** Feature mirroring; **forbid** reuse `Dataset::coverage` / `water_level` / `current` for S-124.
- Fixture `testdata/s124/synthetic_navwarning.gml` **MUST NOT SKIP** on CI (Linux+Win).
- Scrubber / `--list-frames` / `timeline_entries` → **`IHO-TIMELINE-UI`** (separate tip; **not** this case; **S-124 does not enter TIMELINE-UI**).
- PC wiring (`S124HostAdapter` / `portray_nav_warnings_pc` / no invented Lua) → **`IHO-PC-WIRE-111-124`** (separate tip; **do not** close this tip). Open-list CTest keeps heuristic `portray_nav_warnings` tip-truth (**do not** harden to must-run PC path).
- Eng tip already has **`7cdba01`**; Ubuntu+Win (tip `40162996134` `#9` / Eng `38128109542`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog** (DEEPEN O1); **not** this tip.
- Failure face = existing bare CHECKs; do **not** invent `S124-*` prefixes.
- Ubuntu + Windows: `s124_open_list` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** mix TIMELINE-UI / PC-WIRE / CATALOGUE-COMPLETE.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `s124_open_list` MUST NOT SKIP, Passed (already tip CI `40162996134` `#9` / Eng `38128109542`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng diff required
- After Close next: **TBD**

**Eng notes:** verify `7cdba01` on tip; fixture `testdata/s124/synthetic_navwarning.gml` + `SOURCE.md`; driver `tests/test_s124.cpp` `test_s124_open_list`; cmake `s124_open_list`. S-111 Closed already on this stack (`2d2a499`). Do **not** reopen S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Soft pixel / exchange-set **WARN-only** — do **not** harden. **FC+PC on disk** via catalogue-complete; PC wiring → PC-WIRE separate (do **not** close PC-WIRE / CATALOGUE this tip).

**O-questions (BRIEF §3 Open 1–N — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `7cdba01` already; no `ci.yml` / no large Eng) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` — **yes / freeze**.
3. Contract head → Open; keep display **rev 2** (do **not** bump just for Open) — **yes / freeze**.
4. case-ids Active → Open (Status stays Active family) — **yes / freeze**.
5. Notes ≠ TIMELINE-UI; ≠ PC-WIRE / CATALOGUE-COMPLETE; do **not** reopen S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; shim-retire stays Backlog — **yes / freeze**.
6. Closed next → **TBD** (architecture / registered Active; do **not** invent) — **yes / freeze**.
7. No other cases this tip (do **not** Closed TIMELINE / PC-WIRE / CATALOGUE / SIG / POWERUP / etc.) — **yes / freeze**.
8. architecture.md flip **next** to this Open — **yes / freeze**.
9. `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog — **yes / freeze**.
10. Failure face = existing bare CHECKs; do **not** invent `S124-*` prefixes — **yes / freeze**.
11. FC+PC on disk (≠ S-104 FC-only); PC wiring → PC-WIRE separate; open-list keeps heuristic portray tip-truth — **yes / freeze**.

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
