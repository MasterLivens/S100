# S-164 / S-98 power-up / start-up self-check (rev0 skeleton)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer (later). Test: SoftwareTester (later).
Case id: **`IHO-S164-POWERUP`** (**Active** — Pending clauses). CTest: **`powerup_self_check`** — **MUST NOT SKIP**, no platform guard, Linux + Windows CI.

**Status: rev0 skeleton / pending Planner checklist.** Docs only in this Architect commit — **no** engine / fixture / CMake / CI change here. Clause numbers and MUST vs Proposed severity stay **Open** until Research&Planner delivers a frozen self-check checklist (S-98 / IEC 61174 Ed.5 / S-164 instruction-manual mapping).

Primary research already on disk (Architect read, not a Planner freeze):

| Source | What it gives this case |
|--------|-------------------------|
| S-164 Ed 1.0.0 §**2.1.1** Initial Catalogues (`PowerUpCatalogues`) | Catalogue load / version persistence — certification pack entry, **not** a full self-test list |
| S-164 Ed 1.0.0 §**2.2.1** Preparation and Power Up (`InitialPowerUp`); §**2.2.2** `PowerUp` | IEC **61174 / 4.4.1** dataset load + System Database indication |
| S-164 2.0.1 draft §2.2.1 `InitialPowerUp` | Same family; cites S-98 **6.1.1** + IEC 61174 **4.4.1** |
| `docs/certification/ecdis-certified-system.md` §9 | System integrity: software identity / version for survey |
| `docs/certification/iec61174-requirements-matrix.md` | Theme row “Software identity / cyber” → module `integrity` (Track B) |
| `docs/part15-signature-contract.md` (`IHO-S164-SIG`) | SA trust store, `s100_security`, `open_exchange_set` under `VerifyPolicy::Enforce` |

**Explicit exclusion:** SIG **producer-code matching** (CN MRN ↔ dataset / CATALOG producer; S-98 §9.1.1 style) is **out of this contract** and stays a **separate later slice**. Do not draft producer-code rules, oracles, or fixtures here.

## 1. Goal

On every **application start** (CLI process entry, viewer process entry, and a callable library hook used by both), run a **power-up / start-up self-check** that:

1. Proves the process knows who it is (software identity / version).
2. Proves critical on-disk / embedded assets needed for chart use are present.
3. Proves the Part 15 trust path and signature module are alive (call into **`s100_security`**, do not reimplement SIG).
4. Optionally smoke-tests catalogue / exchange-set open under **Enforce** (synthetic fixture).
5. Surfaces a structured report so ChartSession / CLI / viewer can **block operational use** or **warn-and-continue** per policy below.

This is Track B certification scaffolding. It does **not** claim type approval or SOLAS fitness.

## 2. Spec anchors — clause numbers **Open**

| Id | Topic | Spec hook (provisional) | Status |
|----|-------|-------------------------|--------|
| **PU-C1** | Self-check trigger at power-up / app start | IEC 61174 **4.4.1** family; S-164 §2.2.1 / 2.0.1 §2.2.1 | **Open** — exact shall-text pending Planner |
| **PU-C2** | Initial catalogues / catalogue presence | S-164 §**2.1.1** `InitialCatalogues` / `PowerUpCatalogues`; S-98 Annex C **C-21.1** (as cited by S-164) | **Open** |
| **PU-C3** | Data integrity / authentication readiness | S-98 §20.4.3 c / App. B; S-100 Part 15 (via existing SIG contract) | **Open** for which subset runs at start vs only at import |
| **PU-C4** | Software identity / version for survey | MSC / class cyber theme; IEC matrix “Software identity / cyber” | **Open** |
| **PU-C5** | Display / HMI readiness | IEC 62288 / 61174 display themes | **Open** — likely WARN-only in headless CLI |

Until Planner freezes the table, Engineer must not treat Proposed rows as MUST.

## 3. Power-up checklist (rev0 proposals)

Each row: **Proposed** = Architect proposal for Planner review; **MUST** = only after Planner ack (none promoted yet). Severity column is the *intended* failure class once frozen.

| # | Check | Intent | Class | Severity (intent) | Notes |
|---|-------|--------|-------|-------------------|-------|
| **PU-1** | Software identity / version string available and non-empty | Survey / audit can read build id | **Proposed** | Block | Exact string source (CMake `PROJECT_VERSION`, git describe, `S100_VERSION` macro) **Open** |
| **PU-2** | Critical files / embedded assets present | FC/PC pin paths (or build-time embeds) resolvable; default catalogue roots exist | **Proposed** | Block | Align with catalogue-complete pins; exact file list **Open** |
| **PU-3** | SA trust store present | `TrustStore::builtin_iho_52()` (or product-installed SA) loads ≥ 1 trust anchor | **Proposed** | Block | Mirrors Part 15 “SA installed separately”; empty store ↔ product cannot Enforce-import (SSE **105** family) |
| **PU-4** | Signature module self-test | `s100_security` can verify a **tiny synthetic** known-good signed blob / mini exchange under test trust (or a dedicated `security_self_test()` if Eng adds one) | **Proposed** | Block | **Call into** existing verify path — **do not** reimplement SIG / ECDSA |
| **PU-5** | Catalogue / exchange load-path smoke | `ChartSession::open_exchange_set(synthetic, …, VerifyPolicy::Enforce)` succeeds (or returns a structured verify+parse ok) on a committed mini fixture | **Proposed** | Block (product) / see §4 | Reuses exchange + SIG; fixture under `testdata/…/powerup/` (Eng) |
| **PU-6** | Display / basic I/O | Viewer: primary surface / offscreen paint path constructs without fatal error; CLI: N/A or no-op pass | **Proposed** | **Warn** (viewer) / N/A (CLI) | Not EncDotNet / pixel compare |

**Label legend:** every row above is **Proposed** until Planner marks it MUST. Do not add producer-code, permit, or S-102-specific rows in this slice.

## 4. Failure policy

### 4.1 Block vs warn (rev0 proposal — **Open**)

| Outcome | Meaning | When (proposal) |
|---------|---------|-----------------|
| **Block** | Process must **not** enter navigation / operational chart use | Any **Block**-class check fails (PU-1…PU-5 once MUST) |
| **Warn-and-continue** | Operational use allowed; user/operator sees a persistent warning | PU-6 display issues; future non-critical catalogue version notices **Open** |
| **Pass** | All Block checks ok; Warn list may be empty | Normal start |

**Open for Planner:** whether a failed PU-5 (exchange smoke) blocks **all** CLI modes (including standalone `open_dataset` / `UNVERIFIED`) or only exchange-set / “operational” modes. Architect lean: **block exchange-set and viewer chart use**; allow explicit diagnostic flags later — not decided here.

### 4.2 Surface in ChartSession / CLI / viewer (rev0)

```cpp
// Sketch only — names Open until Eng + Planner ack
namespace s100 {

enum class PowerUpSeverity { Pass, Warn, Block };

struct PowerUpItem {
  std::string id;       // "PU-1" …
  PowerUpSeverity severity{PowerUpSeverity::Pass};
  std::string message;
};

struct PowerUpReport {
  PowerUpSeverity overall{PowerUpSeverity::Pass};  // max of items
  std::string software_identity;                   // PU-1
  std::vector<PowerUpItem> items;
  // Optional: last VerificationReport from PU-4/PU-5 smoke (do not fork SIG types)
};

/** Run once at process start (or lazy before first operational open). Idempotent. */
PowerUpReport run_powerup_self_check(/* paths / TrustStore injectables Open */);

}  // namespace s100
```

| Surface | Behaviour (proposal) |
|---------|----------------------|
| **Library** | `run_powerup_self_check()` returns `PowerUpReport`. `ChartSession` may stash `last_powerup()`; **Open** whether `open_exchange_set` refuses when overall==Block |
| **CLI** | On start (before load): print `powerup status=PASS|WARN|BLOCK` + per-item lines. **Exit code:** `0` = Pass or Warn; **non-zero** (propose `2`) = Block. Load paths that would navigate still refuse when Block. Existing load failure exit `1` unchanged |
| **Viewer** | Status bar / modal: `POWERUP BLOCK: …` prevents chart open; `POWERUP WARN: …` allows open with banner. No automation beyond status text in this slice |

Exact exit-code integers and whether Warn is exit 0 are **Open** (Tester oracle later).

## 5. Connection to signature verification and catalogue / exchange load

**Do not reimplement SIG.** This contract **calls** the Active Part 15 surface:

| Power-up need | Existing hook | Policy |
|---------------|---------------|--------|
| Trust store present (PU-3) | `s100::security::TrustStore::builtin_iho_52()` / `add_user_sa` / `for_tests` | Same types as `docs/part15-signature-contract.md` |
| Signature self-test (PU-4) | `verify_exchange_set` or a thin `security` self-test that uses the same crypto path | Test trust + injectable `verify_time` (never wall clock) |
| Exchange smoke (PU-5) | `ChartSession::open_exchange_set(path, err, VerifyPolicy::Enforce)` | Product default **Enforce**; synthetic fixture may use `TrustStore::for_tests` **only inside the self-check / CTest**, not as a product escape hatch |
| Catalogue presence (PU-2 / PU-5) | Existing catalogue load / provenance pins; exchange catalogue parse inside `open_exchange_set` | No second catalogue parser |

Standalone `open_dataset` remains **`UNVERIFIED`** per SIG D7 — power-up does not change that. Power-up **readiness** ≠ per-dataset authentication of every future open.

**Producer-code matching:** **out of scope** (separate later slice). Official/non-official classification from SIG may appear in the smoke report if already returned by `VerificationReport`; do not add CN/MRN matching here.

## 6. CTest · fixtures · platforms

| Item | Requirement |
|------|-------------|
| CTest name | **`powerup_self_check`** |
| SKIP | **Forbidden** — no `SKIP_RETURN_VALUE`, no platform `#ifdef` skip |
| Platforms | **Linux + Windows** CI both green |
| Fixtures | **Synthetic** under e.g. `testdata/s101/powerup/` (Eng): mini signed exchange for PU-4/PU-5; missing-trust / corrupt-sig negatives as needed. Generator may reuse `tools/gen_signature_fixtures.py` patterns; signed bytes **`-text`** in `.gitattributes` (same rule as SIG A4) |
| Oracles | **Open** until Planner checklist freeze — Tester writes MUST rows then |
| Relation to S-164 TDS | Case id cites **2.1.1** Track B pack; this CTest is the **self-check harness**, not a full `PowerUpCatalogues` / `InitialPowerUp` screenshot script (those remain later certification packs) |

## 7. Non-goals

- **SIG producer-code matching** (CN MRN ↔ dataset / CATALOG producer) — separate later slice.
- **S-102** product work (uncertainty, QBC, deepen, overlays) — no S-102-specific power-up rows.
- **EncDotNet / NIWC / pixel compare** — display check is construct/smoke only, not visual oracles (`IHO-S164-DISPLAY-COMPARE` / EncDotNet stay elsewhere).
- Full S-164 `PowerUp` / `InitialPowerUp` own-ship / multi-product screenshot batteries.
- Permits / encryption / S-63.
- Claiming ECDIS type approval or SOLAS carriage.

## 8. Docs sync (this commit)

- `docs/testing/iho-ctest-case-ids.md` — move `IHO-S164-POWERUP` from Backlog → **Active** (Pending clauses OK).
- `docs/api-contracts.md` — short pointer to this contract.
- This file — **rev0 skeleton / pending Planner checklist**.

## 9. Open items for Planner

1. Freeze the checklist: promote which of PU-1…PU-6 are **MUST** vs drop/defer; cite exact S-98 / IEC 61174 Ed.5 / S-164 IM clause IDs (replace **Open** anchors PU-C1…PU-C5).
2. Decide Block scope: does Block refuse **all** chart opens, or only exchange-set / “operational” modes? CLI exit code for Block vs Warn.
3. Exact critical-file list for PU-2 (FC/PC pins, embeds, colour tables, …).
4. Whether PU-4 is a dedicated `security_self_test()` API or solely `verify_exchange_set` on a gold mini fixture.
5. Whether viewer PU-6 is in-scope for first Eng slice or deferred (Architect lean: WARN stub OK in rev1 Eng).
6. Mapping to S-164 TDS exchange names (`PowerUpCatalogues`, `InitialPowerUp`, `PowerUp`) — which, if any, become optional local WARN fixtures vs CI synthetic-only.
7. Confirm producer-code matching remains a **later** SIG follow-on (Architect assumes yes; this contract must stay clean of it).

## 10. Engineer / Tester hold

Do **not** implement engine, fixtures, or CTest from this rev0 alone. Wait for Planner checklist ack → Architect rev1 (MUST rows + oracles) → then Eng.
