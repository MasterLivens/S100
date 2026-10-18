# S-164 display compare automation (rev 2) · Active / Open

Owner: SoftwareArchitecture (2026-10-18). Implement: SoftwareEngineer. Test: SoftwareTester.
Supersedes rev 1. Single source of truth with `docs/testing/visual-acceptance.md` § S-164 (cross-link both ways).

Case id: **`IHO-S164-DISPLAY-COMPARE`** (**Active / Open**). CTest: **`viewer_s164_display_compare`**.

**Status: Active / Open (2026-10-18)** — Planner freeze after **POINT-LABEL Closed**. Eng tip base: **POINT-LABEL Closed tip on `origin/main`** (local Close `1496b55` / stack after `9a1531c`; **Eng waits Close push** before stacking). Implementation already on tip ancestor (`12ae582` dump-layers / driver / CTest — **verify / docs close**, not greenfield). Ubuntu tip-ancestor CI already Passed (`viewer_s164_display_compare` #44 @ TEXT tip `845c51c`); local Passed. This slice = **docs close primarily** + **Win policy O1=(a)**. Independent tip from POINT-LABEL. Do **not** invent oracles; rev2 MUST/WARN unchanged. No algorithm / driver rewrite. Do **not** pull `viewer_golden` into Win narrow-open. BRIDGE #13 pixel defer lands here (soft WARN only — not a hard BRIDGE CTest MUST).

**Planner freeze (MUST — from rev2, unchanged):**
- Reuse case id / CTest name; **forbidden** to invent a second case/test name.
- MUST: ref PNGs + `SOURCE.md` present; SHA matches `SHA256SUMS.txt`; offscreen snapshot produced (exit 0, non-empty PNG); Day palette **RGB anchors** `(97,183,255)`, `(191,190,143)`, `(255,255,255)`, `(147,174,187)` (same as `viewer_golden` — **not** token-name list); `display-presets.yaml` loadable; every `required_on` on / every `required_off` off via dump (`--dump-layers` / CLI); `required_on` must stay non-empty.
- WARN-only: soft pixel drift vs AA5* via `scripts/soft_compare_oracle.py` (default 0.90; letterbox aspect-fit + pad `(255,255,255)` to AA5 size); heuristic cell OK (`101AA00DS0008.000` + `heuristic=true` until true AA5 cell).
- Capture: Day 1280×800; `--no-text-collision`; CTest does **not** pass `--strict`.
- Ubuntu CI: `viewer_s164_display_compare` MUST NOT SKIP, Passed.
- After Closed next: **`IHO-S102-DEEPEN-MULTIGRID`**.

**O1 = (a) Ubuntu-bind (chosen this slice):** Win CI continues to exclude this test under `-E "viewer_"` (TEXT/POINT O1=(b) narrow-open of dump-only tests does **not** cover snapshot+dump-layers+Pillow). Close binds on Ubuntu Passed + full Win matrix green; Notes record exclude (same family as `viewer_golden` / snapshot hang risk). **(b) narrow-open of only `viewer_s164_display_compare` is not chosen this slice** (hang→a would still apply if revisited later; do **not** pull in `viewer_golden`). **Forbidden:** platform SKIP inside the test itself.

**Close criteria (honest):**
- Ubuntu: `viewer_s164_display_compare` MUST NOT SKIP, Passed (already on tip ancestors; re-check after POINT Close on tip)
- Windows: O1=(a) → full matrix green is enough (Notes record Win `viewer_*` exclude for this CTest)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` change required for (a)
- After Close next: **`IHO-S102-DEEPEN-MULTIGRID`**

**Eng notes:** verify `12ae582` on tip; fixtures under `testdata/golden/s164-ref/`; driver `tests/s164_display_compare.py`; cmake `tests/check_s164_display_compare.cmake`. POINT-LABEL Closed must land on `origin/main` before Eng stacks this Open (docs close / Notes). heuristic WARN / soft base drift WARN are expected this slice — not fail. True AA5 cell / viewing_group_hint wiring / presets tighten → **Backlog**.

**Notes (Win CI):** Existing policy after TEXT+POINT: Windows `ctest -E "viewer_"` then narrow-open only `^viewer_text_collision$` and `^viewer_point_label_collision$`. This CTest stays excluded (snapshot family). O1=(a) documents that; (b) not selected.

**O-questions (real gaps only — Planner-resolved where noted):**
1. **Win policy — resolved O1=(a).** Prefer Ubuntu-bind + Notes exclude (align golden/snapshot). Do not treat TEXT/POINT (b) as coverage.
2. heuristic / soft drift WARN this slice — **accept** (contract already WARN-only).
3. `viewing_group_hint` YAML comments unused (driver uses `--layers-on/off`) — **accept MVP**; wiring = Backlog.
4. True AA5 cell + presets tighten — **Backlog**, not this Open MUST.

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
- Case id in `docs/testing/iho-ctest-case-ids.md` — **Active / Open** this tip; flip Closed on docs close.

## Non-goals

Pixel-identical AA5; NIWC; EncDotNet hard gate; point-symbol text collision; tightening `viewer_golden` 25% cap; default CTest `--strict`; inventing new oracles; Win narrow-open (b) this slice; pulling `viewer_golden` into Win CI; reopening TEXT/POINT/BRIDGE; PC Lua / SVG edits; true AA5 cell / viewing_group wiring (Backlog).
