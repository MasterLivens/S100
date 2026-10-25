# S-111 surface currents — first slice (rev 2) (Closed)

Owner: SoftwareArchitecture (2026-10-25). Implement: SoftwareEngineer. Test: SoftwareTester.
Builds on `docs/api-contracts.md` (S-111) / `docs/catalogue-complete-contract.md` (S-111 **FC+PC** MUST — ≠ S-104 FC-only). Case id: **`IHO-S111-OPEN-FRAME`** (**Closed / Done** 2026-10-25).  
CTest name: **`s111_open_frame`** — MUST NOT SKIP; Linux + Windows dual-green (CI `40162377554` @ `d512e30`). Supersedes Open draft on tip `d512e30` (≡ Arch `1be2b1b`).

Primary research: `/workspace/research/s111-open-frame/BRIEF.md`. Former Eng gap **already fixed** @ Eng **`858d51a`**. Scrubber / `--list-frames` / `timeline_entries` → separate Active **`IHO-TIMELINE-UI`** (`docs/timeline-ui-contract.md`). PC wiring (SCAROW / ColorProfiles / XSLT WARN / no invented Lua) → separate Active **`IHO-PC-WIRE-111-124`** (`docs/pc-wire-contract.md`) — **do not** close PC-WIRE this tip.

**Status: Closed / Done (2026-10-25)** — CI [`40162377554`](https://github.com/MasterLivens/S100/actions/runs/40162377554) @ `d512e30` dual-green; **`s111_open_frame` Passed Ubuntu+Win** (native `s100_tests` + HDF5 — **not** `viewer_*`; **no Win O1 (a)/(b)**). Eng **`858d51a`** already on tip — **docs close only**. MUST tip-truth **unchanged** (open≥2 frames + frame API + sample 0.2/0.2 ≈0.5/90° ≠ ≈1.5/180° + Arrow stride=`kS111ArrowStride=2` (count=9) + ChartSession S-101+S-111 + **FC+PC on disk** + soft pixel WARN-only; fixture MUST NOT SKIP). **O1=tip-truth / docs-close SUCCESS**. Failure face = bare CHECK tip-truth (do **not** invent `S111-*`). Do **not** reopen S-104 / QBC-MG / UNCERT / DEEPEN; do **not** mix TIMELINE-UI / PC-WIRE / CATALOGUE-COMPLETE / S-124; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog. Contract display stays **rev 2**. Next: **TBD**.

**Planner freeze (kept — rev 2 tip-truth):** Reuse case id **`IHO-S111-OPEN-FRAME`** / CTest **`s111_open_frame`**; MUST = open≥2 frames + frame API (`set_active_frame` / `set_active_time` / `sample_wgs84` / `bounds`) + sample 0.2/0.2 frame0≈**0.5 m/s / 90°** ≠ frame1≈**1.5 m/s / 180°** + portray ≥1 `Kind::Arrow` + stride **`kS111ArrowStride=2`** (5×5 → count=**9**) + ChartSession S-101+S-111 + **FC+PC on disk** (catalogue-complete; ≠ S-104 FC-only) + soft pixel **WARN-only**; data model `Dataset::current` / `CurrentDataset` (**forbid** reuse `Dataset::coverage` / `water_level`); fixture `synthetic_current_4326.h5` MUST NOT SKIP; scrubber → **`IHO-TIMELINE-UI`** (separate); PC wiring → **`IHO-PC-WIRE-111-124`** (separate; **do not** close); Eng `858d51a` already; `grid()` / `S102-GRID-SHIM-RETIRE` stays Backlog; failure face = bare CHECKs (do **not** invent `S111-*`); after Closed next **TBD** (architecture / registered Active — do **not** invent). Independent tip; do **not** reopen S-104 / QBC-MG / UNCERT / DEEPEN; do **not** mix TIMELINE-UI / PC-WIRE / CATALOGUE-COMPLETE / S-124.

**O1 closed as tip-truth / docs-close:** Eng `858d51a` already on tip (HDF5 open → `Dataset::current`; ≥2 frames; frame API; sample 0.2/0.2 → 0.5/90° & 1.5/180°; portray Arrow + stride; ChartSession S-101+S-111; fixture `synthetic_current_4326.h5`; CTest **`s111_open_frame`**). Slice = Arch docs close only — not greenfield Eng. Do **not** invent case ids or next tip names; do **not** reopen S-104 / QBC-MG / UNCERT / DEEPEN; do **not** write FC-only; do **not** invent `S111-*` prefixes; do **not** retire `grid()` this tip; do **not** close PC-WIRE / CATALOGUE / TIMELINE / S-124. **Forbidden:** platform SKIP; Eng `ci.yml` viewer_* change for this CTest; inventing oracles / next tip id; mixing TIMELINE-UI / PC-WIRE / CATALOGUE / S-124.

---

## Slice close — 2026-10-25

**Status: Closed / Done** (formal docs close — **not** a loader / portray rewrite).

- Evidence: CI [40162377554](https://github.com/MasterLivens/S100/actions/runs/40162377554) tip `d512e30` — Ubuntu + Windows **`s111_open_frame` Passed** (MUST NOT SKIP). Ancestor greens also Eng landing [`38127367335`](https://github.com/MasterLivens/S100/actions/runs/38127367335) @ `858d51a` + pre-Open tip [`40160836622`](https://github.com/MasterLivens/S100/actions/runs/40160836622) `#8`.
- **O1=tip-truth / docs-close SUCCESS** — Eng `858d51a` already on tip; Arch Open→Closed only; Planner BRIEF §7 answers frozen.
- MUST tip-truth unchanged (open≥2 frames + frame API + sample 0.2/0.2 ≈0.5/90° ≠ ≈1.5/180° + Arrow stride=`kS111ArrowStride=2` (count=9) + ChartSession S-101+S-111 + **FC+PC on disk** + soft pixel WARN-only; fixture MUST NOT SKIP).
- Eng `858d51a` already on tip — docs close only; no `ci.yml` change.
- Failure face stays bare CHECK tip-truth — do **not** invent `S111-*`.
- **Notes:** next **TBD** (architecture / registered Active — do **not** invent a next tip id; do **not** auto-open S-124). **Backlog:** `grid()` shim retire (`S102-GRID-SHIM-RETIRE`) — do not insert ahead. Contract display stays **rev 2**. FC+PC on disk kept; PC-WIRE stays separate Active (do **not** close).
- Boundaries: independent; ≠ TIMELINE-UI; ≠ PC-WIRE / CATALOGUE-COMPLETE; ≠ S-124; do **not** reopen S-104 / QBC-MG / UNCERT / DEEPEN.
- **Non-goals this close:** inventing oracles / next tip id; writing FC-only; closing PC-WIRE / CATALOGUE / TIMELINE / S-124; retiring `grid()` shim; loader/portray rewrite; reopening S-104 / QBC-MG / UNCERT / DEEPEN; Win (a)/(b) (N/A — non-`viewer_*`); inventing `S111-*` failure-face prefixes.

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
