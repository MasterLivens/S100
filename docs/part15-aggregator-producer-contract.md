# Part 15 / S-98 aggregator-signed producer policy (rev 1) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S164-SIG-AGGREGATOR`** (**Active / Open**). CTest: **`exchange_signature_aggregator`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev 1** — Planner freeze (MUST below). ST=`DATA_AGGREGATOR` permanent exempt + optional WARN slice after closed **`IHO-S164-SIG-CN-SHAPE`**. Does **not** reopen CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE; does **not** mix/jump Closed **base SIG** crypto; does **not** jump **`S102-GRID-SHIM-RETIRE`**. Does **not** expand tip-truth oracles / add Block match / invent SSE / emit 109 on AGG path.

Frozen with: `docs/api-contracts.md` (Part 15 aggregator), `docs/part15-signature-contract.md` (base SIG — **do not** reopen), `docs/part15-producer-code-contract.md` (PRODUCER Closed — DATA_PRODUCER path — **do not** reopen), `docs/part15-cn-shape-contract.md` (CN-SHAPE Closed — OfficialClass — **do not** reopen), `docs/powerup-self-check-contract.md` (POWERUP Closed — **do not** reopen), `docs/testing/iho-ctest-case-ids.md`. Draw order → **DISPLAY-PRIORITY** already Closed — **do not** reopen. Scale filter → **SCAMIN** already Closed — **do not** reopen. Unified pick → **SESSION-PICK** already Closed — **do not** reopen. Vendor / sha → **CATALOGUE** already Closed — **do not** reopen. Scrubber → **TIMELINE** already Closed — **do not** reopen. Wiring → **PC-WIRE** already Closed — **do not** reopen. Install-tree → **INSTALL-PREFIX** already Closed — **do not** reopen. Producer-code Enforce match → **PRODUCER** already Closed — **do not** reopen. CN MRN→OfficialClass → **CN-SHAPE** already Closed — **do not** reopen. UI styling / S-128 packs / permits / invent new SSE / Block member/aggregator/either match / change `exchange_signature` / `exchange_signature_producer` / `exchange_signature_cn_shape` Ok = Non-goals.

Primary research: `/workspace/research/aggregator-producer/BRIEF.md` (2026-10-25 tip-truth refresh). Former Eng gap **already fixed** @ Eng **`29be187`** (permanent ST=`DATA_AGGREGATOR` exempt + optional WARN `aggregator_producer_code_divergent` + fixture `aggregator_exempt/` + key + CTest).

**rev1 field freeze (kept — Planner Open O1–O6 closed / policy-exempt + WARN):** ST=`DATA_AGGREGATOR` → **permanent exempt** from PRODUCER CN seg3↔`producerCode` Block; DATA_PRODUCER path **unchanged**; missing `producerCode` → N/A-pass; optional WARN reason **`aggregator_producer_code_divergent`** when producerCode present ≠ CN seg3 (**no** skip / **no** SSE 109 / no new SSE); leaf `certificateRef` only (no whole-chain member scan); ST=`AGGREGATOR` (S-128) **out of slice**; fixture `aggregator_exempt/` (AGGREGATOR ST + member producerCode ≠ CN → Enforce **still installs**); reuse PRODUCER/CN-SHAPE helpers; hook `verify_exchange_set` only; MUST NOT break `exchange_signature` / `exchange_signature_producer` / `exchange_signature_cn_shape` Ok; MUST NOT SKIP; Linux+Win.

**Status: Active / Open (2026-10-25)** — after **CN-SHAPE Closed** (tip `ae89f29` ≡ Arch `656db7b`; tip Close CI [`40181637772`](https://github.com/MasterLivens/S100/actions/runs/40181637772) dual-green; Open-bind evidence CI [`40181021692`](https://github.com/MasterLivens/S100/actions/runs/40181021692) @ `7793a63`≡`c799da5`, CTest `#20 exchange_signature_aggregator` Passed, O1=tip-truth/docs-close). Eng tip ancestor **`29be187`** (permanent DATA_AGGREGATOR exempt + optional WARN + fixture/CTest) already on tip — **not greenfield**. Eng green CI [`39115171877`](https://github.com/MasterLivens/S100/actions/runs/39115171877) @ `29be187` (native `s100_tests` + signature fixtures — **not** `viewer_*` — **no Win O1 (a)/(b)**). Ubuntu+Windows already Passed — tip Close CI [`40181637772`](https://github.com/MasterLivens/S100/actions/runs/40181637772) `#20 exchange_signature_aggregator` + Open-bind [`40181021692`](https://github.com/MasterLivens/S100/actions/runs/40181021692) `#20` + Eng landing [`39115171877`](https://github.com/MasterLivens/S100/actions/runs/39115171877). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE / base SIG. Do **not** invent case ids or next tip names; freeze tip-truth below (**keep** AGGREGATOR oracles — **do not** expand). Do **not** reopen CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE. Do **not** mix/jump Closed **base SIG** crypto. Do **not** invent `AGG-*`/`SIGAGG-*` failure-face / skip prefixes (failure face = bare SCHECK / existing reason codes). Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). Contract display header stays **rev 1** (do **not** bump just for Open). After Closed next: **TBD** (architecture / registered Active only — do **not** invent).

**Planner freeze (MUST — from BRIEF §7 + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-S164-SIG-AGGREGATOR`** / CTest **`exchange_signature_aggregator`**; **forbidden** to invent a second case/test name.
- MUST tip-truth = permanent ST=`DATA_AGGREGATOR` exempt + DATA_PRODUCER unchanged + missing→N/A-pass + optional WARN `aggregator_producer_code_divergent` (no skip / no 109) + leaf-only + fixture `aggregator_exempt/` (install + no 109 + no `producer_code_mismatch` + WARN when divergent) + hook `verify_exchange_set` Enforce only + **MUST NOT** break `exchange_signature` / `exchange_signature_producer` / `exchange_signature_cn_shape` Ok + CTest **MUST NOT SKIP**; Linux+Win.
- Eng tip already has **`29be187`**; Ubuntu+Win (tip `40181637772` `#20` / Open-bind `40181021692` `#20` / Eng `39115171877`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield. **Keep** tip-truth oracles — **do not** expand scope / add Block match / invent SSE / emit 109 on AGG path / reopen CN-SHAPE/PRODUCER.
- CN MRN→OfficialClass → **`IHO-S164-SIG-CN-SHAPE`** already Closed — **do not** reopen.
- Producer-code Enforce match → **`IHO-S164-SIG-PRODUCER`** already Closed — **do not** reopen.
- Power-up self-check → **`IHO-S164-POWERUP`** already Closed — **do not** reopen.
- Install-tree root → **`IHO-S164-POWERUP-INSTALL-PREFIX`** already Closed — **do not** reopen.
- Draw order → **`IHO-DISPLAY-PRIORITY`** already Closed — **do not** reopen.
- Scale filter → **`IHO-SCAMIN-FILTER`** already Closed — **do not** reopen.
- Unified pick → **`IHO-SESSION-PICK-UNIFIED`** already Closed — **do not** reopen.
- Vendor / sha → **`IHO-CATALOGUE-COMPLETE`** already Closed — **do not** reopen.
- Scrubber → **`IHO-TIMELINE-UI`** already Closed — **do not** reopen.
- Wiring → **`IHO-PC-WIRE-111-124`** already Closed — **do not** reopen.
- Base Part 15 crypto → **`IHO-S164-SIG`** — **do not** reopen / mix / Closed this tip.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog** (DEEPEN O1); **not** this tip.
- Failure face = bare SCHECK / existing reason codes; do **not** invent `AGG-*`/`SIGAGG-*` skip / failure-face prefixes.
- Ubuntu + Windows: `exchange_signature_aggregator` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE; do **not** mix/jump Closed base SIG.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`; native `s100_tests` + signature fixtures (`aggregator_exempt/`); no SKIP. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `exchange_signature_aggregator` MUST NOT SKIP, Passed (already tip CI `40181637772` `#20` / Open-bind `40181021692` `#20` / Eng `39115171877`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng diff required
- After Close next: **TBD**

**Eng notes:** verify `29be187` on tip; API/path in `s100_security` (`verify.cpp` permanent DATA_AGGREGATOR exempt + optional WARN `aggregator_producer_code_divergent`); driver `tests/test_exchange_signature_aggregator.cpp`; cmake `exchange_signature_aggregator`; fixture `testdata/s101/signatures/aggregator_exempt`; key `test_ds_aggregator.crt`. CN-SHAPE Closed already on this stack (`ae89f29`). Do **not** reopen CN-SHAPE / PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE. Do **not** mix/Closed base SIG crypto. UI / S-128 / permits / invent SSE / Block match **Non-goals** — do **not** harden. Do **not** invent `AGG-*`/`SIGAGG-*` failure prefixes. Do **not** change tip-truth oracles / add Block match / emit 109 on AGG path.

**O-questions (BRIEF §3 Open 1–N — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `29be187` already; no `ci.yml` / no large Eng) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` — **yes / freeze**.
3. Contract head → Open; keep display **rev 1** (do **not** bump just for Open) — **yes / freeze**.
4. case-ids Active → Open (Status stays Active family) — **yes / freeze**.
5. Notes ≠ CN-SHAPE (already Closed; do **not** reopen); ≠ PRODUCER (already Closed; do **not** reopen); ≠ POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE (already Closed; do **not** reopen); ≠ S-124 / S-111 / S-104 open; ≠ base SIG crypto reopen; do **not** reopen QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE; shim-retire stays Backlog; UI / S-128 / permits / invent SSE / Block match stay Non-goals; **keep** tip-truth oracles (do **not** expand) — **yes / freeze**.
6. Closed next → **TBD** (architecture / registered Active; do **not** invent) — **yes / freeze**.
7. No other cases this tip (do **not** Closed base SIG / etc.) — **yes / freeze**.
8. architecture.md flip **next** to this Open — **yes / freeze**.
9. `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog — **yes / freeze**.
10. Failure face = bare SCHECK / existing reason codes; do **not** invent `AGG-*`/`SIGAGG-*` skip / failure-face prefixes — **yes / freeze**.
11. AGGREGATOR exempt+WARN = this tip; CN MRN→OfficialClass = CN-SHAPE Closed (do **not** reopen); producer-code match = PRODUCER Closed (do **not** reopen); base crypto = SIG (do **not** reopen) — **yes / freeze**.

---

## 1. Goal

Under **`VerifyPolicy::Enforce`**, when the authenticating certificate has **ST=`DATA_AGGREGATOR`**:

1. **Permanently exempt** that resource from the PRODUCER CN seg3 ↔ CATALOG `producerCode` **Block** check (chain / expiry / signature value / OfficialClass still apply per SIG + CN-SHAPE).
2. **DATA_PRODUCER** path **unchanged** (PRODUCER rev1 match / N/A-pass / mismatch→skip+`producer_code_mismatch`/109).
3. Optional **WARN** when `producerCode` is present and upper ≠ CN producer segment — reason **`aggregator_producer_code_divergent`**; **no** skip, **no** SSE 109.
4. Prove exempt with a synthetic positive CTest that **MUST NOT SKIP**.

Logic stays **only** in `s100_security` on the existing `verify_exchange_set` Enforce path (same hook family as PRODUCER).

---

## 2. Spec anchors (frozen)

| Id | Topic | Spec / project hook | Notes |
|----|-------|---------------------|-------|
| **AG-C1** | Aggregator role | S-98 Ed 2.0.0 §**9.1.1** ST=`DATA_AGGREGATOR` — RENCs/Aggregators validate, distribute, and (sometimes) digitally sign data **on behalf of** their members; do not create content but may sign as official | CN carries **signer (RENC)** identity, not content HO |
| **AG-C2** | CN match sentence | Same §9.1.1: CN producer integer+alpha in MRN of **at least one** authenticating certificate must match dataset / CATALOG | Literal CN↔member would false-kill single-cert PRIMAR packs; this case **does not** use AGGREGATOR CN to satisfy that Block |
| **AG-C3** | CHS/PRIMAR reality | Research: Subject `ST=DATA_AGGREGATOR`, CN=`urn:mrn:iho:00PM:…`; **no** `producerCode` in CATALOG; member `CA00` / CHS only in fileName / producingAgency | S-164 has **no** aggregator sample; CI uses synthetic fixtures only |
| **AG-C4** | PRODUCER exempt elevate | `docs/part15-producer-code-contract.md` §3.1 #5: ST∈`{DATA_AGGREGATOR}` → exempt | This case makes exempt **permanent policy** (MUST), not temporary Backlog matching |
| **AG-C5** | CN shape | `docs/part15-cn-shape-contract.md` | **Reuse**; aggregator CN still needs conforming MRN for OfficialClass; **do not** reopen |
| **AG-C6** | ST → OfficialClass | SIG **A9**: `DATA_AGGREGATOR` remains official | Unchanged; this slice is Enforce **exempt + optional WARN**, not demotion |
| **AG-C7** | Catalogue field | Part 17 `producerCode` | Same discovery anchor as PRODUCER when present; absence → N/A-pass |

---

## 3. Policy (frozen MUST)

### 3.1 Permanent AGGREGATOR exempt (AG-1)

1. If the authenticating leaf cert (via `certificateRef` → catalogue cert — **same leaf scope as PRODUCER**; do **not** scan the whole chain for a member `DATA_PRODUCER` cert) has **ST=`DATA_AGGREGATOR`** (case per existing `classify_st`):
   - **MUST NOT** run the PRODUCER CN seg3 ↔ `producerCode` Block compare.
   - Crypto / chain / expiry / OfficialClass (ST + CN-SHAPE) still apply.
   - Resource **installs** when crypto Ok (subject to other SIG gates).
2. This exempt is **permanent contract policy** — not “this slice only”, not deferred matching, not temporary until IHO clarifies.
3. **DROP** as Block rules (never implement):
   - Force CN seg3 == member `producerCode`
   - Force CN seg3 == aggregator code / `producerCode` must be aggregator code
   - “Either” (CN==producerCode **OR** member bypass) as a **Block** rule

### 3.2 DATA_PRODUCER unchanged (AG-2)

ST=`DATA_PRODUCER` (and any non-`DATA_AGGREGATOR` path that PRODUCER already covers): **unchanged** — match / N/A-pass / mismatch→skip + reason `producer_code_mismatch` + project-map SSE **109**.

### 3.3 ST=`AGGREGATOR` (S-128 service role) (AG-3)

**Out of this slice.** Do not change classify / match for ST=`AGGREGATOR` (distinct from `DATA_AGGREGATOR`). No sample in CHS COMBINED used that ST for the PRIMAR DS cert.

### 3.4 Missing `producerCode` (AG-O3 closed)

When ST=`DATA_AGGREGATOR` and `producerCode` is **absent** → **N/A-pass** for this gate (same as PRODUCER §3.4). Install when crypto Ok. Matches CHS COMBINED reality.

### 3.5 Multi-cert / “at least one” (AG-O4 closed)

- Authenticating cert for this gate = **leaf** referenced by the resource’s `certificateRef` (PRODUCER lean) — **not** a whole-chain scan for a member HO cert.
- ST=`DATA_AGGREGATOR` path **does not** use CN to satisfy the §9.1.1 “at least one” **Block** literal.
- If a future pack has **both** a member `DATA_PRODUCER` and a RENC `DATA_AGGREGATOR` signature on the same resource: apply PRODUCER match only to the `DATA_PRODUCER` authenticating cert; AGGREGATOR authenticating cert remains exempt. (Per-cert ST branch; no new algorithm this slice beyond documenting the branch.)

### 3.6 Optional WARN (AG-W1)

When ST=`DATA_AGGREGATOR` **and** `producerCode` is **present** **and** `upper(producerCode) ≠ upper(CN seg3)`:

| Field | Value |
|-------|-------|
| Log reason | **`aggregator_producer_code_divergent`** |
| Skip? | **No** |
| SSE? | **None** (not 109, not a new App. B code) |
| Install? | **Yes** (crypto Ok) |

Engineer **SHOULD** emit this WARN (MUST-log preferred; Tester may assert presence when easy). Absence of the WARN string must **not** cause CTest SKIP.

### 3.7 No new SSE (AG-5)

**MUST NOT** invent a new S-98 / project SSE for aggregator divergence. **MUST NOT** emit 109 on the AGGREGATOR path for CN≠`producerCode`.

---

## 4. Truth table (frozen)

| Authenticating leaf ST | `producerCode` | CN seg3 vs producerCode | Enforce result (this gate) | Reason / SSE |
|------------------------|----------------|-------------------------|----------------------------|--------------|
| `DATA_AGGREGATOR` | absent | — | **Install** (N/A-pass) | — |
| `DATA_AGGREGATOR` | present, == CN seg3 | equal | **Install** | — (optional no WARN) |
| `DATA_AGGREGATOR` | present, ≠ CN seg3 | divergent | **Install** + optional WARN | `aggregator_producer_code_divergent`; **no** skip; **no** 109 |
| `DATA_PRODUCER` | (any) | per PRODUCER rev1 | Unchanged | `producer_code_mismatch` / 109 on mismatch |

---

## 5. Open O1–O6 — closed

| Id | Question (rev0) | Freeze (rev1) |
|----|-----------------|---------------|
| **O1** Match truth | Member / aggregator / either Block? | **No Block match.** Permanent **exempt**. DROP member / aggregator / either-as-Block |
| **O2** Failure SSE | skip + 109 / new reason? | **No** mismatch skip/109 this slice. Optional WARN `aggregator_producer_code_divergent` only |
| **O3** Missing producerCode | N/A-pass vs skip? | **N/A-pass** (same as PRODUCER; CHS reality) |
| **O4** Multi-cert “at least one” | Which certs? | Leaf `certificateRef` only; AGGREGATOR path does **not** use CN for Block “at least one”; no whole-chain member-cert scan |
| **O5** PRODUCER exempt | Remove / narrow? | **Keep and elevate to permanent policy** (MUST this contract); DATA_PRODUCER unchanged |
| **O6** Fixtures | match/mismatch trees? | Synthetic **`aggregator_exempt/`** positive (install); optional second missing-`producerCode` also installs; **no** aggregator-mismatch→skip negative |

---

## 6. Fixtures & CTest (frozen)

| Item | Requirement |
|------|-------------|
| CTest | **`exchange_signature_aggregator` MUST NOT SKIP** (Linux + Windows; no platform guard) |
| Primary positive | `testdata/s101/signatures/aggregator_exempt/` — ST=`DATA_AGGREGATOR`, CN seg3 like `00PM` (or TEST-ONLY `00AG`), CATALOG `producerCode` = **member** code (e.g. `CA00` / `99MB`) **≠** CN → under Enforce **still installs** (proves exempt) |
| Optional positive | Same ST + **absent** `producerCode` → also installs |
| Negative | **Not required** this slice (aggregator + divergent → skip would contradict AG-1) |
| Keys | TEST-ONLY substring (SIG A4); signed bytes `-text` |
| Real CHS | **Not** in CI; optional local WARN / `chs_exercised("signatures.aggregator")` only — never CTest SKIP |
| Old trees | `producer_*` / `positive/` / `cn_shape_*` **unchanged** |
| Generator | Extend `tools/gen_signature_fixtures.py` for ST=`DATA_AGGREGATOR` + independent CN / `producerCode` |

**MUST NOT** break closed Ok on `exchange_signature` / `exchange_signature_producer` / `exchange_signature_cn_shape`.

---

## 7. Non-goals

- Block: CN(aggregator) **must** equal member `producerCode` (kills PRIMAR).
- Block: CN **must** equal `producerCode` with forced aggregator code in CATALOG.
- “Either” as Block rule.
- Require a second member `DATA_PRODUCER` cert in the exchange set.
- fileName CCCC fallback (PRODUCER already OFF).
- Reopen PRODUCER 1:1 / SSE 109 mapping; reopen CN-SHAPE grammar; POWERUP; UI OfficialClass styling.
- New S-98 App. B SSE; emit 109 on AGGREGATOR divergent WARN path.
- ST=`AGGREGATOR` (S-128) classify change.
- Real PRIMAR / HO private keys in product or committed fixtures.
- QBC (still Backlog). Install-prefix → **`IHO-S164-POWERUP-INSTALL-PREFIX`** already **Closed** — **do not** reopen (`docs/powerup-install-prefix-contract.md`).

---

## 8. Docs sync (this Open tip)

- `docs/testing/iho-ctest-case-ids.md` — case **`IHO-S164-SIG-AGGREGATOR`** **Active / Open** (Eng `29be187` tip-truth; clear stale «queue next INSTALL-PREFIX»).
- `docs/architecture.md` — **next** → this Open; security row AGGREGATOR **Active / Open**.
- Contract display header stays **rev 1** (do **not** bump just for Open). Body §1–§7 tip-truth frozen — **keep**; do **not** expand oracles / add Block match / invent SSE.
- Cross-ref only this tip; **no** engine / CMake / CI change.

---

## 9. Engineer / Tester

**Engineer:** tip already has `29be187` (permanent DATA_AGGREGATOR exempt + optional WARN + fixture `aggregator_exempt/` + CTest). This Open tip = **docs close / verify primarily** — **no** large Eng / **no** `ci.yml`. Keep PRODUCER DATA_PRODUCER path untouched; **no** new SSE; **no** aggregator mismatch→skip; **MUST NOT** break `exchange_signature` / `exchange_signature_producer` / `exchange_signature_cn_shape` Ok.

**Tester:** gate on truth table §4; CTest MUST NOT SKIP; assert install when AGGREGATOR ST + member `producerCode` ≠ CN; assert **no** 109 / **no** skip on that path; WARN reason optional assert; no real PRIMAR keys. Close binds Ubuntu+Win Passed.
