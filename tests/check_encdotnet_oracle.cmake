# Optional EncDotNet soft-oracle (WARN-only). Enabled only when S100_ENCDOTNET_ORACLE=ON.
# Never replaces viewer_golden / s164-ref. Default OFF; not run in CI.yml.
cmake_minimum_required(VERSION 3.16)
if(NOT DEFINED SRC OR NOT DEFINED BIN)
  message(FATAL_ERROR "pass -DSRC=... -DBIN=...")
endif()
set(_s100 "")
foreach(_cand
    "$ENV{ENCDOTNET_S100}"
    "${SRC}/tools/encdotnet-s100/s100"
    "/workspace/vendor/encdotnet-s100/s100")
  if(NOT _cand STREQUAL "" AND EXISTS "${_cand}")
    set(_s100 "${_cand}")
    break()
  endif()
endforeach()
if(_s100 STREQUAL "")
  message(WARNING "EncDotNet s100 CLI not found — skip oracle (run scripts/fetch_encdotnet_s100.sh)")
  return()
endif()
set(_out "${BIN}/encdotnet-oracle")
file(MAKE_DIRECTORY "${_out}")
set(_cell "${SRC}/testdata/s101/101AA00DS0008.000")
set(_enc "${_out}/enc_iho_0008_day_1280x800.png")
set(_ours "${_out}/ours_iho_0008_day_1280x800.png")
execute_process(
  COMMAND ${_s100} render ${_cell} ${_enc} --palette day -w 1280 -h 800
  RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
  message(WARNING "EncDotNet render failed rc=${rc} (warn-only)")
  return()
endif()
set(_viewer "")
foreach(_cand
    "${BIN}/s100_viewer"
    "${BIN}/s100_viewer.exe"
    "${BIN}/Release/s100_viewer"
    "${BIN}/Release/s100_viewer.exe")
  if(EXISTS "${_cand}")
    set(_viewer "${_cand}")
    break()
  endif()
endforeach()
if(_viewer STREQUAL "")
  message(WARNING "s100_viewer not found — EncDotNet PNG at ${_enc} only")
  return()
endif()
execute_process(
  COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen QT_OPENGL=software
          ${_viewer} ${_cell} --palette Day --snapshot ${_ours}
  RESULT_VARIABLE vrc)
if(NOT vrc EQUAL 0)
  message(WARNING "viewer snapshot for oracle failed rc=${vrc} (warn-only)")
  return()
endif()
find_program(PYTHON3 NAMES python3 python)
if(NOT PYTHON3)
  message(WARNING "python3 not found — skip soft compare")
  return()
endif()
execute_process(
  COMMAND ${PYTHON3} ${SRC}/scripts/soft_compare_oracle.py ${_ours} ${_enc} --threshold 0.90
  RESULT_VARIABLE prc
  OUTPUT_VARIABLE pout
  ERROR_VARIABLE perr)
message(STATUS "encdotnet oracle: ${pout}${perr}")
# Always succeed — warn-only path.
