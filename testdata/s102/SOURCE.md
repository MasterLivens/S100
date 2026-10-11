# S-102 test cells

## Tracked (CI)

| File | Role |
|------|------|
| `synthetic_multigrid_4326.h5` | **Gittracked.** Two small EPSG:4326 regular grids with disjoint envelopes for `IHO-S102-DEEPEN-MULTIGRID` / CTest `s102_deepen_multigrid`. Built by Eng in the deepen slice (see `docs/s102-deepen-contract.md`). |

## Local / gitignored (`*.h5` except the synthetic above)

Binaries matching `testdata/s102/*.h5` stay gitignored except the `!` exception above. Fetch NOAA/CHS locally for loader / sample / UTM tests.

### NOAA public PDS (ed 3.0.0)

| Field | Value |
|-------|-------|
| File | `102US004AK5HQ262267.h5` |
| URL | https://noaa-s102-pds.s3.amazonaws.com/ed3.0.0/Alaska/Southeast/102US004AK5HQ262267.h5 |
| Product | INT.IHO.S-102.3.0.0 (horizontal CRS EPSG:32609 UTM 9N; geographic bounds on root attrs) |
| Fetched | 2026-10-11 |
| Size | ~895 KiB |
| Notes | Small public sample. Grid axes are projected metres; `sample_wgs84` converts UTM 326xx/327xx internally (see `docs/s102-contract.md`). Prefer CHS WGS84 cell when present. |

```
curl -L -o testdata/s102/102US004AK5HQ262267.h5 \
  https://noaa-s102-pds.s3.amazonaws.com/ed3.0.0/Alaska/Southeast/102US004AK5HQ262267.h5
```

### CHS (preferred for 4326 sample)

When present (gitignored `testdata/chs/`):

`testdata/chs/COMBINED/S100_ROOT/S-102/DATASET_FILES/CA00/102CA0064682N07116W/16/102CA0064682N07116W.h5`

- Spec `INT.IHO.S-102.3.0.0`, horizontal CRS EPSG:4326, ~635 KiB, fill depth `1e+06`.
