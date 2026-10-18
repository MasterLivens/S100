# S-102 QoBC multi-grid binding (rev1)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S102-QBC-MULTIGRID`** (**Active**). CTest: **`s102_qbc_multigrid`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev1** — Planner Open **frozen** from `/workspace/research/qbc-multigrid/BRIEF.md`. Docs only in this Architect commit — **no** engine / fixture / CMake / CI change here.

Takes the multi-grid QoBC binding gap deferred from closed **`IHO-S102-QBC-META`** (`docs/s102-qbc-meta-contract.md`). Does **not** reopen QBC-META U32+FAT / NullInstruction / **`QBC-*`** / **`BCAST-*`** surfaces, nor UNCERTAINTY VG **`13031`**.

Primary research: `/workspace/research/qbc-multigrid/BRIEF.md` (+ `/workspace/research/s102-uncertainty/BRIEF.md` §10 multiplicity). Code gap: `hdf5_product.cpp` `load_quality_of_bathymetry_coverage` → `load_qbc_id_grid(..., out.grids[0], ...)` hard-bind.

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

## 4. Open

**None** — Planner Open O1–O6 frozen in §3. Remaining Eng pins (exact float compare for origin/spacing, SOURCE.md lon/lat / id cells, generator script name) are implementation detail, not Architect Open.

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
| `docs/architecture.md` | No change this rev1 |

---

## 8. Implement sketch (Eng — after this rev1)

1. Replace `load_qbc_id_grid(..., grids[0], ...)` with: load QoBC.01 once; for each `grids[i]`, if shape+origin/spacing (envelope) match → attach/share `qbc_ids`; if **zero** matchers → `LoadError`.
2. O3: matchers only at `i≠0` → bind them + WARN log.
3. O4: copy or shared view — Eng choice.
4. Generator for MG-PS + `.gitignore` `!` + SOURCE.md **`QBC-MG-*`** oracles (include dual `sample_cell`).
5. CTest `s102_qbc_multigrid` (Linux+Win); regress meta / deepen / uncertainty.
6. No push from Architect; Eng lands on eng branch after Tester plan.
