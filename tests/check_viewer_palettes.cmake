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
foreach(PAL IN ITEMS Day Dusk Night)
  string(TOLOWER ${PAL} pal_l)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen
            ${_viewer}
            ${SRC}/testdata/s101/101AA00DS0008.000
            --palette ${PAL}
            --no-text-collision
            --snapshot ${BIN}/palette_${pal_l}.png
    RESULT_VARIABLE rc)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "viewer --palette ${PAL} failed rc=${rc}")
  endif()
endforeach()
find_program(PYTHON3 NAMES python3 python REQUIRED)
execute_process(
  COMMAND ${PYTHON3} ${SRC}/tests/compare_viewer_palettes.py
          ${BIN}/palette_day.png ${BIN}/palette_dusk.png ${BIN}/palette_night.png
  RESULT_VARIABLE prc OUTPUT_VARIABLE pout ERROR_VARIABLE perr)
message(STATUS "${pout}")
if(NOT prc EQUAL 0)
  message(FATAL_ERROR "palette compare failed:\n${perr}${pout}")
endif()
