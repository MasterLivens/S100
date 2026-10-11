# S-164 display reference images (final visual acceptance)

**Role:** Final human/visual acceptance baselines for S-101 symbol & layer portrayal.
They do **not** gate the current P1 CI golden (`../iho_0008.png`), which remains a soft Day-palette / no-magenta / drift check only.

| File | Meaning | Upstream path |
|------|---------|---------------|
| `AA5DBASE.png` | Display **Base** sample | [iho-ohi/S-164-Sub-Group](https://github.com/iho-ohi/S-164-Sub-Group) `manual/dev/images/3.1/AA5DBASE.png` |
| `AA5STNDR.png` | Display **Standard** sample | same repo `manual/dev/images/3.1/AA5STNDR.png` |

| Field | Value |
|-------|-------|
| Source repo | https://github.com/iho-ohi/S-164-Sub-Group |
| Fetched | 2026-10-11 |
| Status | Draft manual / staging figures — **not** a published type-approval plot pack. Formal S-164 release plots supersede these when available. |
| SHA-256 AA5DBASE | see `SHA256SUMS.txt` |
| SHA-256 AA5STNDR | see `SHA256SUMS.txt` |

## How we use them

1. **Final acceptance (manual):** after portrayal work (symbols, fills, display category Base/Standard), compare our viewer offscreen snapshot of the matching S-164 / IHO display cell to these figures for symbol presence, colours, and layering — not pixel-identical.
2. **Not blocking now:** text collision, OpenS100 side-by-side, and automated SSIM remain follow-ups; current roadmap (S-102 CoverageFill → …) is unchanged.
3. **OpenS100:** optional later capture on Windows/MFC for the same cell; no archived goldens in that project today.
