# Feature / information associations (FASC · INAS) — Bridge slice (rev 2)

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Prerequisite for bridge / structure–equipment composites and Track B.

## Goals

1. Element-level FAUI/IUIN merge on updates (no stub whole-list replace when FAUI/IUIN is present).
2. Portrayal sees associations only via Part 9A Host — viewer never walks FASC.
3. Bridge / composite PC rules that call `HostFeatureGetAssociated*` work on real cells.

## Identity & match keys

Association member identity (used for Delete / Modify match):

```
(rcnm, rcid, association, role)
```

- All four fields required for a match.
- Multiple exact matches → Delete removes **all** of them; Modify updates **all** of them (same attribute patch).
- Modify **must not** change identity fields; only `AssociationRef::attributes` is replaced from the update member.

FAUI / IUIN control byte (Part 10a): `1` Insert, `2` Delete, `3` Modify. Any other value → unknown.

## Update merge

Applies when a feature (or information) record is **Modify** and carries FASC/INAS fields.

| Shape | Behaviour |
|-------|-----------|
| Each FASC/INAS field carries FAUI/IUIN | Apply Insert / Delete / Modify in **field order** against the existing list |
| Complete FASC/INAS set present and **no** FAUI/IUIN substructure | Whole-list replace (legacy shape) |
| FASC/INAS fields absent | Leave existing list untouched |

### Failure granularity

- Unknown FAUI/IUIN value, truncated field, or Modify with **no** identity match → `LoadError` for **that update file application**.
- Do **not** leave that feature's association list half-applied: on error, roll back association changes for the **current feature record** to the pre-record state (other features already applied in this update file stay). Prefer: apply association ops for one feature in a temporary copy, commit only on success.
- Do **not** abort the entire exchange-set session; caller sees `LoadError` from `apply_s101_update` / `open_dataset_with_updates`.

When this lands, set `docs/exchange-set.md` FASC row to **full** and remove the stub sentence from `docs/api-contracts.md` (synced in this rev).

## Host surface

### Feature associations (FASC / INAS filters)

- `HostFeatureGetAssociatedFeatureIDs(featureId, associationCode, roleCode)`
- `HostFeatureGetAssociatedInformationIDs(featureId, associationCode, roleCode)`

| Argument | Rule |
|----------|------|
| `associationCode` | **Required** (non-empty). Empty / nil → Host error (Lua error), do not treat as wildcard. |
| `roleCode` | Optional. `nil` or `""` = **any role** for that association. Non-empty → exact role match. |

Return only peer IDs that exist in the current `Dataset`. Dangling `rcid` → omit + `HostDebuggerEntry`; never crash Lua.

### Spatial associations (separate API — do not mix)

- `HostSpatialGetAssociatedFeatureIDs(spatialId)` / `HostSpatialGetAssociatedInformationIDs(spatialId)`

These are **ID-only** (no association/role filter). They return features/information linked via spatial topology / SPAS, not FASC. Do not add FASC filters to these entry points.

### Viewer

No viewer-side joins. Bridge drawing = official Lua instructions only.

## Test oracles (CTest sketches)

Add under `tests/` (names suggestive; Tester may rename):

1. **`association_faui_merge`**
   - Fixture: minimal in-memory or tiny `.000` + `.001` under `testdata/s101/assoc/` (or build Dataset in C++).
   - Start: feature A has FASC → B (`assoc=supports`, `role=supportedBy`).
   - Update Insert: A → C same assoc/role → list size 2, contains B and C.
   - Update Delete: remove A→B by identity → only C remains.
   - Update Modify: change attributes on A→C → identity unchanged, attributes equal patch.
   - Unknown FAUI=`9` → `apply_s101_update` returns false; A's list unchanged from pre-update.

2. **`association_host_query`**
   - After load (CHS or fixture with known Bridge graph, or synthetic):
   - `HostFeatureGetAssociatedFeatureIDs(A, "supports", "")` returns expected peer id set (order irrelevant).
   - Same with `roleCode="supportedBy"` filters correctly.
   - Empty `associationCode` errors.
   - Dangling rcid in list does not appear in returned IDs.

3. **`association_spatial_no_fasc_filter`** (smoke)
   - `HostSpatialGetAssociatedFeatureIDs` callable with spatial id only; documenting no assoc/role args.

Engineer delivers tests + implementation together; Tester owns pass/fail and fixtures.toml style data if files are used.

## Bridge scope

- Host queries work on IHO/CHS cells that have Bridge / structure–equipment associations.
- No hand-written Bridge symbology in Qt.
- Text collision still later.

## Later

S-98 IO catalogues, cross-dataset associations, Track B alerts.
