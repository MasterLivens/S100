# Feature / information associations (FASC · INAS) — Bridge slice

Owner: SoftwareArchitecture (2026-10-11). Implement: SoftwareEngineer. Test: SoftwareTester.
Prerequisite for bridge / structure–equipment composites and Track B. Supersedes the exchange-set MVP stub for element-level merge.

## Goals

1. Decode and update associations with **element-level** FAUI/IUIN (no whole-list replace when only some members change).
2. Expose associations to portrayal through existing Part 9A Host calls — viewer never walks FASC itself.
3. Enable Bridge and similar composites whose PC rules query associated features/information.

## Data model (already largely present)

`AssociationRef`: `{rcnm, rcid, association, role, attributes}` on:
- `Feature::feature_associations` (FASC)
- `Feature::information_associations` (INAS)
- spatial / information records as already decoded

Association and role codes resolve via dataset code lists + FC (`feature_associations` / `information_associations` maps). Keep opaque string codes at the façade; do not invent parallel enums.

## Update merge (this slice — must ship)

On feature (or information) **Modify**:

| Control | Meaning |
|---------|---------|
| FAUI / IUIN Insert | Append association member |
| FAUI / IUIN Delete | Remove matching member (match on rcnm+rcid+association+role; if ambiguous, delete all exact matches) |
| FAUI / IUIN Modify | Replace attributes on the matched member; identity fields unchanged |
| Field absent | Leave existing list untouched |
| Full-field replace without FAUI | Only if the update record carries a complete FASC/INAS set **and** no FAUI/IUIN substructure — keep current whole-list replace as fallback for that shape only |

Out-of-order / unknown FAUI → `LoadError`, do not partially apply that feature's associations.

Update `docs/exchange-set.md` status row from stub → **full** when this lands.

## Host surface (portrayal)

Required (already in `host-functions.txt`):

- `HostFeatureGetAssociatedFeatureIDs(featureId, associationCode, roleCode)`
- `HostFeatureGetAssociatedInformationIDs(...)`
- `HostSpatialGetAssociatedFeatureIDs` / `...InformationIDs`

Rules:

1. Filter by association + role codes from the PC call; empty role/association means "any" only if Part 9A / PortrayalAPI does that — match whatever `PortrayalAPI.lua` passes through, do not invent wildcards beyond it.
2. Return IDs that exist in the current `Dataset` after updates; dangling rcid → omit + debug log / `HostDebuggerEntry`, do not crash Lua.
3. **No viewer-side joins.** Bridge drawing comes only from instructions emitted by official Lua that already walked associations via Host.

## Bridge / composite scope (this slice)

- Ensure Host association queries work on IHO + CHS cells that contain Bridge / structure–equipment graphs.
- Add CTest: after a synthetic or real update that uses FAUI insert/delete, association lists match expected members; Host query returns the expected peer IDs.
- Do **not** hand-write Bridge symbology in Qt. If a cell's PC emits nothing useful for Bridge, fix Host/data, not the renderer.
- Text collision still out of scope.

## Explicitly later

S-98 interoperability association catalogues, Cross-dataset associations, Track B alert associations.
