# Part 15 / S-98 producer-code matching (rev 1) (Closed)

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/api-contracts.md` (Part 15 producer-code) / `docs/part15-signature-contract.md` (**D8** → this case; base SIG — **do not** reopen) / `docs/part15-cn-shape-contract.md` (Active; CN MRN→OfficialClass — **do not** mix/Closed this tip) / `docs/part15-aggregator-producer-contract.md` (Active; AGGREGATOR exempt+WARN — **do not** mix/Closed this tip; shared ST=`DATA_AGGREGATOR` permanent exempt MUST kept) / `docs/powerup-self-check-contract.md` (POWERUP Closed — **do not** reopen) / `docs/testing/iho-ctest-case-ids.md` / `docs/powerup-install-prefix-contract.md` (Closed; install-tree → INSTALL-PREFIX separate) / `docs/display-priority-contract.md` (Closed; draw order → DISPLAY-PRIORITY separate) / `docs/scamin-contract.md` (Closed; scale filter → SCAMIN separate) / `docs/session-pick-contract.md` (Closed; unified pick → SESSION-PICK separate) / `docs/catalogue-complete-contract.md` (Closed; vendor/sha → CATALOGUE separate) / `docs/timeline-ui-contract.md` (Closed; scrubber → TIMELINE separate) / `docs/pc-wire-contract.md` (Closed; wiring → PC-WIRE separate). Case id: **`IHO-S164-SIG-PRODUCER`** (**Closed / Done** 2026-10-25).  
CTest name: **`exchange_signature_producer`** — MUST NOT SKIP; Linux + Windows dual-green (CI `40179378032` @ `4431d13`). Supersedes Open draft on tip `4431d13` (≡ Arch `9aa7f34`).

Primary research: `/workspace/research/producer-code/BRIEF.md` (2026-10-25 tip-truth refresh). Former Eng gap **already fixed** @ Eng **`0a7fe5f`** (match/mismatch) + **`89e31cd`** (CN-unparseable). Power-up self-check → **POWERUP** already Closed — **do not** reopen. Install-tree root → **INSTALL-PREFIX** already Closed — **do not** reopen. Draw order → **DISPLAY-PRIORITY** already Closed — **do not** reopen. Scale filter → **SCAMIN** already Closed — **do not** reopen. Unified pick → **SESSION-PICK** already Closed — **do not** reopen. Vendor / sha → **CATALOGUE** already Closed — **do not** reopen. Scrubber → **TIMELINE** already Closed — **do not** reopen. Wiring → **PC-WIRE** already Closed — **do not** reopen. CN MRN→OfficialClass → **CN-SHAPE** separate Active — **do not** mix/Closed this tip. AGGREGATOR exempt+WARN → **AGGREGATOR** separate Active — **do not** mix/Closed this tip (shared exempt MUST kept). Base Part 15 crypto → **SIG** — **do not** reopen. UI styling / S-128 packs / permits / invent new SSE / change `exchange_signature` Ok = Non-goals.

**rev1 field/SSE freeze (kept — from draft 2ed3a63):** freeze CN producer segment ↔ CATALOG `producerCode` match; O out; ST unchanged; missing `producerCode` → N/A-pass; mismatch → per-dataset skip + reason `producer_code_mismatch` (primary) + project-map SSE **109** (secondary; not App. B literal; not 126); AGGREGATOR ST → **permanent exempt** (elevated by `IHO-S164-SIG-AGGREGATOR` rev1 — **do not** Closed AGGREGATOR this tip); aggregator↔member Block matching → **Closed as permanent exempt**; fileName fallback **OFF**; compare exact upper (no zero-strip); segment 4 never compared. CN-unparseable skip clarification kept.

**Status: Closed / Done (2026-10-25)** — CI [`40179378032`](https://github.com/MasterLivens/S100/actions/runs/40179378032) @ `4431d13`≡`9aa7f34` dual-green; **`exchange_signature_producer` Passed Ubuntu+Win** (native `s100_tests` + signature fixtures — **not** `viewer_*`; **no Win O1 (a)/(b)**). Eng **`0a7fe5f`/`89e31cd`** already on tip — **docs close only**. Eng green [`39111414989`](https://github.com/MasterLivens/S100/actions/runs/39111414989) @ `89e31cd`. MUST tip-truth **unchanged** (CN **seg3** upper exact vs CATALOG `producerCode`; **seg4 never**; O out; no zero-strip; missing→N/A-pass; mismatch→skip/`producer_code_mismatch`/SSE **109**; CN-unparseable→skip/`producer_cn_unparseable`/no 109; ST=`DATA_AGGREGATOR` permanent exempt shared; fileName **OFF**; fixtures `producer_match`/`mismatch`/`cn_unparseable` + old `positive/` N/A-pass; **MUST NOT** break `exchange_signature` Ok; fixtures **MUST NOT SKIP**). **Keep** tip-truth oracles — **do not** expand. **O1=tip-truth / docs-close SUCCESS**. Failure face = existing `producer_code_mismatch` / `producer_cn_unparseable` / SSE map (do **not** invent `PRODUCER-*` / `SIGPROD-*`). Do **not** reopen POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE; do **not** mix/jump Closed CN-SHAPE / AGGREGATOR / base SIG crypto; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog. Contract display stays **rev 1**. Next: **TBD**.

**Planner freeze (kept — rev 1 tip-truth):** Reuse case id **`IHO-S164-SIG-PRODUCER`** / CTest **`exchange_signature_producer`**; MUST tip-truth = CN **seg3** upper exact vs CATALOG `producerCode` (**seg4 never**; O out; no zero-strip; no `00AA`≡`AA00`); missing `producerCode` → N/A-pass; mismatch → skip + reason **`producer_code_mismatch`** (primary) + project-map SSE **109** (secondary; not App.B; not 126); CN unparseable + producerCode present → Enforce skip / reason `producer_cn_unparseable` / **no** 109; ST=`DATA_AGGREGATOR` permanent exempt (shared AGGREGATOR rev1 MUST — **do not** Closed AGGREGATOR this tip); fileName fallback **OFF**; hook `verify_exchange_set` only; **MUST NOT** break `exchange_signature` Ok; fixtures `producer_match`/`producer_mismatch`/`producer_cn_unparseable` + old `positive/` N/A-pass; CTest **MUST NOT SKIP**; Linux+Win; **keep** tip-truth oracles (do **not** expand scope / SSE / fileName / zero-strip / seg4); POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE already Closed (do **not** reopen); CN-SHAPE / AGGREGATOR separate Active (do **not** mix/Closed); base SIG — **do not** reopen; Eng `0a7fe5f`/`89e31cd` already; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog; failure face = existing `producer_code_mismatch` / `producer_cn_unparseable` / SSE map (do **not** invent `PRODUCER-*` / `SIGPROD-*` prefixes / new CTest); after Closed next **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE; do **not** mix/jump Closed CN-SHAPE / AGGREGATOR / base SIG.

**O1 closed as tip-truth / docs-close:** Eng `0a7fe5f`/`89e31cd` already on tip (`verify.cpp` CN seg3 vs CATALOG `producerCode`; `SigRejectCode::ProducerCodeMismatch` → SSE 109; CN-unparseable skip; driver `tests/test_exchange_signature_producer.cpp`; cmake `exchange_signature_producer`; fixtures `testdata/s101/signatures/producer_*`). Slice = Arch docs close only — not greenfield Eng. Do **not** invent case ids or next tip names; do **not** reopen POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE; do **not** mix/Closed CN-SHAPE / AGGREGATOR / base SIG; do **not** invent `PRODUCER-*` / `SIGPROD-*` prefixes; do **not** retire `grid()` this tip; do **not** expand tip-truth oracles; do **not** harden UI / S-128 / permits / invent SSE. **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest; inventing oracles / next tip id; inventing `PRODUCER-*` / `SIGPROD-*` failure-face prefixes.

---

## Slice close — 2026-10-25

**Status: Closed / Done** (formal docs close — **not** a producer-code / Eng rewrite).

- Evidence: CI [40179378032](https://github.com/MasterLivens/S100/actions/runs/40179378032) tip `4431d13`≡`9aa7f34` — Ubuntu + Windows **`exchange_signature_producer` Passed** (MUST NOT SKIP). Ancestor greens also Eng landing [`39111414989`](https://github.com/MasterLivens/S100/actions/runs/39111414989) @ `89e31cd` + prior tip `#18 exchange_signature_producer` Passed (POWERUP Close CI [`40178466855`](https://github.com/MasterLivens/S100/actions/runs/40178466855) @ `b37312e` / Open-bind [`40177947235`](https://github.com/MasterLivens/S100/actions/runs/40177947235)).
- **O1=tip-truth / docs-close SUCCESS** — Eng `0a7fe5f`/`89e31cd` already on tip; Arch Open→Closed only; Planner BRIEF §7 answers frozen.
- MUST tip-truth unchanged (CN seg3 match + N/A-pass + mismatch skip+109 + cn_unparseable skip + AGGREGATOR exempt shared + fileName OFF + fixtures MUST NOT SKIP). **Keep** tip-truth oracles — **do not** expand.
- Eng `0a7fe5f`/`89e31cd` already on tip — docs close only; no `ci.yml` change; **no Win (a)/(b)**; no large Eng.
- Failure face stays `producer_code_mismatch` / `producer_cn_unparseable` / SSE map — do **not** invent `PRODUCER-*` / `SIGPROD-*`.
- **Notes:** next **TBD** (architecture / registered Active — do **not** invent a next tip id). **Backlog:** `grid()` shim retire (`S102-GRID-SHIM-RETIRE`) — do not insert ahead. Contract display stays **rev 1**. POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE already Closed (do **not** reopen); CN-SHAPE / AGGREGATOR separate Active (do **not** mix/Closed); base SIG — **do not** reopen.
- Boundaries: independent; ≠ POWERUP (already Closed); ≠ INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE (already Closed); ≠ CN-SHAPE / AGGREGATOR / base SIG (do **not** mix/Closed / reopen); do **not** reopen POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE.
- **Non-goals this close:** inventing oracles / next tip id; retiring `grid()` shim; producer-code/Eng rewrite; expanding tip-truth oracles; reopening POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE; mixing/Closed CN-SHAPE / AGGREGATOR / base SIG crypto; Win (a)/(b) (N/A — non-`viewer_*`); inventing `PRODUCER-*` / `SIGPROD-*` failure-face prefixes; hardening UI / S-128 / permits / invent SSE.

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
5. **ST** continues to drive `OfficialClass` only (SIG A9). Additionally: if ST ∈ `{DATA_AGGREGATOR}` → **permanent exempt** from producer-code Block match (chain + OfficialClass only) — policy elevated by **`IHO-S164-SIG-AGGREGATOR`** rev1 (`docs/part15-aggregator-producer-contract.md`); avoids PRIMAR `00PM` signing member HO false-kills. Fixtures remain **DATA_PRODUCER 1:1** only. Optional WARN on divergent `producerCode` → that case (reason `aggregator_producer_code_divergent`; no skip / no 109).
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
| Scope this slice | **DATA_PRODUCER 1:1** only; **DATA_AGGREGATOR** certs **permanently exempt** from this Block (AGGREGATOR rev1) |
| Positive | CN producer **`99ZZ`** = CATALOG `producerCode` **`99ZZ`** → Ok + install |
| Negative | CATALOG `producerCode` **`00AA`**, cert still **`99ZZ`** → that cell **skipped** (`producer_code_mismatch` / SSE 109); other members unaffected |
| Old `signatures/positive/` | **Exempt** (N/A-pass / no producerCode) — do not assert producer match there |
| Filename in fixtures | Construction convenience only; **no** fileName fallback check |
| Closed SIG Ok | **`exchange_signature`** Ok cases stay green (N/A-pass when no `producerCode`); **no** assertion edits on that positive path |
| Keys | TEST-ONLY substring in private-key filenames (SIG A4) |
| Dual platform | No `#ifdef` skip; OpenSSL pin unchanged (A6) |

---

## 8. Tester gates (rev1 checklist)

1. **Check list** — cert leaf CN **producer segment** (MRN segment 3 only, upper; **not** segment 4) vs CATALOG **`producerCode`** (upper, exact; no zero-strip). N/A-pass when `producerCode` absent. ST=`DATA_AGGREGATOR` → **permanent exempt** (AGGREGATOR rev1). CN unparseable + `producerCode` present → Enforce **skip** that file, **no** 109.
2. **After failure** — under Enforce, mismatched resource is **skipped** with reason `producer_code_mismatch` and project-map SSE **109**; **not** silently installed; other members continue.
3. **Hook** — producer-code runs on existing SIG / catalogue load path (`verify_exchange_set` → `open_exchange_set` Enforce); **no** parallel stack.
4. **Named CTest** — `exchange_signature_producer` **MUST NOT SKIP** on Linux + Windows.
5. **SSE / reason** — CTest asserts reason **`producer_code_mismatch` first**, project-map SSE **109** secondary (not an S-98 App. B literal; not 126).

---

## 9. Open / Backlog

| Item | Status |
|------|--------|
| Project-map SSE **109** for `producer_code_mismatch` | **Closed** — Planner+Tester aligned (project mapping, not App. B literal; **not** 126). Reason `producer_code_mismatch` primary; CTest asserts reason first, 109 secondary |
| **DATA_AGGREGATOR** ST → exempt producer match | **Closed — permanent policy** (elevated by **`IHO-S164-SIG-AGGREGATOR`** rev1; chain + OfficialClass only; optional WARN `aggregator_producer_code_divergent`; avoids PRIMAR false-kill) |
| Aggregator CN ↔ member HO producer matching | **Closed as permanent exempt** → **`IHO-S164-SIG-AGGREGATOR`** rev1 (`docs/part15-aggregator-producer-contract.md`). **No** Block match (not member / not aggregator / not either); optional WARN `aggregator_producer_code_divergent`; PRODUCER DATA_PRODUCER 1:1 unchanged |
| Field set / missing-`producerCode` / O / ST / Enforce skip | **Closed** (this rev1) |

---

## 10. Non-goals

- Reimplement SIG crypto / chain / CATALOG.SIGN (closed `IHO-S164-SIG`).
- POWERUP changes (`IHO-S164-POWERUP`); producer-code stays out of power-up.
- UI / display styling for official vs non-official.
- S-128 multi-producer packs.
- Permits / encryption / S-63.
- Aggregator CN ↔ member HO Block matching → **Closed** by **`IHO-S164-SIG-AGGREGATOR`** rev1 (permanent exempt + optional WARN). ST=`DATA_AGGREGATOR` exempt is **permanent** MUST (shared with that contract).
- Changing Ok assertions on `exchange_signature` positive path.
- Inventing a new S-98 SSE number for producer mismatch.

---

## 11. Docs sync (this commit)

- `docs/part15-signature-contract.md` — D8 → **`IHO-S164-SIG-PRODUCER`** rev1 (field/SSE locked; this case **Closed**).
- `docs/testing/iho-ctest-case-ids.md` — case **Closed/Done** rev1; Open CI `40179378032` @ `4431d13`≡`9aa7f34`.
- `docs/api-contracts.md` — pointer + Enforce hook note (rev1).
- `docs/architecture.md` — this case Closed; **next** **TBD**; security siblings CN-SHAPE/AGGREGATOR/base SIG remain Active.
- `docs/powerup-self-check-contract.md` — already excludes producer-code; no behaviour change.
- Sibling Active (do **not** mix/Closed this tip): aggregator policy-exempt → **`IHO-S164-SIG-AGGREGATOR`** rev1; CN OfficialClass → **`IHO-S164-SIG-CN-SHAPE`** rev1; base crypto → **`IHO-S164-SIG`**.

---

## 12. Engineer / Tester

**Engineer:** implement §3–§7 MUST; `producer_match` / `producer_mismatch` trees with `producerCode` + re-sign; CTest `exchange_signature_producer`; ST=`DATA_AGGREGATOR` **permanent** exempt (AGGREGATOR rev1); do not touch closed Ok / old `positive/`; aggregator Block matching closed by **`IHO-S164-SIG-AGGREGATOR`**.

**Tester:** gate on §8; assert reason `producer_code_mismatch` first, SSE 109 secondary.
