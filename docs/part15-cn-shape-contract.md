# Part 15 / S-98 CN MRN shape → OfficialClass (rev 1) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S164-SIG-CN-SHAPE`** (**Active / Open**). CTest: **`exchange_signature_cn_shape`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev 1** — Planner freeze (MUST below). CN MRN shape → OfficialClass / ST-then-CN / accept `org:` slice after closed **`IHO-S164-SIG-PRODUCER`**. Does **not** reopen PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE; does **not** mix/jump Closed **AGGREGATOR** / base SIG crypto reopen; does **not** jump **`S102-GRID-SHIM-RETIRE`**. Does **not** expand tip-truth oracles.

Frozen with: `docs/api-contracts.md` (Part 15 CN MRN shape), `docs/part15-signature-contract.md` (**D8 / A9** → this case; base SIG — **do not** reopen), `docs/part15-producer-code-contract.md` (PRODUCER Closed — CN parse reuse — **do not** reopen), `docs/part15-aggregator-producer-contract.md` (Active; AGGREGATOR exempt+WARN — **do not** mix/Closed this tip), `docs/powerup-self-check-contract.md` (POWERUP Closed — **do not** reopen), `docs/testing/iho-ctest-case-ids.md`. Draw order → **DISPLAY-PRIORITY** already Closed — **do not** reopen. Scale filter → **SCAMIN** already Closed — **do not** reopen. Unified pick → **SESSION-PICK** already Closed — **do not** reopen. Vendor / sha → **CATALOGUE** already Closed — **do not** reopen. Scrubber → **TIMELINE** already Closed — **do not** reopen. Wiring → **PC-WIRE** already Closed — **do not** reopen. Install-tree → **INSTALL-PREFIX** already Closed — **do not** reopen. Producer-code Enforce match → **PRODUCER** already Closed — **do not** reopen. UI styling / S-128 packs / permits / invent new SSE / change `exchange_signature` Ok / change `exchange_signature_producer` Ok = Non-goals.

Primary research: `/workspace/research/cn-shape/BRIEF.md` (2026-10-25 tip-truth refresh). Former Eng gap **already fixed** @ Eng **`0c21b4e`** (shared CN/MRN grammar + `classify_official(ST,CN)` + fixtures `cn_shape_ok`/`cn_shape_bad` + keys + CTest).

**rev1 field freeze (kept — Planner Open frozen MRN grammar / `org:` / ST order / no new skip):** conforming CN = whole-string match form 1 `urn:mrn:iho:<prod>:<id>` **or** form 2 `urn:mrn:iho:org:<prod>:<id>` (case-insensitive; `<prod>`=`[A-Za-z0-9]{2,8}`; `<id>`=`[0-9]{1,8}`; **accept `org:` = yes**); classify order **ST first, then CN shape**; ST ∈ official roles + CN shape Ok → **Official**; ST official + CN non-conforming → **NonOfficial** (even SA); **MUST NOT** new Enforce skip / SSE / reject; **MUST NOT** change PRODUCER skip / SSE 109 / `producer_code_mismatch`; reuse PRODUCER CN parser (single grammar); no producerCode gate; fixtures `cn_shape_ok`/`cn_shape_bad`; hook `verify_exchange_set` only; MUST NOT break `exchange_signature` / `exchange_signature_producer` Ok; MUST NOT SKIP; Linux+Win.

**Status: Active / Open (2026-10-25)** — after **PRODUCER Closed** (tip `7bac31a` ≡ Arch `c08cbe7`; tip Close CI [`40179998165`](https://github.com/MasterLivens/S100/actions/runs/40179998165) dual-green; Open-bind evidence CI [`40179378032`](https://github.com/MasterLivens/S100/actions/runs/40179378032) @ `4431d13`≡`9aa7f34`, CTest `#19 exchange_signature_cn_shape` Passed, O1=tip-truth/docs-close). Eng tip ancestor **`0c21b4e`** (shared CN/MRN grammar + `classify_official(ST,CN)` + fixtures/CTest) already on tip — **not greenfield**. Eng green CI [`39112498473`](https://github.com/MasterLivens/S100/actions/runs/39112498473) @ `0c21b4e` (native `s100_tests` + signature fixtures — **not** `viewer_*` — **no Win O1 (a)/(b)**). Ubuntu+Windows already Passed — tip Close CI [`40179998165`](https://github.com/MasterLivens/S100/actions/runs/40179998165) `#19 exchange_signature_cn_shape` + Open-bind [`40179378032`](https://github.com/MasterLivens/S100/actions/runs/40179378032) `#19` + Eng landing [`39112498473`](https://github.com/MasterLivens/S100/actions/runs/39112498473). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE / AGGREGATOR / base SIG. Do **not** invent case ids or next tip names; freeze tip-truth below (**keep** CN-shape oracles — **do not** expand). Do **not** reopen PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE. Do **not** mix/jump Closed **AGGREGATOR** / base SIG crypto. Do **not** invent `CNSHAPE-*` failure-face / skip prefixes (failure face = OfficialClass report only). Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). Contract display header stays **rev 1** (do **not** bump just for Open). After Closed next: **TBD** (architecture / registered Active only — do **not** invent).

**Planner freeze (MUST — from BRIEF §7 + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-S164-SIG-CN-SHAPE`** / CTest **`exchange_signature_cn_shape`**; **forbidden** to invent a second case/test name.
- MUST tip-truth = conforming CN forms 1–2 (accept `org:`) + ST then CN → OfficialClass + bad CN → NonOfficial + **still install** + **MUST NOT** new Enforce skip + reuse PRODUCER CN parser + no producerCode gate + fixtures `cn_shape_ok` (Official+install) / `cn_shape_bad` (NonOfficial+still install) + old `positive/` still Official + hook `verify_exchange_set` / report OfficialClass only + **MUST NOT** break `exchange_signature` / `exchange_signature_producer` Ok + CTest **MUST NOT SKIP**; Linux+Win.
- Eng tip already has **`0c21b4e`**; Ubuntu+Win (tip `40179998165` `#19` / Open-bind `40179378032` `#19` / Eng `39112498473`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield. **Keep** tip-truth oracles — **do not** expand scope / add skip / invent SSE / fork CN grammar / open producerCode gate.
- Producer-code Enforce match → **`IHO-S164-SIG-PRODUCER`** already Closed — **do not** reopen.
- Power-up self-check → **`IHO-S164-POWERUP`** already Closed — **do not** reopen.
- Install-tree root → **`IHO-S164-POWERUP-INSTALL-PREFIX`** already Closed — **do not** reopen.
- Draw order → **`IHO-DISPLAY-PRIORITY`** already Closed — **do not** reopen.
- Scale filter → **`IHO-SCAMIN-FILTER`** already Closed — **do not** reopen.
- Unified pick → **`IHO-SESSION-PICK-UNIFIED`** already Closed — **do not** reopen.
- Vendor / sha → **`IHO-CATALOGUE-COMPLETE`** already Closed — **do not** reopen.
- Scrubber → **`IHO-TIMELINE-UI`** already Closed — **do not** reopen.
- Wiring → **`IHO-PC-WIRE-111-124`** already Closed — **do not** reopen.
- AGGREGATOR exempt+WARN → **`IHO-S164-SIG-AGGREGATOR`** separate Active Eng `29be187` — **do not** mix / Closed this tip.
- Base Part 15 crypto → **`IHO-S164-SIG`** — **do not** reopen.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog** (DEEPEN O1); **not** this tip.
- Failure face = OfficialClass report only; do **not** invent `CNSHAPE-*` skip / failure-face prefixes.
- Ubuntu + Windows: `exchange_signature_cn_shape` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE; do **not** mix/jump Closed AGGREGATOR / base SIG.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`; native `s100_tests` + signature fixtures (`cn_shape_ok`/`cn_shape_bad`); no SKIP. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `exchange_signature_cn_shape` MUST NOT SKIP, Passed (already tip CI `40179998165` `#19` / Open-bind `40179378032` `#19` / Eng `39112498473`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng diff required
- After Close next: **TBD**

**Eng notes:** verify `0c21b4e` on tip; API/path in `s100_security` (`verify.cpp` `cn_prod_token_ok` / `cn_id_token_ok` / `cn_mrn_shape_ok` / `classify_official(st,cn)`; leaf uses `classify_official(cert_st, cert_cn)`); driver `tests/test_exchange_signature_cn_shape.cpp`; cmake `exchange_signature_cn_shape`; fixtures `testdata/s101/signatures/cn_shape_ok` / `cn_shape_bad`; keys `test_ds_cn_org.crt` / `test_ds_cn_shape_bad.crt`. PRODUCER Closed already on this stack (`7bac31a`). Do **not** reopen PRODUCER / POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE. Do **not** mix/Closed AGGREGATOR; do **not** reopen base SIG crypto. UI / S-128 / permits / invent SSE **Non-goals** — do **not** harden. Do **not** invent `CNSHAPE-*` failure prefixes. Do **not** change tip-truth oracles / add Enforce skip / open producerCode gate / fork CN grammar.

**O-questions (BRIEF §3 Open 1–N — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `0c21b4e` already; no `ci.yml` / no large Eng) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` — **yes / freeze**.
3. Contract head → Open; keep display **rev 1** (do **not** bump just for Open) — **yes / freeze**.
4. case-ids Active → Open (Status stays Active family) — **yes / freeze**.
5. Notes ≠ PRODUCER (already Closed; do **not** reopen); ≠ POWERUP / INSTALL-PREFIX / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE (already Closed; do **not** reopen); ≠ S-124 / S-111 / S-104 open; ≠ AGGREGATOR (do **not** mix / Closed this tip); ≠ base SIG crypto reopen; do **not** reopen QBC-MG / UNCERT / DEEPEN / DISPLAY-COMPARE / FASC / BRIDGE; shim-retire stays Backlog; UI / S-128 / permits / invent SSE stay Non-goals; **keep** tip-truth oracles (do **not** expand) — **yes / freeze**.
6. Closed next → **TBD** (architecture / registered Active; do **not** invent) — **yes / freeze**.
7. No other cases this tip (do **not** Closed AGGREGATOR / base SIG / etc.) — **yes / freeze**.
8. architecture.md flip **next** to this Open — **yes / freeze**.
9. `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog — **yes / freeze**.
10. Failure face = OfficialClass report only; do **not** invent `CNSHAPE-*` skip / failure-face prefixes — **yes / freeze**.
11. CN MRN→OfficialClass = this tip; producer-code match = PRODUCER Closed (do **not** reopen); AGGREGATOR WARN = AGGREGATOR Active (do **not** mix); base crypto = SIG (do **not** reopen) — **yes / freeze**.

---

## 1. Goal

Nail **S-98 §9.1.1** Official / NonOfficial classification so that a certificate **CN that does not conform to the IHO MRN shape** is **never** classified **Official**, even if ST is `DATA_PRODUCER` / `DATA_AGGREGATOR` / `SCHEME_ADMINISTRATOR`.

Current impl (`classify_st`) only reads **ST**; BRIEF confirmed **CN is not read for OfficialClass**. This slice wires CN shape into that classification only.

**Install semantics unchanged:** this slice **MUST NOT** open a new Enforce **skip** path. Unparseable CN + `producerCode` present already skips under **`IHO-S164-SIG-PRODUCER`**. Here we only set / report **`OfficialClass`** (and related report fields). Crypto-Ok resources that pass PRODUCER (or N/A-pass) still **install**; they may be marked **NonOfficial** when CN shape fails.

Logic lives **only** in `s100_security`; exchange / session keep calling `verify_exchange_set` (no parallel stack).

---

## 2. Spec anchors (frozen)

| Id | Topic | Spec / project hook | Notes |
|----|-------|---------------------|-------|
| **CS-C1** | Official vs non-official | S-98 Ed 2.0.0 §**9.1.1**: ST role defines official status; ST = `OTHER_DATA_PRODUCER` / unlisted role / **CN not conforming to the MRN format** → **non official** | BRIEF §1.1; SIG **D8 / A9** already stated “non-conforming CN → NonOfficial”; impl gap closed here |
| **CS-C2** | CN = MRN | §9.1.1: CN = IHO data producer integer + alpha as part of a colon-separated **MRN**, e.g. `urn:mrn:iho:aa00:01810`, `urn:mrn:iho:GB00:00540`; S-164 staging also uses `urn:mrn:iho:org:…` | BRIEF §1.7 samples; **`org:` accepted** (Planner) |
| **CS-C3** | ST official roles | Same set as SIG A9: `DATA_PRODUCER` / `DATA_AGGREGATOR` / `SCHEME_ADMINISTRATOR` | Reuse `classify_st` official set; then apply CN shape |
| **CS-C4** | Producer-code match | Separate case **`IHO-S164-SIG-PRODUCER`** | **Out of this slice.** Do not change skip / SSE 109 / `producer_code_mismatch` |
| **CS-C5** | Parse reuse | PRODUCER `cn_producer_segment` / CN MRN parse path | **Reuse**; do not fork a second grammar |

---

## 3. Conforming CN shape (frozen MUST)

Whole CN string (trimmed as already read from X.509) **MUST** match **exactly one** of the following, **case-insensitive** on the URN prefix and segment literals as stated:

1. **`urn:mrn:iho:<prod>:<id>`**
2. **`urn:mrn:iho:org:<prod>:<id>`** — **accept `org:` = yes** (S-164 staging samples).

Where:

| Token | Grammar | Role |
|-------|---------|------|
| `<prod>` | `[A-Za-z0-9]{2,8}` | Producer token (e.g. `aa00`, `GB00`, `99ZZ`, `00AA`, `00PM`) |
| `<id>` | `[0-9]{1,8}` | Certificate instance id — **required for shape**; **not** used for Official beyond “shape Ok” |

**Accept `org:`:** **yes** (Planner). This slice does **not** fold `org:` vs non-`org:` producer equivalence for any match gate (PRODUCER compare stays its own contract).

**Non-conforming (→ NonOfficial even if ST is official), including:**

- Missing CN / empty CN
- Prefix not `urn:mrn:iho:` (case-insensitive)
- Wrong segment count (too few / too many colons for the chosen form)
- Empty or illegal `<prod>` (fails `[A-Za-z0-9]{2,8}`)
- Missing / illegal `<id>` (fails `[0-9]{1,8}`)

**Engineer note:** Prefer extending / sharing the existing PRODUCER CN parser (`cn_producer_segment` family) so shape-Ok and producer-segment extraction stay one grammar. Shape check may additionally require a present numeric `<id>` segment (PRODUCER today only needs a non-empty producer token after prefix).

---

## 4. Classification rule (frozen MUST)

### 4.1 Order

1. **ST first:** if ST ∈ `{OTHER_DATA_PRODUCER}` or any value **not** in the official set → **`NonOfficial`** (empty ST → existing **`Unknown`** semantics from SIG — do not invent Official).
2. **Then CN shape:** if ST ∈ official roles **and** CN is conforming → **`Official`**; if ST ∈ official roles **but** CN is non-conforming → **`NonOfficial`**.
3. Both ST official **and** CN shape Ok → **`Official`**.

Official ST set (unchanged A9): **`DATA_PRODUCER`**, **`DATA_AGGREGATOR`**, **`SCHEME_ADMINISTRATOR`**.

**`SCHEME_ADMINISTRATOR`:** same CN-shape rule (root / SA CN must also be MRN-shaped to stay Official).

### 4.2 Per-file vs exchange-level

| Level | Source | This slice |
|-------|--------|------------|
| **Per-file** `FileVerification::official` | Leaf **data-server** (or resource-authenticating) cert used for that resource’s verify | Apply §4.1 to **that leaf** ST + CN |
| **Exchange-level** `VerificationReport::official` | Scheme-administrator / catalogue path as already defined in SIG | Apply §4.1 to the **SA** (or exchange-level) cert ST + CN; **do not** demote exchange-level solely because a leaf CN is bad (or vice versa) unless SIG already ties them — **Architect lean: keep levels independent** |

### 4.3 Relation to PRODUCER skip (MUST document; MUST NOT change)

| Situation | OfficialClass (this slice) | Enforce install |
|-----------|----------------------------|-----------------|
| ST official + CN shape Ok | Official | Unchanged (PRODUCER / crypto rules still apply) |
| ST official + CN non-conforming | **NonOfficial** | **No new skip from this slice** |
| ST other / unknown | NonOfficial / Unknown | Unchanged |
| CN unparseable + `producerCode` present | NonOfficial (A9/this) may coexist | **Skip already** by PRODUCER §3.1 #8 — **this slice does not add or remove that skip** |
| PRODUCER mismatch (`producer_code_mismatch`) | Classification per this rule on the cert (optional on skipped row) | Skip remains PRODUCER-only |

---

## 5. Proposed MUST (implement checklist)

| Id | Requirement | Sev |
|----|-------------|-----|
| **CS-1** | After leaf (and SA/exchange) cert ST is read, also read **CN** and apply §3–§4 to set `OfficialClass` | MUST |
| **CS-2** | Reuse PRODUCER CN/MRN parse; single grammar for shape + producer segment | MUST |
| **CS-3** | Shape Ok + ST ∈ official roles → Official; else NonOfficial (per §4.1) | MUST |
| **CS-4** | **MUST NOT** introduce a new Enforce skip / SSE / reject reason from this slice | MUST |
| **CS-5** | **MUST NOT** change PRODUCER skip rules, `producer_code_mismatch`, or project-map SSE 109 | MUST |
| **CS-6** | CTest **`exchange_signature_cn_shape`** MUST NOT SKIP (Linux + Windows) | MUST |
| **CS-7** | Closed CTests `exchange_signature` / `exchange_signature_producer` Ok paths stay green (well-formed fixture CNs already Official) | MUST |

---

## 6. Fixtures · CTest · platforms

| Item | Requirement |
|------|-------------|
| CTest | **`exchange_signature_cn_shape`** — **MUST NOT SKIP**; Linux + Windows |
| Hook | Existing `verify_exchange_set` / report `OfficialClass` fields only |
| Positive | Well-formed CN (form 1 or 2) + official ST → **`Official`**; crypto passes; resource **installs** (no producerCode required for this oracle) |
| Negative | Deliberately **bad CN** (e.g. missing prefix, bad `<prod>`, wrong segment count) but **crypto still passes** → **`NonOfficial`**; **MUST still install** under Enforce if PRODUCER N/A-pass / not applicable (proves classification, not skip) |
| Optional SA row | SA / exchange-level CN shape → exchange `official` per §4.2 |
| Reuse | Same Test SA / `verify_time=2026-01-15T00:00:00Z` / `-text` gitattributes pattern as SIG; generator may extend `tools/gen_signature_fixtures.py` |
| Old trees | Do not force-redden closed SIG/PRODUCER positives; new trees under e.g. `testdata/s101/signatures/cn_shape_ok/` + `cn_shape_bad/` |
| Dual platform | No `#ifdef` skip; OpenSSL pin unchanged (SIG A6) |

---

## 7. Non-goals

- **New Enforce skip** from non-conforming CN (PRODUCER already covers unparseable + `producerCode` present).
- Producer-code matching / `producerCode` compare / SSE 109 (`IHO-S164-SIG-PRODUCER`).
- Folding `org:` vs non-`org:` producer-token equivalence for any match gate.
- Aggregator producer policy (permanent exempt + WARN) → **`IHO-S164-SIG-AGGREGATOR`** rev1.
- Official / NonOfficial **UI styling** / colours.
- SignatureOnSignature; POWERUP changes; S-102; permits / encryption.
- Inventing a new S-98 SSE for bad CN shape.
- Changing closed Ok assertions on `exchange_signature` / `exchange_signature_producer`.

---

## 8. Open / frozen

| Item | Status |
|------|--------|
| Exact MRN regex / grammar for conforming CN | **Closed** — §3 forms 1–2 |
| Accept `urn:mrn:iho:org:<…>` | **Closed — yes** |
| Official ST roles for this gate | **Closed** — reuse A9: `DATA_PRODUCER` / `DATA_AGGREGATOR` / `SCHEME_ADMINISTRATOR` |
| Classify order | **Closed** — ST first, then CN shape (§4.1) |
| Exchange-level vs per-file when one CN bad | **Closed** — independent levels (§4.2 Architect lean) |
| Interaction with PRODUCER skip | **Closed** — this slice does **not** change skip rules (§4.3) |
| Aggregator ↔ member HO Block match | **Closed as permanent exempt** → **`IHO-S164-SIG-AGGREGATOR`** rev1 (`docs/part15-aggregator-producer-contract.md`) |
| QBC multi-grid `.nn` | **Backlog** (queue note; not this case) |
| Install-prefix | **Active** → **`IHO-S164-POWERUP-INSTALL-PREFIX`** (`docs/powerup-install-prefix-contract.md` rev1) |

---

## 9. Docs sync (this commit)

- `docs/part15-signature-contract.md` — A9 / D8 cross-ref → **`IHO-S164-SIG-CN-SHAPE`** for CN shape; PRODUCER remains producer-code match.
- `docs/testing/iho-ctest-case-ids.md` — case **Active** rev1 summary.
- `docs/api-contracts.md` — pointer section.
- `docs/architecture.md` — security row adds CN-shape follow-on Active.
- `docs/part15-producer-code-contract.md` — unchanged behaviour; CN shape OfficialClass is sibling case (no edit required beyond optional cross-ref if present).

---

## 10. Engineer / Tester

**Engineer:** implement §3–§6 MUST; reuse PRODUCER CN parser; wire CN shape into OfficialClass after ST; new fixtures `cn_shape_ok` / `cn_shape_bad` + CTest `exchange_signature_cn_shape`; **do not** add skip/SSE; do not touch PRODUCER mismatch path; aggregator policy → **`IHO-S164-SIG-AGGREGATOR`** rev1; leave QBC `.nn` for Backlog; install-prefix → **`IHO-S164-POWERUP-INSTALL-PREFIX`** rev1.

**Tester:** gate on §5–§6; positive → Official + install; negative bad-CN + crypto Ok → NonOfficial + **still installed** (when PRODUCER N/A); assert **no** new skip reason from this case; CTest MUST NOT SKIP Linux + Windows.
