# S-111 surface currents — first slice (rev 2) · Active / Open

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Case id: **`IHO-S111-OPEN-FRAME`** (**Active / Open**). CTest: **`s111_open_frame`** — **MUST NOT SKIP**, Linux + Windows CI.
Status: **rev 2** — Planner freeze (MUST below). First S-111 open/frame slice after closed **`IHO-S104-OPEN-FRAME`**. Does **not** reopen S-104 / QBC-MG / UNCERT / DEEPEN; does **not** mix **`IHO-TIMELINE-UI`** / **`IHO-PC-WIRE-111-124`** / **`IHO-CATALOGUE-COMPLETE`** / **`IHO-S124-OPEN-LIST`**; does **not** jump **`S102-GRID-SHIM-RETIRE`**.

Builds on `docs/api-contracts.md` (S-111) / `docs/catalogue-complete-contract.md` (S-111 **FC+PC** MUST — ≠ S-104 FC-only). Scrubber / `--list-frames` / `timeline_entries` → separate Active **`IHO-TIMELINE-UI`** (`docs/timeline-ui-contract.md`). PC wiring (SCAROW / ColorProfiles / XSLT WARN / no invented Lua) → separate Active **`IHO-PC-WIRE-111-124`** (`docs/pc-wire-contract.md`) — **do not** close PC-WIRE this tip.

Primary research: `/workspace/research/s111-open-frame/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`858d51a`**.

**Status: Active / Open (2026-10-25)** — after **S-104 Closed** (tip `2504c72` ≡ Arch `cc5a62d`; tip CI [`40160836622`](https://github.com/MasterLivens/S100/actions/runs/40160836622) dual-green; formal Close CI bind [`40159993218`](https://github.com/MasterLivens/S100/actions/runs/40159993218) @ `28e0f2b`≡`06da68d`, O1=tip-truth/docs-close). Eng tip ancestor **`858d51a`** already on tip — **not greenfield** (HDF5 open → `Dataset::current`; ≥2 frames; frame API; sample 0.2/0.2 → frame0≈**0.5 m/s / 90°** ≠ frame1≈**1.5 m/s / 180°**; portray `Kind::Arrow` + stride **`kS111ArrowStride=2`** (5×5 → count=9); ChartSession S-101+S-111; fixture `synthetic_current_4326.h5`; CTest **`s111_open_frame`**). Ubuntu+Windows already Passed — tip CI [`40160836622`](https://github.com/MasterLivens/S100/actions/runs/40160836622) `#8` + Eng landing [`38127367335`](https://github.com/MasterLivens/S100/actions/runs/38127367335) @ `858d51a` (native `s100_tests` + HDF5, **not** `viewer_*` — **no Win O1 (a)/(b)**). This slice = **Arch docs close / verify primarily** (Open→Closed), not greenfield Eng. Independent tip from S-104 / QBC-MG / UNCERT / DEEPEN / TIMELINE-UI / PC-WIRE / CATALOGUE-COMPLETE / S-124. Do **not** invent case ids or next tip names; freeze tip-truth below. Do **not** reopen S-104 / QBC-MG / UNCERT / DEEPEN. Do **not** write this tip as S-104-style FC-only — **FC+PC on disk** (catalogue-complete); PC **wiring** stays PC-WIRE separate. Do **not** invent `S111-*` failure-face prefixes (keep existing bare CHECKs). Do **not** retire `grid()` shim this tip (DEEPEN O1=Backlog). Contract display header stays **rev 2** (do **not** bump just for Open). After Closed next: **TBD** (architecture / registered Active only — do **not** invent).

**Planner freeze (MUST — from BRIEF §7 + tip truth; Arch O1 = tip-truth / docs-close freeze yes):**
- Reuse case id **`IHO-S111-OPEN-FRAME`** / CTest **`s111_open_frame`**; **forbidden** to invent a second case/test name.
- MUST = open≥2 frames + frame API (`set_active_frame` / `set_active_time` / `sample_wgs84` / `bounds`) + sample 0.2/0.2 frame0≈**0.5 m/s / 90°** ≠ frame1≈**1.5 m/s / 180°** + portray ≥1 `Kind::Arrow` + stride **`kS111ArrowStride=2`** (5×5 → count=**9**) + ChartSession S-101+S-111 + **FC+PC on disk** (catalogue-complete; ≠ S-104 FC-only) + soft pixel **WARN-only**.
- Data model: `Dataset::current` / `CurrentDataset`; **forbid** reuse `Dataset::coverage` / `water_level` for S-111.
- Fixture `testdata/s111/synthetic_current_4326.h5` **MUST NOT SKIP** on CI (Linux+Win).
- Scrubber / `--list-frames` / `timeline_entries` → **`IHO-TIMELINE-UI`** (separate tip; **not** this case).
- PC wiring (SCAROW / ColorProfiles / XSLT / no invented Lua) → **`IHO-PC-WIRE-111-124`** (separate tip; **do not** close this tip).
- Eng tip already has **`858d51a`**; Ubuntu+Win (tip `40160836622` `#8` / Eng `38127367335`) Passed → this slice = **Arch docs close** (Open→Closed), not greenfield.
- **`grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog** (DEEPEN O1); **not** this tip.
- Failure face = existing bare CHECKs; do **not** invent `S111-*` prefixes.
- Ubuntu + Windows: `s111_open_frame` MUST NOT SKIP, Passed (already on tip).
- After Closed next: **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen S-104 / QBC-MG / UNCERT / DEEPEN; do **not** mix TIMELINE-UI / PC-WIRE / CATALOGUE-COMPLETE / S-124.

**Win policy:** **no (a)/(b)** — not viewer/snapshot family; already dual-green under Win `-E "viewer_"`. Close binds Ubuntu+Win Passed. Notes: non-`viewer_*`. **Forbidden:** platform SKIP inside the test; Eng `ci.yml` viewer_* change for this CTest.

**Close criteria (honest):**
- Ubuntu + Windows: `s111_open_frame` MUST NOT SKIP, Passed (already tip CI `40160836622` `#8` / Eng `38127367335`; re-check on tip after this Open if needed)
- Docs → Closed + CI Notes when done; no Eng `ci.yml` / large Eng diff required
- After Close next: **TBD**

**Eng notes:** verify `858d51a` on tip; fixture `testdata/s111/synthetic_current_4326.h5` + `SOURCE.md`; driver `tests/test_s111.cpp` `test_s111_open_frame`; cmake `s111_open_frame`. S-104 Closed already on this stack (`2504c72`). Do **not** reopen S-104 / QBC-MG / UNCERT / DEEPEN. Soft pixel **WARN-only** — do **not** harden. **FC+PC on disk** via catalogue-complete; PC wiring → PC-WIRE separate (do **not** close PC-WIRE / CATALOGUE this tip).

**O-questions (BRIEF §3 Open 1–N — Arch answers; freeze tip-truth):**
1. Docs close / verify primarily (Eng `858d51a` already; no `ci.yml` / no large Eng) — **yes / freeze** (O1 = tip-truth / docs-close).
2. Win: **no (a)/(b)**; Close binds dual-green Notes non-`viewer_*` — **yes / freeze**.
3. Contract head → Open; keep display **rev 2** (do **not** bump just for Open) — **yes / freeze**.
4. case-ids Active → Open (Status stays Active family) — **yes / freeze**.
5. Notes ≠ TIMELINE-UI; ≠ PC-WIRE / CATALOGUE-COMPLETE; ≠ S-124; do **not** reopen S-104 / QBC-MG / UNCERT / DEEPEN; shim-retire stays Backlog — **yes / freeze**.
6. Closed next → **TBD** (architecture / registered Active; do **not** invent) — **yes / freeze**.
7. No other cases this tip (do **not** Closed TIMELINE / PC-WIRE / CATALOGUE / S-124 / SIG / POWERUP / etc.) — **yes / freeze**.
8. architecture.md flip **next** to this Open — **yes / freeze**.
9. `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog — **yes / freeze**.
10. Failure face = existing bare CHECKs; do **not** invent `S111-*` prefixes — **yes / freeze**.
11. FC+PC on disk (≠ S-104 FC-only); PC wiring → PC-WIRE separate — **yes / freeze**.

---

## Goal

HDF5 open → ≥2 frames → sample/portray **one active frame** as arrows → `ChartSession` overlay with S-101.

## Frozen sample & storage

```cpp
struct CurrentSample {
  float speed_ms{0};       // m/s, >= 0
  float direction_deg{0};  // from-direction, deg clockwise from true N, in [0, 360)
  bool valid{false};
};

struct CurrentFrame {
  std::string time_iso;    // UTC
  CoverageGrid speed;      // values = speed_ms
  CoverageGrid direction;  // values = direction_deg (same shape as speed)
};

struct CurrentDataset {
  ProductId product{ProductId::S111};
  std::filesystem::path path;
  std::vector<CurrentFrame> frames;
  std::size_t active_frame{0};

  Bounds bounds() const;
  bool set_active_frame(std::size_t index);          // false if index >= frames.size(); no change
  bool set_active_time(std::string_view iso_utc);    // nearest |Δt|; ties → lower index; empty frames → false
  bool sample_wgs84(double lon, double lat, CurrentSample& out) const;  // active frame only
};
```

- Façade is **speed + `direction_deg` only**. No `direction_or_v`, no u/v at API.
- If HDF5 stores u/v, convert to speed/direction **at load** into the two planes.
- `Dataset::current` only. Wrong product must **not** fill `current` → `LoadError` (same as S-104 water_level).

## Arrow DrawingInstruction (locked)

Add `DrawingInstruction::Kind::Arrow`.

| Field | Use |
|-------|-----|
| `kind` | `Arrow` |
| `geometries` | single Point anchor (WGS84) |
| `rotation` | = `direction_deg` (PortrayalCRS / GeographicCRS as documented in emit) |
| `scale_factor` | proportional to `speed_ms` (clamp in viewer) |
| `symbol_ref` | **PC id** `SCAROW01`…`SCAROW09` (band table in `pc-wire-contract`); interim `"Arrow"` superseded when PC wired |
| `fill_ref` | colour token (default `CHGRD` if no PC) |

Do not overload `Kind::Point` for currents.

## Subsample

Constant: **`kS111ArrowStride = 2`** (emit arrow every 2×2 cells on the active frame grid; edges inclusive). Named in code exactly `kS111ArrowStride`.

## Frame API

On `CurrentDataset` only; CLI `--s111` / `--s111-frame` / `--s111-time` wrappers. Session key `S-111:<stem>`.

## Fixture oracle (`testdata/s111/SOURCE.md`)

File: **`synthetic_current_4326.h5`** (gittracked). Layout pinned there:

- 4326, envelope ~0–0.4, 5×5, spacing 0.1
- Frame0 `2024-01-01T00:00:00Z`: speed 0.5 m/s, direction 90°
- Frame1 `2024-01-01T06:00:00Z`: speed 1.5 m/s, direction 180°
- Sample **lon=0.2, lat=0.2** must match those values

MUST tests use this file — **no SKIP**.

## MUST / WARN

| Check | Sev |
|-------|-----|
| Open; frames≥2; OOR `set_active_frame` false; `set_active_time` nearest | MUST |
| sample at (0.2,0.2) matches SOURCE oracle per frame | MUST |
| Wrong product → LoadError, `current` empty | MUST |
| Portray active frame: Arrow count follows stride; ≥1 Arrow | MUST |
| ChartSession S-101+S-111 | MUST |
| Soft pixel | WARN |
| Official S-111 FC/PC on disk + provenance | **MUST** via `docs/catalogue-complete-contract.md` |
| Portray→PC wiring (SCAROW + ColorProfiles) | **MUST** via `docs/pc-wire-contract.md` (XSLT engine WARN; no invented Lua) |

## Timeline UI

Frame scrubber / `--list-frames`: `docs/timeline-ui-contract.md` (`IHO-TIMELINE-UI`).

## Non-goals

Animation, S-104 sync UI, S-98, uncertainty ellipses.
