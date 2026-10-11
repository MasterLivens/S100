# S-111 test cells

## Tracked (CI)

| File | Role |
|------|------|
| `synthetic_current_4326.h5` | **Gittracked.** Two EPSG:4326 time frames for `IHO-S111-OPEN-FRAME` / CTest `s111_open_frame`. |

### Fixture layout (oracle)

Built with h5py (regular grid):

- Spec `INT.IHO.S-111.2.0.0`, `horizontalCRS` = 4326
- Envelope lon/lat ≈ 0–0.4; **5×5** grid, origin (0,0), spacing **0.1**
- Speed + direction planes (not u/v at façade)
- `/SurfaceCurrent/…/Group_001` `timePoint=2024-01-01T00:00:00Z`, uniform **speed=0.5 m/s**, **direction=90°**
- `/SurfaceCurrent/…/Group_002` `timePoint=2024-01-01T06:00:00Z`, uniform **speed=1.5 m/s**, **direction=180°**
- Known sample: **lon=0.2, lat=0.2** → frame0 (0.5, 90°), frame1 (1.5, 180°)
- Arrow subsample: `kS111ArrowStride = 2`

```
python3 scripts/gen_s111_synthetic.py   # requires h5py; Eng adds in slice
```

## Local / gitignored

Other `testdata/s111/*.h5` stay gitignored.
