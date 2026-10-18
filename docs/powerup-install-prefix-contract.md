# S-164 / S-98 power-up install-prefix / install-tree catalogue root (rev1)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S164-POWERUP-INSTALL-PREFIX`** (**Active**). CTest: **`powerup_install_prefix`** — **MUST NOT SKIP**, no platform guard, Linux + Windows CI.

**Status: rev1 — Planner BRIEF freeze applied (Open O1–O9 closed).** Docs only in this Architect commit — **no** engine / fixture / CMake / CI change here. Engineer may implement after this rev1.

**Follow-on to** **`IHO-S164-POWERUP`** (`docs/powerup-self-check-contract.md` rev1). **Do not** reopen PU-1…PU-6 severity, Block scope, CLI exits, or PU-2 relative path table. This slice only moves **where** the product catalogue root resolves after `cmake --install`.

Primary research: `/workspace/research/powerup-install-prefix/BRIEF.md` (Planner, 2026-10-18). Prior: `/workspace/research/powerup/BRIEF.md` §3.3 (injectable catalogue root only — **no** install layout). Anchors: S-98 Ed 2.0.0 §**20.1** (persistent FC/PC install); POWERUP PU-2 / `docs/catalogue-complete-contract.md`; CMake `install()` today = bin/lib + `include/s100` only (`CMakeLists.txt`).

Queue: closes AGGREGATOR / CN-SHAPE “queue next = install-prefix”. **FASC does not cut in line.**

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
