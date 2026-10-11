#!/usr/bin/env python3
"""Day/Dusk/Night soft check: Day brightest, Night darkest, large pixel drift between them."""
import sys
from collections import Counter
from PIL import Image

paths = sys.argv[1:4]  # day, dusk, night
imgs = [Image.open(p).convert("RGB") for p in paths]
if len({im.size for im in imgs}) != 1:
    sys.exit(f"size mismatch: {[im.size for im in imgs]}")

def luma(im):
    c = Counter(im.getdata())
    n = im.size[0] * im.size[1]
    return sum((0.299 * r + 0.587 * g + 0.114 * b) * v for (r, g, b), v in c.items()) / n

def drift(a, b):
    n = a.size[0] * a.size[1]
    return sum(1 for p, q in zip(a.getdata(), b.getdata()) if p != q) / n

ld, lu, ln = (luma(im) for im in imgs)
if not (ld > lu > ln):
    sys.exit(f"luma order Day>Dusk>Night failed: {ld:.1f} {lu:.1f} {ln:.1f}")
# Palettes must actually change the image (not a no-op remap).
for a, b, name in ((0, 1, "day-dusk"), (0, 2, "day-night"), (1, 2, "dusk-night")):
    d = drift(imgs[a], imgs[b])
    if d < 0.2:
        sys.exit(f"{name} drift {d:.4f} < 0.20 — palettes look too similar")
print(f"palettes ok luma Day={ld:.1f} Dusk={lu:.1f} Night={ln:.1f}")
