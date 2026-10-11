# IHO-S101-POINT-LABEL-COLLISION: PointLabel pairs_off >= pairs_on, pairs_off > 0;
# point_glyphs_drawn(ON) == point_glyphs_drawn(OFF). Fixture 0008 Day 1280x800.
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
set(OFF_CSV "${BIN}/point_label_boxes_off.csv")
set(ON_CSV "${BIN}/point_label_boxes_on.csv")
set(OFF_ERR "${BIN}/point_label_off.err")
set(ON_ERR "${BIN}/point_label_on.err")
execute_process(
  COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen
          ${_viewer} ${CELL}
          --no-text-collision
          --dump-text-boxes ${OFF_CSV}
  RESULT_VARIABLE rc_off
  ERROR_FILE ${OFF_ERR})
if(NOT rc_off EQUAL 0)
  message(FATAL_ERROR "viewer dump OFF failed rc=${rc_off}")
endif()
execute_process(
  COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen
          ${_viewer} ${CELL}
          --dump-text-boxes ${ON_CSV}
  RESULT_VARIABLE rc_on
  ERROR_FILE ${ON_ERR})
if(NOT rc_on EQUAL 0)
  message(FATAL_ERROR "viewer dump ON failed rc=${rc_on}")
endif()
execute_process(
  COMMAND ${PYTHON3} ${SRC}/tests/count_text_box_pairs.py ${OFF_CSV} --class PointLabel
  RESULT_VARIABLE prc OUTPUT_VARIABLE pairs_off ERROR_VARIABLE perr OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT prc EQUAL 0)
  message(FATAL_ERROR "count OFF PointLabel failed: ${perr}")
endif()
execute_process(
  COMMAND ${PYTHON3} ${SRC}/tests/count_text_box_pairs.py ${ON_CSV} --class PointLabel
  RESULT_VARIABLE prc OUTPUT_VARIABLE pairs_on ERROR_VARIABLE perr OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT prc EQUAL 0)
  message(FATAL_ERROR "count ON PointLabel failed: ${perr}")
endif()
# Parse point_glyphs_drawn=N from stderr dumps
function(parse_glyphs err_file out_var)
  file(READ "${err_file}" _err)
  string(REGEX MATCH "point_glyphs_drawn=([0-9]+)" _m "${_err}")
  if(NOT CMAKE_MATCH_1)
    message(FATAL_ERROR "point_glyphs_drawn not found in ${err_file}")
  endif()
  set(${out_var} "${CMAKE_MATCH_1}" PARENT_SCOPE)
endfunction()
parse_glyphs("${OFF_ERR}" glyphs_off)
parse_glyphs("${ON_ERR}" glyphs_on)
message(STATUS "IHO-S101-POINT-LABEL-COLLISION pairs_off=${pairs_off} pairs_on=${pairs_on} glyphs_off=${glyphs_off} glyphs_on=${glyphs_on}")
if(pairs_off LESS_EQUAL 0)
  message(FATAL_ERROR "expected PointLabel pairs_off > 0 (got ${pairs_off}); need synth fixture if 0008 has none")
endif()
if(pairs_off LESS pairs_on)
  message(FATAL_ERROR "expected pairs_off >= pairs_on (off=${pairs_off} on=${pairs_on})")
endif()
if(NOT glyphs_off STREQUAL glyphs_on)
  message(FATAL_ERROR "point_glyphs_drawn mismatch ON=${glyphs_on} OFF=${glyphs_off}")
endif()
