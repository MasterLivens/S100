# HDF5 (system dependency, not vendored)

S-102 / future S-104 / S-111 loaders link the **system** HDF5 C library (serial). Core stays
Qt-free; only `s100_core` links HDF5.

| Field | Value |
|-------|-------|
| API | HDF5 C API (`hdf5.h`), CMake `find_package(HDF5 REQUIRED COMPONENTS C)` |
| Linux (dev) | Debian/Ubuntu `libhdf5-dev` → `/usr/include/hdf5/serial`, `libhdf5_serial` |
| Why not vendored | HDF5 is large; packaging provides a maintained build. Pin via distro package in CI. |
| Probe | `pkg-config --cflags --libs hdf5` or CMake `HDF5_INCLUDE_DIRS` / `HDF5_LIBRARIES` |

Fetched / linked 2026-10-11 on the Eng box: `libhdf5-dev` (serial). No HDF5 sources are copied
into `third_party/`.
