# S-124 test cells

## Tracked (CI)

| File | Role |
|------|------|
| `synthetic_navwarning.gml` | **Gittracked** (shipped with contract). `IHO-S124-OPEN-LIST` / `s124_open_list`. |

### Oracle

See `docs/s124-contract.md` fixture table: `NW.1` point (0.2,0.2); `NW.2` ring `0.0 0.0 0.4 0.0 0.4 0.4 0.0 0.4 0.0 0.0`; times `2024-01-01T00:00:00Z` / `T06:00:00Z`.

Parser accepts the synthetic element names in that file (`S124:NavigationalWarning`, `gml:pos` / `gml:posList`, lon lat order).
