# IHO-S101-TEXT-COLLISION: pairs_off >= pairs_on and pairs_off > 0 on 0008 Day 1280x800.
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
  message(FATAL_ERROR "s100_viewer not found under ${BIN}")
endif()
find_program(PYTHON3 NAMES python3 python REQUIRED)
set(CELL "${SRC}/testdata/s101/101AA00DS0008.000")
set(OFF_CSV "${BIN}/text_boxes_off.csv")
set(ON_CSV "${BIN}/text_boxes_on.csv")
execute_process(
  COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen
          ${_viewer} ${CELL}
          --no-text-collision
          --dump-text-boxes ${OFF_CSV}
  RESULT_VARIABLE rc_off)
if(NOT rc_off EQUAL 0)
  message(FATAL_ERROR "viewer dump OFF failed rc=${rc_off}")
endif()
execute_process(
  COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen
          ${_viewer} ${CELL}
          --dump-text-boxes ${ON_CSV}
  RESULT_VARIABLE rc_on)
if(NOT rc_on EQUAL 0)
  message(FATAL_ERROR "viewer dump ON failed rc=${rc_on}")
endif()
execute_process(
  COMMAND ${PYTHON3} ${SRC}/tests/count_text_box_pairs.py ${OFF_CSV}
  RESULT_VARIABLE prc OUTPUT_VARIABLE pairs_off ERROR_VARIABLE perr OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT prc EQUAL 0)
  message(FATAL_ERROR "count OFF failed: ${perr}")
endif()
execute_process(
  COMMAND ${PYTHON3} ${SRC}/tests/count_text_box_pairs.py ${ON_CSV}
  RESULT_VARIABLE prc OUTPUT_VARIABLE pairs_on ERROR_VARIABLE perr OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT prc EQUAL 0)
  message(FATAL_ERROR "count ON failed: ${perr}")
endif()
message(STATUS "IHO-S101-TEXT-COLLISION pairs_off=${pairs_off} pairs_on=${pairs_on}")
if(pairs_off LESS_EQUAL 0)
  message(FATAL_ERROR "expected pairs_off > 0 (got ${pairs_off})")
endif()
if(pairs_off LESS pairs_on)
  message(FATAL_ERROR "expected pairs_off >= pairs_on (off=${pairs_off} on=${pairs_on})")
endif()
