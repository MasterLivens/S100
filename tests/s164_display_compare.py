#!/usr/bin/env python3
"""IHO-S164-DISPLAY-COMPARE driver (MUST hard-fail; soft pixel WARN-only).

Runs offscreen viewer for Base/Standard presets against testdata/golden/s164-ref/.
Cell heuristic: 101AA00DS0008.000 (label heuristic=true) until real AA5 cells land.

Usage (from CMake):
  s164_display_compare.py --viewer BIN --src SRC --out DIR [--strict]
"""
from __future__ import annotations

import argparse
import hashlib
import re
import subprocess
import sys
from collections import Counter
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("FATAL: Pillow required for S-164 display compare", file=sys.stderr)
    sys.exit(1)

# Same four Day palette RGB anchors as tests/compare_viewer_golden.py (not token names).
DAY_ANCHORS = [(97, 183, 255), (191, 190, 143), (255, 255, 255), (147, 174, 187)]
CELL_REL = Path("testdata/s101/101AA00DS0008.000")
REF_DIR_REL = Path("testdata/golden/s164-ref")
PRESET_REF = {
    "base": "AA5DBASE.png",
    "standard": "AA5STNDR.png",
}


def die(msg: str) -> None:
    print(f"FATAL: {msg}", file=sys.stderr)
    sys.exit(1)


def warn(msg: str) -> None:
    print(f"WARN: {msg}", file=sys.stderr)


def parse_presets_yaml(path: Path) -> dict[str, dict[str, list[str]]]:
    """Minimal stub parser for display-presets.yaml (no PyYAML dependency)."""
    text = path.read_text(encoding="utf-8")
    presets: dict[str, dict[str, list[str]]] = {}
    cur: str | None = None
    for raw in text.splitlines():
        line = raw.split("#", 1)[0].rstrip()
        if not line.strip():
            continue
        m = re.match(r"^  (\w+):\s*$", line)
        if m and line.startswith("  ") and not line.startswith("    "):
            # top-level under presets: only when we already saw "presets:"
            name = m.group(1)
            if name in ("base", "standard") or (
                presets or "presets:" in text[: text.find(line) + 1]
            ):
                if name in ("day_palette_rgb_anchors",):
                    cur = None
                    continue
                if name not in ("viewing_group_hint",) and re.match(
                    r"^  \w+:\s*$", line
                ):
                    # preset name under presets:
                    if name in ("base", "standard") or (
                        "presets:" in text and name not in ("day_palette_rgb_anchors",)
                    ):
                        cur = name
                        presets.setdefault(cur, {"required_on": [], "required_off": []})
                        continue
        if line.strip() == "presets:":
            continue
        if cur is None:
            continue
        m_on = re.match(r"^\s+required_on:\s*\[(.*)\]\s*$", line)
        m_off = re.match(r"^\s+required_off:\s*\[(.*)\]\s*$", line)
        if m_on:
            presets[cur]["required_on"] = [
                p.strip() for p in m_on.group(1).split(",") if p.strip()
            ]
            continue
        if m_off:
            presets[cur]["required_off"] = [
                p.strip() for p in m_off.group(1).split(",") if p.strip()
            ]
            continue
    return presets


def verify_sha256(ref_dir: Path) -> None:
    sums = ref_dir / "SHA256SUMS.txt"
    if not sums.is_file():
        die(f"missing {sums}")
    if not (ref_dir / "SOURCE.md").is_file():
        die(f"missing {ref_dir / 'SOURCE.md'}")
    for line in sums.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if len(parts) < 2:
            die(f"bad SHA256SUMS line: {line!r}")
        expect, name = parts[0], parts[-1]
        p = ref_dir / name
        if not p.is_file():
            die(f"ref PNG missing: {p}")
        h = hashlib.sha256(p.read_bytes()).hexdigest()
        if h != expect:
            die(f"SHA mismatch {name}: got={h} want={expect}")
        print(f"sha256 ok {name}")


def check_anchors(png: Path) -> None:
    im = Image.open(png).convert("RGB")
    counts = Counter(im.getdata())
    n = im.size[0] * im.size[1]
    for a in DAY_ANCHORS:
        g = counts[a]
        if g < n * 0.02:
            die(f"anchor {a} too rare in {png.name}: got={g} of {n}")
    print(f"anchors ok {png.name} size={im.size}")


def letterbox_to(src: Path, ref_size: tuple[int, int], out: Path) -> None:
    """Aspect-fit src into ref_size, pad with white (255,255,255)."""
    im = Image.open(src).convert("RGB")
    tw, th = ref_size
    sw, sh = im.size
    scale = min(tw / sw, th / sh)
    nw, nh = max(1, int(round(sw * scale))), max(1, int(round(sh * scale)))
    scaled = im.resize((nw, nh), Image.Resampling.BILINEAR)
    canvas = Image.new("RGB", (tw, th), (255, 255, 255))
    canvas.paste(scaled, ((tw - nw) // 2, (th - nh) // 2))
    canvas.save(out)


def load_dump(path: Path) -> dict[str, str]:
    out: dict[str, str] = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line:
            continue
        if "\t" in line:
            code, vis = line.split("\t", 1)
        elif "," in line:
            code, vis = line.split(",", 1)
        else:
            die(f"bad dump-layers line: {line!r}")
        out[code.strip()] = vis.strip().lower()
    return out


def verify_preset_layers(
    dump: dict[str, str], required_on: list[str], required_off: list[str]
) -> None:
    """MUST for codes present in session; missing codes → heuristic WARN (0008 stub)."""
    for code in required_on:
        if code not in dump:
            warn(
                f"required_on {code} not in session dump (heuristic cell; skip assert)"
            )
            continue
        if dump[code] != "on":
            die(f"required_on {code} is {dump[code]!r}, want on")
    for code in required_off:
        if code not in dump:
            warn(
                f"required_off {code} not in session dump (heuristic cell; skip assert)"
            )
            continue
        if dump[code] != "off":
            die(f"required_off {code} is {dump[code]!r}, want off")


def run_viewer(
    viewer: Path,
    cell: Path,
    snapshot: Path,
    dump_layers: Path,
    layers_on: list[str],
    layers_off: list[str],
) -> None:
    cmd = [
        str(viewer),
        str(cell),
        "--palette",
        "Day",
        "--no-text-collision",
        "--snapshot",
        str(snapshot),
        "--dump-layers",
        str(dump_layers),
    ]
    if layers_on:
        cmd += ["--layers-on", ",".join(layers_on)]
    if layers_off:
        cmd += ["--layers-off", ",".join(layers_off)]
    env = {
        **dict(**{k: v for k, v in __import__("os").environ.items()}),
        "QT_QPA_PLATFORM": "offscreen",
        "QT_OPENGL": "software",
    }
    print("run:", " ".join(cmd))
    r = subprocess.run(cmd, env=env, capture_output=True, text=True)
    if r.stdout:
        print(r.stdout, end="" if r.stdout.endswith("\n") else "\n")
    if r.stderr:
        print(r.stderr, end="" if r.stderr.endswith("\n") else "\n", file=sys.stderr)
    if r.returncode != 0:
        die(f"viewer exit {r.returncode}")
    if not snapshot.is_file() or snapshot.stat().st_size == 0:
        die(f"snapshot missing or empty: {snapshot}")
    if not dump_layers.is_file() or dump_layers.stat().st_size == 0:
        die(f"dump-layers missing or empty: {dump_layers}")


def soft_compare(ours_scaled: Path, ref: Path, soft_script: Path, strict: bool) -> None:
    cmd = [sys.executable, str(soft_script), str(ours_scaled), str(ref)]
    if strict:
        cmd.append("--strict")
    r = subprocess.run(cmd, capture_output=True, text=True)
    out = (r.stdout or "") + (r.stderr or "")
    if out.strip():
        print(out.rstrip())
    if strict and r.returncode != 0:
        die(f"soft_compare --strict failed rc={r.returncode}")
    # warn-only path: soft_compare exits 0 even on WARN unless --strict


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--viewer", required=True, type=Path)
    ap.add_argument("--src", required=True, type=Path, help="repo source root")
    ap.add_argument("--out", required=True, type=Path, help="build output dir")
    ap.add_argument(
        "--strict",
        action="store_true",
        help="fail on soft pixel WARN (CTest path leaves this off)",
    )
    args = ap.parse_args()

    src: Path = args.src.resolve()
    out: Path = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    viewer = args.viewer.resolve()
    if not viewer.is_file():
        die(f"viewer not found: {viewer}")

    ref_dir = src / REF_DIR_REL
    verify_sha256(ref_dir)

    presets_path = ref_dir / "display-presets.yaml"
    if not presets_path.is_file():
        die(f"missing {presets_path}")
    presets = parse_presets_yaml(presets_path)
    for name in ("base", "standard"):
        if name not in presets:
            die(f"preset {name!r} missing in {presets_path}")
        if not presets[name]["required_on"]:
            die(f"preset {name}: required_on must not be empty")

    cell = src / CELL_REL
    if not cell.is_file():
        die(f"missing cell {cell}")

    soft_script = src / "scripts" / "soft_compare_oracle.py"
    if not soft_script.is_file():
        die(f"missing {soft_script}")

    print("IHO-S164-DISPLAY-COMPARE cell=101AA00DS0008.000 heuristic=true")

    for preset, ref_name in PRESET_REF.items():
        req_on = presets[preset]["required_on"]
        req_off = presets[preset]["required_off"]
        snap = out / f"s164_{preset}.png"
        dump = out / f"s164_{preset}_layers.txt"
        run_viewer(viewer, cell, snap, dump, req_on, req_off)
        check_anchors(snap)
        layer_state = load_dump(dump)
        verify_preset_layers(layer_state, req_on, req_off)
        print(
            f"preset {preset}: layers dump ok "
            f"(on={req_on} off={req_off}; present="
            f"{[c for c in req_on + req_off if c in layer_state]})"
        )

        ref_png = ref_dir / ref_name
        ref_im = Image.open(ref_png)
        scaled = out / f"s164_{preset}_letterbox.png"
        letterbox_to(snap, ref_im.size, scaled)
        print(
            f"letterbox {snap.name} {Image.open(snap).size} -> {ref_im.size} "
            f"(aspect-fit + pad white)"
        )
        soft_compare(scaled, ref_png, soft_script, args.strict)

    print("IHO-S164-DISPLAY-COMPARE MUST checks passed (soft pixel WARN-only)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
