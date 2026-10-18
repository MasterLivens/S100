# Part 15 data protection · signature verification (rev 1.2)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S164-SIG`** (**Active**). CTest: **`exchange_signature`** — **MUST NOT SKIP**, no platform guard, Linux + Windows CI.

**D8 producer-code matching** is tracked as **`IHO-S164-SIG-PRODUCER`** (`docs/part15-producer-code-contract.md`); out of this SIG contract’s implement scope.

Primary source: `/workspace/research/part15/BRIEF.md` (Planner) and `/workspace/research/part15/verify/{verify_chs,verify_generic}.py`. Spec texts: S-100 Ed 5.2.1 Part 15 / Part 17, S-98 Ed 2.0.0 §9.1.1 / §20.4.3 c / App. B, S-164 Ed 1.0.0 §2.5–2.7 (+ 2.0.1 draft names). Docs only in this slice; **no engine / fixture / CMake / CI change in the Architect commit**.

**rev1.1 changes:** (1) Planner overrule of A5 — library C++ default is **`VerifyPolicy::Enforce`**, not `Off`. Reason: signature verification is a security boundary; a new import call site that forgets to pass a policy must refuse unsigned data, not silently accept it. Existing 34 tests pass `Off` explicitly (one-time, intentional and visible). Product CLI hardcodes `Enforce`; viewer has no `open_exchange_set` yet (oracle 7). `Off` unreachable from the product (grep). Standalone `open_dataset` stays `UNVERIFIED`. Do not re-sign or modify old fixture bytes. (2) Tester — oracle **3a** pins exact SSE **126** when the trust store is non-empty `builtin_iho_52()` (105 only for empty trust / oracle 2f). (3) Tester — oracle **7** / viewer: CLI Enforce ≥ 1; viewer may be 0 until an exchange-set open exists, then each must pass Enforce. OPENSSL_API_COMPAT, check order, other oracles, SSE mapping unchanged from rev1.

**rev1.2 changes:** CI learnings made durable. (1) Fixture generator `openssl` CLI MUST work on OpenSSL **3.0.13** (CI Ubuntu): do not use flags only available in 3.4+ such as `openssl req -not_before`/`-not_after`; use 3.0-compatible forms (e.g. `openssl ca -startdate`/`-enddate`). Same spirit as `OPENSSL_API_COMPAT=30000` for C++. (2) All **signed** fixture files under `testdata/s101/signatures/**` (including `CATALOG.XML`, `CATALOG.SIGN`, datasets, `.sig`) MUST be listed in `.gitattributes` as `-text` (binary checkout). Verification is over on-disk raw bytes; CRLF conversion breaks signatures (Windows CI failure mode).

Strong guarantees (same family as update-robustness):
1. A **rejected exchange set** (SSE 134 / 105 / … at catalogue level) leaves the caller’s `ChartSession` **bit-for-bit unchanged** (no members inserted, catalogue/series maps cleared only if `open_exchange_set` had already mutated — prefer verify-before-mutate: fail before writing session state).
2. A **skipped dataset** (SSE 109 / 129 / …) does not affect other datasets: they install; the skipped one is absent from the session and listed in the report.

## Scope

1. Verify S-100 Part 15 signatures on the **exchange-set import path** only (`ChartSession::open_exchange_set` + `load_member` / `load_all_members` under `VerifyPolicy::Enforce`).
2. Trust store: built-in IHO 5.2 root + optional user-installed SA; test-mode trust of the S-164 test SA.
3. Map internal reject codes → S-98 SSE codes in **one** dedicated place.
4. Surface official / non-official classification; CLI and viewer show verification status (incl. `UNVERIFIED` for standalone `open_dataset`).
5. New CTest `exchange_signature` + signed fixtures under `testdata/s101/signatures/` (Engineer).

## Planner decisions (D1–D8) — carry verbatim in meaning

| Id | Decision |
|----|----------|
| **D1** | Build the certificate chain by catalogue `id` / `issuer` lookup and verify each certificate with the issuer’s public key. **No** strict RFC 5280 DN name chaining (S-164 v2.0.0 test cert issuer names do not match SA subject; `openssl verify` error 20). DN mismatch → warning log only. |
| **D2** | Any expired certificate in the chain → reject (SSE 122). Verification time is an **injectable** parameter; fixtures use a **fixed** date; CI must never depend on the wall clock. |
| **D3** | Verify the **raw on-disk bytes** of each file. A missing `dataStatus` is treated as `unencrypted` (info log). `dataStatus` ∈ {compressed, encrypted} → unsupported, that resource skipped (not installed). |
| **D4** | Trust store defaults to the built-in IHO 5.2 root; the user may install an SA. The S-164 test SA is trusted **only in test mode**. A self-signed certificate shipped **inside** an exchange set is **never** a trust anchor. |
| **D5** | Internal enum, mapped to S-98 SSE codes. S-164 116 / 124 map to **109**. The mapping table lives in **ONE** separate place (own file or clearly bounded section), because S-164 2.0.1 and S-98 3.0.0 are drafts and codes may change. |
| **D6** | Data signatures accept **only** ECDSA-384-SHA2 (P-384 + SHA-384, Base64 DER `SEQUENCE{r,s}`, over raw file bytes). Certificate signatures accept `ecdsa-with-SHA256` and `ecdsa-with-SHA384`. **No** mandatory X.509 extension checks (BasicConstraints / KeyUsage). |
| **D7** | Verification is enforced **only** on the exchange-set import path. Standalone `open_dataset(.000)` stays for tests and dev; CLI and viewer show **`UNVERIFIED`**. **No** `--allow-unsigned` (or equivalent) in the product. The existing 34 tests keep **assertions and fixture bytes** unchanged; they pass `VerifyPolicy::Off` **explicitly** at every exchange-set import call site (A5 / A8). |
| **D8** | This slice outputs an **official / non-official** classification (ST role + CN shape). Producer-code matching (CN MRN ↔ dataset producer) → case **`IHO-S164-SIG-PRODUCER`** / `docs/part15-producer-code-contract.md` (next slice after POWERUP; draft pending Planner field/SSE lock). |

## SSE behaviour (S-98 2.0.0 §20.4.3 c + App. B)

| Situation | SSE | Effect |
|-----------|-----|--------|
| `CATALOG.SIGN` missing or signature / auth invalid | **134** | Whole exchange set rejected; nothing installs |
| `CATALOG.SIGN` unparseable | **128** | Whole set rejected (Figure B-1) |
| No SA root installed | **105** | Whole set rejected |
| Chain does not reach the SA | **126** | Whole set rejected (catalogue) / that resource skipped (dataset) |
| Any cert in chain expired at `verify_time` | **122** | Reject (catalogue → whole set; dataset → that resource) |
| Single dataset / support-file signature missing or invalid | **109** (also absorbs S-164 116/124) | **Skip that resource**; rest install |
| Signature path / value missing (B-2 SSE 132) | **132** | Skip that resource |
| FC / PC not signed **directly** by the SA | **129** | Skip that catalogue resource |
| DS certificate not present in catalogue | **107** | Skip that resource |
| SA certificate unreadable / wrong format | **108** / **106** | Whole set rejected |

Do not apply rejected content; notify the user (may coalesce UI prompts); **log every** rejection (S-98 20.4.3 c).

## Architect decisions (A1–A9)

### A1 Module boundary

New library target **`s100_security`** (sources under `src/security/`, public headers under `include/s100/security/`). Narrow façade — roughly:

```cpp
namespace s100::security {

enum class VerifyPolicy { Off, Enforce };

struct VerifyOptions {
  std::chrono::system_clock::time_point verify_time;  // REQUIRED; never defaulted to now()
  bool test_mode{false};  // when true, TrustStore may include the S-164 test SA
};

enum class AuthStatus {
  Ok,
  Unverified,           // policy Off, or standalone open_dataset
  RejectedExchange,     // whole set (134/105/128/…)
  SkippedResource,      // one file (109/129/132/…)
};

enum class OfficialClass { Official, NonOfficial, Unknown };

/** Internal codes — NEVER serialize as S-98 numbers from call sites; map via sse_code(). */
enum class SigRejectCode {
  None,
  CatalogSignMissing,      // → 134
  CatalogSignParse,        // → 128
  CatalogSignInvalid,      // → 134
  NoSchemeAdministrator,   // → 105
  SaCertificateInvalid,    // → 106 / 108
  IssuerNotSa,             // → 126
  CertificateExpired,      // → 122
  DatasetSignatureInvalid, // → 109  (tamper, wrong alg, bad DER/Base64, wrong-file sig)
  DatasetSignatureMissing, // → 132
  CertificateMissing,      // → 107
  CatalogueNotSaDirect,    // → 129
  UnsupportedDataStatus,   // unsupported compressed/encrypted — skip, no SSE number required
};

/** D5: THE single mapping table. Own translation unit, e.g. src/security/sse_map.cpp
 *  (+ declaration in include/s100/security/sse_map.hpp). No other file may hard-code SSE integers. */
int sse_code(SigRejectCode c);
const char* sse_message(SigRejectCode c);  // S-98 Table B-1 text where applicable

struct FileVerification {
  std::filesystem::path path;
  AuthStatus status{AuthStatus::Ok};
  SigRejectCode code{SigRejectCode::None};
  int sse{0};                         // sse_code(code), 0 if None
  std::string message;
  OfficialClass official{OfficialClass::Unknown};
};

struct VerificationReport {
  AuthStatus status{AuthStatus::Ok};
  SigRejectCode code{SigRejectCode::None};  // first whole-set failure, else None
  int sse{0};
  std::string message;
  OfficialClass official{OfficialClass::Unknown};  // exchange-level (schemeAdministrator ST)
  std::vector<FileVerification> files;             // per CATALOG.XML resource + CATALOG.XML itself
  std::vector<std::filesystem::path> installed;    // resources that passed (exact list)
  std::vector<std::filesystem::path> skipped;      // resources skipped (109/129/…)
};

/** Value type; injectable. Default = built-in IHO 5.2 root only. */
class TrustStore {
 public:
  static TrustStore builtin_iho_52();                 // embedded IHO-S100ed5.2-root
  static TrustStore for_tests(const std::filesystem::path& test_sa_pem);  // test_mode SA
  bool add_user_sa(const std::filesystem::path& pem_or_der_or_crt);
  // … no OpenSSL types in the header
};

VerificationReport verify_exchange_set(
    const std::filesystem::path& exchange_root,
    const TrustStore& trust,
    const VerifyOptions& options);
}
```

- **Exchange / session call only that interface.** `s100_core` links `s100_security`; it does not `#include <openssl/…>`.
- **No OpenSSL headers or types leak** outside `src/security/*.cpp`. Grep oracle (Tester):  
  `rg -n '#include\s*<openssl/|EVP_|X509_|PEM_|BIO_' include/ src/cli src/viewer src/core` → **empty**; hits only under `src/security/`.
- `TrustStore` is a value type (copyable), built in tests with `for_tests(...)`.
- Report surfaces:
  - **ChartSession**: stores last `VerificationReport`; `load_member` under Enforce consults per-file results (skip failed resources).
  - **CLI**: exchange-set mode prints `sse=<N> status=… installed=… skipped=… official=…`; standalone cell prints `status=UNVERIFIED`.
  - **Viewer**: status bar shows `UNVERIFIED` for standalone `.000`; for Enforce import shows `SSE <n>: …` / `SIGNED official|non-official` (WARN-level UI, no automation).

### A2 Reproducibility

ECDSA signing is randomized. RFC 6979 deterministic ECDSA needs OpenSSL ≥ 3.2, which conflicts with the 3.0-only API pin (A6). Therefore the fixture reproducibility assertion is:

> **Generated signatures verify** under the test trust store, **and** all **unsigned** parts of each fixture tree (payload files, unsigned XML text nodes that the generator does not rewrite, scripts, manifests) are **byte-identical** across two generator runs.

Signed fixture trees under `testdata/s101/signatures/**` MUST remain byte-identical after Git checkout (see A4 `.gitattributes` `-text`); otherwise A2 / D3 verification fails on Windows.

`update_fixtures_reproducible` / `tests/check_update_fixtures.py` stay **untouched**. New check lives under the `exchange_signature` CTest (or a sibling `signature_fixtures_reproducible` invoked from it).

### A3 Local optional real cases (WARN, never SKIP)

| Case | Location | Rule |
|------|----------|------|
| IHO-signed S-101 PC 2.0.0 (`IHO_SIGNED`) | Optional local copy of the S-164 repo sample (no LICENSE → **not** in CI, **not** committed) | Print `signatures: iho_pc exercised` / `not exercised`; WARN oracle |
| CHS COMBINED expired DS cert (notAfter 2026-04-02) | `testdata/chs/…` via **TEST-DATA-ROOTS** (`chs_exercised("signatures.chs122")`) | Under Enforce + `verify_time` after notAfter → SSE **122**; absent → `not exercised`; never CTest SKIP |

### A4 Fixtures

- Self-generated **test SA**, data-server cert, and **TEST-ONLY** private keys, committed; every private-key filename contains the substring `TEST-ONLY` (e.g. `testdata/s101/signatures/keys/test_sa_TEST-ONLY.pem`).
- Generator: `tools/gen_signature_fixtures.py`.
- **Crypto dependency decision:** the Planner’s verify scripts use **openssl CLI + Python stdlib** only. CI images already have OpenSSL on both platforms; they do **not** ship `cryptography`. **Commit the generated fixtures + TEST-ONLY keys**; the generator uses `subprocess` + `openssl` (Part 15 §15-8.5 command patterns) and Python 3 stdlib — **no pip crypto package**. CI runs the generator twice into temp dirs and asserts A2 (unsigned bytes identical + signatures verify). Offline / local regenerates the same way.
- **OpenSSL CLI pin (generator):** `tools/gen_signature_fixtures.py` MUST invoke only OpenSSL **3.0**-compatible CLI flags so regeneration succeeds on CI Ubuntu OpenSSL **3.0.13**. Do **not** use 3.4+-only options such as `openssl req -not_before` / `-not_after`; prefer 3.0 forms (e.g. `openssl ca -startdate` / `-enddate`). Same spirit as `OPENSSL_API_COMPAT=30000` for C++ (A6).
- **Binary checkout for signed fixtures:** all **signed** files under `testdata/s101/signatures/**` (including `CATALOG.XML`, `CATALOG.SIGN`, datasets, `.sig`, and any other signed payload) MUST be listed in `.gitattributes` as `-text` so Git checks them out as raw bytes on every platform. Verification hashes on-disk bytes (D3); CRLF conversion is a Windows CI failure mode.
- Layout (Engineer): `testdata/s101/signatures/` with one subtree per negative/positive case (below).
- Negative variants (one per SSE / condition above), plus: tampered data byte, tampered `CATALOG.XML`, missing `CATALOG.SIGN`, wrong algorithm string, self-signed cert in set, expired cert at the injected time, FC/PC not SA-signed, signature belonging to a different file, bad Base64/DER.

### A5 VerifyPolicy — resolution for existing unsigned fixtures (Tester binding)

```cpp
enum class VerifyPolicy { Off, Enforce };
```

| Caller | Policy |
|--------|--------|
| Product CLI exchange-set path | **`Enforce`** (hardcoded at the call site) |
| Product viewer exchange-set path | **`Enforce`** (hardcoded) |
| Standalone `open_dataset` / CLI cell / viewer `.000` | No verification; UI shows **`UNVERIFIED`** (D7) |
| Library `ChartSession::open_exchange_set` | Optional `VerifyPolicy` parameter; **C++ default = `Enforce`** |
| Library `open_cell_series` | Optional `VerifyPolicy` parameter; **C++ default = `Enforce`** |

**Why default Enforce (Planner overrule of rev1 Off):** signature verification is a **security boundary**. A new import call site that forgets to pass a policy must **refuse** unsigned data, not silently accept it. Existing 34 tests that open unsigned catalogues pass `VerifyPolicy::Off` **explicitly** at every exchange-set import site (one-time, intentional and visible). “Product default is Enforce” means both the **library default** and the **shipped apps** (CLI / viewer hardcode `Enforce`). Document in the header comment that `Off` is for tests / explicit opt-out only — not a product escape hatch.

**Every existing test import call site must pass `Off` explicitly** (see A8 enumeration). Relying on a default of `Off` is forbidden.

**Off is unreachable from the product:**
- No CLI flag, no env var, no settings key that selects `Off` or “allow unsigned”.
- Grep oracle (MUST):  
  `rg -n 'VerifyPolicy::Off|--allow-unsigned|S100_ALLOW_UNSIGNED|allow_unsigned' src/cli src/viewer` → **empty**.  
  `rg -n 'VerifyPolicy::Enforce' src/cli` → **≥ 1** (every CLI exchange-set open site).  
  `rg -n 'open_exchange_set' src/viewer` → today **0** (viewer has no exchange-set open yet); **if any hit appears**, each must pass `VerifyPolicy::Enforce` and `rg -n 'VerifyPolicy::Enforce' src/viewer` → **≥ 1**. Do not leave a viewer `open_exchange_set` on the default without an explicit `Enforce`.

**Standalone `open_dataset` stays `UNVERIFIED`** (D7) — that path never calls `verify_exchange_set`.

### A6 OpenSSL build pin (Eng + Tester + Planner binding)

```cmake
find_package(OpenSSL 3.0 REQUIRED COMPONENTS Crypto)
add_library(s100_security …)
target_link_libraries(s100_security PUBLIC OpenSSL::Crypto)  # or PRIVATE if types stay .cpp-only
target_compile_definitions(s100_security PRIVATE
  OPENSSL_API_COMPAT=30000
  OPENSSL_NO_DEPRECATED)
```

- Pins the **compile** to the OpenSSL **3.0 API surface** so a newer local OpenSSL (e.g. 3.5.7) cannot silently accept post-3.0 APIs that then fail on CI Ubuntu **3.0.13**.
- Windows: `C:\Program Files\OpenSSL` (Shining Light 3.6.x under `windows-latest`); CI MUST add a step that **FAIL-fast** checks `libcrypto-3-x64.dll` is on `PATH` (`where libcrypto-3-x64.dll` / Test-Path) — never hang on a missing-DLL dialog (same lesson as HDF5).
- Grep oracle (MUST): both `OPENSSL_API_COMPAT=30000` and `OPENSSL_NO_DEPRECATED` appear on the `s100_security` target in `CMakeLists.txt` (or its module file). Tester rejects the push if either is missing.

### A7 Check order (deterministic; first failure wins)

Whole-set phase (any failure → `RejectedExchange`, session unchanged):

1. `CATALOG.SIGN` exists → else **134**
2. `CATALOG.SIGN` parses as `StandaloneDigitalSignature` → else **128**
3. Trust store contains an SA root (builtin and/or user; test SA only if `test_mode`) → else **105**
4. Collect certificates from `CATALOG.SIGN` (+ catalogue `certificates` if present); **ignore** any self-signed cert in the set as a trust anchor (D4 / warning log)
5. For the signing cert of `CATALOG.XML`: build chain by `id`/`issuer` (D1); verify each cert signature with issuer pubkey; accept cert algs per D6
6. Any cert `notAfter` < `verify_time` or `notBefore` > `verify_time` → **122**
7. Chain does not terminate at a trust-store SA → **126**
8. Verify `CATALOG.XML` raw bytes with ECDSA-384-SHA2 → else **134**

Per-resource phase (dataset / support / FC / PC metadata rows; continue after skips):

9. Missing `digitalSignatureValue` → **132**, skip
10. `digitalSignatureReference` ≠ `ECDSA-384-SHA2` → **109**, skip (log “unsupported algorithm”)
11. Base64 / DER decode fail → **109**, skip
12. CertificateRef missing in catalogue → **107**, skip
13. Chain / expiry checks for that resource’s cert (same as 5–7) → **122** / **126** / …, skip that resource
14. FC/PC resource whose signing cert is not **directly** signed by the SA → **129**, skip
15. `EVP_DigestVerify` over on-disk bytes fails (tamper, wrong-file signature, …) → **109**, skip
16. `dataStatus` compressed/encrypted → unsupported, skip (no install); missing → unencrypted (D3)
17. Else accept → append to `installed`; classify official/non-official (D8)

When several defects exist on one resource, the **lowest step number** in 9–16 wins. Whole-set failures (1–8) always beat per-resource codes.

### A8 Existing tests / fixtures under enforcement — enumeration and resolution

**Resolution (Tester):** do **not** re-sign or modify old fixture bytes. Diff on the old 34 tests’ **assertions** and on existing **fixture bytes** must be **empty of semantic changes**. Every call site on the exchange-set import path (`ChartSession::open_exchange_set`, `open_cell_series`) that must stay green **passes `VerifyPolicy::Off` explicitly**. `group_cell_series` is catalogue grouping only — **no** `VerifyPolicy` parameter; sites that call it then `open_cell_series` attach `Off` on the `open_cell_series` call.

#### Every committed `CATALOG.XML` used by exchange-set import today

| Path | Used by | Signed today? |
|------|---------|---------------|
| `testdata/exchange_set/CATALOG.XML` | `test_exchange_set`, `test_update_robustness` (ok_session) | **No** |
| `testdata/s101/updates/catalog/CATALOG.XML` | `test_update_robustness` (`open_cell_series`) | **No** |
| `testdata/s101/updates/session/CATALOG.XML` | `test_update_robustness` (`ChartSession`) | **No** |
| `testdata/s101/updates/newed/CATALOG.XML` | `test_cancel_reissue`, `test_edition_hardening` | **No** |
| `testdata/s101/updates/session_replace/{A,B,X,C}/CATALOG.XML` | `test_cancel_reissue` | **No** |
| `testdata/s101/updates/edition/base_bad_cat/CATALOG.XML` | `test_edition_hardening` | **No** |
| `testdata/s101/updates/edition/nocat/CATALOG.XML` | `test_edition_hardening` | **No** |
| CHS `COMBINED/S100_ROOT/CATALOG.XML` (gitignored) | `test_exchange_set`, `test_s102`, … via TEST-DATA-ROOTS | **Yes** (PRIMAR; DS notAfter **2026-04-02**) |

#### Call sites that open an exchange set (re-enumerated from contract + code search 2026-10-18)

| File | Approx. function / site | Call | Under library default Enforce would… | Required edit |
|------|-------------------------|------|--------------------------------------|---------------|
| `tests/test_exchange_set.cpp` | `test_exchange_set_synthetic_and_chs` | `session.open_exchange_set(synthetic)` | Reject unsigned → **134** | Pass **`VerifyPolicy::Off`** explicitly |
| `tests/test_exchange_set.cpp` | `test_exchange_set_synthetic_and_chs` | `open_cell_series(*with_upd, …)` (CHS) | Wall-clock **122** after 2026-04-02 | Pass **`Off`** explicitly; local WARN case under Enforce + injected time (A3) |
| `tests/test_exchange_set.cpp` | `test_exchange_set_synthetic_and_chs` | `chs_session.open_exchange_set(chs)` | 122 risk | Pass **`Off`** explicitly |
| `tests/test_update_robustness.cpp` | `test_exchange_update_robustness` (catalog block) | `open_cell_series(series[0], …)` on `catalog/CATALOG.XML` | Reject / break oracles | **Named deviation** — pass **`Off`** (call-arg only) |
| `tests/test_update_robustness.cpp` | `test_exchange_update_robustness` (oracle 11) | `session.open_exchange_set(session/CATALOG.XML)` | Reject | **Named deviation** — pass **`Off`** |
| `tests/test_update_robustness.cpp` | `test_exchange_update_robustness` (oracle 11 ok path) | `ok_session.open_exchange_set(exchange_set/CATALOG.XML)` | Reject | **Named deviation** — pass **`Off`** |
| `tests/test_cancel_reissue.cpp` | `run` (oracle 5 newed) | `open_cell_series(series[0], …)` on `newed/CATALOG.XML` | Reject | **Named deviation** — pass **`Off`** |
| `tests/test_cancel_reissue.cpp` | `run` (session_replace) | `session.open_exchange_set` A / B / X / C | Reject | **Named deviation** — pass **`Off`** (each site) |
| `tests/test_cancel_reissue.cpp` | `run` (session_replace fresh) | `open_cell_series(group_cell_series(cat).front(), …)` | Reject | **Named deviation** — pass **`Off`** |
| `tests/test_edition_hardening.cpp` | `via_series` helper | `open_cell_series(series.front(), …)` | Reject | **Named deviation** — pass **`Off`** |
| `tests/test_edition_hardening.cpp` | `run` (oracle 4 / nocat) | `session.open_exchange_set(edition/nocat/…)` | Reject | **Named deviation** — pass **`Off`** |
| `tests/test_edition_hardening.cpp` | `run` (base_bad_cat, two sites) | `session.open_exchange_set(edition/base_bad_cat/…)` | Reject | **Named deviation** — pass **`Off`** (each site) |
| `tests/test_s102.cpp` | `test_s102_session_overlay` | `chs_session.open_exchange_set(chs)` | 122 risk | Pass **`Off`** explicitly |
| `src/cli/main.cpp` | exchange-set open (~line 401) | `session.open_exchange_set(path)` | Product | Pass **`VerifyPolicy::Enforce`** (hardcoded) |
| `src/viewer/main.cpp` | no `open_exchange_set` today (overlays use `insert` on already-open cells) | — | When an exchange-set open is added, each site must hardcode **`Enforce`**; standalone `.000` stays `UNVERIFIED` |

`group_cell_series` call sites (no policy arg; listed for completeness): `test_exchange_set_synthetic_and_chs`, `test_exchange_update_robustness` (catalog), `run` in cancel-reissue (newed + session_replace), `via_series` in edition-hardening. Policy attaches on the subsequent `open_cell_series` / `open_exchange_set` only.

#### Named deviations (frozen files)

Edits to these three frozen files are a **named deviation** from the prior “byte-identical frozen” rule:

| Frozen file | Allowed edit | Forbidden |
|-------------|--------------|-----------|
| `tests/test_update_robustness.cpp` | Call-argument only: add `VerifyPolicy::Off` (and maybe an `#include` for the policy enum) at each `open_exchange_set` / `open_cell_series` site listed above | Assertion changes; fixture-byte changes; behaviour / oracle rewrites |
| `tests/test_cancel_reissue.cpp` | Same — call-argument only | Same |
| `tests/test_edition_hardening.cpp` | Same — call-argument only | Same |

**Tester check:** `git diff` on assertions and fixtures must still be empty of semantic changes; only the `Off` argument (and maybe an include) appears in those three files. Existing fixture bytes under `testdata/s101/updates/**` and `testdata/exchange_set/**` remain **byte-identical**. `update_fixtures_reproducible` untouched. Do **not** re-sign or modify old fixture bytes.

**Non-CATALOG paths unchanged by D7:** `open_dataset_with_updates`, `discover_update_files`, standalone `apply_s101_update`, all portrayal/S-102/104/111/124 unit tests — no signature gate.

### A9 Official / non-official (D8)

`VerificationReport::official` / per-file `OfficialClass` from certificate ST: `DATA_PRODUCER` / `DATA_AGGREGATOR` / `SCHEME_ADMINISTRATOR` → Official; `OTHER_DATA_PRODUCER` or unknown / non-conforming CN → NonOfficial. UI distinction beyond exposing the enum → later. Producer-code matching → **`IHO-S164-SIG-PRODUCER`** (`docs/part15-producer-code-contract.md`).

## Fixed verification time (fixtures)

All committed signature fixtures and the `exchange_signature` CTest inject:

```text
verify_time = 2026-01-15T00:00:00Z
```

(Asia/Shanghai label when printed: **2026-01-15 08:00:00 UTC+8**.) Test SA / DS certs are generated with `notBefore` ≤ that instant ≤ `notAfter`. Expired negative fixture: `notAfter = 2025-12-01T00:00:00Z` (before the injected time). Grep oracle: `rg -n 'system_clock::now|time\(nullptr\)|gettimeofday|GetSystemTime' src/security` → **empty** (verify path never reads the wall clock).

## Crypto & CI

- OpenSSL 3 via `find_package(OpenSSL 3.0 REQUIRED COMPONENTS Crypto)` + A6 macros.
- APIs used (3.0-safe): `PEM_read_bio_X509` / `d2i_X509`, `X509_verify`, `X509_get0_pubkey`, `X509_cmp_time` / `ASN1_TIME_cmp_time_t` against **injected** time, `EVP_DigestVerifyInit` + `EVP_sha384`, Base64 decode, `EVP_PKEY_get_group_name` (secp384r1).
- Windows CI: sanity check `libcrypto-3-x64.dll` on PATH before `ctest` (FAIL fast).
- Linux: `libssl-dev` already on ubuntu-24.04; keep apt install explicit if desired.

## API (additive) — also recorded in `docs/api-contracts.md`

- New headers under `include/s100/security/`.
- `ChartSession::open_exchange_set(path, err, VerifyPolicy policy = VerifyPolicy::Enforce)` (or equivalent overload); likewise `open_cell_series(..., VerifyPolicy policy = VerifyPolicy::Enforce)`. Session retains policy + last `VerificationReport`.
- `load_member` / `load_all_members` under Enforce skip resources listed in `report.skipped` and do not insert them.
- No change to `open_dataset` / `load_s101` signatures.
- No product `--allow-unsigned`.

## Oracle — CTest `exchange_signature` (no SKIP; Linux + Windows)

| # | Check | Sev |
|---|-------|-----|
| 1 | **Positive control:** exchange set signed by the test SA fully verifies and installs under `TrustStore::for_tests`; `status == Ok`; `installed` lists every catalogue resource; `skipped` empty | MUST |
| 2a | Missing `CATALOG.SIGN` → SSE **134**; session datasets empty / unchanged bit-for-bit | MUST |
| 2b | Invalid `CATALOG.SIGN` (bad sig) → SSE **134**; unchanged | MUST |
| 2c | Unparseable `CATALOG.SIGN` → SSE **128**; unchanged | MUST |
| 2d | 1 byte changed in one dataset → SSE **109**; **only** that path in `skipped`; `installed` lists exactly the others; those members load; caller pre-state unchanged for rejected whole-set cases | MUST |
| 2e | Issuer not found / chain ≠ SA → SSE **126** | MUST |
| 2f | Empty trust store (no SA) → SSE **105** | MUST |
| 2g | Injected time after cert `notAfter` → SSE **122** | MUST |
| 2h | FC/PC not signed directly by SA → SSE **129**; that resource skipped | MUST |
| 2i | Non-`ECDSA-384-SHA2` algorithm string → SSE **109** | MUST |
| 2j | Bad Base64 / bad DER → SSE **109** | MUST |
| 2k | Signature value belonging to a different file → SSE **109** | MUST |
| 2l | For every rejection above: caller session / output dataset bit-for-bit unchanged where the contract requires (whole-set → no inserts; per-file → other members intact) | MUST |
| 3a | Production `TrustStore::builtin_iho_52()` (non-empty) **rejects** the test SA (no test_mode) → exact SSE **126** only; **105** is reserved for an empty trust store with no SA root (oracle 2f) | MUST |
| 3b | Self-signed root shipped inside the exchange set is **not** trusted → reject (**126** / **134**) | MUST |
| 4 | `verify_time` always injected; grep: no wall-clock reads in `src/security`; fixtures use **2026-01-15T00:00:00Z** | MUST |
| 5 | A2 reproducibility: two generator runs → unsigned parts byte-identical; signatures verify; private-key filenames contain `TEST-ONLY`; `rg -n 'TEST-ONLY' src/cli src/viewer src/core src/security` (product/security code) does not load those key paths for verify (keys used only by `tools/` generator) | MUST |
| 6 | Build: `find_package(OpenSSL 3.0 REQUIRED)`; `OPENSSL_API_COMPAT=30000` **and** `OPENSSL_NO_DEPRECATED` on `s100_security`; Windows missing `libcrypto-3-x64.dll` FAIL-fast; CTest `exchange_signature` no SKIP both platforms | MUST |
| 7 | `VerifyPolicy::Off` / `--allow-unsigned` grep empty in `src/cli` + `src/viewer`; CLI `VerifyPolicy::Enforce` ≥ 1; viewer may have 0 `open_exchange_set` today, but any new viewer call site must pass `Enforce` | MUST |
| 8 | OpenSSL include grep empty outside `src/security/*.cpp` | MUST |
| 9 | SSE mapping: only `sse_map` TU contains integer SSE codes used for Part 15 outcomes | MUST |
| 10 | Named-deviation frozen files + old fixture bytes: for `test_update_robustness.cpp`, `test_cancel_reissue.cpp`, `test_edition_hardening.cpp`, `git diff` shows **only** `VerifyPolicy::Off` call-args (and maybe an include) — **no** assertion or fixture-byte semantic changes; existing unsigned CATALOG trees listed in A8 remain byte-identical | MUST |
| 11 | Existing 34-test set still green with every exchange-set import call site passing `VerifyPolicy::Off` **explicitly** (Linux + Windows); library default remains Enforce | MUST |
| 12 | Standalone CLI/viewer `.000` shows `UNVERIFIED` | WARN |
| 13 | Local optional IHO_SIGNED PC and CHS SSE 122: print `exercised` / `not exercised` per TEST-DATA-ROOTS; never SKIP | WARN |

## Non-goals

- AES encryption / decryption, permits (`PERMIT.XML` / `.SIGN`), User Permit / HW_ID / M_KEY (SSE 110–121 / 125).
- `dataStatus` = compressed / encrypted payload handling beyond skip/unsupported.
- SignatureOnSignature chains (15-8.8) — Backlog; if encountered in Enforce, prefer verify-all (S-98 B-3) or reject if unsupported (Engineer picks one and documents in the PR; default = reject with 109).
- SA/DS issuance flows (CSR, SSE 101–104).
- S-63 / S-57 DSA signatures.
- Online CRL / OCSP revocation.
- “Validated system date” / GNSS time (SSE 114).
- 15-8.10 MRN discovery.
- Producer-code matching (CN MRN ↔ dataset) — **D8** → **`IHO-S164-SIG-PRODUCER`** (`docs/part15-producer-code-contract.md`); not in this SIG slice.
- Official/non-official **display styling** beyond exposing `OfficialClass`.
- Re-signing or modifying pre-slice fixture bytes.

## Open points

1. Exact UI copy for coalesced SSE prompts (S-98 allows combining) — Viewer can ship a single status-bar line listing codes in this slice.
2. Whether `parse_exchange_catalogue` should expose certificate / signature XML fields now or only via the security module’s own XML parse of `CATALOG.SIGN` + re-read of discovery metadata — **prefer security module reads what it needs** so `s100_core` catalogue parse stays unchanged this slice (smaller freeze risk).
3. S-164 2.0.1 draft SSE numbering may change — mitigated by D5 single map.
4. IHO root redistributability **【未核实】** in BRIEF — embedding the publicly published certificate is accepted for this project; do not commit S-164 `IHO_SIGNED` PC (no LICENSE).
5. Part 15 §15-8.4 vs Part 17 NOTE 2 compression-order conflict — moot while we only handle unencrypted on-disk bytes (D3).

## Unverified BRIEF items this contract depends on

| BRIEF tag | Dependency | How the contract treats it |
|-----------|------------|----------------------------|
| IHO root redistributability 【未核实】 | Built-in trust anchor | Embed; low risk (public cert). Do not claim a redistrib license text we do not have. |
| S-164 IHO_SIGNED / v2.0.0 LICENSE 【未核实】 | Positive real-SA sample | Local optional only; CI does not download; not committed (A3). |
| S-98 Figures B-1/B-4/B-5 pixels vs formal PDF 【未核实】 | Check order / SSE | Follow **checked** B-1/B-2 text + Planner tables; figures treated as supportive. |
| S-164 2.0.1 draft SSE / clause churn | 116/124, test names | D5 single map; map 116/124 → 109. |
| Part 15 vs Part 17 sign-before/after compress conflict | D3 | On-disk bytes only; unencrypted. |
| mbedTLS CI cost 【未实测】 | Library choice | OpenSSL chosen; mbedTLS not used. |

