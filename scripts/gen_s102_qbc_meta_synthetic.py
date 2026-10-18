#!/usr/bin/env python3
"""Generate S-102 QBC-META fixture C + BCAST-2 depth-only fill-minmax (docs/s102-qbc-meta-contract.md).

Needs h5py. Envelope C: lon/lat ≈40–40.4 (disjoint from deepen / unc A/B).
BCAST-2: lon/lat ≈35–35.4, depth-only, minUncertainty==maximumUncertainty==1e6 (fill) → no broadcast.
"""
from pathlib import Path
import numpy as np
import h5py

ROOT = Path(__file__).resolve().parents[1]
OUT_C = ROOT / "testdata/s102/synthetic_qbc_meta_4326.h5"
OUT_B2 = ROOT / "testdata/s102/synthetic_uncertainty_depth_only_fillunc_4326.h5"

FILL = np.float32(1.0e6)
ORIGIN_C = (40.0, 40.0)
ORIGIN_B2 = (35.0, 35.0)
SPACING = 0.1
NROWS = NCOLS = 5

DEPTH_C = np.float32(15.0)
UNC_C = np.float32(1.5)
DEPTH_B2 = np.float32(9.0)

# Fixture C oracles (SOURCE.md):
# C1 (40.0, 40.0) row0,col0 → qbc_id=1, surveyAuthority Alpha
# C2 (40.1, 40.0) row0,col1 → qbc_id=0 (fill)
AUTH_1 = "Synthetic Survey Authority Alpha"
AUTH_2 = "Synthetic Survey Authority Beta"
SRC_1 = "SYN-QBC-001"
SRC_2 = "SYN-QBC-002"


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


def _id_row():
    return ("iD", "iD", "", "0", "H5T_STD_U32LE", "1", "", "geSemiInterval")


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


def _write_bathy_instance(parent, name, origin, west, south, east, north, values, min_d, max_d, min_u, max_u):
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


def _fat_dtype():
    str_t = h5py.string_dtype("utf-8")
    return np.dtype([
        ("id", "<u4"),
        ("dataAssessment", "u1"),
        ("featuresDetected.leastDepthOfDetectedFeaturesMeasured", "u1"),
        ("featuresDetected.significantFeaturesDetected", "u1"),
        ("featuresDetected.sizeOfFeaturesDetected", "<f4"),
        ("featureSizeVar", "<f4"),
        ("fullSeafloorCoverageAchieved", "u1"),
        ("bathyCoverage", "u1"),
        ("zoneOfConfidence.horizontalPositionUncertainty.uncertaintyFixed", "<f4"),
        ("zoneOfConfidence.horizontalPositionUncertainty.uncertaintyVariableFactor", "<f4"),
        ("surveyDateRange.dateStart", str_t),
        ("surveyDateRange.dateEnd", str_t),
        ("sourceSurveyID", str_t),
        ("surveyAuthority", str_t),
        ("typeOfBathymetricEstimationUncertainty", "u1"),
    ])


def write_c():
    west, south = ORIGIN_C
    east, north = west + (NCOLS - 1) * SPACING, south + (NROWS - 1) * SPACING
    dtype = np.dtype([("depth", "<f4"), ("uncertainty", "<f4")])
    arr = np.zeros((NROWS, NCOLS), dtype=dtype)
    arr["depth"] = DEPTH_C
    arr["uncertainty"] = UNC_C

    ids = np.ones((NROWS, NCOLS), dtype=np.uint32)
    ids[0, 1] = 0  # C2 fill
    ids[0, 2] = 2  # second authority
    # remaining stay 1

    fat = np.zeros(2, dtype=_fat_dtype())
    fat[0] = (
        1, 1, 0, 0, 0.0, 0.0, 1, 1, 5.0, 0.0,
        "2020-01-01", "2020-12-31", SRC_1, AUTH_1, 1,
    )
    fat[1] = (
        2, 2, 0, 0, 0.0, 0.0, 1, 1, 10.0, 0.05,
        "2018-06-01", "2018-06-30", SRC_2, AUTH_2, 2,
    )

    with h5py.File(OUT_C, "w") as f:
        _write_root(f, west, south, east, north)
        bc = f.create_group("BathymetryCoverage")
        _write_bathy_instance(
            bc, "BathymetryCoverage.01", ORIGIN_C, west, south, east, north, arr,
            float(DEPTH_C), float(DEPTH_C), float(UNC_C), float(UNC_C),
        )
        qbc = f.create_group("QualityOfBathymetryCoverage")
        qbc.attrs.create("dataCodingFormat", np.uint8(9))
        qbc.attrs.create("dimension", np.uint8(2))
        qbc.attrs.create("numInstances", np.uint8(1))
        qbc.create_dataset("featureAttributeTable", data=fat)
        inst = qbc.create_group("QualityOfBathymetryCoverage.01")
        ox, oy = ORIGIN_C
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
        g.create_dataset("values", data=ids)

        gf = f.create_group("Group_F")
        gf.create_dataset(
            "featureCode",
            data=np.array([b"BathymetryCoverage", b"QualityOfBathymetryCoverage"], dtype=object),
        )
        rows = np.array([_depth_row(), _unc_row()], dtype=_group_f_dtype())
        gf.create_dataset("BathymetryCoverage", data=rows)
        qrows = np.array([_id_row()], dtype=_group_f_dtype())
        gf.create_dataset("QualityOfBathymetryCoverage", data=qrows)

    print("wrote", OUT_C)
    print(f"  C1 oracle: lon={west} lat={south} qbc_id=1 auth={AUTH_1!r} src={SRC_1!r}")
    print(f"  C2 oracle: lon={west + SPACING} lat={south} qbc_id=0 (fill)")
    print(f"  C3: has_qbc_coverage=true dataCodingFormat=9")


def write_b2():
    """Depth-only with fill min==max → no broadcast (BCAST-2)."""
    west, south = ORIGIN_B2
    east, north = west + (NCOLS - 1) * SPACING, south + (NROWS - 1) * SPACING
    dtype = np.dtype([("depth", "<f4")])
    arr = np.zeros((NROWS, NCOLS), dtype=dtype)
    arr["depth"] = DEPTH_B2
    with h5py.File(OUT_B2, "w") as f:
        _write_root(f, west, south, east, north)
        bc = f.create_group("BathymetryCoverage")
        _write_bathy_instance(
            bc, "BathymetryCoverage.01", ORIGIN_B2, west, south, east, north, arr,
            float(DEPTH_B2), float(DEPTH_B2), float(FILL), float(FILL),
        )
        gf = f.create_group("Group_F")
        gf.create_dataset("featureCode", data=np.array([b"BathymetryCoverage"], dtype=object))
        rows = np.array([_depth_row()], dtype=_group_f_dtype())
        gf.create_dataset("BathymetryCoverage", data=rows)
    print("wrote", OUT_B2)
    print(f"  depth-only fill min==max; sample lon={west} lat={south} → has_uncertainty==false")


if __name__ == "__main__":
    write_c()
    write_b2()
