# Viewer golden snapshots

| File | Cell | Notes |
|------|------|-------|
| `iho_0008.png` | `101AA00DS0008.000` | Official PC Day palette via DrawingInstructions + AreaFillReference symbolFill tiling. Soft compare in CTest `viewer_golden` (size + Day colour anchors + ≤2% pixel drift + no magenta). |

CHS goldens stay local (test-licence data is gitignored). Byte-identical PNG hashes are not required — Qt font AA drifts between runs.

Regenerate after intentional render changes:

```
QT_QPA_PLATFORM=offscreen ./build/s100_viewer testdata/s101/101AA00DS0008.000 --snapshot testdata/golden/iho_0008.png
```
