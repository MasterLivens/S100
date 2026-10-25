# Wire official S-111 / S-124 Portrayal Catalogues (rev 2) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-PC-WIRE-111-124`** (**Active / Open**). CTest: **`pc_wire_111_124`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev 2** — Planner freeze (MUST below). S-111/S-124 PC wire slice after closed **`IHO-S124-OPEN-LIST`**. Does **not** reopen S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; does **not** mix **`IHO-TIMELINE-UI`** / **`IHO-CATALOGUE-COMPLETE`**; does **not** jump **`S102-GRID-SHIM-RETIRE`**.

Builds on `docs/api-contracts.md` (PC wiring) / `docs/s111-contract.md` / `docs/s124-contract.md` (both Closed; PC-WIRE separate) / `docs/catalogue-complete-contract.md` (on-disk FC/PC sha+pair — **≠** this tip). Scrubber / `--list-frames` / `timeline_entries` → separate Active **`IHO-TIMELINE-UI`** (`docs/timeline-ui-contract.md`). Vendor / sha / pair / SOURCE.md → separate Active **`IHO-CATALOGUE-COMPLETE`** — **do not** close CATALOGUE this tip. Wiring ≠ CATALOGUE vendor.

Primary research: `/workspace/research/pc-wire-111-124/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`e6c8f4a`** (+Win **`d885505`**).

**Status: Active / Open (2026-10-25)** — after **S-124 Closed** (tip `35301c1` ≡ Arch `5f55f60`; tip Close CI [`40164798588`](https://github.com/MasterLivens/S100/actions/runs/40164798588) dual-green; Open-bind evidence CI [`40164164518`](https://github.com/MasterLivens/S100/actions/runs/40164164518) @ `f2dccb1`≡`647bd75`, O1=tip-truth/docs-close). Eng tip ancestors **`e6c8f4a`** (+Win **`d885505`**) already on tip — **not greenfield** (S-124 `S124HostAdapter` + `portray_nav_warnings_pc` — **not** heuristic CHYLW default; S-111 `SCAROW01`…`09` Frame0→`SCAROW02` / Frame1→`SCAROW03` + ColorProfiles; **no** S-111 Lua; `catalogue_provenance` FATAL-clean; S-104 unchanged; fixture MUST NOT SKIP; XSLT host → **WARN**; CTest **`pc_wire_111_124`**). Ubuntu+Windows already Passed — tip Close CI [`40164798588`](https://github.com/MasterLivens/S100/actions/runs/40164798588) `#13` + Eng landing [`38131323424`](https://github.com/MasterLivens/S100/actions/runs/38131323424) @ `d885505` (native `s100_tests` + catalogues/GML/HDF5, **not** `viewer_*` — **no Win O1 (a)/(b)**). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / TIMELINE-UI / CATALOGUE-COMPLETE. Do **not** invent case ids or next tip names; freeze tip-truth below. Do **not** reopen S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. Do **not** mix TIMELINE / CATALOGUE. Do **not** invent `PCWIRE-*` failure-face prefixes (keep existing bare CHECKs). Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). Contract display header stays **rev 2** (do **not** bump just for Open). After Closed next: **TBD** (architecture / registered Active only — do **not** invent).

**Planner freeze (MUST — from BRIEF §7 + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-PC-WIRE-111-124`** / CTest **`pc_wire_111_124`**; **forbidden** to invent a second case/test name.
- MUST tip-truth = S-124 `S124HostAdapter` + `portray_nav_warnings_pc` (**not** heuristic CHYLW default) + S-111 `SCAROW01`…`09` (Frame0→`SCAROW02` / Frame1→`SCAROW03`) + ColorProfiles + **no** S-111 Lua + `catalogue_provenance` FATAL-clean + S-104 unchanged; fixture **MUST NOT SKIP**; XSLT host → **WARN**; vendor/sha → **CATALOGUE-COMPLETE** separate; scrubber → **TIMELINE-UI** separate.
- Official catalogue trees **read-only** — forbid edit S-111/S-124 FC/PC; forbid forge S-104 PC; forbid invent S-111 Lua HostAdapter / rewrite XSLT→Lua.
- Fixture `testdata/s124/synthetic_navwarning.gml` + `testdata/s111/synthetic_current_4326.h5` **MUST NOT SKIP** on CI (Linux+Win).
- Scrubber / `--list-frames` / `timeline_entries` → **`IHO-TIMELINE-UI`** (separate tip; **not** this case).
- Vendor / sha / pair / SOURCE.md → **`IHO-CATALOGUE-COMPLETE`** (separate tip; **do not** close this tip). Wiring ≠ CATALOGUE vendor.
- Eng tip already has **`e6c8f4a`** (+Win **`d885505`**); Ubuntu+Win (tip `40164798588` `#13` / Eng `38131323424`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog** (DEEPEN O1); **not** this tip.
- Failure face = existing bare CHECKs; do **not** invent `PCWIRE-*` prefixes.
- Ubuntu + Windows: `pc_wire_111_124` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** mix TIMELINE-UI / CATALOGUE-COMPLETE.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `pc_wire_111_124` MUST NOT SKIP, Passed (already tip CI `40164798588` `#13` / Eng `38131323424`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng diff required
- After Close next: **TBD**

**Eng notes:** verify `e6c8f4a` (+`d885505`) on tip; fixtures `testdata/s124/synthetic_navwarning.gml` + `testdata/s111/synthetic_current_4326.h5` + SOURCE; driver `tests/test_pc_wire_111_124.cpp`; cmake `pc_wire_111_124`. S-124 Closed already on this stack (`35301c1`). Do **not** reopen S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN. XSLT host **WARN-only** — do **not** harden. Official trees read-only; vendor/sha → CATALOGUE separate (do **not** close CATALOGUE / TIMELINE this tip).

**O-questions (BRIEF §3 Open 1–N — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `e6c8f4a`+`d885505` already; no `ci.yml` / no large Eng) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` — **yes / freeze**.
3. Contract head → Open; keep display **rev 2** (do **not** bump just for Open) — **yes / freeze**.
4. case-ids Active → Open (Status stays Active family) — **yes / freeze**.
5. Notes ≠ TIMELINE-UI; ≠ CATALOGUE-COMPLETE; do **not** reopen S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; shim-retire stays Backlog; XSLT stays WARN — **yes / freeze**.
6. Closed next → **TBD** (architecture / registered Active; do **not** invent) — **yes / freeze**.
7. No other cases this tip (do **not** Closed TIMELINE / CATALOGUE / SESSION-PICK / SIG / POWERUP / etc.) — **yes / freeze**.
8. architecture.md flip **next** to this Open — **yes / freeze**.
9. `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog — **yes / freeze**.
10. Failure face = existing bare CHECKs; do **not** invent `PCWIRE-*` prefixes — **yes / freeze**.
11. Wiring ≠ CATALOGUE vendor; official trees read-only; XSLT host → WARN — **yes / freeze**.

---

## Goal

Engines/viewer **consume** vendored catalogues from `catalogue-complete` — not only hash them.

| Product | Catalogue tech | This slice |
|---------|----------------|------------|
| S-124 | **Lua** Part 9 | **MUST** run official PC via `PortrayalEngine` + new **`S124HostAdapter`** |
| S-111 | **XSLT** rules | **MUST** load PC + map Arrow `symbol_ref` → `SCAROW01`…`09` + PC ColorProfiles; **XSLT host = WARN**; **forbidden: invent S-111 Lua** |
| S-104 | No official PC | unchanged |

## Pinned paths (literals)

```
catalogues/S-124/S-124_FC_2.0.0.xml
catalogues/S-124/PC_2.0.0/portrayal_catalogue.xml
catalogues/S-111/S-111_FC_2.0.0.xml
catalogues/S-111/PC_2.0.0/portrayal_catalogue.xml
```

Default resolve when `--fc` / `--pc` omitted: exactly these paths (same literal style for both products). Do not invent alternate filename patterns.

## S-124 engine hook (MUST)

**Today:** `portray_nav_warnings(NavWarningDataset&)` emits heuristic CHYLW Point/Area/Text (no FC/PC).

**This slice:**

1. `load_feature_catalogue("catalogues/S-124/S-124_FC_2.0.0.xml")` + `load_portrayal_catalogue("catalogues/S-124/PC_2.0.0")`.
2. Add **`S124HostAdapter : HostAdapter`** (product `"S-124"`) that exposes each `NavWarning` as a Host feature for official rules (`NavwarnPart` / `NAVWARNAreaAffected` / `main.lua`). Map geometries + title attributes the Lua rules read; Host registry follows `docs/portrayal-contract.md` rule 7 (unknown → Lua error).
3. Entry point (name locked): **`portray_nav_warnings_pc(const NavWarningDataset&, const FeatureCatalogue&, const PortrayalCatalogue&, DrawingInstructions&, PortrayalStats*, std::string*)`** — runs `PortrayalEngine` + `S124HostAdapter`. Viewer / CLI call this when PC loads; keep old `portray_nav_warnings` only as internal fallback behind an explicit fail path (document; prefer PC path in default viewer).
4. Official Lua under `catalogues/S-124/PC_2.0.0/Rules/` is **read-only** — no edits, no local Lua forks.

### S-124 oracle (no SKIP)

Fixture: `testdata/s124/synthetic_navwarning.gml` (2 warnings).

| Check | Sev |
|-------|-----|
| Load FC+PC from pinned paths succeeds | MUST |
| `portray_nav_warnings_pc` returns true **or** false only with documented Host/Lua error string | MUST |
| Per warning: ≥1 DI with `kind != Null`, **or** an explicit Null emission from the rule for that feature id | MUST |
| Heuristic-only CHYLW path is **not** the default when PC is present | MUST |
| `catalogue_provenance` still FATAL-clean | MUST |

## S-111 engine hook (MUST / WARN)

**Today:** `hdf5_s111` portray emits `Kind::Arrow` with `symbol_ref = "Arrow"` (viewer private glyph).

**This slice MUST (no XSLT engine):**

1. Load FC+PC from pinned S-111 paths.
2. Keep stride emit (`kS111ArrowStride`) and `Kind::Arrow` / rotation / scale_factor as in `s111-contract`.
3. Replace literal `"Arrow"`: set `symbol_ref` to a **PC symbol id** from speed bands mirrored from `Rules/select_arrow.xsl` (geLtInterval unless noted):

| speed_ms `v` | `symbol_ref` |
|--------------|--------------|
| `[0.00, 0.50)` | `SCAROW01` |
| `[0.50, 1.00)` | `SCAROW02` |
| `[1.00, 2.00)` | `SCAROW03` |
| `[2.00, 3.00)` | `SCAROW04` |
| `[3.00, 5.00)` | `SCAROW05` |
| `[5.00, 7.00)` | `SCAROW06` |
| `[7.00, 10.00)` | `SCAROW07` |
| `[10.00, 13.00)` | `SCAROW08` |
| `≥ 13.00` | `SCAROW09` |
| zero / fill / noData | **no** Arrow emit (match PC note) |

4. Viewer resolves `SCAROWNN` via `catalogues/S-111/PC_2.0.0/Symbols/SCAROWNN.svg` + Day/Dusk/Night from `ColorProfiles/` (same remap pattern as S-101). `fill_ref` may stay a PC colour token when available.
5. **Forbidden:** inventing S-111 Lua rules, a fake `S111HostAdapter` Lua registry, or rewriting XSLT into Lua.

**WARN follow-up:** Part 9 XSLT host that runs `main.xsl` / `SurfaceCurrent.xsl` / `select_arrow.xsl` for real. Until then the C++ band table above is the interim rule mirror (cite xsl in SOURCE / comment).

### S-111 oracle (no SKIP)

Fixture: `testdata/s111/synthetic_current_4326.h5`.

| Frame | sample (0.2,0.2) | expected `symbol_ref` on emitted Arrows for that speed |
|-------|------------------|--------------------------------------------------------|
| Frame0 `0.5 m/s` | → | `SCAROW02` (`[0.50, 1.00)`) |
| Frame1 `1.5 m/s` | → | `SCAROW03` (`[1.00, 2.00)`) |

| Check | Sev |
|-------|-----|
| Load FC+PC from pinned paths succeeds | MUST |
| Every emitted Arrow: `symbol_ref` ∈ {`SCAROW01`…`SCAROW09`} and file exists under PC `Symbols/` | MUST |
| Active-frame Arrows match band table for fixture speeds (Frame0→02, Frame1→03 on non-zero cells) | MUST |
| ColorProfiles load; Day palette remap does not crash | MUST |
| No S-111 `*.lua` added under `catalogues/` or `src/` as portrayal rules | MUST |
| `catalogue_provenance` still FATAL-clean | MUST |

## S-104

Out of scope. No `PC_2.0.0/`; token/heuristic unchanged.

## CLI / session

`--fc` / `--pc` optional; defaults = pinned literals by product. Document in `--help`. Session keys unchanged (`S-111:<stem>`, `S-124:<stem>`).

## MUST / WARN summary

| Check | Sev |
|-------|-----|
| S-124 `S124HostAdapter` + `portray_nav_warnings_pc` on synthetic | MUST |
| S-111 SCAROW `symbol_ref` + ColorProfiles on synthetic | MUST |
| CTest `pc_wire_111_124` no SKIP | MUST |
| S-111 XSLT rule execution | WARN |
| Invent S-111 Lua | **FORBIDDEN** |
| S-104 unchanged | MUST |
| `catalogue_provenance` green | MUST |

## Non-goals

Forged S-104 PC; S-98; S-111 Lua PC; editing official catalogue trees.
