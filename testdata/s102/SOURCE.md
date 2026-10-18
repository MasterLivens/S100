# S-102 test cells

## Tracked (CI)

| File | Role |
|------|------|
| `synthetic_multigrid_4326.h5` | **Gittracked.** Two small EPSG:4326 regular grids with disjoint envelopes for `IHO-S102-DEEPEN-MULTIGRID` / CTest `s102_deepen_multigrid`. Built with h5py: `BathymetryCoverage.01` (lon/lat≈0–0.4, depth=5) and `.02` (≈10–10.4, depth=50). See `docs/s102-deepen-contract.md`. |
| `synthetic_uncertainty_4326.h5` | **Gittracked.** Fixture **A** for `IHO-S102-UNCERTAINTY` / CTest `s102_uncertainty`. EPSG:4326, single `BathymetryCoverage.01`, compound `depth`+`uncertainty`. Envelope lon/lat≈20–20.4 (disjoint from deepen). Regenerate: `scripts/gen_s102_uncertainty_synthetic.py`. |
| `synthetic_uncertainty_depth_only_4326.h5` | **Gittracked.** Fixture **B** (PS §10 depth-only omit). Compound **depth only**; Group_F **no** uncertainty row; `minimumUncertainty==maximumUncertainty==0.75` (not fill). **`IHO-S102-QBC-META` elects broadcast** → `has_uncertainty==true` / unc **0.75** (`BCAST-1`; see `docs/s102-qbc-meta-contract.md`). |
| `synthetic_qbc_meta_4326.h5` | **Gittracked.** Fixture **C** for `IHO-S102-QBC-META` / CTest `s102_qbc_meta`. QoBC `dataCodingFormat=9`, U32 id grid + FAT (≥2 rows). Envelope lon/lat≈40–40.4. Regenerate: `scripts/gen_s102_qbc_meta_synthetic.py`. |
| `synthetic_qbc_multigrid_vd_4326.h5` | **Gittracked.** Fixture **MG-PS** for `IHO-S102-QBC-MULTIGRID` / CTest `s102_qbc_multigrid`. Co-located BC.01+BC.02 (distinct verticalDatum), one QoBC.01 + FAT. Envelope lon/lat≈50–50.4. Regenerate: `scripts/gen_s102_qbc_multigrid_synthetic.py`. |
| `synthetic_uncertainty_depth_only_fillunc_4326.h5` | **Gittracked.** **BCAST-2**: depth-only with fill min==max (`1e6`) → no broadcast (`has_uncertainty==false`). Envelope ≈35–35.4. Same generator script. |

### Fixture A oracles (`synthetic_uncertainty_4326.h5`)

| Point | Lon | Lat | depth | uncertainty | flags |
|-------|-----|-----|-------|-------------|-------|
| **A** | 20.0 | 20.0 | **12.5** | **2.5** | `has_depth` + `has_uncertainty` |
| **B** | 20.1 | 20.0 | **12.5** | fill `1e6` | `has_depth` && !`has_uncertainty` |

Grid: 5×5, origin (20,20), spacing 0.1°. All other cells: depth=12.5, unc=2.5.

### Fixture B oracles (`synthetic_uncertainty_depth_only_4326.h5`)

| Point | Lon | Lat | depth | uncertainty |
|-------|-----|-----|-------|-------------|
| sample | 30.0 | 30.0 | **8.0** | **0.75** (`has_uncertainty==true`, broadcast elect, oracle **`BCAST-1`**) |

Envelope lon/lat≈30–30.4. `minimumUncertainty==maximumUncertainty==0.75` documented.

### Uncertainty CoverageFill band table (vendor extension, VG `13031`)

Symbolic colour tokens only (no hard-coded RGB). Day→Dusk: tokens stable; RGB remaps via PC `colorProfile.xml`.

| Band label | metres (2-sigma) | token |
|------------|------------------|-------|
| UncLow | &lt; 1 | `CHGRN` |
| UncModest | [1, 5) | `ADINF` |
| UncModerate | [5, 20) | `CHYLW` |
| UncHigh | [20, 50) | `CHCOR` |
| UncVeryHigh | ≥ 50 | `CHRED` |

Depth CoverageFill remains official PC path: `coverage_attribute=="depth"`, VG **`13030`**.

## Local / gitignored (`*.h5` except the synthetics above)

Binaries matching `testdata/s102/*.h5` stay gitignored except the `!` exceptions above. Fetch NOAA/CHS locally for loader / sample / UTM / uncertainty real-sample tests.

### NOAA public PDS (ed 3.0.0)

| Field | Value |
|-------|-------|
| File | `102US004AK5HQ262267.h5` |
| URL | https://noaa-s102-pds.s3.amazonaws.com/ed3.0.0/Alaska/Southeast/102US004AK5HQ262267.h5 |
| Product | INT.IHO.S-102.3.0.0 (horizontal CRS EPSG:32609 UTM 9N; geographic bounds on root attrs) |
| Fetched | 2026-10-11 |
| Size | ~895 KiB |
| Notes | Small public sample. Grid axes are projected metres; `sample_wgs84` converts UTM 326xx/327xx internally (see `docs/s102-contract.md`). Prefer CHS WGS84 cell when present. **Uncertainty:** per-cell metres present (e.g. cell `(700,700)` → unc **40.34**). **QoBC:** U32 id grid + `featureAttributeTable` (14 rows); cell `(700,700)` id **1** → `surveyAuthority` `DOC/NOAA/NOS/OCS -- Office of Coast Survey` — asserted by **`IHO-S102-QBC-META`**. |

```
curl -L -o testdata/s102/102US004AK5HQ262267.h5 \
  https://noaa-s102-pds.s3.amazonaws.com/ed3.0.0/Alaska/Southeast/102US004AK5HQ262267.h5
```

### CHS (preferred for 4326 sample)

When present (gitignored `testdata/chs/`):

`testdata/chs/COMBINED/S100_ROOT/S-102/DATASET_FILES/CA00/102CA0064682N07116W/16/102CA0064682N07116W.h5`

- Spec `INT.IHO.S-102.3.0.0`, horizontal CRS EPSG:4326, ~635 KiB, fill depth `1e+06`.
- Uncertainty channel present but typically all-fill; positive `has_uncertainty` oracle is NOAA-only for UNCERTAINTY.
- **No `QualityOfBathymetryCoverage` group** (Group_F featureCode is BathymetryCoverage only). QBC-META: CHS-without-QoBC → `has_qbc_coverage==false`, not a FAIL.

### Fixture C oracles (`synthetic_qbc_meta_4326.h5`) — `IHO-S102-QBC-META`

| Point | Lon | Lat | qbc_id | Expect |
|-------|-----|-----|--------|--------|
| **C1** | 40.0 | 40.0 | **1** | `has_qbc==true`; `surveyAuthority`=`Synthetic Survey Authority Alpha`; `sourceSurveyID`=`SYN-QBC-001` |
| **C2** | 40.1 | 40.0 | **0** (fill) | `has_qbc==false` |
| **C3** | — | — | — | Open → `has_qbc_coverage==true`; `dataCodingFormat` validated as 9 |

Grid: 5×5, origin (40,40), spacing 0.1°. Bathymetry depth=15, unc=1.5. FAT rows id=1 and id=2 (`Synthetic Survey Authority Beta` / `SYN-QBC-002` at lon 40.2). Envelope lon/lat≈40–40.4 (disjoint from deepen / unc A/B).

### BCAST-2 fixture (`synthetic_uncertainty_depth_only_fillunc_4326.h5`)

Depth-only compound; `minimumUncertainty==maximumUncertainty==1e6` (fill). Sample (35.0, 35.0) depth=9.0 → **`has_uncertainty==false`** (no broadcast). Oracle **`BCAST-2`**.



### Fixture MG-PS oracles (`synthetic_qbc_multigrid_vd_4326.h5`) — `IHO-S102-QBC-MULTIGRID`

| Id | Assert |
|----|--------|
| **QBC-MG-1** | Open; `grids.size()≥2`; `has_qbc_coverage==true` |
| **QBC-MG-2** | `sample_wgs84(50.0, 50.0)` → `has_qbc`, `qbc_id==1`, lookup auth |
| **QBC-MG-3** | `grids[0]` and `grids[1]` `sample_cell(0,0)` both `has_qbc` |
| **QBC-MG-4** | Portray: no QBC id CoverageFill; depth CoverageFill ≥2 |
