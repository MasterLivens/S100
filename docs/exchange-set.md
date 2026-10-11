# Exchange set + update packages (P1)

Owner: SoftwareArchitecture (contracts). Implement: SoftwareEngineer. Test: SoftwareTester.

## Goals

- Parse S-100 `CATALOG.XML` into a structured member list.
- Open an exchange-set root: load base `.000` cells and apply sequential `.001`, `.002`, … updates.
- Hold multiple loaded `Dataset`s in a `ChartSession` for multi-cell viewing (Coverage hooks later).

`open_dataset(path)` stays the single-file entry. Exchange-set APIs are parallel.

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
| FOID continuity / DSID ED·UPD validation | **stub** |
| Compressed / encrypted / signed exchange sets | **out of scope** |

`open_dataset` on a lone `.001` still decodes that file standalone. Prefer `open_dataset_with_updates` / `ChartSession`.

## CLI

```
s100_cli <CATALOG.XML|exchange-set-dir> [--list] [--load <cell>|--load-all]
s100_cli <file.000> ...   # unchanged; auto-applies sibling .001… when present
```
