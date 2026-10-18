# Part 15 / S-98 producer-code matching (rev0 draft)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S164-SIG-PRODUCER`** (**Active**, draft). CTest: **`exchange_signature_producer`** — **MUST NOT SKIP**, no platform guard, Linux + Windows CI.

**Status: rev0 / rev1 draft — pending Planner lock of field set + SSE.** Docs only in this Architect commit — **no** engine / fixture / CMake / CI change here. Engineer holds implement until Planner closes §9 Open items (or Architect publishes a frozen rev1).

Primary sources: `/workspace/research/part15/BRIEF.md` (S-98 §9.1.1 CN MRN quote), `docs/part15-signature-contract.md` (**D8** backlog → this case), S-100 Part 17 discovery (`producingAgency` / `producerCode`), existing `s100_security` (`OfficialClass`, cert ST/CN, `sse_map`, `verify_exchange_set`). Power-up contract explicitly deferred producer-code here (`docs/powerup-self-check-contract.md`).

---

## 1. Goal

Under **`VerifyPolicy::Enforce`**, after cryptographic signature / chain checks succeed for a resource, match the **data-server (leaf) certificate identity** against what the **dataset / CATALOG.XML entry declares as producer**. On mismatch: **reject or skip that resource with a mapped SSE** — prefer **not** silent accept. Logic lives **only** in `s100_security`; exchange / session call the existing verify interface (no parallel stack).

This is the frozen next SIG slice after **`IHO-S164-POWERUP`**.

---

## 2. Spec anchors

| Id | Topic | Spec / project hook | Notes |
|----|-------|---------------------|-------|
| **PC-C1** | CN MRN ↔ producer | S-98 Ed 2.0.0 §**9.1.1**: CN = IHO data producer **integer** and **alpha** as colon-separated MRN (e.g. `urn:mrn:iho:aa00:01810`, `urn:mrn:iho:GB00:00540`); for digitally signed datasets, those elements in **at least one** authenticating certificate **must match** those in the **dataset and its CATALOG.XML entry** | BRIEF §1.4 verbatim intent |
| **PC-C2** | Catalogue producer fields | Part 17 `S100_DatasetDiscoveryMetadata`: **`producingAgency`** (1, CI_Organisation) + optional **`producerCode`** (0..1, “official IHO Producer Code from S-62”) | XSD / Table 17-*; CHS COMBINED has org **name** text, often **no** `producerCode` |
| **PC-C3** | Dataset-side codes | S-101 DSID **AGEN** (b12 agency); cell / `fileName` / DSNM naming often embeds producer alpha (e.g. `CA00`, fixture `AA00`) | Exact parse rules **Open** (§9) |
| **PC-C4** | Official vs non-official | Existing SIG **D8 / A9**: ST → `OfficialClass`; CN shape / OTHER_DATA_PRODUCER → NonOfficial | **Unchanged.** Producer-code mismatch is an **Enforce hard fail** (skip/reject), **not** a demotion to NonOfficial |
| **PC-C5** | Failure UX / ops | S-98 §**20.4.3 c** + SIG Enforce: rejected/skipped content must not apply; log every rejection | Align with catalogue / per-file SSE effects already in SIG |

---

## 3. Proposed match rule (Architect — **Proposed**, not final MUST)

Until Planner locks §9, Engineer treats the following as the **working proposal**:

### 3.1 Certificate side (leaf signing cert of the resource)

1. Read X.509 subject **CN** (already available in verify path alongside ST).
2. Parse IHO producer MRN: case-insensitive prefix `urn:mrn:iho:` then **`<alpha>:<integer>`** (BRIEF / S-98 examples). Reject parse → treat as **non-conforming CN** for this check (see §3.4).
3. **Do not** use **O** (Organisation) text as the primary match key (CHS: org name ≠ MRN codes; PRIMAR `O=PRIMAR` vs CN `00PM:02020`).
4. **ST** continues to drive `OfficialClass` only (A9). ST does not substitute for producer-code match.

### 3.2 Dataset / catalogue side (declared producer)

**Proposed primary (when present):** discovery metadata **`producerCode`** string on the resource’s `S100_DatasetDiscoveryMetadata` row (Part 17 / XSD).

**Proposed secondary / fallbacks (Open — Planner picks subset):**

| Candidate | Evidence | Risk |
|-----------|----------|------|
| `producingAgency` → `CI_Organisation.name` | Present on real CHS packs | Human name; **poor** direct MRN match |
| S-101 cell `fileName` / DSNM stem producer alpha (e.g. chars after product digits) | Common encoding; fixtures use `101AA00…` | Product-specific; S-102/104/111 paths differ |
| DSID **AGEN** (ISO 8211) | S-101 PS | Requires reading cell bytes inside security or a narrow core helper; prefer catalogue fields first |
| Certificate `id` attribute when it is itself an MRN | Fixture / CHS `certificate id="urn:mrn:iho:…"` | Usually equals CN; still cert-side |

**Proposed comparison:** normalize alpha (case-insensitive) + integer (leading zeros significant as in MRN examples `01810` / `00540` / `02020` — **Open** whether to strip leading zeros). Match if cert MRN **(alpha, integer)** equals the declared pair from catalogue/dataset for that resource. S-98 “at least one of the authenticating certificates” → **Proposed:** leaf DS cert used for `EVP_DigestVerify` on that resource (chain intermediates / SA not required to carry the producer MRN).

### 3.3 When declared producer fields are absent

Existing SIG **positive** fixtures (`testdata/s101/signatures/positive/`) have **no** `producerCode` / `producingAgency`. **MUST NOT** break closed Ok assertions on `exchange_signature`.

**Proposed (compat):** if this slice cannot extract a declared producer code for the resource, **do not fail** producer-code matching (check N/A → continue Ok). Negative fixtures **must** declare producer codes (and mismatched cert CN) explicitly. Planner may overrule to “missing declared code → skip” later — that would require regenerating positive fixtures with matching `producerCode` without changing Ok crypto assertions.

### 3.4 Relation to OfficialClass / official vs non-official

| Situation | OfficialClass | Enforce install |
|-----------|---------------|-----------------|
| ST official + CN MRN matches declared producer | Official (unchanged A9) | Install (if crypto Ok) |
| ST OTHER_DATA_PRODUCER / bad CN shape | NonOfficial (A9) | Still subject to producer match **if** declared codes present; mismatch → skip/reject (**not** “install as NonOfficial”) |
| Crypto Ok + producer mismatch | Unchanged classification on report row optional | **Skip/reject** (§4) |
| Crypto Ok + no declared producer (compat) | Per A9 | Install (producer check N/A) |

---

## 4. Enforce behaviour (Tester gate)

| Gate | Requirement |
|------|-------------|
| **Hook** | Inside existing **`verify_exchange_set`** (and thus `open_exchange_set` / `open_cell_series` under **Enforce**). **No** separate parallel verifier, CLI flag, or power-up-only path. |
| **When** | After per-resource crypto / chain steps succeed (SIG A7 steps 9–16 family); before appending path to `installed`. Whole-set phase (1–8) unchanged. |
| **Policy Off** | Producer-code matching **not** applied (same as other SIG Enforce gates). Existing tests that pass `Off` stay green. |
| **Mismatch → ops** | **Do not** silently accept. Prefer **per-file skip** (109-class): resource in `skipped`, not installed; other resources unaffected; session otherwise proceeds. |
| **Catalogue-level producer wrong** | If Planner agrees the **exchange / CATALOG.XML signer** identity fails a catalogue-declared producer check → **whole-set reject** (`RejectedExchange`), session unchanged (SIG strong guarantee). Default Architect lean if unclear: **per-file skip** unless the failure is on CATALOG.XML itself. |
| **Nav / operational use** | Skipped / rejected resources must not appear as loaded chart members under Enforce (align SIG). Standalone `open_dataset` remains **UNVERIFIED** (D7) — no producer gate. |

---

## 5. SSE mapping (**Proposed** — Open)

S-98 App. B has **no** dedicated “producer code mismatch” SSE in BRIEF. Single-map rule (SIG **D5**) still applies: new internal enum value → **one** `sse_map` entry; no SSE integers outside that TU.

| Internal (Proposed) | Proposed SSE | Effect | Status |
|---------------------|--------------|--------|--------|
| `ProducerCodeMismatch` (dataset / support row) | **109** | Skip that resource | **Proposed** — Planner may pick 126 / 132 / other |
| `ProducerCodeMismatch` on CATALOG.XML / exchange signer | **134** or **126** | Whole set rejected | **Open** |
| Message text | Log explicit “producer code mismatch (cert CN MRN vs declared)” + sse_message | Required | Proposed |

Do **not** overload `IssuerNotSa` (126) without Planner ack — chain-to-SA and producer-identity are different defects.

---

## 6. Module / API

- **All logic in `s100_security`** (parse CN MRN, read discovery producer fields the security module already re-reads or minimally extends, compare, set `SigRejectCode`, map SSE).
- **`s100_core` / exchange** only call `verify_exchange_set` / honour `VerificationReport` skipped list — **no** OpenSSL, **no** duplicate match.
- Additive API only if needed (e.g. `SigRejectCode::ProducerCodeMismatch`; optional fields on `FileVerification` for cert_mrn / declared_code — nice-to-have, not required for rev0).
- **MUST NOT** change assertions or Ok outcomes of closed positive path in CTest **`exchange_signature`**. New negatives + new CTest **`exchange_signature_producer`**.

---

## 7. Fixtures · CTest · platforms

| Item | Requirement |
|------|-------------|
| CTest | **`exchange_signature_producer`** — **MUST NOT SKIP**; Linux + Windows |
| Synthetic fixtures | Required under e.g. `testdata/s101/signatures/producer/` (or sibling); generator may extend `tools/gen_signature_fixtures.py`; signed bytes **`-text`** in `.gitattributes` (SIG A4) |
| Positive control | Matching CN MRN ↔ declared `producerCode` (and/or Planner-locked fields) → Ok + install |
| Negative | Mismatched cert CN vs declared producer → mapped SSE + skip/reject per §4; multi-resource set where only mismatched row is skipped |
| Closed SIG Ok | **`exchange_signature`** Ok cases stay green; **no** assertion edits on that positive path |
| Keys | TEST-ONLY substring in private-key filenames (SIG A4) |
| Dual platform | No `#ifdef` skip; OpenSSL pin unchanged (A6) |

---

## 8. Tester gates (rev1 checklist — explicit)

1. **Check list (what is compared)** — document and assert: cert leaf **CN MRN (alpha + integer)** vs Planner-locked declared fields (Proposed: discovery **`producerCode`** first). Record N/A path when declared fields absent (compat).
2. **After failure** — under Enforce, mismatched resource is **skipped or whole-set rejected** with mapped SSE; **not** silently installed; other members follow SIG per-file vs whole-set rules.
3. **Hook** — producer-code runs on existing SIG / catalogue load path (`verify_exchange_set` → `open_exchange_set` Enforce); **no** parallel stack.
4. **Named CTest** — `exchange_signature_producer` **MUST NOT SKIP** on Linux + Windows.
5. **Open** — field set + exact SSE remain Planner-owned before any row is called final MUST (§9).

---

## 9. Open items (Planner)

1. **Exact cert fields:** confirm CN MRN only vs also require O / certificate `id` equality.
2. **Exact dataset / CATALOG fields:** `producerCode` only vs + `producingAgency` name heuristics vs S-101 AGEN / fileName alpha. CHS reality: name present, `producerCode` often absent, cert is PRIMAR `00PM:02020` while paths use `CA00`.
3. **DATA_AGGREGATOR** signing member HO data: does match require aggregator MRN, member MRN, or either (“at least one”)?
4. **Exact SSE** on mismatch (Proposed 109 per-file; catalogue-level 134 vs 126).
5. **Missing declared producer under Enforce:** N/A-pass (Proposed compat) vs skip vs require fixture regeneration for positive SIG trees.
6. **Integer normalization:** keep leading zeros or numeric equality.
7. **Support files / FC / PC:** same producer match as datasets, or FC/PC remain SA-direct-only (SSE 129) without producer-code?

Until these close, this contract stays **rev0/rev1 draft**; Engineer may scaffold fixtures/tests behind the Proposed rule but must not claim Planner-frozen MUST for field/SSE rows.

---

## 10. Non-goals

- Reimplement SIG crypto / chain / CATALOG.SIGN (closed `IHO-S164-SIG`).
- POWERUP changes (`IHO-S164-POWERUP`); producer-code stays out of power-up.
- S-102-specific producer UX; EncDotNet; full S-164 TDS ingest.
- Permits / encryption / S-63.
- Official/non-official **display styling** (still A9 exposure only).
- Changing Ok assertions on `exchange_signature` positive path.

---

## 11. Docs sync (this commit)

- `docs/part15-signature-contract.md` — one-line note: D8 producer-code matching → **`IHO-S164-SIG-PRODUCER`** (this file).
- `docs/testing/iho-ctest-case-ids.md` — case **Active** (draft).
- `docs/api-contracts.md` — pointer + Enforce hook note.
- `docs/architecture.md` — security row mentions producer-code follow-on.
- `docs/powerup-self-check-contract.md` — already excludes producer-code; no behaviour change.

---

## 12. Engineer / Tester hold

**Engineer:** wait for Planner field/SSE lock **or** implement strictly behind §3 Proposed + §9 Open labels; add synthetic negatives; new CTest name above; do not touch closed Ok assertions.

**Tester:** gate on §8; treat field/SSE rows as draft until Planner ack.
