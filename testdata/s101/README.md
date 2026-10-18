
# S-101 test cells

Public IHO samples from [iho-ohi/S-101-Test-Datasets](https://github.com/iho-ohi/S-101-Test-Datasets).

- `101AA00DS0003.000`
- `101AA00DS0006.000`
- `101AA00DS0008.000` (default for P0 smoke)

Test/development only — not for navigation.

## `updates/` — synthetic update-robustness fixtures

Generated from `101AA00DS0003.000` by `tools/gen_update_fixtures.py` (stdlib only, fixed seed,
deterministic; see `docs/update-robustness-contract.md` rev 1.2). Bytes are committed; sha256 of
each file is pinned in `fixtures.toml` (`[[update_fixture]]`). Regenerate with
`python3 tools/gen_update_fixtures.py --write-manifest`. Test/development only.
