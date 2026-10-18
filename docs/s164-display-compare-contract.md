# S-164 display compare automation (rev 2) (Closed)

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Supersedes rev 1. Single source of truth with `docs/testing/visual-acceptance.md` § S-164 (cross-link both ways).

Case id: **`IHO-S164-DISPLAY-COMPARE`** (**Closed / Done** 2026-10-18). CTest: **`viewer_s164_display_compare`** — MUST NOT SKIP on Ubuntu; Linux + Windows dual-green (CI `39133583148` @ `ba63833`).
**Status: Closed / Done (2026-10-18)** — CI [`39133583148`](https://github.com/MasterLivens/S100/actions/runs/39133583148) @ `ba63833` dual-green; **O1=(a)** Ubuntu-bind: Close binds Ubuntu `viewer_s164_display_compare` MUST NOT SKIP Passed + Win full matrix green; Notes Win `viewer_*` exclude (same family as golden/snapshot). No `ci.yml` change. Eng `12ae582` already on tip — **docs close only**. Oracles **rev2 unchanged** (MUST: SHA+SOURCE, snapshot, Day RGB anchors, presets on/off via dump; soft pixel WARN; heuristic OK). **BRIDGE #13** pixel defer lands here as **soft WARN only** (not a hard BRIDGE CTest MUST). Next: **`IHO-S102-DEEPEN-MULTIGRID`**.

**Planner freeze (kept — rev2 unchanged):** Reuse case id / CTest name; MUST = ref PNGs + `SOURCE.md` + SHA; offscreen snapshot; Day RGB anchors `(97,183,255)`, `(191,190,143)`, `(255,255,255)`, `(147,174,187)`; `display-presets.yaml` required_on/off via dump; WARN-only soft pixel + heuristic cell; Day 1280×800; `--no-text-collision`; CTest does **not** pass `--strict`.

**O1 closed as (a):** Ubuntu-bind + Notes Win `viewer_*` exclude (snapshot family — TEXT/POINT (b) dump-only does **not** cover). (b) narrow-open of only this CTest **not** chosen. No Eng `ci.yml` change. **Forbidden:** platform SKIP inside the test itself.

## Slice close — 2026-10-18

**Status: Closed / Done** (formal docs close — **not** an algorithm / driver rewrite).

- Evidence: CI [39133583148](https://github.com/MasterLivens/S100/actions/runs/39133583148) tip `ba63833` — Ubuntu **`viewer_s164_display_compare` Passed** (MUST NOT SKIP) + Win full matrix green (O1=a).
- **O1=(a) SUCCESS** — Ubuntu bind; Win Notes exclude (`viewer_*` / same family as golden/snapshot); no `ci.yml` change.
- Oracles stay as frozen rev2 MUST/WARN (anchors + presets dump; soft pixel WARN; heuristic OK).
- **BRIDGE #13 soft WARN note:** pixel defer from BRIDGE lands here as soft WARN only — not a hard BRIDGE CTest MUST.
- **Notes:** next Eng **`IHO-S102-DEEPEN-MULTIGRID`** (`s102_deepen_multigrid`).
- **Non-goals this close:** inventing oracles; Win narrow-open (b); pulling `viewer_golden` into Win CI; algorithm/driver rewrite; PC Lua / SVG; true AA5 cell / viewing_group wiring (Backlog).

## Goal

Automate Base / Standard display checks against `testdata/golden/s164-ref/AA5DBASE.png` & `AA5STNDR.png`.  
**Not** type-approval; **not** a replacement for `viewer_golden`.

## MUST (hard fail, exit ≠ 0) vs WARN-only

| Check | Severity | Mechanism |
|-------|----------|-----------|
| Ref PNGs + `SOURCE.md` present; SHA matches `SHA256SUMS.txt` | **MUST** | CTest / script |
| Our offscreen snapshot produced (exit 0, non-empty PNG) | **MUST** | |
| Day palette **RGB anchors** present in our snapshot (same four tuples as `tests/compare_viewer_golden.py`) | **MUST** | Reuse golden anchor logic — **not** a free-form token name list |
| `display-presets.yaml` loadable; for chosen preset, every `required_on` layer code is on / every `required_off` is off in the session used for the shot | **MUST** | Layer visibility dump from CLI or `--dump-layers` |
| Soft pixel drift vs AA5* ref | **WARN-only** | `scripts/soft_compare_oracle.py` defaults (exit 0 unless `--strict`) |
| EncDotNet / NIWC compare | Out of this slice | See visual-acceptance.md |

CTest target `viewer_s164_display_compare`: fails only on MUST rows. Soft-compare annotations print as warnings.

## Colour assertion (pinned)

- Use **RGB anchors** identical to `viewer_golden`: `(97,183,255)`, `(191,190,143)`, `(255,255,255)`, `(147,174,187)`.
- Do **not** mix in symbolic token-name presence checks in this CTest.
- Token→RGB remains the palette path; this test only asserts the rendered Day anchors.

## Layer checklist (pinned file)

`testdata/golden/s164-ref/display-presets.yaml` (stub checked in with this contract).  
Eng may extend lists with Tester ack; emptying `required_on` is not allowed.

## Soft pixel + size mismatch

- Reuse `scripts/soft_compare_oracle.py` (default threshold **0.90** drift fraction, same as EncDotNet oracle).
- AA5 refs are **not** 1280×800 (`AA5DBASE` ≈ 579×582, `AA5STNDR` ≈ 655×523). Before compare: **letterbox-scale our snapshot to the ref size** (keep aspect, pad with Day background or center-crop — pick one, document in script help; default = **aspect-fit + pad** with `(255,255,255)`).
- Warn-only unless `--strict`.

## Capture settings

- Day, default 1280×800 for our shot; then scale for soft-compare.
- Text collision: pass `--no-text-collision` for stable boxes (same as golden).
- Cell: prefer matching S-164 AA5 cell if added under testdata; else `101AA00DS0008.000` and label report `heuristic=true`.

## Docs sync

- Update `docs/testing/visual-acceptance.md` S-164 row: “automation per `docs/s164-display-compare-contract.md`; MUST = anchors + presets; soft pixel WARN”.
- Case id in `docs/testing/iho-ctest-case-ids.md` — **Closed / Done** this tip.

## Non-goals

Pixel-identical AA5; NIWC; EncDotNet hard gate; point-symbol text collision; tightening `viewer_golden` 25% cap; default CTest `--strict`; inventing new oracles; Win narrow-open (b) this slice; pulling `viewer_golden` into Win CI; reopening TEXT/POINT/BRIDGE; PC Lua / SVG edits; true AA5 cell / viewing_group wiring (Backlog).
