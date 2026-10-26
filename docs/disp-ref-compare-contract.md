# Display Test Template / reference-implementation screenshots (rev 1) · Active / Open

Owner: SoftwareArchitecture (2026-10-26). Implement: SoftwareEngineer. Test: SoftwareTester.
Single source of truth with `docs/testing/visual-acceptance.md` §A EncDotNet / §B NIWC (cross-link both ways).
Sibling Closed contract `docs/s164-display-compare-contract.md` (`IHO-S164-DISPLAY-COMPARE`) explicitly put **EncDotNet / NIWC compare = Out of that slice** — this tip is the complementary landing; **do not reopen** DISPLAY-COMPARE.

Case id: **`IHO-DISP-REF-COMPARE`** (**Active / Open**). CTest: **`viewer_disp_ref_compare`**.
**Status: Active / Open (2026-10-26)** — Planner freeze after **EDITION-HARDEN Closed** tip `e60efd2` (Close CI [`40254220444`](https://github.com/MasterLivens/S100/actions/runs/40254220444) dual-green; Open-bind [`40199743304`](https://github.com/MasterLivens/S100/actions/runs/40199743304) @ `bbc4b64`≡`312c8ae`). Promoted from Backlog (case-ids former L60). **Eng tip-truth TBD** — predecessor Eng `96149a0` (EncDotNet soft-oracle toolchain + optional `viewer_encdotnet_oracle` default OFF / WARN-only / not in `ci.yml`) is **ancestor tooling only**, **≠** this case tip-truth. Likely **needs new Eng** after APPROVE (register tip CTest + Ubuntu strategy); **not** docs-close-only.

**Planner freeze (rev 1 — Open nail):**
- Reuse case id **`IHO-DISP-REF-COMPARE`**; **forbidden** to invent a second case id.
- CTest name **nailed:** **`viewer_disp_ref_compare`** (snake_case; `viewer_*` snapshot/oracle family, sibling pattern `viewer_s164_display_compare`). **Do not** steal-bind Sibling `viewer_s164_display_compare` / Reuse `viewer_golden`. Predecessor optional `viewer_encdotnet_oracle` stays OFF/WARN tooling under `S100_ENCDOTNET_ORACLE` — Eng may wrap/reuse its scripts into the tip CTest under the **new** name; do **not** silently rename or delete the optional helper without Eng ack.
- Goal: **Display Test Template / reference-implementation screenshots** — alternative to talking about portrayal feel via pixel-drift alone (relative to `viewer_golden` 25% cap and DISPLAY soft-pixel WARN).
- Scope **(iii):** EncDotNet Linux soft-compare automation (interim; tip CTest face) **+** NIWC human checklist (final preferred; manual Windows / registry login; **not** CI CTest). Automation MUST face is EncDotNet path; NIWC stays manual evidence.
- Ubuntu: tip CTest **`viewer_disp_ref_compare`** — **MUST NOT SKIP** on Linux CI once Eng lands tip-truth; **Forbidden:** platform SKIP inside the test itself.
- **Win O1=(a) Ubuntu-bind (chosen):** Close binds Ubuntu tip-CTest Passed + Win full matrix green; Notes Win `viewer_*` exclude (same family as golden/snapshot / DISPLAY-COMPARE). **No** `ci.yml` change unless Arch later chooses (b). **(b)** narrow-open of only this CTest **not** chosen; hang→fallback (a); do **not** pull `viewer_golden` into Win CI. NIWC manual does **not** go through Win CI CTest.
- After Closed next: **TBD** (architecture / registered Active — **do not** invent). `S102-GRID-SHIM-RETIRE` stays Backlog — **do not** insert ahead. Defer `IHO-S101-LOAD-0004..0010`.

## MUST (hard fail, exit ≠ 0) vs WARN-only

| Check | Severity | Mechanism |
|-------|----------|-----------|
| Tip CTest **`viewer_disp_ref_compare`** registered and runs on Ubuntu (MUST NOT SKIP) once Eng lands | **MUST** | CTest / cmake |
| EncDotNet pin + provenance: `tools/encdotnet-s100/SOURCE.md` present; pin **v0.24.0** matches visual-acceptance / SOURCE | **MUST** | file / sha pin check |
| Fetch/render/dual-capture scripts present and invocable (`scripts/fetch_encdotnet_s100.sh`, `render_encdotnet_oracle.sh`, `dual_capture_oracle.sh`, `soft_compare_oracle.py`) | **MUST** | path existence / help or dry plumbing Eng chooses |
| Dual-capture produces non-empty our-side + oracle-side PNG when tip CTest exercises the path (Eng defines fixture cell; prefer IHO TDS 0008 Day 1280×800) | **MUST** | CTest / script |
| Soft pixel drift vs EncDotNet oracle PNG | **WARN-only** | `scripts/soft_compare_oracle.py` defaults (exit 0 unless `--strict`); **do not** default `--strict` / harden soft unless a future Arch Open |
| NIWC ShoreECDIS / S-100 Viewer screenshot checklist (human) | **WARN / manual** | Tester attachment / Notes — **not** automated download; **not** Win CI CTest |
| EncDotNet / NIWC compare under DISPLAY-COMPARE | **Out of DISPLAY slice** (Closed) | This tip owns that face — **do not** reopen `IHO-S164-DISPLAY-COMPARE` / change `viewer_s164_display_compare` oracles / AA5 refs / presets |

CTest target `viewer_disp_ref_compare`: fails only on MUST rows. Soft-compare annotations print as warnings. Failure face = bare CHECK / cmake `WARNING`/`FATAL_ERROR` / existing script exit — **do not** invent `DISPREF-*` / `DISPREFCMP-*` prefixes.

## Predecessor vs tip-truth

| Item | Role |
|------|------|
| Eng `96149a0` | EncDotNet soft-oracle toolchain + visual-acceptance §A/B; optional `viewer_encdotnet_oracle` (OFF / always-success WARN / not in `ci.yml`) — **reusable ancestor**, **≠** tip-truth for this case |
| Sibling Eng `12ae582` / tip `ba63833` / Close CI `39133583148` | DISPLAY-COMPARE S-164 AA5 — **Closed**; **do not reopen** |
| This tip Eng | **TBD / new** after APPROVE — land `viewer_disp_ref_compare` tip-truth (register + Ubuntu MUST NOT SKIP strategy; soft pixel stays WARN) |

## Goal

Automate / evidence **reference-implementation screenshots** (EncDotNet interim Linux soft-compare + NIWC final manual) against our Day snapshot path.  
**Not** type-approval; **not** a replacement for `viewer_golden`; **not** a reopen of S-164 AA5 DISPLAY-COMPARE.

## Capture / oracle settings (rev 1 freeze)

- Day, default 1280×800 for our shot (align golden / EncDotNet scripts).
- EncDotNet pin **v0.24.0** per `tools/encdotnet-s100/SOURCE.md` / visual-acceptance.
- Soft-compare: reuse `scripts/soft_compare_oracle.py` (default threshold **0.90** drift fraction); WARN-only unless `--strict`.
- Do **not** vendor EncDotNet tarball / oracle PNG into git (remain gitignored) unless a future Arch Open asks.
- Do **not** automate NIWC download (visual-acceptance forbids).

## Win / CI policy

| Path | Win @ tip `e60efd2` / this Open | Notes |
|------|--------------------------------|-------|
| `viewer_text_collision` / `viewer_point_label_collision` | narrow-open (b) already Closed | dump-only — **does not** cover this tip |
| `viewer_s164_display_compare` / `viewer_golden` / snapshot | excluded | DISPLAY O1=(a) / hang family |
| **`viewer_disp_ref_compare` (this tip)** | **O1=(a)** exclude | Ubuntu-bind; no `ci.yml` change |
| NIWC manual | not a CI CTest | Windows human only |

## Docs sync

- Case id in `docs/testing/iho-ctest-case-ids.md` — **Active / Open** this tip (promoted from Backlog); flip Closed on docs close after Eng tip-truth + CI.
- Cross-link `docs/testing/visual-acceptance.md` EncDotNet/NIWC rows → this contract.
- `docs/architecture.md` **next** → this Active / Open.

## Close criteria (honest — after Eng)

- Eng tip-truth lands `viewer_disp_ref_compare` (MUST rows green on Ubuntu; MUST NOT SKIP).
- Ubuntu CI Passed for tip CTest; Win full matrix green under O1=(a) exclude Notes.
- Docs → Closed + CI Notes; soft pixel remains WARN unless a later Arch Open hardens.
- After Close next: **TBD**. Do **not** reopen DISPLAY-COMPARE / EDITION / CANCEL / UPD-SEQ / UPD-CORRUPT / SIG family / FASC / BRIDGE; shim Backlog; LOAD deferred.

## Non-goals

Reopen DISPLAY-COMPARE / change AA5 refs / presets / `viewer_s164_display_compare` oracles; silently harden EncDotNet/NIWC soft to hard gate / default `--strict`; tighten `viewer_golden` 25% cap; automate NIWC download / vendor NIWC binaries; vendor EncDotNet tarball or oracle PNG into git; reopen EDITION-HARDEN / CANCEL-REISSUE / UPD-SEQ / UPD-CORRUPT / SIG family / FASC / BRIDGE / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC / UNCERT / DEEPEN; insert `S102-GRID-SHIM-RETIRE` ahead; pull `IHO-S101-LOAD-0004..0010`; invent case ids / `DISPREF-*` failure prefixes / Closed-next tip name; Win narrow-open (b) this slice; pull `viewer_golden` into Win CI; treat predecessor `96149a0` / `viewer_encdotnet_oracle` as already Closed tip-truth for this case.
