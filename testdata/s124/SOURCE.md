# S-124 test cells

## Tracked (CI)

| File | Role |
|------|------|
| `synthetic_navwarning.gml` | **Gittracked.** ≥2 navigational warnings for `IHO-S124-OPEN-LIST` / CTest `s124_open_list`. |

### Fixture layout (oracle)

Minimal S-124-shaped GML (Eng may adjust element names to match chosen parser, keep oracle table):

| id | title | geometry | note |
|----|-------|----------|------|
| `NW.1` | `Synthetic Hazard Alpha` | Point **lon=0.2, lat=0.2** | |
| `NW.2` | `Synthetic Area Bravo` | Polygon envelope ~0.0–0.4 lon/lat | contains (0.2,0.2) |

```
# generator optional
python3 scripts/gen_s124_synthetic.py
```

## Local / gitignored

Other files under `testdata/s124/` stay gitignored except SOURCE + synthetic gml/xml.
