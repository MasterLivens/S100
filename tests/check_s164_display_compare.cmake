# IHO-S164-DISPLAY-COMPARE: MUST = refs/SHA/SOURCE + snapshot + Day anchors + preset layers.
# Soft pixel vs AA5* is WARN-only (see docs/s164-display-compare-contract.md).
set(_viewer "")
foreach(_cand
    "${BIN}/s100_viewer"
    "${BIN}/s100_viewer.exe"
    "${BIN}/Release/s100_viewer"
    "${BIN}/Release/s100_viewer.exe"
    "${BIN}/Debug/s100_viewer"
    "${BIN}/Debug/s100_viewer.exe"
    "${BIN}/RelWithDebInfo/s100_viewer.exe"
    "${BIN}/MinSizeRel/s100_viewer.exe")
  if(EXISTS "${_cand}")
    set(_viewer "${_cand}")
    break()
  endif()
endforeach()
if(_viewer STREQUAL "")
  message(FATAL_ERROR "s100_viewer not found under ${BIN} (tried uni-config and Release/Debug .exe)")
endif()
find_program(PYTHON3 NAMES python3 python REQUIRED)
execute_process(
  COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen QT_OPENGL=software
          ${PYTHON3} ${SRC}/tests/s164_display_compare.py
          --viewer ${_viewer}
          --src ${SRC}
          --out ${BIN}
  RESULT_VARIABLE prc
  OUTPUT_VARIABLE pout
  ERROR_VARIABLE perr)
message(STATUS "${pout}")
if(NOT pout STREQUAL "")
  message("${pout}")
endif()
if(NOT perr STREQUAL "")
  message("${perr}")
endif()
if(NOT prc EQUAL 0)
  message(FATAL_ERROR "viewer_s164_display_compare failed rc=${prc}:\n${perr}${pout}")
endif()
