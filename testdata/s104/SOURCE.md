# S-104 test cells

## Tracked (CI)

| File | Role |
|------|------|
| `synthetic_waterlevel_4326.h5` | **Gittracked.** Two EPSG:4326 time frames for `IHO-S104-OPEN-FRAME` / CTest `s104_open_frame`. |

### Fixture layout

Built with h5py (regular grid dcf2):

- Spec `INT.IHO.S-104.2.0.0`, `horizontalCRS` = 4326
- Envelope lon/lat ≈ 0–0.4; 5×5 grid, origin (0,0), spacing 0.1
- `/WaterLevel/WaterLevel.01/Group_001` `timePoint=2024-01-01T00:00:00Z`, `waterLevelHeight=1.0`
- `/WaterLevel/WaterLevel.01/Group_002` `timePoint=2024-01-01T06:00:00Z`, `waterLevelHeight=2.5`
- Known sample: **lon=0.2, lat=0.2** → frame0 = 1.0 m, frame1 = 2.5 m

```
python3 scripts/gen_s104_synthetic.py   # requires h5py
```

## Local / gitignored

Other `testdata/s104/*.h5` stay gitignored. Document public CHS/NOAA URLs here when fetched.
