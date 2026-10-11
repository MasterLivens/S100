# S-164 display compare automation (rev 2)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Supersedes rev 1. Single source of truth with `docs/testing/visual-acceptance.md` § S-164 (cross-link both ways).

Case id: **`IHO-S164-DISPLAY-COMPARE`**.

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
- Add case id to `docs/testing/iho-ctest-case-ids.md` when implementing.

## Non-goals

Pixel-identical AA5; NIWC; point-symbol text collision; tightening `viewer_golden` 25% cap.
