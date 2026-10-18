#!/usr/bin/env python3
"""Generate S-102 uncertainty synthetic fixtures A + B (docs/s102-uncertainty-contract.md).

Needs h5py. Envelope disjoint from deepen synthetic_multigrid_4326 (0–0.4 / 10–10.4).
"""
from pathlib import Path
import numpy as np
import h5py

ROOT = Path(__file__).resolve().parents[1]
OUT_A = ROOT / "testdata/s102/synthetic_uncertainty_4326.h5"
OUT_B = ROOT / "testdata/s102/synthetic_uncertainty_depth_only_4326.h5"

FILL = np.float32(1.0e6)
# Fixture A: lon/lat ≈ 20–20.4 (disjoint from deepen)
ORIGIN_A = (20.0, 20.0)
# Fixture B: lon/lat ≈ 30–30.4
ORIGIN_B = (30.0, 30.0)
SPACING = 0.1
NROWS = NCOLS = 5

# Oracle points (cell centres ≈ origin + i*spacing)
# Point A (fixture A): row0,col0 → depth=12.5, unc=2.5
# Point B (fixture A): row0,col1 → depth=12.5, unc=FILL → has_uncertainty=false
DEPTH_A = np.float32(12.5)
UNC_A = np.float32(2.5)
# Fixture B constant (min==max, not fill); loader does not broadcast → has_uncertainty=false
UNC_CONST_B = np.float32(0.75)
DEPTH_B = np.float32(8.0)


def _group_f_dtype():
    return np.dtype([
        ("code", h5py.string_dtype("utf-8")),
        ("name", h5py.string_dtype("utf-8")),
        ("uom.name", h5py.string_dtype("utf-8")),
        ("fillValue", h5py.string_dtype("utf-8")),
        ("datatype", h5py.string_dtype("utf-8")),
        ("lower", h5py.string_dtype("utf-8")),
        ("upper", h5py.string_dtype("utf-8")),
        ("closure", h5py.string_dtype("utf-8")),
    ])


def _depth_row():
    return ("depth", "depth", "metres", "1000000", "H5T_FLOAT", "-14", "11050", "closedInterval")


def _unc_row():
    return ("uncertainty", "uncertainty", "metres", "1000000", "H5T_FLOAT", "0", "", "geSemiInterval")


def _write_root(f, west, south, east, north):
    f.attrs["productSpecification"] = np.bytes_("INT.IHO.S-102.3.0.0")
    f.attrs.create("horizontalCRS", np.int32(4326))
    for name, val in [
        ("westBoundLongitude", west),
        ("eastBoundLongitude", east),
        ("southBoundLatitude", south),
        ("northBoundLatitude", north),
    ]:
        f.attrs.create(name, np.float32(val))


def _write_instance(parent, name, origin, west, south, east, north, values, min_d, max_d, min_u, max_u):
    inst = parent.create_group(name)
    ox, oy = origin
    inst.attrs.create("gridOriginLongitude", np.float64(ox))
    inst.attrs.create("gridOriginLatitude", np.float64(oy))
    inst.attrs.create("gridSpacingLongitudinal", np.float64(SPACING))
    inst.attrs.create("gridSpacingLatitudinal", np.float64(SPACING))
    inst.attrs.create("numPointsLongitudinal", np.uint32(NCOLS))
    inst.attrs.create("numPointsLatitudinal", np.uint32(NROWS))
    inst.attrs.create("numGRP", np.uint8(1))
    inst.attrs["startSequence"] = np.bytes_("0,0")
    for n, v in [
        ("westBoundLongitude", west),
        ("eastBoundLongitude", east),
        ("southBoundLatitude", south),
        ("northBoundLatitude", north),
    ]:
        inst.attrs.create(n, np.float32(v))
    g = inst.create_group("Group_001")
    g.attrs.create("minimumDepth", np.float32(min_d))
    g.attrs.create("maximumDepth", np.float32(max_d))
    g.attrs.create("minimumUncertainty", np.float32(min_u))
    g.attrs.create("maximumUncertainty", np.float32(max_u))
    g.create_dataset("values", data=values)


def write_a():
    west, south = ORIGIN_A
    east, north = west + (NCOLS - 1) * SPACING, south + (NROWS - 1) * SPACING
    dtype = np.dtype([("depth", "<f4"), ("uncertainty", "<f4")])
    arr = np.zeros((NROWS, NCOLS), dtype=dtype)
    arr["depth"] = DEPTH_A
    arr["uncertainty"] = UNC_A
    arr["uncertainty"][0, 1] = FILL  # Point B: missing unc
    with h5py.File(OUT_A, "w") as f:
        _write_root(f, west, south, east, north)
        bc = f.create_group("BathymetryCoverage")
        _write_instance(
            bc, "BathymetryCoverage.01", ORIGIN_A, west, south, east, north, arr,
            float(DEPTH_A), float(DEPTH_A), float(UNC_A), float(UNC_A),
        )
        gf = f.create_group("Group_F")
        gf.create_dataset("featureCode", data=np.array([b"BathymetryCoverage"], dtype=object))
        rows = np.array([_depth_row(), _unc_row()], dtype=_group_f_dtype())
        gf.create_dataset("BathymetryCoverage", data=rows)
    print("wrote", OUT_A)
    print(f"  Point A oracle: lon={west} lat={south} depth={DEPTH_A} unc={UNC_A}")
    print(f"  Point B oracle: lon={west + SPACING} lat={south} depth={DEPTH_A} unc=FILL")


def write_b():
    west, south = ORIGIN_B
    east, north = west + (NCOLS - 1) * SPACING, south + (NROWS - 1) * SPACING
    dtype = np.dtype([("depth", "<f4")])  # depth-only compound (§10)
    arr = np.zeros((NROWS, NCOLS), dtype=dtype)
    arr["depth"] = DEPTH_B
    with h5py.File(OUT_B, "w") as f:
        _write_root(f, west, south, east, north)
        bc = f.create_group("BathymetryCoverage")
        _write_instance(
            bc, "BathymetryCoverage.01", ORIGIN_B, west, south, east, north, arr,
            float(DEPTH_B), float(DEPTH_B), float(UNC_CONST_B), float(UNC_CONST_B),
        )
        gf = f.create_group("Group_F")
        gf.create_dataset("featureCode", data=np.array([b"BathymetryCoverage"], dtype=object))
        # No uncertainty Group_F row
        rows = np.array([_depth_row()], dtype=_group_f_dtype())
        gf.create_dataset("BathymetryCoverage", data=rows)
    print("wrote", OUT_B)
    print(f"  depth-only; minUncertainty==maxUncertainty=={UNC_CONST_B} (not broadcast)")
    print(f"  sample lon={west} lat={south} → has_uncertainty==false")


if __name__ == "__main__":
    write_a()
    write_b()
