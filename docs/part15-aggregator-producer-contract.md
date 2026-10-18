# Part 15 / S-98 aggregator-signed producer policy (rev1)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S164-SIG-AGGREGATOR`** (**Active**). CTest: **`exchange_signature_aggregator`** — **MUST NOT SKIP**, no platform guard, Linux + Windows CI.

**Status: rev1 — policy contract frozen (Planner Open O1–O6 closed).** Docs only in this Architect commit — **no** engine / fixture / CMake / CI change here. Engineer may implement after this rev1.

Takes the **Aggregator CN ↔ member HO producer matching** Backlog deferred from **`IHO-S164-SIG-PRODUCER`** and **closes it as permanent exempt policy** — **not** as a Block match rule. Under PRODUCER, ST=`DATA_AGGREGATOR` was already exempt “this slice”; this case **elevates** that exempt to a lasting contract MUST so RENC-signed member HO packs (CHS/PRIMAR: CN=`00PM`, member `CA00` in fileName/agency, often **no** `producerCode`) are **never** false-killed.

**Do not** reopen PRODUCER / CN-SHAPE / POWERUP — **reuse** their CN parsers, ST→OfficialClass rules, SSE map TU, and Enforce hook. **No** new S-98 SSE.

Primary sources: S-98 Ed 2.0.0 §9.1.1 (ST=`DATA_AGGREGATOR` “on behalf of members” + CN match sentence); `/workspace/research/aggregator-producer/BRIEF.md` (Planner freeze correction); CHS/PRIMAR COMBINED sample (ST=`DATA_AGGREGATOR`, CN `urn:mrn:iho:00PM:…`, `producerCode` absent, member `CA00` in fileName); `docs/part15-producer-code-contract.md`; `docs/part15-cn-shape-contract.md`; SIG A9 ST.

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
- QBC / install-prefix (queue: **next after this = install-prefix**).

---

## 8. Docs sync (this commit)

- `docs/testing/iho-ctest-case-ids.md` — case **`IHO-S164-SIG-AGGREGATOR`** **Active** rev1 (policy-exempt + WARN).
- `docs/api-contracts.md` — pointer: permanent exempt + optional WARN; not member match.
- `docs/part15-producer-code-contract.md` — AGGREGATOR exempt → **permanent** (closed by this case); Backlog matching row → **Closed as permanent exempt**.
- `docs/architecture.md` — security row aggregator **rev1**.
- `docs/part15-signature-contract.md` / `docs/part15-cn-shape-contract.md` — cross-ref rev1 policy-exempt.
- Cross-ref only; no engine / CMake / CI.

---

## 9. Engineer / Tester

**Engineer:** implement §3–§6; keep PRODUCER DATA_PRODUCER path untouched; elevate ST=`DATA_AGGREGATOR` exempt to permanent (shared helper ok); optional WARN `aggregator_producer_code_divergent`; fixture `aggregator_exempt/` + CTest `exchange_signature_aggregator`; **no** new SSE; **no** aggregator mismatch→skip.

**Tester:** gate on truth table §4; CTest MUST NOT SKIP; assert install when AGGREGATOR ST + member `producerCode` ≠ CN; assert **no** 109 / **no** skip on that path; WARN reason optional assert; no real PRIMAR keys.
