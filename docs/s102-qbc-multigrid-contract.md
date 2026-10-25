# S-102 QoBC multi-grid binding (rev1) (Closed)

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/s102-contract.md` / `docs/s102-deepen-contract.md` / `docs/s102-uncertainty-contract.md` / `docs/s102-qbc-meta-contract.md`. Case id: **`IHO-S102-QBC-MULTIGRID`** (**Closed / Done** 2026-10-25).  
CTest name: **`s102_qbc_multigrid`** — MUST NOT SKIP; Linux + Windows dual-green (CI `40156013552` @ `30f3546`). Supersedes Open draft on tip `30f3546`.

Primary research: `/workspace/research/qbc-multigrid/BRIEF.md` (+ `/workspace/research/s102-uncertainty/BRIEF.md` §10 multiplicity). Former code gap (`hdf5_product.cpp` `grids[0]` hard-bind) **already fixed** @ Eng **`aabbb54`**.

**Status: Closed / Done (2026-10-25)** — CI [`40156013552`](https://github.com/MasterLivens/S100/actions/runs/40156013552) @ `30f3546` dual-green; **`s102_qbc_multigrid` Passed Ubuntu+Win** (native `s100_tests` + HDF5 — **not** `viewer_*`; **no Win O1 (a)/(b)**). Eng **`aabbb54`** already on tip — **docs close only**. Oracles MUST **QBC-MG-1..5** **unchanged** (tip-truth). **O1=tip-truth / docs-close freeze SUCCESS**. Failure face **`QBC-MG-*`** (do **not** reopen META **`QBC-*`/`BCAST-*`**). Forbid `grids[0]` hard-bind; O1 hybrid shape+origin. `grid()` shim stays Backlog. Next: **TBD**.

**Planner freeze (kept — rev1 tip-truth):** Reuse case id / CTest name; MUST **QBC-MG-1..5** (fixture **MG-PS** co-located multi-VD + dual `sample_cell` + portray **NullInstruction** / **no** id colour band + prior `s102_qbc_meta` / deepen / uncertainty green); fixture MG-PS MUST NOT SKIP; failure face **`QBC-MG-*`** (do not reopen META **`QBC-*`/`BCAST-*`**); O1 hybrid shape+origin/spacing bind to every matching `grids[i]`; **forbid** `grids[0]` hard-bind; zero match → **`LoadError`**; Eng `aabbb54` already; `grid()` shim stays Backlog (DEEPEN O1); after Closed next **TBD** (architecture / backlog — do **not** invent).

**O1 closed as tip-truth / docs-close:** Eng `aabbb54` already on tip (shape+origin/spacing bind; forbid `grids[0]` hard-bind; zero match → LoadError; fixture MG-PS; CTest **`QBC-MG-1..5`**). Slice = Arch docs close only — not greenfield Eng. Do **not** invent oracles; do **not** reopen META / UNCERT / DEEPEN; do **not** conflate **`QBC-MG-*` ≠ `QBC-*` / `BCAST-*`**; do **not** retire `grid()` this tip. **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest; inventing oracles; retiring `grid()` this tip; inventing next tip id.

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

## 4. Slice close — 2026-10-25

**Status: Closed / Done** (formal docs close — **not** a loader / portray rewrite).

- Evidence: CI [40156013552](https://github.com/MasterLivens/S100/actions/runs/40156013552) tip `30f3546` — Ubuntu + Windows **`s102_qbc_multigrid` Passed** (MUST NOT SKIP). Ancestor greens also Eng landing [`39113487504`](https://github.com/MasterLivens/S100/actions/runs/39113487504) @ `aabbb54` `#6` + tip ancestor [`40098659137`](https://github.com/MasterLivens/S100/actions/runs/40098659137) `#6`.
- **O1=tip-truth / docs-close freeze SUCCESS** — Eng `aabbb54` already on tip; Arch Open→Closed only; Planner BRIEF §7 answers frozen.
- Oracles stay as frozen rev1 MUST **QBC-MG-1..5** — tip-truth unchanged.
- Eng `aabbb54` already on tip — docs close only; no `ci.yml` change.
- Failure face stays **`QBC-MG-*`** — do **not** reopen META **`QBC-*`/`BCAST-*`**.
- Forbid `grids[0]` hard-bind; O1 hybrid shape+origin/spacing attach; zero match → `LoadError`.
- **Notes:** next **TBD** (architecture / backlog — do **not** invent a next tip id). **Backlog:** `grid()` shim retire (`S102-GRID-SHIM-RETIRE`) — do not insert ahead of Active S-102 tips.
- Historical Eng pins (float compare origin/spacing; SOURCE.md lon/lat / id cells; generator `scripts/gen_s102_qbc_multigrid_synthetic.py`; O4 per-grid `qbc_ids` copy) — already landed @ `aabbb54`; not Close blockers.
- **Non-goals this close:** inventing oracles; inventing next tip id; retiring `grid()` shim; loader/portray rewrite; reopening META / UNCERT / DEEPEN; Win (a)/(b) (N/A — non-`viewer_*`); conflating **`QBC-MG-*`** with META **`QBC-*`/`BCAST-*`**.

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

## 7. Sync stubs (Closed)

| Doc | Change |
|-----|--------|
| `docs/testing/iho-ctest-case-ids.md` | **Closed/Done** row `IHO-S102-QBC-MULTIGRID` → `s102_qbc_multigrid`; next **TBD** |
| `docs/api-contracts.md` | Multi-grid QoBC: O1 hybrid bind; no `grids[0]` hard-bind; no-match → LoadError; pointer here (tip-truth kept) |
| `docs/s102-qbc-meta-contract.md` | One-line / Backlog pointer → this case (O6; taken / Closed) |
| `docs/architecture.md` | **Closed** row for this case; **next** / Active → **TBD** (architecture / backlog — no invented case) |

---

## 8. Implement sketch (Eng — **already landed** @ `aabbb54`; docs close only)

1. ~~Replace `load_qbc_id_grid(..., grids[0], ...)`~~ — **done**: load QoBC.01 once; for each `grids[i]`, if shape+origin/spacing match → attach/share `qbc_ids`; zero matchers → `LoadError`.
2. O3 / O4 / MG-PS generator / SOURCE.md **`QBC-MG-*`** / CTest `s102_qbc_multigrid` — **done** on tip.
3. This Close tip = **Arch docs close only**; no Eng rewrite; no push from Architect.
