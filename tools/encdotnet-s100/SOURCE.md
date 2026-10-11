# EncDotNet.S100 `s100` CLI (Linux final-render soft-compare oracle)

| Field | Value |
|-------|-------|
| Product | EncDotNet.S100 headless CLI (`s100`) |
| Pinned version | **0.24.0** (`0.24.0+701db435ab4a9ff52cb13a2d0e51f7418e7df149`) |
| Release tag | `v0.24.0` |
| Asset | `s100-0.24.0-linux-x64.tar.gz` (linux-x64 self-contained, ~62 MiB) |
| Download URL | https://github.com/philliphoff/EncDotNet.S100/releases/download/v0.24.0/s100-0.24.0-linux-x64.tar.gz |
| SHA-256 | `9ea3d351aa071d4ae2fcbb5d545e00d318128ade8551f8b016dde2302e5f3416` |
| Upstream | https://github.com/philliphoff/EncDotNet.S100 |
| License | MIT |
| Role in this repo | **Warn-only** soft oracle for Day-palette final-render compares. Does **not** replace `testdata/golden/iho_0008.png` or `testdata/golden/s164-ref/`. |

## Install (do not commit the binary)

From the repo root:

```bash
./scripts/fetch_encdotnet_s100.sh
# → extracts into tools/encdotnet-s100/ (gitignored except this SOURCE.md)
./tools/encdotnet-s100/s100 --version
```

Optional: set `ENCDOTNET_S100` to an already-extracted `s100` binary (e.g. a CI cache path).

The tarball and extracted tree are **gitignored** (tens of MB of .NET runtime + Skia). Keep them in a local/vendor/CI cache only.

## Render smoke

```bash
./scripts/render_encdotnet_oracle.sh
# PNGs under Testing/encdotnet-oracle/ (gitignored)
```

See `docs/testing/visual-acceptance.md`.
