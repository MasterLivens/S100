#!/usr/bin/env python3
"""Regenerate testdata/s104/synthetic_waterlevel_4326.h5 (needs h5py)."""
from pathlib import Path
import numpy as np
import h5py

ROOT = Path(__file__).resolve().parents[1]
out = ROOT / "testdata/s104/synthetic_waterlevel_4326.h5"

nrows, ncols = 5, 5
origin_lon, origin_lat = 0.0, 0.0
spacing = 0.1
west, south, east, north = 0.0, 0.0, 0.4, 0.4
dtype = np.dtype([("waterLevelHeight", "<f4"), ("waterLevelTrend", "u1")])
frames = [("2024-01-01T00:00:00Z", 1.0), ("2024-01-01T06:00:00Z", 2.5)]

def make_grid(height):
    arr = np.zeros((nrows, ncols), dtype=dtype)
    arr["waterLevelHeight"] = np.float32(height)
    arr["waterLevelTrend"] = np.uint8(1)
    return arr

with h5py.File(out, "w") as f:
    f.attrs["productSpecification"] = np.bytes_("INT.IHO.S-104.2.0.0")
    f.attrs.create("horizontalCRS", np.int32(4326))
    for name, val in [("westBoundLongitude", west), ("eastBoundLongitude", east),
                      ("southBoundLatitude", south), ("northBoundLatitude", north)]:
        f.attrs.create(name, np.float32(val))
    wl = f.create_group("WaterLevel")
    wl.attrs.create("dataCodingFormat", np.int32(2))
    inst = wl.create_group("WaterLevel.01")
    inst.attrs.create("gridOriginLongitude", np.float64(origin_lon))
    inst.attrs.create("gridOriginLatitude", np.float64(origin_lat))
    inst.attrs.create("gridSpacingLongitudinal", np.float64(spacing))
    inst.attrs.create("gridSpacingLatitudinal", np.float64(spacing))
    inst.attrs.create("numPointsLongitudinal", np.uint32(ncols))
    inst.attrs.create("numPointsLatitudinal", np.uint32(nrows))
    inst.attrs.create("numGRP", np.uint32(len(frames)))
    for name, val in [("westBoundLongitude", west), ("eastBoundLongitude", east),
                      ("southBoundLatitude", south), ("northBoundLatitude", north)]:
        inst.attrs.create(name, np.float32(val))
    inst.attrs["startSequence"] = np.bytes_("0,0")
    for i, (tp, h) in enumerate(frames, start=1):
        g = inst.create_group(f"Group_{i:03d}")
        g.attrs["timePoint"] = np.bytes_(tp)
        g.create_dataset("values", data=make_grid(h))
print("wrote", out)
