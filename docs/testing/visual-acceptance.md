# Visual acceptance (S-101 / S-102 portrayal)

| Level | Artifact | Gates CI? | Scope |
|-------|----------|-----------|-------|
| Smoke / regression | `testdata/golden/iho_0008.png` + `viewer_golden` | Yes | Day palette anchors, no magenta placeholders, ≤25% pixel drift |
| S-164 figure refs | `testdata/golden/s164-ref/AA5DBASE.png`, `AA5STNDR.png` + `display-presets.yaml` | MUST subset via `viewer_s164_display_compare` (anchors + presets); soft pixel WARN | Automation: `docs/s164-display-compare-contract.md` |
| **External oracle (interim / Linux)** | EncDotNet.S100 `s100` **v0.24.0** PNGs | **No** (warn-only) | Soft compare vs our `--snapshot` on same cell + Day |
| **External oracle (final, preferred)** | NIWC S-100 Viewer / ShoreECDIS screenshots | No | Official IHO-listed free reference; Windows-only today |

Provenance: `testdata/golden/s164-ref/SOURCE.md`. Soft golden `iho_0008.png` and S-164 refs stay as-is — EncDotNet wiring is **additional** and must not tighten or replace them.

---

## A. EncDotNet.S100 (Linux final-render soft-compare oracle)

| Item | Detail |
|------|--------|
| Repo | https://github.com/philliphoff/EncDotNet.S100 |
| License | MIT |
| **Pinned CLI** | **v0.24.0** linux-x64 — see `tools/encdotnet-s100/SOURCE.md` |
| Asset URL | https://github.com/philliphoff/EncDotNet.S100/releases/download/v0.24.0/s100-0.24.0-linux-x64.tar.gz |
| SHA-256 | `9ea3d351aa071d4ae2fcbb5d545e00d318128ade8551f8b016dde2302e5f3416` |
| Install | `./scripts/fetch_encdotnet_s100.sh` → `tools/encdotnet-s100/s100` (binary **gitignored**) |
| Platforms | macOS, Windows, Linux |
| Headless export | `s100 render <dataset> out.png --palette day -w W -h H` |
| Docs | https://raw.githubusercontent.com/philliphoff/EncDotNet.S100/main/docs/cli.md |

### How to run (repo scripts)

```bash
# 1) Pin + extract CLI (once; ~62 MiB download, ~150 MiB extracted; not in git)
./scripts/fetch_encdotnet_s100.sh

# 2) EncDotNet-only Day renders @ 1280×800 → Testing/encdotnet-oracle/
./scripts/render_encdotnet_oracle.sh

# 3) Dual capture (ours + EncDotNet) + warn-only soft compare
./scripts/dual_capture_oracle.sh
# Soft compare alone:
#   python3 scripts/soft_compare_oracle.py ours.png enc.png --threshold 0.90
```

Optional CMake (default **OFF**, not enabled in `.github/workflows/ci.yml`):

```bash
cmake -S . -B build -DS100_ENCDOTNET_ORACLE=ON
ctest --test-dir build -R viewer_encdotnet_oracle --output-on-failure
```

The cmake helper always returns success (warn-only). Prefer the shell scripts for local Tester runs.

### Cells rendered by `render_encdotnet_oracle.sh`

| Label | Path (if present) | Output |
|-------|-------------------|--------|
| IHO TDS 0008 | `testdata/s101/101AA00DS0008.000` | `Testing/encdotnet-oracle/enc_iho_0008_day_1280x800.png` |
| CHS harbour | `testdata/chs/.../101CA00P468N0712W.000` | `.../enc_chs_101CA00P468N0712W_day_1280x800.png` |
| NOAA S-102 | `testdata/s102/102US004AK5HQ262267.h5` | `.../enc_s102_102US004AK5HQ262267_day_1280x800.png` |

Missing files are **skipped** (CHS / S-102 are often gitignored locally).

### Linux box notes (verified 2026-10-11)

- Prefer release CLI over building from source (.NET 10 SDK not required for the self-contained tarball).
- Verified renders: IHO 0008, CHS `101CA00P468N0712W`, NOAA S-102 sample.
- Do **not** vendor the tarball or extracted tree into git.

---

## B. NIWC S-100 Viewer (preferred final oracle — Windows only)

| Item | Detail |
|------|--------|
| Listed name (S100Resources) | **NIWC S-100 viewer** and **NIWC S-100 ShoreECDIS** (`Free`) |
| Registry listing title | **NIWC S100Viewer and Shore ECDIS** |
| Listing URL | https://registry.iho.int/repository/list.do (also https://iho-ohi.github.io/S100Resources/) |
| Product support | S-101 Ed.2.0.0, S-102 Ed.3.0.0, S-104/111/124/129 Ed.2.0.0 |
| Public direct download | **None** — registry requires Sign in / Join |
| OS | **Windows** installer historically; **no Linux build** documented |
| Screenshot / export | Manual UI capture (no headless PNG CLI) |

### What blocks NIWC on Linux CI / this box

1. No anonymous download URL.
2. Windows-only installer (Wine/VM out of scope for CI).
3. Package version/filename only visible after registry sign-in.

**Do not attempt to download NIWC from automation.** On a Windows host: Sign in → download → install → Day palette → screenshot chart pane for the same cells → archive under local `testdata/golden/niwc-ref/` (gitignored if licence requires) with `SOURCE.md`.

---

## C. SoftwareTester acceptance procedure

### Capture — our viewer

```bash
QT_QPA_PLATFORM=offscreen QT_OPENGL=software ./build/s100_viewer \
  testdata/s101/101AA00DS0008.000 \
  --palette Day \
  --snapshot /tmp/ours_iho_0008_day.png
```

Window size is **1280×800** (matches soft golden). Align EncDotNet with `-w 1280 -h 800`.

### Capture — EncDotNet (`s100`)

```bash
./scripts/fetch_encdotnet_s100.sh
./tools/encdotnet-s100/s100 render testdata/s101/101AA00DS0008.000 \
  Testing/encdotnet-oracle/enc_iho_0008_day_1280x800.png \
  --palette day -w 1280 -h 800
```

Or use `./scripts/render_encdotnet_oracle.sh` / `./scripts/dual_capture_oracle.sh`.

### Capture — NIWC (Windows host)

1. Install from IHO GI Registry after Sign in.
2. Load the same `.000` / exchange set; set Day colour scheme.
3. Fit similar geographic extent (full cell preferred).
4. Screenshot chart pane only; save next to EncDotNet/ours captures.

### Compare policy

| Mode | Use when | Criteria |
|------|----------|----------|
| **Soft** (interim) | EncDotNet vs ours; S-164 vs ours | Same size (or scale); Day anchors / no magenta on **ours**; allow font / AA / symbol-scale / engine drift. Default threshold **90%** differing pixels (`scripts/soft_compare_oracle.py`) — **warn-only**, never fail CI |
| **Pixel-perfect** | Not for cross-engine gates | QtSvg vs Skia will not match bit-for-bit |
| **Human checklist** | NIWC vs ours (final) | Base/Standard content; safety contour; land tint; buoy/beacon; depth areas — attach screenshots to release notes |

### Recommended gates

| Gate | Role |
|------|------|
| **CI (unchanged)** | `viewer_golden` + palettes only. Soft golden + S-164 refs untouched. EncDotNet **not** in `ci.yml`. |
| **Local / Tester** | `dual_capture_oracle.sh` before portrayal milestones; inspect WARN drift numbers. |
| **Final** | Manual NIWC (Windows) + EncDotNet Linux pack for the same cells. |

---

## D. Feasibility matrix

| Oracle | Linux native | Headless PNG | S-101 `.000` | S-102 | Barrier |
|--------|--------------|--------------|--------------|-------|---------|
| EncDotNet `s100` v0.24.0 | **Yes** | **Yes** | **Yes** | **Yes** | Fetch pinned tarball once |
| NIWC S-100 Viewer / ShoreECDIS | **No** | Manual UI | Expected | Expected | Registry login + Windows |

---

## E. Files & push policy

| Path | Commit? |
|------|---------|
| `tools/encdotnet-s100/SOURCE.md` | Yes |
| `tools/encdotnet-s100/s100` + DLLs / tarball | **No** (gitignored) |
| `scripts/fetch_encdotnet_s100.sh` | Yes |
| `scripts/render_encdotnet_oracle.sh` | Yes |
| `scripts/dual_capture_oracle.sh` | Yes |
| `scripts/soft_compare_oracle.py` | Yes |
| `tests/check_encdotnet_oracle.cmake` | Yes (optional; OFF by default) |
| `docs/testing/visual-acceptance.md` | Yes |
| `Testing/encdotnet-oracle/*.png` | **No** (gitignored) |

Do not push EncDotNet clone, vendor CLI, or oracle PNGs unless explicitly requested.
