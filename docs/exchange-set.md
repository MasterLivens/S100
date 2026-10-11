# Exchange set + update packages (P1)

Owner: SoftwareArchitecture (contracts). Implement: SoftwareEngineer. Test: SoftwareTester.

## Goals

- Parse S-100 `CATALOG.XML` into a structured member list.
- Open an exchange-set root: load base `.000` cells and apply sequential `.001`, `.002`, … updates.
- Hold multiple loaded `Dataset`s in a `ChartSession` for multi-cell viewing (Coverage hooks later).

`open_dataset(path)` stays the single-file entry for **base cells** (`.000` and non-update products). Exchange-set APIs are parallel. Lone update files (`.001`…) are **rejected** by `open_dataset` — must use `open_dataset_with_updates` / `ChartSession`.

## API surface

```cpp
bool parse_exchange_catalogue(path /*file or dir*/, ExchangeCatalogue&, LoadError*);
std::vector<ExchangeCellSeries> group_cell_series(const ExchangeCatalogue&);

std::vector<path> discover_update_files(path base_000);
bool open_dataset_with_updates(path base, const vector<path>& updates, Dataset&, LoadError*);
bool open_cell_series(const ExchangeCellSeries&, Dataset&, LoadError*);
bool apply_s101_update(Dataset&, path update, LoadError*);

class ChartSession {
  bool open_exchange_set(path);
  vector<string> member_keys() const;
  bool load_member(const string& key, LoadError*);
  size_t load_all_members(LoadError* first = nullptr);
  Dataset* find(const string& key);
};
```

## Update semantics coverage

| Area | Status |
|------|--------|
| Discover / order `.001`… by extension | **full** |
| Feature RUIN Insert / Delete | **full** (by FRID RCID) |
| Feature RUIN Modify + ATTR ATIN insert/delete/modify | **full** |
| Feature Modify + SPAS SAUI insert/delete/modify | **full** |
| Spatial RUIN Insert / Delete / Modify (replace record) | **full** for PRID/MRID/CRID/CCID/SRID |
| Rebuild feature geometries from `SpatialStore` after updates | **full** |
| Information RUIN Insert / Delete / Modify (+ ATIN) | **best-effort** |
| FASC / INAS on feature Modify | **stub**: replace whole association list when field present (no FAUI/IUIN element merge) |
| FOID continuity / DSID ED·UPD validation | **required** — updates apply in ED/UPD order; each step must match Dataset DSID (and catalogue metadata when present). Filename sort alone is not enough. |
| Compressed / encrypted / signed exchange sets | **out of scope** |

## Frozen decisions (2026-10-11)

1. **One `ChartSession` for all products.** S-101 and S-102 (and later S-104/111/124) share the same session. Member key = `ProductId` + dataset identity (prefer catalogue file identifier / DSID; fall back to relative path). No parallel coverage map.
2. **FASC/INAS element-level merge (FAUI/IUIN)** stays stub for exchange-set MVP; must be real before bridge/composite features and before Track B. Until then, whole-list replace when the field is present is acceptable.
3. **Update sequencing** uses DSID edition/update numbers (and catalogue metadata when present), not extension order alone. Out-of-order or mismatched ED/UPD → `LoadError`, do not partially apply.
4. **`open_dataset(.001)` rejected.** Return `LoadErrorKind::Invalid` with a clear message pointing at `open_dataset_with_updates`. Base `.000` (and non-update product paths) unchanged.


## CLI

```
s100_cli <CATALOG.XML|exchange-set-dir> [--list] [--load <cell>|--load-all]
s100_cli <file.000> ...   # base cell; may auto-discover sibling .001… via with_updates
s100_cli <file.001> ...   # must fail (use exchange set / with_updates)
```
