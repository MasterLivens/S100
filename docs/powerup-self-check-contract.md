# S-164 / S-98 power-up / start-up self-check (rev 1) (Closed)

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/api-contracts.md` (Power-up) / `docs/powerup-install-prefix-contract.md` (Closed; install-tree → INSTALL-PREFIX separate — **do not** reopen) / `docs/testing/iho-ctest-case-ids.md` / `docs/display-priority-contract.md` (Closed; draw order → DISPLAY-PRIORITY separate) / `docs/scamin-contract.md` (Closed; scale filter → SCAMIN separate) / `docs/session-pick-contract.md` (Closed; unified pick → SESSION-PICK separate) / `docs/catalogue-complete-contract.md` (Closed; vendor/sha → CATALOGUE separate) / `docs/timeline-ui-contract.md` (Closed; scrubber → TIMELINE separate) / `docs/pc-wire-contract.md` (Closed; wiring → PC-WIRE separate). Case id: **`IHO-S164-POWERUP`** (**Closed / Done** 2026-10-25).  
CTest name: **`powerup_self_check`** — MUST NOT SKIP; Linux + Windows dual-green (CI `40177947235` @ `2d2e259`). Supersedes Open draft on tip `2d2e259` (≡ Arch `c64f9a6`).

Primary research: `/workspace/research/powerup/BRIEF.md` (2026-10-25 tip-truth refresh). Former Eng gap **already fixed** @ Eng **`980a10c`** (docs prep **`a93da6e`** / **`5491924`**). Install-tree root → **INSTALL-PREFIX** already Closed — **do not** reopen. Draw order → **DISPLAY-PRIORITY** already Closed — **do not** reopen. Scale filter → **SCAMIN** already Closed — **do not** reopen. Unified pick → **SESSION-PICK** already Closed — **do not** reopen. Vendor / sha → **CATALOGUE** already Closed — **do not** reopen. Scrubber → **TIMELINE** already Closed — **do not** reopen. Wiring → **PC-WIRE** already Closed — **do not** reopen. Part 15 crypto / producer-code → **SIG** family separate Active — **do not** mix. FASC / BRIDGE already Closed — **do not** mix. Official PowerUp TDS / producer-code / EncDotNet / type approval = Non-goals.

**Status: Closed / Done (2026-10-25)** — CI [`40177947235`](https://github.com/MasterLivens/S100/actions/runs/40177947235) @ `2d2e259`≡`c64f9a6` dual-green; **`powerup_self_check` Passed Ubuntu+Win** (native `s100_tests` + source-tree catalogues + synthetic smoke — **not** `viewer_*`; **no Win O1 (a)/(b)**). Eng **`980a10c`** already on tip — **docs close only**. MUST tip-truth **unchanged** (PU-1…5 Block ops-only + PU-6 WARN/N/A + PU-2 catalogue pins + PU-4 `verify_exchange_set` + CLI 0/2 + D7 + CI synthetic; fixtures **MUST NOT SKIP**). **Keep** PU-1…PU-6 oracles — **do not** expand. **O1=tip-truth / docs-close SUCCESS**. Failure face = existing bare SCHECK tip-truth (do **not** invent `POWERUP-*`). Do **not** reopen INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE; do **not** mix SIG / FASC; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog. Contract display stays **rev 1**. Next: **TBD**.

**Planner freeze (kept — rev 1 tip-truth):** Reuse case id **`IHO-S164-POWERUP`** / CTest **`powerup_self_check`**; MUST tip-truth = PU-1…5 MUST+Block (ops/exchange only); PU-6 WARN stub / CLI N/A→Pass; PU-2 = catalogue-complete pins (S-101/102/111/124 FC+PC; S-104 FC only / **no** S-104 PC); PU-3/4 Track B project decisions (SIG); PU-4 mini gold `verify_exchange_set`; CLI Pass/Warn=0 Block=2; D7 UNVERIFIED allowed; CI synthetic only; no producer-code; fixtures **MUST NOT SKIP**; official PowerUp TDS / EncDotNet / type approval = Non-goals; **keep** PU-1…PU-6 oracles (do **not** expand scope / severity / Block / exit / path table); install-tree → **INSTALL-PREFIX** already Closed (do **not** reopen); draw order → **DISPLAY-PRIORITY** already Closed (do **not** reopen); scale filter → **SCAMIN** already Closed (do **not** reopen); unified pick → **SESSION-PICK** already Closed (do **not** reopen); vendor/sha → **CATALOGUE** already Closed (do **not** reopen); scrubber → **TIMELINE** already Closed (do **not** reopen); wiring → **PC-WIRE** already Closed (do **not** reopen); SIG / FASC separate (do **not** mix); Eng `980a10c` already (docs prep `a93da6e`/`5491924`); `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog; failure face = bare SCHECK (do **not** invent `POWERUP-*` prefixes / new CTest); after Closed next **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE.

**O1 closed as tip-truth / docs-close:** Eng `980a10c` already on tip (`run_powerup_self_check` / `security_self_test` / `PowerUpReport` + PU-1…PU-6 + CLI/viewer hooks + session Block ops paths + CTest `powerup_self_check`; docs prep `a93da6e`/`5491924`; CTest **`powerup_self_check`**). Slice = Arch docs close only — not greenfield Eng. Do **not** invent case ids or next tip names; do **not** reopen INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE; do **not** mix SIG / FASC; do **not** invent `POWERUP-*` prefixes; do **not** retire `grid()` this tip; do **not** expand PU-1…PU-6; do **not** harden official TDS / producer-code / EncDotNet. **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest; inventing oracles / next tip id; inventing `POWERUP-*` failure-face prefixes.

---

## Slice close — 2026-10-25

**Status: Closed / Done** (formal docs close — **not** a self-check / Eng rewrite).

- Evidence: CI [40177947235](https://github.com/MasterLivens/S100/actions/runs/40177947235) tip `2d2e259`≡`c64f9a6` — Ubuntu + Windows **`powerup_self_check` Passed** (MUST NOT SKIP). Ancestor greens also Eng landing [`39109478325`](https://github.com/MasterLivens/S100/actions/runs/39109478325) @ `980a10c` + prior tip `#21 powerup_self_check` Passed (INSTALL-PREFIX Close CI [`40177052197`](https://github.com/MasterLivens/S100/actions/runs/40177052197) @ `f7e8418`).
- **O1=tip-truth / docs-close SUCCESS** — Eng `980a10c` already on tip; Arch Open→Closed only; Planner BRIEF §7 answers frozen.
- MUST tip-truth unchanged (PU-1…5 Block ops-only + PU-6 WARN/N/A + PU-2 catalogue pins + PU-4 `verify_exchange_set` + CLI 0/2 + D7 + CI synthetic + fixtures **MUST NOT SKIP**). **Keep** PU-1…PU-6 oracles — **do not** expand.
- Eng `980a10c` already on tip — docs close only; no `ci.yml` change; **no Win (a)/(b)**; no large Eng.
- Failure face stays bare SCHECK tip-truth — do **not** invent `POWERUP-*`.
- **Notes:** next **TBD** (architecture / registered Active — do **not** invent a next tip id). **Backlog:** `grid()` shim retire (`S102-GRID-SHIM-RETIRE`) — do not insert ahead. Contract display stays **rev 1**. Install-tree → INSTALL-PREFIX already Closed (do **not** reopen); draw order → DISPLAY-PRIORITY already Closed (do **not** reopen); scale filter → SCAMIN already Closed (do **not** reopen); unified pick → SESSION-PICK already Closed (do **not** reopen); vendor/sha → CATALOGUE already Closed (do **not** reopen); scrubber → TIMELINE already Closed (do **not** reopen); wiring → PC-WIRE already Closed (do **not** reopen); SIG / FASC separate (do **not** mix).
- Boundaries: independent; ≠ INSTALL-PREFIX (already Closed); ≠ DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE (already Closed); ≠ SIG / FASC (do **not** mix); do **not** reopen INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE.
- **Non-goals this close:** inventing oracles / next tip id; retiring `grid()` shim; self-check/Eng rewrite; expanding PU-1…PU-6; reopening INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE; mixing SIG / FASC; Win (a)/(b) (N/A — non-`viewer_*`); inventing `POWERUP-*` failure-face prefixes; hardening official TDS / producer-code / EncDotNet.

---

### Naming context (locked)

**S-164 “PowerUp” / `InitialPowerUp` / `PowerUpCatalogues`** = TDS **exchange-set directory names** for loading catalogues / datasets at certification startup (S-164 Ed1.0.0 §2.1.1 / §2.2.1–2.2.2, S-98 §20.1, IEC 61174 4.4.1 via S-164). They are **not** an OEM “crypto module BIT / self-test” checklist.

**PU-3 / PU-4** are **Track B project decisions** (hardening tied to SIG / `s100_security`) — label them as such; do **not** present them as S-164 mandate text.

**Explicit exclusion:** SIG **producer-code matching** (CN MRN ↔ dataset / CATALOG producer; S-98 §9.1.1 style) is **out of this contract** — tracked as **`IHO-S164-SIG-PRODUCER`** (`docs/part15-producer-code-contract.md`).

---

## 1. Goal

On every **application start** (CLI process entry, viewer process entry, and a callable library hook used by both), run a **power-up / start-up self-check** that:

1. Proves the process knows who it is (software identity / version).
2. Proves critical on-disk catalogues needed for chart use are present (align catalogue-complete pins).
3. Proves the Part 15 trust path and signature module are alive (**call into** `s100_security`; do not reimplement SIG).
4. Smoke-tests catalogue / exchange-set open under **Enforce** on a **synthetic** mini fixture.
5. Surfaces a structured report so ChartSession / CLI / viewer can **block operational use** or **warn-and-continue** per §4.

This is Track B certification scaffolding. It does **not** claim type approval or SOLAS fitness.

---

## 2. Spec anchors (frozen)

| Id | Topic | Spec hook (frozen) | Notes |
|----|-------|--------------------|-------|
| **PU-C1** | Self-check trigger at power-up / app start | S-164 §**2.2.1** `InitialPowerUp` / §**2.2.2** `PowerUp`; IEC 61174 **4.4.1** (cited by S-164 only — no IEC PDF on disk) | Certification pack family; process entry MUST run power-up |
| **PU-C2** | Initial catalogues / catalogue presence | S-98 Ed 2.0.0 §**20.1** (persistent FC/PC install); S-164 §**2.1.1** `InitialCatalogues` / `PowerUpCatalogues` | **Do NOT cite** S-164 1.0.0 “Annex C C-21.1” — obsolete (S-98 2.0.0 Annex C is Update Status Reports; S-164 2.0.1 correctly cites §20.1) |
| **PU-C3** | Data integrity / authentication readiness | S-98 §**20.4.3 c** + App. B (SSE **105** / B-3 SA installed separately); Part 15 via SIG | Import-path authentication; PU-3/4 prove the path is alive |
| **PU-C4** | Software identity / version for survey | `docs/certification/ecdis-certified-system.md` §9; `iec61174-requirements-matrix.md` theme “Software identity / cyber” → module `integrity` | **Project / Track B survey scaffolding** — not an S-164 shall |
| **PU-C5** | Display / HMI readiness | S-98 §19.6 BLKADJ family (mariner-callable); S-164 InitialPowerUp display shots are certification packs | WARN-only / CLI N/A in this slice |

---

## 3. Power-up checklist (MUST / WARN — frozen)

| # | Check | Class | Severity | Spec / decision | Notes |
|---|-------|-------|----------|-----------------|-------|
| **PU-1** | Software identity / version string available, **non-empty printable** | **MUST** | **Block** | PU-C4 (cert survey / IEC matrix) | Source: `S100_VERSION` macro **or** CMake `PROJECT_VERSION` equivalent exposed at runtime. Empty / non-printable → Block |
| **PU-2** | Critical catalogues present under resolvable default catalogue root | **MUST** | **Block** | PU-C2: S-98 **20.1** + S-164 §**2.1.1** | File list §3.1 — align `check_catalogues.cmake` / catalogue-complete. Runtime: path resolvable + file exists. **SHA** stays in CTest `catalogue_provenance` only |
| **PU-3** | SA trust store present | **MUST** | **Block** | **Project decision** (Track B / SIG); aligns S-98 B-2 SSE **105** + B-3 + Part 15 D4 | `TrustStore::builtin_iho_52()` (or product-installed user SA) has **≥ 1** trust anchor; empty → Block (product cannot Enforce-import) |
| **PU-4** | Signature module self-test | **MUST** | **Block** | **Project decision** (Track B / SIG); aligns S-98 §20.4.3 c readiness | Mini gold via existing `verify_exchange_set` (test trust store + existing **TEST-ONLY** signed fixtures). Thin `security_self_test()` wrapper OK if it calls the **same** verify path. **Do not** reimplement ECDSA |
| **PU-5** | Catalogue / exchange load-path smoke | **MUST** | **Block** (operational path only — §4.1) | PU-C1: S-164 §**2.2.1** / §**2.2.2** | Synthetic mini exchange set + `open_exchange_set(..., VerifyPolicy::Enforce)` succeeds. Official PowerUp / PowerUpCatalogues TDS **NOT in CI** |
| **PU-6** | Display / basic I/O | **MUST** (stub) | **Warn** (viewer) / **N/A→Pass** (CLI) | PU-C5 | First Eng slice: viewer offscreen / main surface creatable (or stub Warn). CLI records N/A as Pass. **No** pixel compare / EncDotNet |

### 3.1 PU-2 critical-file list (align `tests/check_catalogues.cmake`)

Relative to the product **catalogue root** (injectable for tests; same resolver as session/CLI):

| Product | MUST exist (Block if missing) | MUST NOT exist |
|---------|-------------------------------|----------------|
| S-101 | `catalogues/S-101/S-101_FC_2.0.0.xml` | — |
| S-101 | `catalogues/S-101/PC_2.0.0/portrayal_catalogue.xml` | — |
| S-102 | `catalogues/S-102/S-102_FC_3.0.0.xml` | — |
| S-102 | `catalogues/S-102/PC_3.0.0/portrayal_catalogue.xml` | — |
| S-104 | `catalogues/S-104/S-104_FC_2.0.0.xml` | **`catalogues/S-104/PC_2.0.0/`** (forged PC → Block; official N/A) |
| S-111 | `catalogues/S-111/S-111_FC_2.0.0.xml` | — |
| S-111 | `catalogues/S-111/PC_2.0.0/portrayal_catalogue.xml` | — |
| S-124 | `catalogues/S-124/S-124_FC_2.0.0.xml` | — |
| S-124 | `catalogues/S-124/PC_2.0.0/portrayal_catalogue.xml` | — |

- **Default catalogue root** must be resolvable; if the root itself cannot be resolved → Block (all PU-2 items fail).
- **Any** missing MUST path → Block.
- **SHA256** pins remain in `catalogue_provenance` / SOURCE.md — power-up does **not** re-hash at start (existence + resolvable path only).
- IHO 5.2 SA root is covered by **PU-3** (`builtin_iho_52`), not listed as a catalogue-root file.
- `SOURCE.md` and full PC colour/symbol trees are **not** PU-2 MUST (provenance / portrayal contracts).

Pinned sha (for Eng cross-ref only — asserted by `catalogue_provenance`, not by power-up):

| File | sha256 (`check_catalogues.cmake`) |
|------|-----------------------------------|
| S-101 FC | `2743d0689d8a2130f9b809554591d794f01cd9953f222f065ac891b590bd3cfc` |
| S-101 PC | `a6165e950084dbb4ee6f75144a467f398809d57d8548d8d10ae80d715ac37962` |
| S-102 FC | `1aab69dacf18d3bf431c053fb284d74b97ec86b1d9042411956917203b6e50b5` |
| S-102 PC | `381606dfcea27d6f38be26f3f0574e79ad4149e7cc4e3e20ae176579d2f756d2` |
| S-104 FC | `a8e3fa6af176b380d8efe6730edcc4af9c7c8f39f7b889ca9499191c05ec7dfd` |
| S-111 FC | `6701de3b38540bac2a998f1fe1cd8d1a88b038a4bd71d750322ca9c377fe93a7` |
| S-111 PC | `40789188f6074e1849b0650d546812043d07a8fa58f18ebe350183c6a0c10e94` |
| S-124 FC | `0239ab29d2a1d9733e7042a4279c6c74aa6c2d37c2366bef91467466febce153` |
| S-124 PC | `48a748c1abd0d659773aabc92b0cb129e6b4278ea7edaa62f54d49b9fb08fcef` |

---

## 4. Failure policy (frozen)

### 4.1 Block scope — operational paths only

| Path | When overall == **Block** |
|------|---------------------------|
| `open_exchange_set` / product CLI exchange-set import / viewer **operational chart open** | **Refuse** |
| Standalone `open_dataset` / diagnostics / explicit `VerifyPolicy::Off` test paths | **Allow** — remain **`UNVERIFIED`** per SIG **D7**; may show a `POWERUP BLOCK` banner without changing D7 |
| `--version` / help / pure diagnostics | **Allow**; may still print the power-up report |

Hook **may** run lazily before the first operational open; **CLI and viewer process entry MUST run power-up** (eager).

### 4.2 CLI exit codes (frozen)

| overall | exit | Output |
|---------|------|--------|
| **Pass** | **0** | `powerup status=PASS` (+ items) |
| **Warn** only (no Block) | **0** | `powerup status=WARN`; warn lines on **stderr** |
| **Block** | **2** | Print failing items; exit **2**. Exit **1** remains reserved for **load / parse failures** (unchanged) |

### 4.3 Surfaces

```cpp
// Names normative for Eng; layout may gain fields but must stay backward-readable.
namespace s100 {

enum class PowerUpSeverity { Pass, Warn, Block };

struct PowerUpItem {
  std::string id;       // "PU-1" … "PU-6"
  PowerUpSeverity severity{PowerUpSeverity::Pass};
  std::string message;
};

struct PowerUpReport {
  PowerUpSeverity overall{PowerUpSeverity::Pass};  // max of items (Block > Warn > Pass); PU-6 CLI N/A does not raise overall
  std::string software_identity;                   // PU-1
  std::vector<PowerUpItem> items;
  // Optional: last VerificationReport from PU-4/PU-5 smoke (do not fork SIG types)
};

/** Run at process start (CLI/viewer MUST) or lazy before first operational open. Idempotent. */
PowerUpReport run_powerup_self_check(/* catalogue root + TrustStore / fixture injectables */);

}  // namespace s100
```

| Surface | Behaviour |
|---------|-----------|
| **Library** | `run_powerup_self_check()` → `PowerUpReport`. `ChartSession` may stash `last_powerup()`. `open_exchange_set` under product Enforce **refuses** when overall==Block |
| **CLI** | On start (before load): print status + per-item lines. Exit codes §4.2. Exchange-set / operational load still refuse when Block |
| **Viewer** | `POWERUP BLOCK: …` prevents operational chart open; `POWERUP WARN: …` allows open with banner. No automation beyond status text in this slice |

---

## 5. Connection to signature verification and catalogue / exchange load

**Do not reimplement SIG.** This contract **calls** the Active Part 15 surface (`docs/part15-signature-contract.md`):

| Power-up need | Existing hook | Policy |
|---------------|---------------|--------|
| Trust store present (PU-3) | `TrustStore::builtin_iho_52()` / `add_user_sa` / `for_tests` | Same types as SIG; empty ↔ SSE **105** semantics |
| Signature self-test (PU-4) | `verify_exchange_set` on mini gold, or thin `security_self_test()` → **same** path | `TrustStore::for_tests` + injectable `verify_time` (**never** wall clock — SIG D2). Prefer existing `testdata/s101/signatures/positive/` (TEST-ONLY keys) or a dedicated `testdata/s101/powerup/` subtree generated the same way |
| Exchange smoke (PU-5) | `ChartSession::open_exchange_set(path, err, VerifyPolicy::Enforce)` | Product default **Enforce**; synthetic fixture may use test trust **only inside** self-check / CTest — not a product escape hatch |
| Catalogue presence (PU-2) | Paths under catalogue root; no second catalogue parser | Align catalogue-complete pins |

Standalone `open_dataset` remains **`UNVERIFIED`** (SIG **D7**). Power-up **readiness** ≠ per-dataset authentication of every future open.

**Producer-code matching:** **out of scope** (separate later SIG slice).

---

## 6. CTest · fixtures · platforms

| Item | Requirement |
|------|-------------|
| CTest name | **`powerup_self_check`** |
| SKIP | **Forbidden** — no `SKIP_RETURN_VALUE`, no platform `#ifdef` skip |
| Platforms | **Linux + Windows** CI both green |
| Fixtures | **Synthetic only** in CI: mini signed exchange for PU-4/PU-5 (reuse `testdata/s101/signatures/positive/` and/or `testdata/s101/powerup/`). Signed bytes **`-text`** in `.gitattributes` (SIG A4). Generator may reuse `tools/gen_signature_fixtures.py` patterns |
| Official S-164 TDS | `PowerUpCatalogues` / `InitialPowerUp` / `PowerUp` — **local optional**, **not exercised** in CI (no SKIP if absent; do not assert) |
| Oracles | Tester: each PU-1…PU-6 row; Block refuses exchange/ops open; CLI exit 0/0/2; PU-2 list §3.1; PU-4 = mini gold verify; PU-6 WARN stub present in first Eng slice |

---

## 7. Non-goals

- **SIG producer-code matching** (CN MRN ↔ dataset / CATALOG producer) — **`IHO-S164-SIG-PRODUCER`**.
- **Full S-164 PowerUp / InitialPowerUp / PowerUpCatalogues TDS** in repo or CI.
- **EncDotNet / NIWC / pixel compare** — PU-6 is construct/smoke (or stub) only (`IHO-S164-DISPLAY-COMPARE` stays elsewhere).
- Reimplement SIG / ECDSA / OpenSSL outside `s100_security`.
- S-102-specific power-up rows (uncertainty, QBC, deepen).
- Permits / encryption / S-63.
- Claiming ECDIS type approval or SOLAS carriage.
- Extending uncited IEC 61174 clauses beyond the S-164 4.4.1 reference.

---

## 8. Docs sync (this commit)

- `docs/testing/iho-ctest-case-ids.md` — `IHO-S164-POWERUP` Active, rev1 locks (no “Pending clauses”).
- `docs/api-contracts.md` — power-up pointer updated to rev1 (Block scope, exit codes, PU table).
- This file — **rev1 / Planner checklist frozen**; Open section cleared.
- Install-prefix follow-on (later commit / sibling case): see `docs/powerup-install-prefix-contract.md` — **do not** edit PU table here for that work.

---

## 9. Open items

**None.** Planner BRIEF + steering (2026-10-18) froze PU-1…PU-6, Block scope, CLI exits, PU-2 file list, PU-4 mechanism, PU-6 stub, CI-synthetic-only TDS, and producer-code exclusion.

---

## 10. Engineer / Tester hold

**Engineer:** implement `run_powerup_self_check`, CLI/viewer process-entry hooks, synthetic fixtures, and CTest `powerup_self_check` from this **rev1**.

**Tester:** lock the six opens below; no-SKIP dual platform.

### Tester six opens (locked)

1. **Block scope:** forbids exchange-set import and viewer operational chart open; allows standalone `open_dataset` / diagnostics / **UNVERIFIED** (SIG D7). Hook may run before first operational open; CLI/viewer process entry **MUST** run power-up.
2. **CLI exits:** overall Block → exit **2** + print failures; Warn-only → exit **0**, warns on **stderr**; Pass → **0**. Exit **1** = load failures only.
3. **PU-2 list:** §3.1 (S-101/102/111/124 FC+PC, S-104 FC only / no S-104 PC; sha in provenance CTest only).
4. **PU-4:** mini gold `verify_exchange_set`; thin `security_self_test()` wrap OK; no second crypto.
5. **PU-6:** WARN stub in **first** Eng slice (not deferred); CLI N/A → Pass.
6. **CI:** synthetic fixtures only; official PowerUp TDS local optional, **not exercised**.
