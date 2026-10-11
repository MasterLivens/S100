#!/usr/bin/env python3
"""Warn-only soft compare: our viewer snapshot vs EncDotNet oracle PNG.

Loose thresholds for cross-engine (QtSvg vs Skia) drift. Never fails CI by default:
exit 0 always unless --strict is passed (then non-zero on threshold breach).

Usage:
  soft_compare_oracle.py ours.png encdotnet.png [--threshold 0.90] [--strict]
"""
from __future__ import annotations

import argparse
import sys
import warnings

try:
    from PIL import Image
except ImportError:
    print("WARN: Pillow not installed; skip soft compare", file=sys.stderr)
    sys.exit(0)


def rgb_pixels(im: Image.Image) -> list[tuple[int, int, int]]:
    with warnings.catch_warnings():
        warnings.simplefilter("ignore", DeprecationWarning)
        return list(im.convert("RGB").getdata())


def magenta_count(pixels: list[tuple[int, int, int]]) -> int:
    return sum(1 for r, g, b in pixels if r > 200 and b > 200 and g < 80)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("ours")
    ap.add_argument("oracle")
    ap.add_argument(
        "--threshold",
        type=float,
        default=0.90,
        help="max differing-pixel fraction before WARN (default 0.90; cross-engine)",
    )
    ap.add_argument(
        "--strict",
        action="store_true",
        help="exit 1 on WARN conditions (default: warn-only, exit 0)",
    )
    args = ap.parse_args()

    warns: list[str] = []
    ours_im = Image.open(args.ours).convert("RGB")
    ora_im = Image.open(args.oracle).convert("RGB")

    if ours_im.size != ora_im.size:
        warns.append(
            f"size mismatch ours={ours_im.size} oracle={ora_im.size}; scaling oracle for estimate"
        )
        ora_im = ora_im.resize(ours_im.size, Image.Resampling.BILINEAR)

    ours = rgb_pixels(ours_im)
    ora = rgb_pixels(ora_im)
    n = len(ours)
    mag_o = magenta_count(ours)
    mag_e = magenta_count(ora)
    if mag_o > 0:
        warns.append(f"ours has magenta placeholder pixels: {mag_o}")
    if mag_e > 0:
        warns.append(f"oracle has magenta-like pixels: {mag_e}")

    diff = sum(1 for pa, pb in zip(ours, ora) if pa != pb)
    drift = diff / n if n else 1.0
    line = (
        f"soft-compare size={ours_im.size} drift={diff}/{n}={drift:.4f} "
        f"threshold={args.threshold} magenta_ours={mag_o} magenta_oracle={mag_e}"
    )
    if drift > args.threshold:
        warns.append(f"pixel drift {drift:.4f} > {args.threshold}")

    if warns:
        print(f"WARN: {line}")
        for w in warns:
            print(f"  - {w}")
        return 1 if args.strict else 0

    print(f"OK (informational drift): {line}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
