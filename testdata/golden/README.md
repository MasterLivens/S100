# Viewer golden snapshots

| File | Cell | Notes |
|------|------|-------|
| `iho_0008.png` | `101AA00DS0008.000` | Official PC **Day** palette (default; Dusk/Night selectable in viewer without re-portray) via DrawingInstructions + AreaFillReference symbolFill tiling + SpatialReference clip (engine-resolved geometries). Soft compare in CTest `viewer_golden` (size + Day colour anchors + ≤25% pixel drift (CI/local fonts); Day colour anchors still required + no magenta). Per-palette goldens may follow later. |

CHS goldens stay local (test-licence data is gitignored). Baseline is the Linux CI offscreen render (Actions artifact). Local fonts may drift ~20%; anchors + 25% cap keep the check meaningful.

Regenerate after intentional render changes:

```
QT_QPA_PLATFORM=offscreen ./build/s100_viewer testdata/s101/101AA00DS0008.000 --snapshot testdata/golden/iho_0008.png
```

## Final visual acceptance (S-164)

See [`s164-ref/`](s164-ref/) — IHO S-164 Sub-Group Display Base / Standard reference plots.
Used for **final** portrayal acceptance; does not replace or tighten `viewer_golden` until we wire automated checks.
