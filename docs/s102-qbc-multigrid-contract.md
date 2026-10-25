# S-102 QoBC multi-grid binding (rev1) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S102-QBC-MULTIGRID`** (**Active / Open**). CTest: **`s102_qbc_multigrid`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev1** — Planner freeze (MUST below). Takes the multi-grid QoBC binding gap deferred from closed **`IHO-S102-QBC-META`** (`docs/s102-qbc-meta-contract.md`).

Builds on `docs/s102-contract.md` / `docs/s102-deepen-contract.md` / `docs/s102-uncertainty-contract.md` / `docs/s102-qbc-meta-contract.md`. Does **not** reopen QBC-META U32+FAT / NullInstruction / **`QBC-*`** / **`BCAST-*`** surfaces, nor UNCERTAINTY VG **`13031`**, nor DEEPEN routing.

Primary research: `/workspace/research/qbc-multigrid/BRIEF.md` (+ `/workspace/research/s102-uncertainty/BRIEF.md` §10 multiplicity). Former code gap (`hdf5_product.cpp` `grids[0]` hard-bind) **already fixed** @ Eng **`aabbb54`**.

**Status: Active / Open (2026-10-25)** — after **QBC-META Closed** (tip `305bc89` ≡ Arch `921351d`; CI [`40155189914`](https://github.com/MasterLivens/S100/actions/runs/40155189914) dual-green; Closed Open-tip CI [`40154401620`](https://github.com/MasterLivens/S100/actions/runs/40154401620) @ `670708f`, O1=tip-truth/docs-close). Eng tip ancestor **`aabbb54`** already on tip — **not greenfield** (shape+origin/spacing bind; forbid `grids[0]` hard-bind; zero match → LoadError; fixture MG-PS; CTest **`QBC-MG-1..5`**). Ubuntu+Windows already Passed — Eng landing [`39113487504`](https://github.com/MasterLivens/S100/actions/runs/39113487504) @ `aabbb54` `#6` + tip ancestor [`40098659137`](https://github.com/MasterLivens/S100/actions/runs/40098659137) `#6` (native `s100_tests` + HDF5, **not** `viewer_*` — **no Win O1 (a)/(b)**). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from META / UNCERT / DEEPEN. Do **not** invent oracles; freeze tip-truth below. Do **not** reopen META / UNCERT / DEEPEN. Do **not** conflate failure faces **`QBC-MG-*` ≠ `QBC-*` / `BCAST-*`**. Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). After Closed next: **TBD** (architecture / backlog — do **not** invent).

**Planner freeze (MUST — from rev1 + BRIEF §7 + tip truth):**
- Reuse case id **`IHO-S102-QBC-MULTIGRID`** / CTest **`s102_qbc_multigrid`**; **forbidden** to invent a second case/test name.
- MUST = **QBC-MG-1..5** (fixture **MG-PS** co-located multi-VD + dual `sample_cell` + portray **NullInstruction** / **no** id colour band + prior `s102_qbc_meta` / deepen / uncertainty green).
- Fixture MG-PS **MUST NOT SKIP** on CI (Linux+Win).
- Failure face **`QBC-MG-*`** — do **not** reopen / conflate META **`QBC-*` / `BCAST-*`**.
- O1 hybrid shape+origin/spacing bind to every matching `grids[i]`; **forbid** `grids[0]` hard-bind; zero match → **`LoadError`**.
- Eng tip already has **`aabbb54`**; Ubuntu+Win (Eng `39113487504` `#6` / ancestor tip `#6`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield.
- **`grid()` shim stays Backlog** (DEEPEN O1); QBC-MG may still use `grid()`; retire = separate Backlog, **not** this tip.
- Ubuntu + Windows: `s102_qbc_multigrid` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / backlog — do **not** invent). Independent tip; do **not** reopen META / UNCERT / DEEPEN.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `s102_qbc_multigrid` MUST NOT SKIP, Passed (already Eng `39113487504` `#6` / ancestor tip `#6`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng diff required
- After Close next: **TBD**

**Eng notes:** verify `aabbb54` on tip; fixture MG-PS `synthetic_qbc_multigrid_vd_4326.h5` + `SOURCE.md`; driver `tests/test_s102.cpp` `test_s102_qbc_multigrid`; cmake `s102_qbc_multigrid`. META Closed already on this stack (`305bc89`). Do **not** reopen META `QBC-*`/`BCAST-*` / UNCERT / DEEPEN. Soft WARN **QBC-MG-W1** only — do **not** harden EncDotNet. O4 storage tip-truth = per-grid `qbc_ids` copy.

**O-questions (BRIEF §7 — Planner already answered; freeze tip-truth):**
1. Docs close / verify primarily (Eng `aabbb54` already; no `ci.yml` / no large Eng) — **recommend yes / freeze**.
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` — **recommend yes / freeze**.
3. Notes failure face **`QBC-MG-*`**; do **not** reopen META **`QBC-*`/`BCAST-*`** — **recommend yes / freeze**.
4. Closed next → **TBD** (architecture / backlog; do **not** invent) — **recommend yes / freeze**.
5. `grid()` shim stays Backlog — **freeze, not a question**.

---

## 1. Goal

When a product has **multiple** `BathymetryCoverage.nn` and optional `/QualityOfBathymetryCoverage`, **MUST NOT** hard-bind the single QoBC id raster to **`grids[0]`** alone.

**S-102 §10 model (frozen):**

- QoBC root multiplicity **0..1**; instance path **`QualityOfBathymetryCoverage.01` only** (no `QualityOfBathymetryCoverage.nn` packs).
- Multiple `BathymetryCoverage.nn` = **co-located** instances at **different verticalDatum**, sharing **one** QoBC aligned to **BC.01** geometry (same spatial metadata / same dimensions).
- **Not** per-`.nn` multiple QoBC containers.
- Deepen’s `synthetic_multigrid_4326.h5` (**disjoint** envelopes) is **not** the PS multi-VD model — see Non-goals / optional WARN.

Failure-face ids: **`QBC-MG-*`**. **Do not** reuse or mix with closed **`QBC-*`** / **`BCAST-*`**.

---

## 2. Spec anchors

| Id | Topic | Spec / project hook | Notes |
|----|-------|---------------------|-------|
| **MG-A1** | QoBC optional 0..1 | S-102 Ed 3.0.0 **§10** Root — multiplicity **0..1**; `dataCodingFormat` **must be 9**; `featureAttributeTable` mandatory within QoBC | BRIEF §1.1; closed QBC-META Q1 |
| **MG-A2** | Single instance `.01` | §10 Mandatory Naming — `QualityOfBathymetryCoverage.01` only; multi BC **does not** imply multi QoBC | “share a common QualityOfBathymetryCoverage” |
| **MG-A3** | Multi BC = multi VD, co-located | §10 Feature Instance — each `.nn` unique verticalDatum; **all** share same spatial location & extent | QoBC.01 metadata/dims align to **BathymetryCoverage.01** |
| **MG-A4** | Part 10c dcf=9 | S-100 Part 10c Ed 5.2.0 — `featureOrientedRegularGrid` = 9; extents must match reference FC | S-102 sets reference = BathymetryCoverage |
| **MG-A5** | Closed meta | `docs/s102-qbc-meta-contract.md` | U32+FAT, fill-0, lookup, NullInstruction, **`QBC-*`** / **`BCAST-*`** stay closed |
| **MG-A6** | Deepen (orthogonal) | `docs/s102-deepen-contract.md` | Disjoint-envelope fixture ≠ PS multi-VD; deepen routing unchanged |
| **MG-A7** | Code gap | `src/core/hdf5_product.cpp` hard-bind `grids[0]` | This case removes it |

---

## 3. Frozen MUST (Planner Open closed)

### 3.1 Binding key — **O1 = (a)+(c) hybrid**

| Step | Rule |
|------|------|
| Load | Exactly **one** QoBC: `/QualityOfBathymetryCoverage` + **`.01`** values + FAT + **dcf=9** (same validation as QBC-META). |
| Match | For **each** `grids[i]`: if **shape (`nrows`/`ncols`) AND origin/spacing (or equivalent envelope)** match QoBC.01 → **attach / share** the same `qbc_ids` on that grid. |
| None match | QoBC group present but **no** BC grid matches → **`LoadError`** (same severity class as META unreadable id grid). |
| Forbid | **Hard-bind to `grids[0]`** regardless of match. |
| Sample | **`grids[1].sample_cell` (same row/col as a known QoBC cell) MUST also report `has_qbc`** — cannot rely only on `sample_wgs84` (co-located equal area always picks lowest index → `[0]`). |

**(a)** = share/copy the BC.01-aligned QoBC raster onto every co-located matching grid.  
**(c)** = one logical QoBC raster; cell lookup uses the same row/col once a matching BC grid is selected. Behaviour pinned; storage → O4.

### 3.2 Other Open freezes

| Id | Decision |
|----|----------|
| **O2** | Deepen **disjoint** multi-grid + QoBC → **Non-goal this slice** (optional WARN / `QBC-MG-W-*` only if Eng adds a side fixture). Primary MUST fixture = **new co-located multi-VD + single QoBC** (`synthetic_qbc_multigrid_vd_4326.h5` — name flexible). |
| **O3** | Shape matches only non-`[0]` (not `grids[0]`) → **still bind matcher(s)** + log **WARN** (no silent drop; not LoadError solely for “not index 0”). |
| **O4** | Storage = **Eng choice** (per-grid `qbc_ids` copy **or** dataset-level shared view). Contract pins **behaviour** only. |
| **O5** | **`sample_cell` MUST** as in §3.1 — required oracle **QBC-MG-3**. |
| **O6** | QBC-META gets a **one-line pointer**: multi-grid bind → this case (done in sync stubs). |

### 3.3 MUST table

| Id | MUST |
|----|------|
| **MG-M1** | Load one QoBC.01 + FAT + dcf=9; dataset `has_qbc_coverage` when group loaded. |
| **MG-M2** | Bind by O1 hybrid to **every** matching `grids[i]`; **forbid** `grids[0]`-only attach. |
| **MG-M3** | **No** matching grid → **`LoadError`**. |
| **MG-M4** | CTest fixture MG-PS: ≥2 co-located BC.nn (distinct verticalDatum), one QoBC.01; `grids.size()≥2`; matched grids sample with `has_qbc`. |
| **MG-M5** | **`grids[0]` and `grids[1]` `sample_cell` (same row/col) both `has_qbc`** (**QBC-MG-3**). |
| **MG-M6** | Portray: **no** QBC id CoverageFill (NullInstruction); depth N× CoverageFill semantics from deepen retained. |
| **MG-M7** | Regress `s102_qbc_meta` / `s102_deepen_multigrid` / `s102_uncertainty` still green; **do not** flip BCAST / **13031**. |
| **MG-M8** | CTest **`s102_qbc_multigrid` MUST NOT SKIP**; all asserts prefixed **`QBC-MG-*`**. |

### 3.4 DROP (frozen)

- Multi-QoBC container / `QualityOfBathymetryCoverage.02`… (violates 0..1 / mandatory naming).
- Paint id raster as CoverageFill / colour ramp; edit official QoBC.lua.
- Touch **`BCAST-*`** election or VG **`13031`**.
- S-98 worst-case buffer; EncDotNet pixel; Annex H multi-resolution.

---

## 4. Open (historical Eng pins — tip-truth already landed @ `aabbb54`)

**None remaining as Architect Open** — Planner Open O1–O6 frozen in §3; Eng pins (float compare for origin/spacing, SOURCE.md lon/lat / id cells, generator `scripts/gen_s102_qbc_multigrid_synthetic.py`, O4 per-grid copy) **done** on tip `aabbb54`.

Planner O-questions (BRIEF §7) → **answered / freeze** in header Status block (docs close; Win no a/b; `QBC-MG-*` ≠ META `QBC-*`/`BCAST-*`; next=**TBD**; `grid()` Backlog).

---

## 5. Fixtures + oracles

| Fixture | File | Surface | Purpose |
|---------|------|---------|---------|
| **MG-PS** (MUST) | `synthetic_qbc_multigrid_vd_4326.h5` (tracked; name flexible) | **`QBC-MG-*`** | BC.01 + BC.02 **same** envelope/dims, different `verticalDatum`; one QoBC.01 + FAT ≥2 rows; EPSG:4326 |
| Deepen disjoint | `synthetic_multigrid_4326.h5` | — | **Non-goal** this slice (optional WARN only) |
| Fixture C / B | unchanged | `QBC-*` / `BCAST-*` | Stay under `s102_qbc_meta` |

### CTest `s102_qbc_multigrid` (Tester MUST)

| # | Id | Assert | Sev |
|---|-----|--------|-----|
| 1 | **QBC-MG-1** | Open MG-PS; `grids.size()≥2`; `has_qbc_coverage==true` | MUST |
| 2 | **QBC-MG-2** | Co-located point: `sample_wgs84` → `has_qbc` + pinned `qbc_id` + `lookup_qbc_record` | MUST |
| 3 | **QBC-MG-3** | `grids[0]` and `grids[1]` `sample_cell` (same row/col) **both** `has_qbc` | MUST |
| 4 | **QBC-MG-4** | Portray: no QBC id CoverageFill; depth Coverage instructions ≥2 | MUST |
| 5 | **QBC-MG-5** | Prior `s102_qbc_meta` / deepen / uncertainty still green | MUST |

Soft / optional: **QBC-MG-W1** disjoint+QoBC-on-.01 → `.02` zone `!has_qbc` — **WARN only**, not required to ship.

---

## 6. Non-goals

- Aggregator ↔ member HO (SIG-PRODUCER backlog).
- Reopening **`IHO-S164-SIG-CN-SHAPE`** / **`IHO-S164-SIG-PRODUCER`**.
- Deepen **disjoint**-envelope + shared QoBC as a **MUST** oracle (≠ PS multi-VD).
- Painting QoBC id / editing NullInstruction; install-prefix.
- S-104 / S-111 time; EncDotNet; S-98 uncertainty buffer; Annex H multi-resolution.
- Treating CHS-without-QoBC as a product defect.

---

## 7. Sync stubs (this rev1)

| Doc | Change |
|-----|--------|
| `docs/testing/iho-ctest-case-ids.md` | **Active** row `IHO-S102-QBC-MULTIGRID` → `s102_qbc_multigrid` (rev1 wording) |
| `docs/api-contracts.md` | Multi-grid QoBC: O1 hybrid bind; no `grids[0]` hard-bind; no-match → LoadError; pointer here |
| `docs/s102-qbc-meta-contract.md` | One-line / Backlog pointer → this case (O6) |
| `docs/architecture.md` | **Active / Open** row for this case; **next** = this case; after Closed → **TBD** |

---

## 8. Implement sketch (Eng — **already landed** @ `aabbb54`; docs close only)

1. ~~Replace `load_qbc_id_grid(..., grids[0], ...)`~~ — **done**: load QoBC.01 once; for each `grids[i]`, if shape+origin/spacing match → attach/share `qbc_ids`; zero matchers → `LoadError`.
2. O3 / O4 / MG-PS generator / SOURCE.md **`QBC-MG-*`** / CTest `s102_qbc_multigrid` — **done** on tip.
3. This Open tip = **Arch docs close / verify**; no Eng rewrite; no push from Architect.
