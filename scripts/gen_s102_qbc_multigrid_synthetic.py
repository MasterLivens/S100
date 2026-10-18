#!/usr/bin/env python3
"""Generate S-102 QBC-MULTIGRID fixture MG-PS (docs/s102-qbc-multigrid-contract.md).

Co-located BathymetryCoverage.01 + .02 (distinct verticalDatum), shared QoBC.01.
Needs h5py. Envelope ≈50–50.4 (disjoint from deepen / unc / qbc-meta).
"""
from pathlib import Path
import numpy as np
import h5py

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "testdata/s102/synthetic_qbc_multigrid_vd_4326.h5"

ORIGIN = (50.0, 50.0)
SPACING = 0.1
NROWS = NCOLS = 5
DEPTH_01 = np.float32(12.0)
DEPTH_02 = np.float32(22.0)
UNC = np.float32(1.0)
AUTH = "Synthetic QBC-MG Authority"
SRC = "SYN-QBC-MG-001"


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


def _write_bathy(parent, name, depth_val, vertical_datum):
    west, south = ORIGIN
    east = west + (NCOLS - 1) * SPACING
    north = south + (NROWS - 1) * SPACING
    dtype = np.dtype([("depth", "<f4"), ("uncertainty", "<f4")])
    arr = np.zeros((NROWS, NCOLS), dtype=dtype)
    arr["depth"] = depth_val
    arr["uncertainty"] = UNC
    inst = parent.create_group(name)
    ox, oy = ORIGIN
    inst.attrs.create("gridOriginLongitude", np.float64(ox))
    inst.attrs.create("gridOriginLatitude", np.float64(oy))
    inst.attrs.create("gridSpacingLongitudinal", np.float64(SPACING))
    inst.attrs.create("gridSpacingLatitudinal", np.float64(SPACING))
    inst.attrs.create("numPointsLongitudinal", np.uint32(NCOLS))
    inst.attrs.create("numPointsLatitudinal", np.uint32(NROWS))
    inst.attrs.create("numGRP", np.uint8(1))
    inst.attrs.create("verticalDatum", np.uint16(vertical_datum))
    inst.attrs["startSequence"] = np.bytes_("0,0")
    for n, v in [
        ("westBoundLongitude", west),
        ("eastBoundLongitude", east),
        ("southBoundLatitude", south),
        ("northBoundLatitude", north),
    ]:
        inst.attrs.create(n, np.float32(v))
    g = inst.create_group("Group_001")
    g.attrs.create("minimumDepth", np.float32(depth_val))
    g.attrs.create("maximumDepth", np.float32(depth_val))
    g.attrs.create("minimumUncertainty", np.float32(UNC))
    g.attrs.create("maximumUncertainty", np.float32(UNC))
    g.create_dataset("values", data=arr)


def main():
    west, south = ORIGIN
    east = west + (NCOLS - 1) * SPACING
    north = south + (NROWS - 1) * SPACING

    ids = np.ones((NROWS, NCOLS), dtype=np.uint32)
    ids[0, 1] = 0  # fill cell for contrast
    fat = np.zeros(2, dtype=_fat_dtype())
    fat[0] = (1, 1, 0, 0, 0.0, 0.0, 1, 1, 5.0, 0.0, "2021-01-01", "2021-12-31", SRC, AUTH, 1)
    fat[1] = (2, 2, 0, 0, 0.0, 0.0, 1, 1, 8.0, 0.0, "2019-01-01", "2019-06-30", "SYN-QBC-MG-002",
              "Synthetic QBC-MG Authority Beta", 2)
    ids[0, 2] = 2

    with h5py.File(OUT, "w") as f:
        f.attrs["productSpecification"] = np.bytes_("INT.IHO.S-102.3.0.0")
        f.attrs.create("horizontalCRS", np.int32(4326))
        for n, v in [
            ("westBoundLongitude", west),
            ("eastBoundLongitude", east),
            ("southBoundLatitude", south),
            ("northBoundLatitude", north),
        ]:
            f.attrs.create(n, np.float32(v))

        bc = f.create_group("BathymetryCoverage")
        # Distinct verticalDatum; same geometry (co-located multi-VD).
        _write_bathy(bc, "BathymetryCoverage.01", DEPTH_01, vertical_datum=3)  # MSL-ish
        _write_bathy(bc, "BathymetryCoverage.02", DEPTH_02, vertical_datum=24)  # LAT-ish

        qbc = f.create_group("QualityOfBathymetryCoverage")
        qbc.attrs.create("dataCodingFormat", np.uint8(9))
        qbc.attrs.create("dimension", np.uint8(2))
        qbc.attrs.create("numInstances", np.uint8(1))
        qbc.create_dataset("featureAttributeTable", data=fat)
        inst = qbc.create_group("QualityOfBathymetryCoverage.01")
        ox, oy = ORIGIN
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
        rows = np.array([
            ("depth", "depth", "metres", "1000000", "H5T_FLOAT", "-14", "11050", "closedInterval"),
            ("uncertainty", "uncertainty", "metres", "1000000", "H5T_FLOAT", "0", "", "geSemiInterval"),
        ], dtype=_group_f_dtype())
        gf.create_dataset("BathymetryCoverage", data=rows)
        qrows = np.array([
            ("iD", "iD", "", "0", "H5T_STD_U32LE", "1", "", "geSemiInterval"),
        ], dtype=_group_f_dtype())
        gf.create_dataset("QualityOfBathymetryCoverage", data=qrows)

    print("wrote", OUT)
    print(f"  MG-PS: BC.01 depth={DEPTH_01} VD=3; BC.02 depth={DEPTH_02} VD=24; shared QoBC")
    print(f"  QBC-MG-2 sample_wgs84 lon={west} lat={south} → qbc_id=1")
    print(f"  QBC-MG-3 sample_cell(0,0) on grids[0] and grids[1] both has_qbc")


if __name__ == "__main__":
    main()
