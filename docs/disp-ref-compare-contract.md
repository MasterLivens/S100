# Display Test Template / reference-implementation screenshots (rev 1) (Closed — DISP-REF-COMPARE)

Owner: SoftwareArchitecture (2026-10-26). Implement: SoftwareEngineer. Test: SoftwareTester.
Single source of truth with `docs/testing/visual-acceptance.md` §A EncDotNet / §B NIWC (cross-link both ways).
Sibling Closed contract `docs/s164-display-compare-contract.md` (`IHO-S164-DISPLAY-COMPARE`) explicitly put **EncDotNet / NIWC compare = Out of that slice** — this tip is the complementary landing; **do not reopen** DISPLAY-COMPARE.

Case id: **`IHO-DISP-REF-COMPARE`** (**Closed / Done** 2026-10-26). CTest: **`viewer_disp_ref_compare`** — MUST NOT SKIP on Ubuntu; Linux + Windows dual-green (Eng CI [`40292051452`](https://github.com/MasterLivens/S100/actions/runs/40292051452) @ Eng tip-truth `3ba1329`). Supersedes Open draft on tip `4852fc9` (≡ `581fe61`; Open CI [`40257119938`](https://github.com/MasterLivens/S100/actions/runs/40257119938)).

**Status: Closed / Done (2026-10-26)** — Eng tip-truth **`3ba1329`**; Eng CI [`40292051452`](https://github.com/MasterLivens/S100/actions/runs/40292051452) dual-green; Open `4852fc9`≡`581fe61` / Open CI [`40257119938`](https://github.com/MasterLivens/S100/actions/runs/40257119938); **`viewer_disp_ref_compare` Passed Ubuntu MUST NOT SKIP** (45/45); **Win O1=(a)** Ubuntu-bind: Close binds Ubuntu tip-CTest Passed + Win full matrix green; Notes Win `viewer_*` exclude (same family as golden/snapshot / DISPLAY-COMPARE; Win O1=`ctest -E viewer_`). Soft pixel stays **WARN**. Failure face = bare CHECK / cmake `WARNING`/`FATAL_ERROR` / existing script exit — **do not** invent `DISPREF-*` / `DISPREFCMP-*`. Do **not** reopen DISPLAY-COMPARE / EDITION / CANCEL / UPD / SIG / FASC / BRIDGE; `S102-GRID-SHIM-RETIRE` stays Backlog; LOAD-0004..0010 deferred. Contract display stays **rev 1**. Next: **TBD**.

**Planner freeze (kept — rev 1 tip-truth):** Reuse case id **`IHO-DISP-REF-COMPARE`**; CTest **`viewer_disp_ref_compare`**; scope EncDotNet Linux soft-compare (interim tip CTest face) + NIWC human checklist (manual; not CI); Ubuntu MUST NOT SKIP; **Win O1=(a)**; soft pixel WARN; predecessor `96149a0` / optional `viewer_encdotnet_oracle` OFF/WARN = ancestor tooling only ≠ tip-truth; Eng tip-truth `3ba1329` landed; failure face = bare CHECK (do **not** invent `DISPREF-*`); DISPLAY-COMPARE already Closed (do **not** reopen); shim Backlog; after Closed next **TBD** (do **not** invent).

**O1 closed as (a):** Ubuntu-bind + Notes Win `viewer_*` exclude (snapshot/oracle family — same as DISPLAY-COMPARE / golden). (b) narrow-open of only this CTest **not** chosen; hang→fallback (a); do **not** pull `viewer_golden` into Win CI. No Eng `ci.yml` change. **Forbidden:** platform SKIP inside the test itself.

---

## Slice close — DISP-REF-COMPARE 2026-10-26

**Status: Closed / Done** (formal docs close after Eng tip-truth — **not** a DISPLAY-COMPARE reopen / soft-harden).

- Evidence: Eng tip-truth `3ba1329`; Eng CI [40292051452](https://github.com/MasterLivens/S100/actions/runs/40292051452) — Ubuntu **`viewer_disp_ref_compare` Passed** (MUST NOT SKIP, 45/45) + Win full matrix green (O1=a `ctest -E viewer_`). Open tip `4852fc9`≡`581fe61` / Open CI [40257119938](https://github.com/MasterLivens/S100/actions/runs/40257119938) dual-green.
- **O1=(a) SUCCESS** — Ubuntu bind; Win Notes exclude (`viewer_*` / same family as golden/snapshot / DISPLAY-COMPARE); no `ci.yml` change.
- MUST tip-truth unchanged (EncDotNet pin v0.24.0 + SOURCE.md; fetch/render/dual-capture/soft_compare scripts; dual-capture non-empty PNGs; tip CTest registered Ubuntu MUST NOT SKIP; soft pixel WARN; NIWC manual). Soft pixel remains WARN — do **not** default `--strict`.
- Failure face stays bare CHECK / cmake WARNING/FATAL / existing script exit — do **not** invent `DISPREF-*` / `DISPREFCMP-*`.
- **Notes:** next **TBD** (architecture / registered Active — do **not** invent). **Backlog:** `grid()` shim retire (`S102-GRID-SHIM-RETIRE`) — do not insert ahead. LOAD-0004..0010 deferred. Contract display stays **rev 1**. DISPLAY-COMPARE already Closed (do **not** reopen); EDITION / CANCEL / UPD / SIG / FASC / BRIDGE already Closed (do **not** reopen).
- Boundaries: complementary to DISPLAY-COMPARE (EncDotNet/NIWC were out-of-slice there); ≠ DISPLAY-COMPARE (already Closed — **do not** reopen); ≠ EDITION / CANCEL / UPD / SIG / FASC / BRIDGE (already Closed).
- **Non-goals this close:** inventing oracles / next tip id / `DISPREF-*` prefixes; Win narrow-open (b); pulling `viewer_golden` into Win CI; silently harden soft pixel to hard gate / default `--strict`; reopen DISPLAY-COMPARE / change AA5 refs / presets / `viewer_s164_display_compare`; automate NIWC download / vendor EncDotNet or NIWC binaries; insert `S102-GRID-SHIM-RETIRE` ahead; pull LOAD-0004..0010; reopen EDITION / CANCEL / UPD / SIG / FASC / BRIDGE.

---

## MUST (hard fail, exit ≠ 0) vs WARN-only

| Check | Severity | Mechanism |
|-------|----------|-----------|
| Tip CTest **`viewer_disp_ref_compare`** registered and runs on Ubuntu (MUST NOT SKIP) | **MUST** | CTest / cmake |
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
| Eng tip-truth `3ba1329` / Eng CI `40292051452` | Landed `viewer_disp_ref_compare` (register + Ubuntu MUST NOT SKIP; soft pixel stays WARN) — **Closed** this tip |

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

| Path | Win @ Closed Eng tip `3ba1329` | Notes |
|------|--------------------------------|-------|
| `viewer_text_collision` / `viewer_point_label_collision` | narrow-open (b) already Closed | dump-only — **does not** cover this tip |
| `viewer_s164_display_compare` / `viewer_golden` / snapshot | excluded | DISPLAY O1=(a) / hang family |
| **`viewer_disp_ref_compare` (this tip)** | **O1=(a)** exclude | Ubuntu-bind; no `ci.yml` change |
| NIWC manual | not a CI CTest | Windows human only |

## Docs sync

- Case id in `docs/testing/iho-ctest-case-ids.md` — **Closed / Done** this tip.
- Cross-link `docs/testing/visual-acceptance.md` EncDotNet/NIWC rows → this contract.
- `docs/architecture.md` **next** → **TBD**.

## Non-goals

Reopen DISPLAY-COMPARE / change AA5 refs / presets / `viewer_s164_display_compare` oracles; silently harden EncDotNet/NIWC soft to hard gate / default `--strict`; tighten `viewer_golden` 25% cap; automate NIWC download / vendor NIWC binaries; vendor EncDotNet tarball or oracle PNG into git; reopen EDITION-HARDEN / CANCEL-REISSUE / UPD-SEQ / UPD-CORRUPT / SIG family / FASC / BRIDGE / DISPLAY-PRIORITY / SCAMIN / SESSION-PICK / CATALOGUE / TIMELINE / PC-WIRE / S-124 / S-111 / S-104 / QBC / UNCERT / DEEPEN; insert `S102-GRID-SHIM-RETIRE` ahead; pull `IHO-S101-LOAD-0004..0010`; invent case ids / `DISPREF-*` failure prefixes / Closed-next tip name; Win narrow-open (b) this slice; pull `viewer_golden` into Win CI; treat predecessor `96149a0` / `viewer_encdotnet_oracle` as already Closed tip-truth for this case without Eng `3ba1329`.
