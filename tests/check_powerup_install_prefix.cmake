# IHO-S164-POWERUP-INSTALL-PREFIX — docs/powerup-install-prefix-contract.md rev1.
# MUST NOT SKIP. Installs into a temp prefix and asserts PU-2 layout + library Pass.
cmake_minimum_required(VERSION 3.16)

if(NOT SRC OR NOT BIN)
  message(FATAL_ERROR "usage: -DSRC=... -DBIN=... [-DCONFIG=Release]")
endif()

if(NOT CONFIG OR CONFIG STREQUAL "")
  set(CONFIG Release)
endif()

set(PREFIX "${BIN}/install_prefix_smoke")
file(REMOVE_RECURSE "${PREFIX}")
file(MAKE_DIRECTORY "${PREFIX}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" --install "${BIN}" --prefix "${PREFIX}" --config "${CONFIG}"
  RESULT_VARIABLE ir
  OUTPUT_VARIABLE iout
  ERROR_VARIABLE ierr
)
if(NOT ir EQUAL 0)
  message(FATAL_ERROR "cmake --install failed (${ir}):\n${iout}\n${ierr}")
endif()

set(ROOT "${PREFIX}/share/s100")
if(NOT IS_DIRECTORY "${ROOT}")
  message(FATAL_ERROR "missing install catalogue root: ${ROOT}")
endif()

set(MUST
  catalogues/S-101/S-101_FC_2.0.0.xml
  catalogues/S-101/PC_2.0.0/portrayal_catalogue.xml
  catalogues/S-102/S-102_FC_3.0.0.xml
  catalogues/S-102/PC_3.0.0/portrayal_catalogue.xml
  catalogues/S-104/S-104_FC_2.0.0.xml
  catalogues/S-111/S-111_FC_2.0.0.xml
  catalogues/S-111/PC_2.0.0/portrayal_catalogue.xml
  catalogues/S-124/S-124_FC_2.0.0.xml
  catalogues/S-124/PC_2.0.0/portrayal_catalogue.xml
)
foreach(rel ${MUST})
  if(NOT EXISTS "${ROOT}/${rel}")
    message(FATAL_ERROR "PU-2 missing after install: ${ROOT}/${rel}")
  endif()
endforeach()

if(EXISTS "${ROOT}/catalogues/S-104/PC_2.0.0")
  message(FATAL_ERROR "S-104 PC must not be installed")
endif()

if(EXISTS "${PREFIX}/testdata" OR EXISTS "${ROOT}/testdata")
  message(FATAL_ERROR "testdata must not be installed into prefix")
endif()

# Library check with env override (install-tree; PU-4/5 N/A).
set(ENV{S100_CATALOGUE_ROOT} "${ROOT}")
find_program(S100_TESTS_BIN s100_tests HINTS "${BIN}" "${BIN}/${CONFIG}" REQUIRED)
execute_process(
  COMMAND "${S100_TESTS_BIN}" powerup_install_prefix
  RESULT_VARIABLE tr
  OUTPUT_VARIABLE tout
  ERROR_VARIABLE terr
)
if(NOT tr EQUAL 0)
  message(FATAL_ERROR "powerup_install_prefix oracle failed (${tr}):\n${tout}\n${terr}")
endif()

message(STATUS "powerup_install_prefix ok prefix=${PREFIX}")
