# Part 15 / S-98 producer-code matching (rev 1) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S164-SIG-PRODUCER`** (**Active / Open**). CTest: **`exchange_signature_producer`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev 1** — Planner freeze (MUST below). Producer-code Enforce match / CN seg3↔CATALOG `producerCode` slice after closed **`IHO-S164-POWERUP`**. Does **not** reopen POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE; does **not** mix/jump Closed **CN-SHAPE** / **AGGREGATOR** / base SIG crypto reopen; does **not** jump **`S102-GRID-SHIM-RETIRE`**. Does **not** expand tip-truth oracles.

Frozen with: `docs/api-contracts.md` (Part 15 producer-code), `docs/part15-signature-contract.md` (**D8** → this case; base SIG — **do not** reopen), `docs/part15-cn-shape-contract.md` (Active; CN MRN→OfficialClass — **do not** mix/Closed this tip), `docs/part15-aggregator-producer-contract.md` (Active; AGGREGATOR exempt+WARN — **do not** mix/Closed this tip; shared ST=`DATA_AGGREGATOR` permanent exempt MUST kept), `docs/powerup-self-check-contract.md` (POWERUP Closed — **do not** reopen), `docs/testing/iho-ctest-case-ids.md`. Draw order → **DISPLAY-PRIORITY** already Closed — **do not** reopen. Scale filter → **SCAMIN** already Closed — **do not** reopen. Unified pick → **SESSION-PICK** already Closed — **do not** reopen. Vendor / sha → **CATALOGUE** already Closed — **do not** reopen. Scrubber → **TIMELINE** already Closed — **do not** reopen. Wiring → **PC-WIRE** already Closed — **do not** reopen. Install-tree → **INSTALL-PREFIX** already Closed — **do not** reopen. UI styling / S-128 packs / permits / invent new SSE / change `exchange_signature` Ok = Non-goals.

Primary research: `/workspace/research/producer-code/BRIEF.md` (2026-10-25 tip-truth refresh). Former Eng gap **already fixed** @ Eng **`0a7fe5f`** (match/mismatch) + **`89e31cd`** (CN-unparseable).

**rev1 field/SSE freeze (kept — from draft 2ed3a63):** freeze CN producer segment ↔ CATALOG `producerCode` match; O out; ST unchanged; missing `producerCode` → N/A-pass; mismatch → per-dataset skip + reason `producer_code_mismatch` (primary) + project-map SSE **109** (secondary; not App. B literal; not 126); AGGREGATOR ST → **permanent exempt** (elevated by `IHO-S164-SIG-AGGREGATOR` rev1 — **do not** Closed AGGREGATOR this tip); aggregator↔member Block matching → **Closed as permanent exempt**; fileName fallback **OFF**; compare exact upper (no zero-strip); segment 4 never compared. CN-unparseable skip clarification kept.

**Status: Active / Open (2026-10-25)** — after **POWERUP Closed** (tip `b37312e` ≡ Arch `11a1319`; tip Close CI [`40178466855`](https://github.com/MasterLivens/S100/actions/runs/40178466855) dual-green; Open-bind evidence CI [`40177947235`](https://github.com/MasterLivens/S100/actions/runs/40177947235) @ `2d2e259`≡`c64f9a6`, CTest `#18 exchange_signature_producer` Passed, O1=tip-truth/docs-close). Eng tip ancestors **`0a7fe5f`** (CN seg3 match/mismatch + fixtures/CTest) + **`89e31cd`** (CN-unparseable + producerCode → Enforce skip / no 109) already on tip — **not greenfield**. Eng green CI [`39111414989`](https://github.com/MasterLivens/S100/actions/runs/39111414989) @ `89e31cd` (native `s100_tests` + signature fixtures — **not** `viewer_*` — **no Win O1 (a)/(b)**). Ubuntu+Windows already Passed — tip Close CI [`40178466855`](https://github.com/MasterLivens/S100/actions/runs/40178466855) `#18 exchange_signature_producer` + Open-bind [`40177947235`](https://github.com/MasterLivens/S100/actions/runs/40177947235) `#18` + Eng landing [`39111414989`](https://github.com/MasterLivens/S100/actions/runs/39111414989). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / CN-SHAPE / AGGREGATOR / base SIG. Do **not** invent case ids or next tip names; freeze tip-truth below (**keep** producer-code oracles — **do not** expand). Do **not** reopen POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE. Do **not** mix/jump Closed **CN-SHAPE** / **AGGREGATOR** / base SIG crypto. Do **not** invent `PRODUCER-*` / `SIGPROD-*` failure-face prefixes (keep existing `producer_code_mismatch` / `producer_cn_unparseable` / bare SSE map). Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). Contract display header stays **rev 1** (do **not** bump just for Open). After Closed next: **TBD** (architecture / registered Active only — do **not** invent).

**Planner freeze (MUST — from BRIEF §7 + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-S164-SIG-PRODUCER`** / CTest **`exchange_signature_producer`**; **forbidden** to invent a second case/test name.
- MUST tip-truth = CN **seg3** upper exact vs CATALOG `producerCode` (**seg4 never**; O out; no zero-strip; no `00AA`≡`AA00`); missing `producerCode` → N/A-pass; mismatch → skip + reason **`producer_code_mismatch`** (primary) + project-map SSE **109** (secondary; not App.B; not 126); CN unparseable + producerCode present → Enforce skip / reason `producer_cn_unparseable` / **no** 109; ST=`DATA_AGGREGATOR` permanent exempt (shared AGGREGATOR rev1 MUST — **do not** Closed AGGREGATOR this tip); fileName fallback **OFF**; hook `verify_exchange_set` only; **MUST NOT** break `exchange_signature` Ok; fixtures `producer_match`/`producer_mismatch`/`producer_cn_unparseable` + old `positive/` N/A-pass; CTest **MUST NOT SKIP**; Linux+Win.
- Eng tip already has **`0a7fe5f`** + **`89e31cd`**; Ubuntu+Win (tip `40178466855` `#18` / Open-bind `40177947235` `#18` / Eng `39111414989`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield. **Keep** tip-truth oracles — **do not** expand scope / SSE / fileName / zero-strip / seg4.
- Power-up self-check → **`IHO-S164-POWERUP`** already Closed — **do not** reopen.
- Install-tree root → **`IHO-S164-POWERUP-INSTALL-PREFIX`** already Closed — **do not** reopen.
- Draw order → **`IHO-DISPLAY-PRIORITY`** already Closed — **do not** reopen.
- Scale filter → **`IHO-SCAMIN-FILTER`** already Closed — **do not** reopen.
- Unified pick → **`IHO-SESSION-PICK-UNIFIED`** already Closed — **do not** reopen.
- Vendor / sha → **`IHO-CATALOGUE-COMPLETE`** already Closed — **do not** reopen.
- Scrubber → **`IHO-TIMELINE-UI`** already Closed — **do not** reopen.
- Wiring → **`IHO-PC-WIRE-111-124`** already Closed — **do not** reopen.
- CN MRN→OfficialClass → **`IHO-S164-SIG-CN-SHAPE`** separate Active — **do not** mix / Closed this tip.
- AGGREGATOR exempt+WARN → **`IHO-S164-SIG-AGGREGATOR`** separate Active — **do not** mix / Closed this tip (shared exempt MUST kept).
- Base Part 15 crypto → **`IHO-S164-SIG`** — **do not** reopen.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog** (DEEPEN O1); **not** this tip.
- Failure face = existing `producer_code_mismatch` / `producer_cn_unparseable` / SSE map; do **not** invent `PRODUCER-*` / `SIGPROD-*` prefixes.
- Ubuntu + Windows: `exchange_signature_producer` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE; do **not** mix/jump Closed CN-SHAPE / AGGREGATOR / base SIG.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`; native `s100_tests` + signature fixtures (`producer_match`/`mismatch`/`cn_unparseable`); no SKIP. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `exchange_signature_producer` MUST NOT SKIP, Passed (already tip CI `40178466855` `#18` / Open-bind `40177947235` `#18` / Eng `39111414989`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng diff required
- After Close next: **TBD**

**Eng notes:** verify `0a7fe5f` + `89e31cd` on tip; API/path in `s100_security` (`verify.cpp` CN seg3 vs CATALOG `producerCode`; `SigRejectCode::ProducerCodeMismatch` → SSE 109; CN-unparseable skip); driver `tests/test_exchange_signature_producer.cpp`; cmake `exchange_signature_producer`; fixtures `testdata/s101/signatures/producer_*`. POWERUP Closed already on this stack (`b37312e`). Do **not** reopen POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE. Do **not** mix/Closed CN-SHAPE / AGGREGATOR; do **not** reopen base SIG crypto. UI / S-128 / permits / invent SSE **Non-goals** — do **not** harden. Do **not** invent `PRODUCER-*` / `SIGPROD-*` failure prefixes. Do **not** change tip-truth oracles / SSE 109 / fileName OFF / zero-strip / seg4.

**O-questions (BRIEF §3 Open 1–N — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `0a7fe5f`/`89e31cd` already; no `ci.yml` / no large Eng) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` — **yes / freeze**.
3. Contract head → Open; keep display **rev 1** (do **not** bump just for Open) — **yes / freeze**.
4. case-ids Active → Open (Status stays Active family) — **yes / freeze**.
5. Notes ≠ POWERUP (already Closed; do **not** reopen); ≠ INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE (already Closed; do **not** reopen); ≠ S-124 / S-111 / S-104 open; ≠ CN-SHAPE / AGGREGATOR (do **not** mix / Closed this tip); ≠ base SIG crypto reopen; do **not** reopen QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE; shim-retire stays Backlog; UI / S-128 / permits / invent SSE stay Non-goals; **keep** tip-truth oracles (do **not** expand) — **yes / freeze**.
6. Closed next → **TBD** (architecture / registered Active; do **not** invent) — **yes / freeze**.
7. No other cases this tip (do **not** Closed CN-SHAPE / AGGREGATOR / base SIG / etc.) — **yes / freeze**.
8. architecture.md flip **next** to this Open — **yes / freeze**.
9. `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog — **yes / freeze**.
10. Failure face = existing `producer_code_mismatch` / `producer_cn_unparseable` / SSE map; do **not** invent `PRODUCER-*` / `SIGPROD-*` prefixes — **yes / freeze**.
11. producer-code Enforce match = this tip; PU self-check = POWERUP Closed; CN OfficialClass = CN-SHAPE Active (do **not** mix); AGGREGATOR WARN = AGGREGATOR Active (do **not** mix; shared exempt MUST kept); base crypto = SIG (do **not** reopen) — **yes / freeze**.

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

- `docs/part15-signature-contract.md` — D8 → **`IHO-S164-SIG-PRODUCER`** rev1 (field/SSE locked).
- `docs/testing/iho-ctest-case-ids.md` — case **Active** rev1 summary.
- `docs/api-contracts.md` — pointer + Enforce hook note (rev1).
- `docs/architecture.md` — security row producer-code follow-on Active.
- `docs/powerup-self-check-contract.md` — already excludes producer-code; no behaviour change.
- Closed follow-on: aggregator policy-exempt → **`IHO-S164-SIG-AGGREGATOR`** rev1 (`docs/part15-aggregator-producer-contract.md`).

---

## 12. Engineer / Tester

**Engineer:** implement §3–§7 MUST; `producer_match` / `producer_mismatch` trees with `producerCode` + re-sign; CTest `exchange_signature_producer`; ST=`DATA_AGGREGATOR` **permanent** exempt (AGGREGATOR rev1); do not touch closed Ok / old `positive/`; aggregator Block matching closed by **`IHO-S164-SIG-AGGREGATOR`**.

**Tester:** gate on §8; assert reason `producer_code_mismatch` first, SSE 109 secondary.
