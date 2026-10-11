foreach(PAL IN ITEMS Day Dusk Night)
  string(TOLOWER ${PAL} pal_l)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen
            ${BIN}/s100_viewer
            ${SRC}/testdata/s101/101AA00DS0008.000
            --palette ${PAL}
            --snapshot ${BIN}/palette_${pal_l}.png
    RESULT_VARIABLE rc)
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "viewer --palette ${PAL} failed rc=${rc}")
  endif()
endforeach()
find_program(PYTHON3 python3 REQUIRED)
execute_process(
  COMMAND ${PYTHON3} ${SRC}/tests/compare_viewer_palettes.py
          ${BIN}/palette_day.png ${BIN}/palette_dusk.png ${BIN}/palette_night.png
  RESULT_VARIABLE prc OUTPUT_VARIABLE pout ERROR_VARIABLE perr)
message(STATUS "${pout}")
if(NOT prc EQUAL 0)
  message(FATAL_ERROR "palette compare failed:\n${perr}${pout}")
endif()
