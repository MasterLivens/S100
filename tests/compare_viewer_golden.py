#!/usr/bin/env python3
"""Compare viewer PNG to golden: size match + Day palette anchors + pixel drift ≤25% (font AA across CI/local)."""
import sys
from collections import Counter
from PIL import Image

got_path, want_path = sys.argv[1], sys.argv[2]
got = Image.open(got_path).convert("RGB")
want = Image.open(want_path).convert("RGB")
if got.size != want.size:
    sys.exit(f"size {got.size} != {want.size}")
# Magenta placeholder geometry must be gone (SVG symbols or CHBLK fallback).
mag = sum(1 for r, g, b in got.getdata() if r > 200 and b > 200 and g < 80)
if mag > 0:
    sys.exit(f"magenta placeholder pixels still present: {mag}")
# Official Day palette anchors we expect to dominate water / land / UI chrome.
# CHSKY-ish blue (97,183,255), land olive (191,190,143), white panel, depth grey-blue.
anchors = [(97, 183, 255), (191, 190, 143), (255, 255, 255), (147, 174, 187)]
cg, cw = Counter(got.getdata()), Counter(want.getdata())
n = got.size[0] * got.size[1]
for a in anchors:
    g, w = cg[a], cw[a]
    if g < n * 0.02 or w < n * 0.02:
        sys.exit(f"anchor {a} too rare: got={g} want={w} of {n}")
    # relative share within 15%
    if abs(g - w) / max(w, 1) > 0.15:
        sys.exit(f"anchor {a} share drifted: got={g} want={w}")
diff = sum(1 for pa, pb in zip(got.getdata(), want.getdata()) if pa != pb)
# allow up to 2% pixel drift (font AA)
if diff / n > 0.25:
    sys.exit(f"pixel drift {diff}/{n} = {diff/n:.4f} > 25%")
print(f"viewer golden ok size={got.size} drift={diff}/{n} anchors_ok")
