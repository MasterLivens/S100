# Wire official S-111 / S-124 Portrayal Catalogues (rev 2) (Closed)

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/api-contracts.md` (PC wiring) / `docs/s111-contract.md` / `docs/s124-contract.md` (both Closed; PC-WIRE separate) / `docs/catalogue-complete-contract.md` (on-disk FC/PC sha+pair — **≠** this tip). Case id: **`IHO-PC-WIRE-111-124`** (**Closed / Done** 2026-10-25).  
CTest name: **`pc_wire_111_124`** — MUST NOT SKIP; Linux + Windows dual-green (CI `40165684207` @ `9e7eae0`). Supersedes Open draft on tip `9e7eae0` (≡ Arch `5b27957`).

Primary research: `/workspace/research/pc-wire-111-124/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`e6c8f4a`** (+Win **`d885505`**). Scrubber / `--list-frames` / `timeline_entries` → separate Active **`IHO-TIMELINE-UI`** (`docs/timeline-ui-contract.md`). Vendor / sha / pair / SOURCE.md → separate Active **`IHO-CATALOGUE-COMPLETE`** — **do not** close CATALOGUE this tip. Wiring ≠ CATALOGUE vendor. S-111 XSLT host → **WARN** (do **not** harden).

**Status: Closed / Done (2026-10-25)** — CI [`40165684207`](https://github.com/MasterLivens/S100/actions/runs/40165684207) @ `9e7eae0` dual-green; **`pc_wire_111_124` Passed Ubuntu+Win** (native `s100_tests` + catalogues/GML/HDF5 — **not** `viewer_*`; **no Win O1 (a)/(b)**). Eng **`e6c8f4a`** (+Win **`d885505`**) already on tip — **docs close only**. MUST tip-truth **unchanged** (S-124 `S124HostAdapter` + `portray_nav_warnings_pc` — **not** heuristic CHYLW default; S-111 `SCAROW01`…`09` Frame0→`SCAROW02` / Frame1→`SCAROW03` + ColorProfiles; **no** S-111 Lua; `catalogue_provenance` FATAL-clean; S-104 unchanged; fixture MUST NOT SKIP; XSLT host → **WARN**). **O1=tip-truth / docs-close SUCCESS**. Failure face = bare CHECK tip-truth (do **not** invent `PCWIRE-*`). Do **not** reopen S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** mix TIMELINE-UI / CATALOGUE-COMPLETE; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog. Contract display stays **rev 2**. Next: **TBD**.

**Planner freeze (kept — rev 2 tip-truth):** Reuse case id **`IHO-PC-WIRE-111-124`** / CTest **`pc_wire_111_124`**; MUST tip-truth = S-124 `S124HostAdapter` + `portray_nav_warnings_pc` (**not** heuristic CHYLW default) + S-111 `SCAROW01`…`09` (Frame0→`SCAROW02` / Frame1→`SCAROW03`) + ColorProfiles + **no** S-111 Lua + `catalogue_provenance` FATAL-clean + S-104 unchanged; fixture MUST NOT SKIP; XSLT host → **WARN**; vendor/sha → **CATALOGUE-COMPLETE** separate; scrubber → **TIMELINE-UI** separate; official catalogue trees **read-only**; Eng `e6c8f4a` (+Win `d885505`) already; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog; failure face = bare CHECKs (do **not** invent `PCWIRE-*`); after Closed next **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** mix TIMELINE-UI / CATALOGUE-COMPLETE.

**O1 closed as tip-truth / docs-close:** Eng `e6c8f4a` (+Win `d885505`) already on tip (S-124 `S124HostAdapter` + `portray_nav_warnings_pc`; S-111 `SCAROW01`…`09` Frame0→`SCAROW02` / Frame1→`SCAROW03` + ColorProfiles; **no** S-111 Lua; `catalogue_provenance` FATAL-clean; S-104 unchanged; fixtures `synthetic_navwarning.gml` + `synthetic_current_4326.h5`; CTest **`pc_wire_111_124`**). Slice = Arch docs close only — not greenfield Eng. Do **not** invent case ids or next tip names; do **not** reopen S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; do **not** invent `PCWIRE-*` prefixes; do **not** retire `grid()` this tip; do **not** close CATALOGUE / TIMELINE; do **not** harden XSLT host. **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest; inventing oracles / next tip id; mixing TIMELINE-UI / CATALOGUE; forging S-111 Lua / S-104 PC; editing official catalogue trees.

---

## Slice close — 2026-10-25

**Status: Closed / Done** (formal docs close — **not** a wire / catalogue rewrite).

- Evidence: CI [40165684207](https://github.com/MasterLivens/S100/actions/runs/40165684207) tip `9e7eae0` — Ubuntu + Windows **`pc_wire_111_124` Passed** (MUST NOT SKIP). Ancestor greens also Eng landing [`38131323424`](https://github.com/MasterLivens/S100/actions/runs/38131323424) @ `d885505` + tip Open `#13 pc_wire_111_124` Passed (same Close-bind run / prior tip greens).
- **O1=tip-truth / docs-close SUCCESS** — Eng `e6c8f4a`+`d885505` already on tip; Arch Open→Closed only; Planner BRIEF §7 answers frozen.
- MUST tip-truth unchanged (S-124 `S124HostAdapter` + `portray_nav_warnings_pc` — **not** heuristic CHYLW default; S-111 `SCAROW01`…`09` Frame0→`SCAROW02` / Frame1→`SCAROW03` + ColorProfiles; **no** S-111 Lua; `catalogue_provenance` FATAL-clean; S-104 unchanged; fixture MUST NOT SKIP; XSLT host → **WARN**).
- Eng `e6c8f4a`+`d885505` already on tip — docs close only; no `ci.yml` change; **no Win (a)/(b)**.
- Failure face stays bare CHECK tip-truth — do **not** invent `PCWIRE-*`.
- **Notes:** next **TBD** (architecture / registered Active — do **not** invent a next tip id). **Backlog:** `grid()` shim retire (`S102-GRID-SHIM-RETIRE`) — do not insert ahead. Contract display stays **rev 2**. XSLT host stays WARN; vendor/sha → CATALOGUE-COMPLETE stays separate Active (do **not** close); scrubber → TIMELINE-UI stays separate Active (do **not** close); S-104 unchanged.
- Boundaries: independent; ≠ TIMELINE-UI; ≠ CATALOGUE-COMPLETE; do **not** reopen S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN.
- **Non-goals this close:** inventing oracles / next tip id; closing CATALOGUE / TIMELINE; retiring `grid()` shim; wire/catalogue rewrite; reopening S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN; Win (a)/(b) (N/A — non-`viewer_*`); inventing `PCWIRE-*` failure-face prefixes; hardening XSLT host; forging S-111 Lua / S-104 PC; editing official catalogue trees.

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
