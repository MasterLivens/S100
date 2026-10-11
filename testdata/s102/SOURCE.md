# S-102 test cells (gitignored binaries)

Binaries under this directory are **not** committed (see `.gitignore`). Fetch locally for
loader / sample tests when CHS fixtures are absent.

## NOAA public PDS (ed 3.0.0)

| Field | Value |
|-------|-------|
| File | `102US004AK5HQ262267.h5` |
| URL | https://noaa-s102-pds.s3.amazonaws.com/ed3.0.0/Alaska/Southeast/102US004AK5HQ262267.h5 |
| Product | INT.IHO.S-102.3.0.0 (horizontal CRS EPSG:32609 UTM 9N; geographic bounds on root attrs) |
| Fetched | 2026-10-11 |
| Size | ~895 KiB |
| Notes | Small public sample. Grid axes are projected metres; `sample_wgs84` is unsupported for non-4326 (open/bounds still work). Prefer CHS WGS84 cell for sampling tests. |

```
curl -L -o testdata/s102/102US004AK5HQ262267.h5 \
  https://noaa-s102-pds.s3.amazonaws.com/ed3.0.0/Alaska/Southeast/102US004AK5HQ262267.h5
```

## CHS (preferred for sample)

When present (gitignored `testdata/chs/`):

`testdata/chs/COMBINED/S100_ROOT/S-102/DATASET_FILES/CA00/102CA0064682N07116W/16/102CA0064682N07116W.h5`

- Spec `INT.IHO.S-102.3.0.0`, horizontal CRS EPSG:4326, ~635 KiB, fill depth `1e+06`.
