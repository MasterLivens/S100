# Part 15 / S-98 producer-code matching (rev1)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S164-SIG-PRODUCER`** (**Active**). CTest: **`exchange_signature_producer`** — **MUST NOT SKIP**, no platform guard, Linux + Windows CI.

**Status: rev1 — field/SSE frozen (Planner materials + Planner/Tester SSE 109 final; + CN-unparseable skip clarification).** Docs only in this Architect commit — **no** engine / fixture / CMake / CI change here. Engineer may implement after this rev1.

Primary sources: `/workspace/research/part15/BRIEF.md` (S-98 §9.1.1 CN MRN quote), `docs/part15-signature-contract.md` (**D8** → this case), S-100 Part 17 discovery (`producerCode`), existing `s100_security` (`OfficialClass`, cert ST/CN, `sse_map`, `verify_exchange_set`). Power-up contract explicitly deferred producer-code here (`docs/powerup-self-check-contract.md`).

**rev1 changes (from draft 2ed3a63):** freeze CN producer segment ↔ CATALOG `producerCode` match; O out; ST unchanged; missing `producerCode` → N/A-pass; mismatch → per-dataset skip + reason `producer_code_mismatch` (primary) + project-map SSE **109** (secondary; not App. B literal; not 126); AGGREGATOR ST → **exempt** this slice (PRIMAR); aggregator↔member HO matching → Backlog; fileName fallback **OFF**; compare exact upper (no zero-strip); segment 4 never compared.

---

## 1. Goal

Under **`VerifyPolicy::Enforce`**, after cryptographic signature / chain checks succeed for a resource, match the **authenticating certificate CN producer segment** against the **CATALOG / dataset discovery `producerCode`**. On mismatch: **skip that dataset** (others continue); report reason **`producer_code_mismatch`**; map to **project SSE 109**. Logic lives **only** in `s100_security`; exchange / session call the existing verify interface (no parallel stack).

This is the frozen next SIG slice after **`IHO-S164-POWERUP`**.

---

## 2. Spec anchors (frozen)

| Id | Topic | Spec / project hook | Notes |
|----|-------|---------------------|-------|
| **PC-C1** | CN MRN ↔ producer | S-98 Ed 2.0.0 §**9.1.1**: CN = `urn:mrn:iho:<producer>:<id>` (e.g. `urn:mrn:iho:aa00:01810`, `urn:mrn:iho:GB00:00540`); for digitally signed datasets, those elements in **at least one** authenticating certificate **must match** those in the **dataset and its CATALOG.XML entry** | BRIEF §1.4; “at least one” cited for DATA_PRODUCER 1:1 this slice |
| **PC-C2** | Catalogue producer field | Part 17 `S100_DatasetDiscoveryMetadata.producerCode` (0..1, official IHO GI Producer Code / S-62 family; also S-101 metadata) | **Normative dataset-side anchor.** `producingAgency` / org **name** do **not** participate |
| **PC-C3** | Filename-embedded code | e.g. `101AA00DS0008.000` → `AA00` | **OFF as a check this slice** (old `signatures/positive` is `101AA00…` vs CN `99ZZ` by design). May appear in construction notes only; never overrides / substitutes `producerCode` |
| **PC-C4** | Official vs non-official | Existing SIG **D8 / A9** + sibling **`IHO-S164-SIG-CN-SHAPE`** (`docs/part15-cn-shape-contract.md`): ST + CN shape → `OfficialClass`; malformed / non-conforming CN → NonOfficial (9.1.1) | **Unchanged by this PRODUCER contract.** Producer-code mismatch is an Enforce **skip**, **not** a demotion to NonOfficial and **not** a new SSE for malformed CN. CN-SHAPE does **not** add skip either |
| **PC-C5** | Failure UX / ops | S-98 §**20.4.3 c** + SIG Enforce: skipped content must not apply; log every rejection | Align per-file skip effects already in SIG |

---

## 3. Match rule (frozen MUST)

### 3.1 Certificate side

1. Read X.509 subject **CN** only (already on the verify path alongside ST). **O / certificate id / MRN segment 4** never compared to `producerCode`.
2. Parse IHO MRN: case-insensitive prefix `urn:mrn:iho:` (or `urn:mrn:iho:org:` if present) then take the **producer token** (segment after `iho:` / `iho:org:` — the whole third namespace segment, e.g. `aa00` in `urn:mrn:iho:aa00:01810`).
3. **Producer segment** = that token as one piece (alpha + integer together: `aa00`, `GB00`, `99ZZ`). **Segment 4** (`01810` / `90001`) is the certificate instance id — **MUST NOT** be compared to `producerCode`.
4. **O** (Organisation) does **not** participate.
5. **ST** continues to drive `OfficialClass` only (SIG A9). Additionally: if ST ∈ `{DATA_AGGREGATOR}` → **exempt** producer-code match this slice (chain + OfficialClass only) — avoids PRIMAR `00PM` signing member HO false-kills. Fixtures remain **DATA_PRODUCER 1:1** only; matching aggregator CN vs member HO is **Backlog**.
6. **Malformed / non-conforming CN** → existing **NonOfficial** path (S-98 9.1.1 / SIG A9); **separate** from missing-`producerCode` N/A-pass. **Do not** invent a new SSE from this slice for malformed CN.
7. S-98 “at least one of the authenticating certificates”: for this slice’s **DATA_PRODUCER 1:1** fixtures, the leaf data-server cert used for `EVP_DigestVerify` on that resource supplies the CN.
8. **CN parse failure + `producerCode` present** (Tester 2026-10-18): under **Enforce**, **skip that file** (cannot prove must-match — same install effect as mismatch). **MUST NOT** emit reason `producer_code_mismatch` or project-map SSE **109** (109 reserved for parse-Ok but token ≠ `producerCode`). NonOfficial may coexist via A9. If CN parse fails and `producerCode` is **absent** → this check is N/A-pass (NonOfficial still per A9).

### 3.2 Dataset / CATALOG side (normative)

- **MUST** use discovery metadata **`S100_DatasetDiscoveryMetadata.producerCode`** on the resource’s CATALOG.XML entry (Part 17 / S-101 metadata; IHO GI Producer Code).
- Normalize that string **upper** before compare.
- Filename-embedded producer alpha (e.g. `101AA00…` → `AA00`) may appear in **fixture construction notes** only. **fileName CCCC fallback as a check is OFF** this slice (would redden old `signatures/positive`). Must not override / substitute `producerCode`.

### 3.3 Equality rule

At least one authenticating cert’s CN **producer segment** (upper) **MUST equal** that dataset’s **and** its CATALOG.XML entry **`producerCode`** (upper).

- Compare: **upper**, then **exact string equality**.
- **MUST NOT** strip leading zeros (`AA00` ≠ `AA0`).
- **MUST NOT** treat `00AA` ≡ `AA00` this slice.
- **FC / PC** out of scope (SA direct / SSE 129 already exist). Datasets only (+ support later / Open).

### 3.4 When `producerCode` is absent (frozen)

| Situation | Producer-code check | Enforce install (crypto Ok) |
|-----------|---------------------|-----------------------------|
| CATALOG / discovery **lacks** `producerCode` | **N/A-pass** (check not applied) | Install — keeps closed positive SIG path green |
| `producerCode` **present** | Must match CN producer segment | Mismatch → skip (§4) |
| `producerCode` **present** + CN **unparseable** | Cannot prove must-match | **Skip** that file (§3.1 #8); **no** `producer_code_mismatch` / **no** 109 |

**MUST NOT** break closed Ok assertions on CTest **`exchange_signature`**. Existing positive SIG fixtures without `producerCode` stay Ok via N/A-pass.

### 3.5 Relation to OfficialClass

| Situation | OfficialClass | Enforce install |
|-----------|---------------|-----------------|
| ST official + CN producer matches `producerCode` | Official (unchanged A9) | Install (if crypto Ok) |
| Malformed / non-conforming CN | NonOfficial (A9) | No new SSE from this slice |
| Crypto Ok + `producerCode` present + mismatch | Classification unchanged on report row (optional) | **Skip** that dataset (§4) |
| Crypto Ok + no `producerCode` (N/A-pass) | Per A9 | Install |

---

## 4. Enforce behaviour (frozen)

| Gate | Requirement |
|------|-------------|
| **Hook** | Inside existing **`verify_exchange_set`** (and thus `open_exchange_set` / `open_cell_series` under **Enforce**). **No** separate parallel verifier, CLI flag, or power-up-only path. |
| **When** | **After** per-resource crypto / chain steps succeed (SIG A7 steps 9–16 family); before appending path to `installed`. Whole-set phase (1–8) unchanged. |
| **Policy Off** | Producer-code matching **not** applied (same as other SIG Enforce gates). Existing tests that pass `Off` stay green. |
| **Mismatch → ops** | **Skip that dataset**; others continue. Resource in `skipped`, not installed. Report reason **`producer_code_mismatch`**. |
| **Nav / operational use** | Skipped resources must not appear as loaded chart members under Enforce (align SIG). Standalone `open_dataset` remains **UNVERIFIED** (D7) — no producer gate. |

---

## 5. SSE mapping (frozen — Planner+Tester: project-map **109**, not 126)

S-98 App. B has **no** dedicated “producer code mismatch” SSE. Single-map rule (SIG **D5**) still applies: new internal enum value → **one** `sse_map` entry; no SSE integers outside that TU. **Do not invent a new SSE number.** Planner withdrew any 126 lean — **use 109 only**.

| Internal | SSE | Effect | Note |
|----------|-----|--------|------|
| `ProducerCodeMismatch` (dataset / support row) | **109** | Skip that resource | **Project mapping** — **not** an S-98 App. B literal for this case. Contract / `sse_map` note required. |
| Malformed / unparseable CN + `producerCode` present | *(no 109)* | **Skip** that file under Enforce | NonOfficial via A9 may coexist; **not** `producer_code_mismatch` / **not** 109 |
| Malformed CN (general) | existing NonOfficial path | No new SSE from this slice | SIG A9 / 9.1.1 |
| Message / reason | Primary report reason **`producer_code_mismatch`** (cert CN producer segment vs CATALOG `producerCode`) + `sse_message` | Required | CTest asserts **reason first**, SSE **109** secondary |

Do **not** overload `IssuerNotSa` (**126**) — chain-to-SA and producer-identity are different defects; Planner+Tester final is **109**.

---

## 6. Module / API

- **All logic in `s100_security`** (parse CN producer segment, read discovery `producerCode`, compare upper, set reject/skip reason, map SSE).
- **`s100_core` / exchange** only call `verify_exchange_set` / honour `VerificationReport` skipped list — **no** OpenSSL, **no** duplicate match.
- Additive API as needed (e.g. `SigRejectCode::ProducerCodeMismatch`; optional fields on `FileVerification` for cert producer / declared `producerCode` — nice-to-have).
- **MUST NOT** change assertions or Ok outcomes of closed positive path in CTest **`exchange_signature`**. New negatives + new CTest **`exchange_signature_producer`**.

---

## 7. Fixtures · CTest · platforms

| Item | Requirement |
|------|-------------|
| CTest | **`exchange_signature_producer`** — **MUST NOT SKIP**; Linux + Windows |
| Synthetic fixtures | New trees e.g. `testdata/s101/signatures/producer_match/` + `producer_mismatch/`; must set `producerCode` and **re-sign**; generator may extend `tools/gen_signature_fixtures.py`; signed bytes **`-text`** in `.gitattributes` (SIG A4) |
| Scope this slice | **DATA_PRODUCER 1:1** only; **DATA_AGGREGATOR** certs **exempt** from this check |
| Positive | CN producer **`99ZZ`** = CATALOG `producerCode` **`99ZZ`** → Ok + install |
| Negative | CATALOG `producerCode` **`00AA`**, cert still **`99ZZ`** → that cell **skipped** (`producer_code_mismatch` / SSE 109); other members unaffected |
| Old `signatures/positive/` | **Exempt** (N/A-pass / no producerCode) — do not assert producer match there |
| Filename in fixtures | Construction convenience only; **no** fileName fallback check |
| Closed SIG Ok | **`exchange_signature`** Ok cases stay green (N/A-pass when no `producerCode`); **no** assertion edits on that positive path |
| Keys | TEST-ONLY substring in private-key filenames (SIG A4) |
| Dual platform | No `#ifdef` skip; OpenSSL pin unchanged (A6) |

---

## 8. Tester gates (rev1 checklist)

1. **Check list** — cert leaf CN **producer segment** (MRN segment 3 only, upper; **not** segment 4) vs CATALOG **`producerCode`** (upper, exact; no zero-strip). N/A-pass when `producerCode` absent. ST=`DATA_AGGREGATOR` → exempt. CN unparseable + `producerCode` present → Enforce **skip** that file, **no** 109.
2. **After failure** — under Enforce, mismatched resource is **skipped** with reason `producer_code_mismatch` and project-map SSE **109**; **not** silently installed; other members continue.
3. **Hook** — producer-code runs on existing SIG / catalogue load path (`verify_exchange_set` → `open_exchange_set` Enforce); **no** parallel stack.
4. **Named CTest** — `exchange_signature_producer` **MUST NOT SKIP** on Linux + Windows.
5. **SSE / reason** — CTest asserts reason **`producer_code_mismatch` first**, project-map SSE **109** secondary (not an S-98 App. B literal; not 126).

---

## 9. Open / Backlog

| Item | Status |
|------|--------|
| Project-map SSE **109** for `producer_code_mismatch` | **Closed** — Planner+Tester aligned (project mapping, not App. B literal; **not** 126). Reason `producer_code_mismatch` primary; CTest asserts reason first, 109 secondary |
| **DATA_AGGREGATOR** ST → exempt producer match | **Closed this slice** (chain + OfficialClass only; avoids PRIMAR false-kill) |
| Aggregator CN ↔ member HO producer matching | **Backlog** — cite S-98 9.1.1 “at least one”; fixtures stay DATA_PRODUCER 1:1 |
| Field set / missing-`producerCode` / O / ST / Enforce skip | **Closed** (this rev1) |

---

## 10. Non-goals

- Reimplement SIG crypto / chain / CATALOG.SIGN (closed `IHO-S164-SIG`).
- POWERUP changes (`IHO-S164-POWERUP`); producer-code stays out of power-up.
- UI / display styling for official vs non-official.
- S-128 multi-producer packs.
- Permits / encryption / S-63.
- Aggregator CN ↔ member HO producer matching (Backlog). ST=`DATA_AGGREGATOR` exempt is in-scope MUST.
- Changing Ok assertions on `exchange_signature` positive path.
- Inventing a new S-98 SSE number for producer mismatch.

---

## 11. Docs sync (this commit)

- `docs/part15-signature-contract.md` — D8 → **`IHO-S164-SIG-PRODUCER`** rev1 (field/SSE locked).
- `docs/testing/iho-ctest-case-ids.md` — case **Active** rev1 summary.
- `docs/api-contracts.md` — pointer + Enforce hook note (rev1).
- `docs/architecture.md` — security row producer-code follow-on Active.
- `docs/powerup-self-check-contract.md` — already excludes producer-code; no behaviour change.

---

## 12. Engineer / Tester

**Engineer:** implement §3–§7 MUST; `producer_match` / `producer_mismatch` trees with `producerCode` + re-sign; CTest `exchange_signature_producer`; ST=`DATA_AGGREGATOR` exempt; do not touch closed Ok / old `positive/`; leave aggregator↔member matching for Backlog.

**Tester:** gate on §8; assert reason `producer_code_mismatch` first, SSE 109 secondary.
