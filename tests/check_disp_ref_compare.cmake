# IHO-DISP-REF-COMPARE tip-truth: viewer_disp_ref_compare
# MUST = SOURCE pin + scripts + dual-capture non-empty PNGs (MUST NOT SKIP on Ubuntu).
# Soft pixel vs EncDotNet = WARN-only (no --strict). See docs/disp-ref-compare-contract.md.
# Do not reopen DISPLAY-COMPARE / touch viewer_s164_display_compare.
cmake_minimum_required(VERSION 3.16)
if(NOT DEFINED SRC OR NOT DEFINED BIN)
  message(FATAL_ERROR "pass -DSRC=... -DBIN=...")
endif()

# --- MUST: EncDotNet pin + provenance ---
set(_source "${SRC}/tools/encdotnet-s100/SOURCE.md")
if(NOT EXISTS "${_source}")
  message(FATAL_ERROR "missing ${_source} (EncDotNet pin / provenance)")
endif()
file(READ "${_source}" _source_txt)
if(NOT _source_txt MATCHES "v0\\.24\\.0" AND NOT _source_txt MATCHES "\\*\\*0\\.24\\.0\\*\\*")
  message(FATAL_ERROR "SOURCE.md must pin EncDotNet v0.24.0")
endif()

# --- MUST: scripts present ---
foreach(_script
    "scripts/fetch_encdotnet_s100.sh"
    "scripts/render_encdotnet_oracle.sh"
    "scripts/dual_capture_oracle.sh"
    "scripts/soft_compare_oracle.py")
  if(NOT EXISTS "${SRC}/${_script}")
    message(FATAL_ERROR "missing ${SRC}/${_script}")
  endif()
endforeach()

# --- MUST: s100_viewer (tip lives under Qt viewer build) ---
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

# --- MUST: EncDotNet s100 CLI (fetch if missing — no SKIP) ---
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
  message(STATUS "EncDotNet s100 missing — running fetch_encdotnet_s100.sh")
  execute_process(
    COMMAND bash "${SRC}/scripts/fetch_encdotnet_s100.sh"
    WORKING_DIRECTORY "${SRC}"
    RESULT_VARIABLE _frc
    OUTPUT_VARIABLE _fout
    ERROR_VARIABLE _ferr)
  if(NOT _fout STREQUAL "")
    message(STATUS "${_fout}")
  endif()
  if(NOT _ferr STREQUAL "")
    message(STATUS "${_ferr}")
  endif()
  if(NOT _frc EQUAL 0)
    message(FATAL_ERROR "fetch_encdotnet_s100.sh failed rc=${_frc}")
  endif()
  if(EXISTS "${SRC}/tools/encdotnet-s100/s100")
    set(_s100 "${SRC}/tools/encdotnet-s100/s100")
  endif()
endif()
if(_s100 STREQUAL "" OR NOT EXISTS "${_s100}")
  message(FATAL_ERROR "EncDotNet s100 CLI not available after fetch")
endif()

set(_out "${BIN}/disp-ref-compare")
file(MAKE_DIRECTORY "${_out}")
set(_cell "${SRC}/testdata/s101/101AA00DS0008.000")
if(NOT EXISTS "${_cell}")
  message(FATAL_ERROR "missing fixture cell ${_cell}")
endif()
set(_enc "${_out}/enc_iho_0008_day_1280x800.png")
set(_ours "${_out}/ours_iho_0008_day_1280x800.png")

# EncDotNet oracle render
execute_process(
  COMMAND ${_s100} render ${_cell} ${_enc} --palette day -w 1280 -h 800
  RESULT_VARIABLE _erc
  OUTPUT_VARIABLE _eout
  ERROR_VARIABLE _eerr)
if(NOT _erc EQUAL 0)
  message(FATAL_ERROR "EncDotNet render failed rc=${_erc}:\n${_eerr}${_eout}")
endif()

# Our viewer Day snapshot
execute_process(
  COMMAND ${CMAKE_COMMAND} -E env QT_QPA_PLATFORM=offscreen QT_OPENGL=software
          ${_viewer} ${_cell} --palette Day --snapshot ${_ours}
  RESULT_VARIABLE _vrc
  OUTPUT_VARIABLE _vout
  ERROR_VARIABLE _verr)
if(NOT _vrc EQUAL 0)
  message(FATAL_ERROR "viewer snapshot failed rc=${_vrc}:\n${_verr}${_vout}")
endif()

# --- MUST: non-empty dual-capture PNGs ---
foreach(_png ${_enc} ${_ours})
  if(NOT EXISTS "${_png}")
    message(FATAL_ERROR "dual-capture missing PNG ${_png}")
  endif()
  file(SIZE "${_png}" _psz)
  if(_psz EQUAL 0)
    message(FATAL_ERROR "dual-capture empty PNG ${_png}")
  endif()
endforeach()
message(STATUS "disp-ref-compare dual-capture OK enc=${_enc} ours=${_ours}")

# --- WARN-only soft pixel ---
find_program(PYTHON3 NAMES python3 python)
if(PYTHON3)
  execute_process(
    COMMAND ${PYTHON3} ${SRC}/scripts/soft_compare_oracle.py ${_ours} ${_enc} --threshold 0.90
    RESULT_VARIABLE _prc
    OUTPUT_VARIABLE _pout
    ERROR_VARIABLE _perr)
  if(NOT _pout STREQUAL "")
    message(STATUS "soft_compare: ${_pout}")
  endif()
  if(NOT _perr STREQUAL "")
    message(STATUS "soft_compare: ${_perr}")
  endif()
  # Default soft_compare exits 0 unless --strict; never pass --strict here.
  if(NOT _prc EQUAL 0)
    message(WARNING "soft_compare rc=${_prc} (WARN-only; tip does not harden)")
  endif()
else()
  message(WARNING "python3 not found — soft_compare skipped (WARN-only)")
endif()
