# Soft golden: regenerate PNG, require same size and Day-palette dominant colours within
# tolerance (Qt antialiasing / font hinting make byte-identical PNGs flaky across runs).
if(EXISTS "${BIN}/s100_viewer${CMAKE_EXECUTABLE_SUFFIX}")
  set(_viewer "${BIN}/s100_viewer${CMAKE_EXECUTABLE_SUFFIX}")
elseif(EXISTS "${BIN}/Release/s100_viewer${CMAKE_EXECUTABLE_SUFFIX}")
  set(_viewer "${BIN}/Release/s100_viewer${CMAKE_EXECUTABLE_SUFFIX}")
elseif(EXISTS "${BIN}/Debug/s100_viewer${CMAKE_EXECUTABLE_SUFFIX}")
  set(_viewer "${BIN}/Debug/s100_viewer${CMAKE_EXECUTABLE_SUFFIX}")
else()
  message(FATAL_ERROR "s100_viewer not found under ${BIN}")
endif()
execute_process(
  COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen
          ${_viewer}
          ${SRC}/testdata/s101/101AA00DS0008.000
          --snapshot ${BIN}/golden_iho_0008.png
  RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
  message(FATAL_ERROR "viewer snapshot failed rc=${rc}")
endif()
find_program(PYTHON3 NAMES python3 python REQUIRED)
execute_process(
  COMMAND ${PYTHON3} ${SRC}/tests/compare_viewer_golden.py
          ${BIN}/golden_iho_0008.png
          ${SRC}/testdata/golden/iho_0008.png
  RESULT_VARIABLE prc
  OUTPUT_VARIABLE pout
  ERROR_VARIABLE perr)
message(STATUS "${pout}")
if(NOT prc EQUAL 0)
  message(FATAL_ERROR "viewer golden compare failed:\n${perr}${pout}")
endif()
