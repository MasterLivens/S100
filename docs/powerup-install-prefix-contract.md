# S-164 / S-98 power-up install-prefix / install-tree catalogue root (rev 1) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S164-POWERUP-INSTALL-PREFIX`** (**Active / Open**). CTest: **`powerup_install_prefix`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev 1** — Planner freeze (MUST below). Install-tree catalogue root / `(prefix)/share/s100` slice after closed **`IHO-DISPLAY-PRIORITY`**. Does **not** reopen POWERUP PU-1…PU-6 / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE; does **not** jump **`S102-GRID-SHIM-RETIRE`**.

Frozen with: `docs/api-contracts.md` (Power-up install-prefix), `docs/powerup-self-check-contract.md` (PU-1…PU-6 — **do not** reopen), `docs/testing/iho-ctest-case-ids.md`. Draw order → **DISPLAY-PRIORITY** already Closed — **do not** reopen. Scale filter → **SCAMIN** already Closed — **do not** reopen. Unified pick → **SESSION-PICK** already Closed — **do not** reopen. Vendor / sha → **CATALOGUE** already Closed — **do not** reopen. Scrubber → **TIMELINE** already Closed — **do not** reopen. Wiring → **PC-WIRE** already Closed — **do not** reopen. FASC / deb·rpm·MSI / official PowerUp TDS / testdata-in-prefix / code signing = Non-goals.

Primary research: `/workspace/research/powerup-install-prefix/BRIEF.md` (2026-10-25 refresh). Former Eng gap **already fixed** @ Eng **`5d323b8`** + Win fix **`065887b`** (docs seed **`fa85162`**).

**Status: Active / Open (2026-10-25)** — after **DISPLAY-PRIORITY Closed** (tip `c8a1104` ≡ Arch `e8d4b2b`; tip Close CI [`40175675310`](https://github.com/MasterLivens/S100/actions/runs/40175675310) dual-green; Open-bind evidence CI [`40175087823`](https://github.com/MasterLivens/S100/actions/runs/40175087823) @ `dbfa1b6`≡`e6afbf7`, O1=tip-truth/docs-close). Eng tip ancestors **`5d323b8`** + **`065887b`** already on tip — **not greenfield** (`install(DIRECTORY catalogues DESTINATION share/s100)` + shared catalogue-root resolve Dev=`CMAKE_SOURCE_DIR` / Install=`(prefix)/share/s100` / env `S100_CATALOGUE_ROOT` + CLI/viewer `S100_DEFAULT_*` same root + product PU-4/5 `smoke_na_install_tree` + CTest `powerup_install_prefix` cmake `-P` + Win-safe `fs::relative` assert; **no** Win viewer a/b chain — Win path fix already landed). Ubuntu+Windows already Passed — tip Close CI [`40175675310`](https://github.com/MasterLivens/S100/actions/runs/40175675310) `#22 powerup_install_prefix` + Eng landing [`39117072306`](https://github.com/MasterLivens/S100/actions/runs/39117072306) @ `065887b` (cmake `-P` DESTDIR/`--prefix` + native `s100_tests` — **not** `viewer_*` — **no Win O1 (a)/(b)**). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from DISPLAY-PRIORITY / POWERUP self-check / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE. Do **not** invent case ids or next tip names; freeze tip-truth below. Do **not** reopen POWERUP PU-1…PU-6 / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE. Do **not** invent `INSTALLPFX-*` failure-face prefixes (keep existing bare SCHECK / cmake FATAL). Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). Contract display header stays **rev 1** (do **not** bump just for Open). After Closed next: **TBD** (architecture / registered Active only — do **not** invent).

**Planner freeze (MUST — from BRIEF §7 + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-S164-POWERUP-INSTALL-PREFIX`** / CTest **`powerup_install_prefix`**; **forbidden** to invent a second case/test name.
- MUST tip-truth = primary root `${CMAKE_INSTALL_PREFIX}/share/s100`; env `S100_CATALOGUE_ROOT` wins when set; Dev pin `CMAKE_SOURCE_DIR` kept for `powerup_self_check`; Install binaries **MUST NOT** hard-pin engineer source tree; `install()` MUST `catalogues/` → `share/s100/catalogues/…`, MUST NOT `testdata/` / TEST-ONLY SA; product PU-4/5 skip/N/A → Pass + `smoke_na_install_tree`; `S100_DEFAULT_*` same catalogue-root resolve; CTest temp-prefix install → existence-only PU-2 Pass (sha stays `catalogue_provenance`); **MUST NOT SKIP**; FASC / TDS / deb·rpm / reopen PU table = Non-goals.
- Eng tip already has **`5d323b8`** + Win **`065887b`** (docs seed **`fa85162`**); Ubuntu+Win (tip `40175675310` `#22` / Eng `39117072306`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield.
- Self-check PU-1…PU-6 → **`IHO-S164-POWERUP`** still Active — **do not** reopen PU table / severity / Block / CLI exit.
- Draw order → **`IHO-DISPLAY-PRIORITY`** already Closed — **do not** reopen.
- Scale filter → **`IHO-SCAMIN-FILTER`** already Closed — **do not** reopen.
- Unified pick → **`IHO-SESSION-PICK-UNIFIED`** already Closed — **do not** reopen.
- Vendor / sha → **`IHO-CATALOGUE-COMPLETE`** already Closed — **do not** reopen.
- Scrubber → **`IHO-TIMELINE-UI`** already Closed — **do not** reopen.
- Wiring → **`IHO-PC-WIRE-111-124`** already Closed — **do not** reopen.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog** (DEEPEN O1); **not** this tip.
- Failure face = existing bare SCHECK / cmake FATAL; do **not** invent `INSTALLPFX-*` prefixes.
- Ubuntu + Windows: `powerup_install_prefix` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen POWERUP PU-1…PU-6 / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"` (path fix `065887b` already on tip). Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`; cmake `-P` DESTDIR/`--prefix` + native `s100_tests`; existence-only; no SKIP. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `powerup_install_prefix` MUST NOT SKIP, Passed (already tip CI `40175675310` `#22` / Eng `39117072306`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng diff required
- After Close next: **TBD**

**Eng notes:** verify `5d323b8`/`065887b` on tip (docs seed `fa85162`); API `include/s100/powerup.hpp` + `src/core/powerup.cpp` shared root resolve; `install(DIRECTORY catalogues DESTINATION share/s100)`; drivers `tests/check_powerup_install_prefix.cmake` + `tests/test_powerup_install_prefix.cpp`; cmake `powerup_install_prefix`. DISPLAY-PRIORITY Closed already on this stack (`c8a1104`). Do **not** reopen POWERUP PU-1…PU-6 / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE. FASC / TDS / deb·rpm / testdata-in-prefix **Non-goals** — do **not** harden. Do **not** invent `INSTALLPFX-*` failure prefixes. Do **not** change tip-truth install layout / PU-2 relative path table.

**O-questions (BRIEF §3 Open 1–N — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `5d323b8`/`065887b` already; no `ci.yml` / no large Eng) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` (path fix already) — **yes / freeze**.
3. Contract head → Open; keep display **rev 1** (do **not** bump just for Open) — **yes / freeze**.
4. case-ids Active → Open (Status stays Active family) — **yes / freeze**.
5. Notes ≠ POWERUP self-check (PU-1…PU-6 do **not** reopen); ≠ DISPLAY-PRIORITY (already Closed; do **not** reopen); ≠ SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE (already Closed; do **not** reopen); ≠ S-124 / S-111 / S-104 open; do **not** reopen QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE; shim-retire stays Backlog; FASC / TDS / deb·rpm / testdata-in-prefix stay Non-goals — **yes / freeze**.
6. Closed next → **TBD** (architecture / registered Active; do **not** invent) — **yes / freeze**.
7. No other cases this tip (do **not** Closed SIG / POWERUP self-check / etc.) — **yes / freeze**.
8. architecture.md flip **next** to this Open; clarify install-prefix case Active/Open (docs close), not invent Closed — **yes / freeze**.
9. `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog — **yes / freeze**.
10. Failure face = existing bare SCHECK / cmake FATAL; do **not** invent `INSTALLPFX-*` prefixes — **yes / freeze**.
11. install-tree root/layout = this tip; PU-1…PU-6 self-check = POWERUP Active (do **not** reopen); draw order = DISPLAY-PRIORITY Closed — **yes / freeze**.

---

## 1. Goal

After **`cmake --install`** into a prefix (or `DESTDIR` + prefix), the installed product:

1. Resolves **PU-2 catalogue root** under the **install layout** (not `CMAKE_SOURCE_DIR`).
2. Finds the same relative PU-2 FC/PC paths as POWERUP §3.1 under that root.
3. Uses the **same** catalogue-root resolve for CLI/viewer **`S100_DEFAULT_*`** FC/PC defaults (one source of truth).
4. Proves (1)–(2) with CTest **`powerup_install_prefix`**: independent temp prefix / `DESTDIR` install → catalogues land + PU-2 Pass; **MUST NOT SKIP**; Linux + Windows.

**Keep** existing CTest **`powerup_self_check`** as the **build-tree** no-SKIP contract (Dev/CI pin to `CMAKE_SOURCE_DIR` remains valid).

This is Track B install-tree scaffolding. It does **not** claim packaging (deb/rpm/MSI) or type approval.

---

## 2. Spec anchors (frozen)

| Id | Topic | Spec / project hook | Notes |
|----|-------|---------------------|-------|
| **IP-C1** | Persistent catalogues | S-98 Ed 2.0.0 §**20.1**; S-164 §**2.1.1** `InitialCatalogues` / `PowerUpCatalogues` | Install method OEM-defined; this case defines **our** CMake layout |
| **IP-C2** | PU-2 file list | POWERUP §3.1 / `check_catalogues.cmake` relative paths | **Unchanged** relative paths; only the **root** changes for install config |
| **IP-C3** | Prior packaging deferral | POWERUP / SIG “queue next = install-prefix”; prior BRIEF had no layout | This case **is** that follow-on |
| **IP-C4** | Build-tree CTest | POWERUP §6 `powerup_self_check` | **MUST** remain no-SKIP; this slice adds a **parallel** install CTest |
| **IP-C5** | Provenance sha | `catalogue_provenance` / SOURCE.md | Install CTest = **existence only**; sha stays build-tree provenance |

---

## 3. Policy (frozen MUST)

### 3.1 Catalogue root (IP-1) — Open #1 closed

**Primary install catalogue root:**

```text
${CMAKE_INSTALL_PREFIX}/share/s100
```

Under that root, PU-2 relative paths are unchanged, e.g.:

```text
${prefix}/share/s100/catalogues/S-101/S-101_FC_2.0.0.xml
${prefix}/share/s100/catalogues/S-101/PC_2.0.0/portrayal_catalogue.xml
… (full POWERUP §3.1 table)
```

**Optional override:** environment variable **`S100_CATALOGUE_ROOT`** (absolute path to the catalogue root). When set and resolvable, it **wins** over the primary install layout (and over compile-time Dev pin) for runtime PU-2 / default FC/PC resolve. Empty / unset → use config default below.

### 3.2 Dev vs Install config (IP-2) — Open #2 closed

| Config | Catalogue root default | Notes |
|--------|------------------------|-------|
| **Dev / CI build** (current) | Compile-pin `S100_CATALOGUE_ROOT="${CMAKE_SOURCE_DIR}"` (or equivalent) | **`powerup_self_check` MUST keep passing** unchanged |
| **Install / packaged binary** | Resolve to `(prefix)/share/s100` (see §3.7 for Windows relative fallback) | **MUST NOT** leave product binaries hard-pinned to the engineer’s source tree |

CMake may use two compile definitions / generator expressions / install-time rewrite — Eng choice — as long as:

- Build-tree tests still see source-tree catalogues.
- Installed binaries do **not** require `CMAKE_SOURCE_DIR` to exist.

### 3.3 `install()` scope (IP-3) — Open #3 closed

| Path | Install? |
|------|----------|
| `catalogues/` (tree needed for PU-2 + catalogue-complete; includes PC colour/symbol subtrees already under those product PCs) | **MUST** → land under `${CMAKE_INSTALL_PREFIX}/share/s100/catalogues/…` |
| Existing `install(TARGETS …)` bin/lib + `include/s100` | **Unchanged** |
| `testdata/` (incl. `testdata/s101/signatures/**`, TEST-ONLY SA keys) | **MUST NOT** install into the product prefix |
| `docs/` | **Not required** this slice |
| Official S-164 PowerUp TDS | **MUST NOT** (POWERUP non-goal; still local-optional only) |

Suggested CMake shape (non-normative sketch):

```cmake
install(DIRECTORY catalogues DESTINATION share/s100)
# do NOT: install(DIRECTORY testdata …)
```

### 3.4 PU-4 / PU-5 on product install process (IP-4) — Open #4 closed

Installed **product** process entry (CLI / viewer / library default after install):

- **PU-4 / PU-5 smoke → skip / N/A → Pass** for that process (record item as N/A or Pass with message `smoke_na_install_tree`).
- **MUST NOT** depend on source-tree `testdata/` or TEST-ONLY SA paths at product runtime.
- **MUST NOT** ship or install TEST-ONLY signature fixtures / test SA into the product prefix.

**Full PU-4 / PU-5** remain **build-tree / CI** obligations via existing **`powerup_self_check`** (and any Eng-chosen test-only injectables). Install CTest (§3.5) asserts **PU-2** (and layout), not a second full PU-4/5 battery against installed testdata.

PU-1 / PU-3 behaviour on install process: unchanged classes (identity string; builtin SA still embedded — no testdata). PU-6 stub unchanged.

### 3.5 CTest `powerup_install_prefix` (IP-5) — Open #5 closed

| Item | Requirement |
|------|-------------|
| Name | **`powerup_install_prefix`** (**new**; parallel to `powerup_self_check`) |
| SKIP | **Forbidden** — no `SKIP_RETURN_VALUE`, no platform `#ifdef` skip |
| Platforms | **Linux + Windows** CI both green |
| Shape | Independent smoke: configure/build as today → `cmake --install <build> --prefix <tmp>` (and/or `DESTDIR`) → assert PU-2 files exist under `<tmp>/share/s100/…` → run install-aware power-up / library check so **PU-2 Pass** |
| Relation to `powerup_self_check` | **Keep** build-tree test; **do not** merge or weaken it |
| Relation to `check_catalogues.cmake` | **Extend layout via install**, not a fork of sha pins; install CTest does **not** replace `catalogue_provenance` |
| Sha | **Existence only** in this CTest; sha256 remains **`catalogue_provenance`** only (IP-C5) |

### 3.6 `S100_DEFAULT_*` (IP-6) — Open #6 closed

This slice **MUST** relativize CLI/viewer **`S100_DEFAULT_FC` / `S100_DEFAULT_PC` / `S100_DEFAULT_S*`** (and any sibling default catalogue macros) to the **same** catalogue-root resolve as PU-2:

- Dev: under `${CMAKE_SOURCE_DIR}/catalogues/…` (current behaviour OK via shared root).
- Install: under `${catalogue_root}/catalogues/…` where `catalogue_root` is `(prefix)/share/s100` or `S100_CATALOGUE_ROOT`.

**Kill the second source of truth** — no remaining absolute `CMAKE_SOURCE_DIR/catalogues/…` literals in **Install** config that bypass the shared resolver.

### 3.7 Windows (IP-7) — Open #7 closed

- Same **DESTDIR / `--prefix`** CTest pattern as Linux (multi-config: install the tested config into the temp prefix).
- **Post-install fallback OK:** resolve catalogue root relative to the executable as `../share/s100` (i.e. sibling of `bin/`) when the primary prefix metadata is unavailable — **same layout**, not a second tree.
- Env **`S100_CATALOGUE_ROOT`** override still wins when set.

### 3.8 Failure policy (install-tree PU-2)

Align POWERUP §4 for operational paths: if install-tree catalogue root cannot be resolved or any PU-2 MUST path is missing → **Block** (CLI exit **2** on product process when overall Block). Install CTest failure = test fail (non-zero), not a SKIP.

Incomplete install tree in CI → **fail** `powerup_install_prefix` (do not soft-pass).

---

## 4. Dual-root policy (summary)

| Root | Who uses it | CTest |
|------|-------------|-------|
| **Build-tree** (`CMAKE_SOURCE_DIR`) | Dev builds; existing power-up / DEFAULT macros in Dev config | **`powerup_self_check`** (full PU-1…PU-6 per POWERUP rev1) |
| **Install-tree** (`(prefix)/share/s100`) | Packaged / installed binaries; Install config | **`powerup_install_prefix`** (layout + PU-2; PU-4/5 N/A) |
| **Env override** (`S100_CATALOGUE_ROOT`) | Any process when set | Tests may inject |

Relative PU-2 path table = **one** table (POWERUP §3.1). Roots differ; paths do not.

---

## 5. Non-goals

- **FASC** / `IHO-S101-BRIDGE-RENDER` / portrayal pixel / EncDotNet / `IHO-S164-DISPLAY-COMPARE` — **do not cut in line**.
- **Reopen** POWERUP PU-1…PU-6 severity, Block scope, CLI exits, or §3.1 file list.
- Official S-164 `PowerUp*` / `InitialPowerUp` / `PowerUpCatalogues` TDS in repo or CI.
- deb / rpm / MSI / NSIS / code signing / system package-manager integration.
- Installing `testdata/` or TEST-ONLY SA / signature gold into the product prefix.
- Reopen SIG / PRODUCER / CN-SHAPE / AGGREGATOR; producer-code in power-up.
- QBC / uncertainty / deepen; permits / encryption / S-63.
- Claiming ECDIS type approval or SOLAS carriage.
- Changing `powerup_self_check` to SKIP or adding platform guards.
- Making install CTest re-assert sha256 (stays `catalogue_provenance`).

---

## 6. Docs sync (this commit)

- `docs/testing/iho-ctest-case-ids.md` — **`IHO-S164-POWERUP-INSTALL-PREFIX`** **Active** rev1; AGGREGATOR “queue next=install-prefix” → **closed by this case**.
- `docs/api-contracts.md` — new install-prefix pointer; aggregator queue-next closed.
- `docs/architecture.md` — catalogue / integrity install-prefix Active note.
- `docs/part15-aggregator-producer-contract.md` — queue next install-prefix → **Active** here.
- `docs/part15-cn-shape-contract.md` — Install-prefix Backlog → **Active** via this case.
- `docs/powerup-self-check-contract.md` — cross-ref only (PU table **unchanged**).
- This file — **rev1 / Opens cleared**.

---

## 7. Open items

**None.** Planner freeze (2026-10-18) closed:

1. Primary root `(prefix)/share/s100`; env `S100_CATALOGUE_ROOT` override.
2. Dev pin `CMAKE_SOURCE_DIR` kept; Install resolve separate; **MUST NOT** break `powerup_self_check`.
3. `install()` MUST `catalogues/`; **exclude** `testdata/`; headers/libs unchanged.
4. Product install process: PU-4/PU-5 **skip/N/A**; full PU-4/5 stay build/CI.
5. New CTest `powerup_install_prefix` (DESTDIR/`--prefix`); MUST NOT SKIP; Linux+Win.
6. `S100_DEFAULT_*` relativized to same catalogue root this slice.
7. Windows: same DESTDIR pattern; `../share/s100` relative fallback OK.
8. Contract path + case-ids Active; do not change POWERUP PU table.
9. Install CTest existence only; sha remains `catalogue_provenance`.

---

## 8. Engineer / Tester hold

**Engineer:** implement §3 layout + shared catalogue-root resolve; `install(DIRECTORY catalogues …)`; Install-config defaults; product PU-4/5 N/A on install process; CTest `powerup_install_prefix`; **keep** `powerup_self_check` green; **do not** install `testdata/`; **do not** edit POWERUP PU table semantics.

**Tester:** gate on §3–§4; no-SKIP dual platform for **both** `powerup_self_check` and `powerup_install_prefix`; assert temp-prefix layout + PU-2 Pass; assert no testdata in prefix; sha only via `catalogue_provenance`.
