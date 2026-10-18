cmake_minimum_required(VERSION 3.16)
project(s100-chart VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(S100_BUILD_VIEWER "Build Qt6 map viewer" ON)
option(S100_ENCDOTNET_ORACLE "Optional EncDotNet soft-oracle ctest (warn-only; OFF in CI)" OFF)

add_subdirectory(third_party)

# Prefer HDF Group config package (Windows zip); fall back to FindHDF5 module (Linux apt).
find_package(HDF5 CONFIG QUIET COMPONENTS C shared)
if(HDF5_FOUND)
  set(S100_HDF5_LIBS hdf5::hdf5-shared)
else()
  find_package(HDF5 REQUIRED COMPONENTS C)
  set(S100_HDF5_LIBS ${HDF5_LIBRARIES})
  include_directories(${HDF5_INCLUDE_DIRS})
endif()

# IHO-S164-SIG — S-100 Part 15 signature verification (docs/part15-signature-contract.md).
find_package(OpenSSL 3.0 REQUIRED COMPONENTS Crypto)
add_library(s100_security
  src/security/sse_map.cpp
  src/security/trust_store.cpp
  src/security/openssl_certs.cpp
  src/security/iho_root_embed.cpp
  src/security/verify.cpp
)
target_include_directories(s100_security PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(s100_security PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/security)
target_link_libraries(s100_security PUBLIC OpenSSL::Crypto PRIVATE s100_pugixml)
target_compile_definitions(s100_security PRIVATE
  OPENSSL_API_COMPAT=30000
  OPENSSL_NO_DEPRECATED)

add_library(s100_core
  src/core/iso8211.cpp
  src/core/layer.cpp
  src/core/pick.cpp
  src/core/load.cpp
  src/core/hdf5_product.cpp
  src/core/hdf5_s104.cpp
  src/core/hdf5_s111.cpp
  src/core/gml_s124.cpp
  src/core/feature_catalogue.cpp
  src/core/portrayal_catalogue.cpp
  src/core/drawing_instructions.cpp
  src/core/scale_filter.cpp
  src/core/draw_order.cpp
  src/core/viewing_group_filter.cpp
  src/core/timeline.cpp
  src/core/host_adapter.cpp
  src/core/portrayal_engine.cpp
  src/core/exchange_set.cpp
  src/core/s101_update.cpp
  src/core/session.cpp
  src/core/powerup.cpp
)
target_include_directories(s100_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(s100_core PUBLIC s100_security PRIVATE s100_pugixml s100_lua ${S100_HDF5_LIBS})  # Qt-free (contract rule 4)
target_compile_definitions(s100_core PUBLIC
  S100_VERSION="${PROJECT_VERSION}"
  S100_CATALOGUE_ROOT="${CMAKE_SOURCE_DIR}"
  S100_POWERUP_SMOKE_EXCHANGE="${CMAKE_SOURCE_DIR}/testdata/s101/signatures/positive"
  S100_POWERUP_TEST_SA="${CMAKE_SOURCE_DIR}/testdata/s101/signatures/keys/test_sa.crt")
if(DEFINED HDF5_DEFINITIONS)
  target_compile_definitions(s100_core PRIVATE ${HDF5_DEFINITIONS})
endif()
if(HDF5_FOUND AND TARGET hdf5::hdf5-shared)
  # CONFIG package already carries includes
elseif(HDF5_INCLUDE_DIRS)
  target_include_directories(s100_core PRIVATE ${HDF5_INCLUDE_DIRS})
endif()

add_executable(s100_cli src/cli/main.cpp)
target_link_libraries(s100_cli PRIVATE s100_core)
target_compile_definitions(s100_cli PRIVATE
  S100_DEFAULT_FC="${CMAKE_SOURCE_DIR}/catalogues/S-101/S-101_FC_2.0.0.xml"
  S100_DEFAULT_PC="${CMAKE_SOURCE_DIR}/catalogues/S-101/PC_2.0.0"
  S100_DEFAULT_S111_FC="${CMAKE_SOURCE_DIR}/catalogues/S-111/S-111_FC_2.0.0.xml"
  S100_DEFAULT_S111_PC="${CMAKE_SOURCE_DIR}/catalogues/S-111/PC_2.0.0"
  S100_DEFAULT_S124_FC="${CMAKE_SOURCE_DIR}/catalogues/S-124/S-124_FC_2.0.0.xml"
  S100_DEFAULT_S124_PC="${CMAKE_SOURCE_DIR}/catalogues/S-124/PC_2.0.0")

enable_testing()
add_executable(s100_tests
  tests/test_load.cpp
  tests/test_pick.cpp
  tests/test_layers.cpp
  tests/test_fixtures_pick.cpp
  tests/test_aggregates.cpp
  tests/test_chs_bounds.cpp
  tests/test_fc_primitives.cpp
  tests/test_layers_all_cells.cpp
  tests/test_attributes.cpp
  tests/test_soundings.cpp
  tests/test_feature_catalogue.cpp
  tests/test_exchange_set.cpp
  tests/test_s102.cpp
  tests/test_s104.cpp
  tests/test_s111.cpp
  tests/test_s124.cpp
  tests/test_pc_wire_111_124.cpp
  tests/test_session_pick.cpp
  tests/test_associations.cpp
  tests/test_scamin_filter.cpp
  tests/test_display_priority_order.cpp
  tests/test_timeline_ui.cpp
  tests/test_update_robustness.cpp
  tests/test_cancel_reissue.cpp
  tests/test_edition_hardening.cpp
  tests/test_exchange_signature.cpp
  tests/test_exchange_signature_producer.cpp
  tests/test_exchange_signature_cn_shape.cpp
  tests/test_powerup_self_check.cpp
  tests/support/dataset_digest.cpp
  tests/support/sha256.cpp
)
target_link_libraries(s100_tests PRIVATE s100_core)
target_include_directories(s100_tests PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_compile_definitions(s100_tests PRIVATE CMAKE_SOURCE_DIR_STR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(s100_tests PRIVATE S100_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(s100_tests PRIVATE "S100_CLI_PATH=\"$<TARGET_FILE:s100_cli>\"")
add_dependencies(s100_tests s100_cli)
add_test(NAME timeline_ui COMMAND s100_tests timeline_ui)
add_test(NAME s100_tests COMMAND s100_tests)
add_test(NAME s102_deepen_multigrid COMMAND s100_tests s102_deepen_multigrid)
add_test(NAME s102_uncertainty COMMAND s100_tests s102_uncertainty)
add_test(NAME s102_qbc_meta COMMAND s100_tests s102_qbc_meta)
add_test(NAME s104_open_frame COMMAND s100_tests s104_open_frame)
add_test(NAME s111_open_frame COMMAND s100_tests s111_open_frame)
add_test(NAME s124_open_list COMMAND s100_tests s124_open_list)
add_test(NAME session_pick_unified COMMAND s100_tests session_pick_unified)
add_test(NAME scamin_filter COMMAND s100_tests scamin_filter)
add_test(NAME display_priority_order COMMAND s100_tests display_priority_order)
add_test(NAME pc_wire_111_124 COMMAND s100_tests pc_wire_111_124)
# IHO-S164-UPD-SEQ / IHO-S164-UPD-CORRUPT (docs/update-robustness-contract.md rev 1.2) —
# MUST NOT SKIP, no platform guard. Python3 is REQUIRED so a missing interpreter fails configure
# instead of skipping the reproducibility oracle.
add_test(NAME exchange_update_robustness COMMAND s100_tests exchange_update_robustness)
# IHO-S164-CANCEL-REISSUE (docs/cancel-reissue-contract.md rev 2.1) — MUST NOT SKIP, no guard.
add_test(NAME exchange_cancel_reissue COMMAND s100_tests exchange_cancel_reissue)
# IHO-S164-EDITION-HARDEN (docs/edition-hardening-contract.md rev 1) — MUST NOT SKIP, no guard.
add_test(NAME exchange_edition_hardening COMMAND s100_tests exchange_edition_hardening)
# IHO-S164-SIG (docs/part15-signature-contract.md) — MUST NOT SKIP, Linux + Windows.
add_test(NAME exchange_signature COMMAND s100_tests exchange_signature)
# IHO-S164-SIG-PRODUCER (docs/part15-producer-code-contract.md rev1) — MUST NOT SKIP, Linux + Windows.
add_test(NAME exchange_signature_producer COMMAND s100_tests exchange_signature_producer)
# IHO-S164-SIG-CN-SHAPE (docs/part15-cn-shape-contract.md rev1) — MUST NOT SKIP, Linux + Windows.
add_test(NAME exchange_signature_cn_shape COMMAND s100_tests exchange_signature_cn_shape)
# IHO-S164-POWERUP (docs/powerup-self-check-contract.md rev1) — MUST NOT SKIP, Linux + Windows.
add_test(NAME powerup_self_check COMMAND s100_tests powerup_self_check)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME signature_fixtures_reproducible
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/check_signature_fixtures.py
          ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/signature_fixtures_repro)
add_test(NAME update_fixtures_reproducible
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/check_update_fixtures.py
          ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/update_fixtures_repro)
add_test(NAME third_party_provenance COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/tests/check_third_party.cmake)
add_test(NAME catalogue_provenance COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/tests/check_catalogues.cmake)
add_executable(s100_portrayal_tests tests/test_portrayal.cpp tests/test_bridge_render.cpp)
target_link_libraries(s100_portrayal_tests PRIVATE s100_core s100_lua)
target_include_directories(s100_portrayal_tests PRIVATE ${CMAKE_SOURCE_DIR}/third_party/lua-5.1.5/src)
target_compile_definitions(s100_portrayal_tests PRIVATE S100_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
foreach(c portrayal_all_cells host_functions_static host_missing_function safety_contour s102_engine drawing_instruction_parser spatial_reference_resolve colour_tokens_symbolic association_host_query association_spatial_host_signatures)
  add_test(NAME portrayal.${c} COMMAND s100_portrayal_tests ${c})
endforeach()
# IHO-S101-BRIDGE-RENDER (docs/bridge-render-contract.md rev 2.1) — MUST NOT SKIP, Linux + Windows.
# fasc_bridge_render is the contract name; bridge_render is the rev-1 name kept as an alias.
add_test(NAME fasc_bridge_render COMMAND s100_portrayal_tests bridge_render)
add_test(NAME bridge_render COMMAND s100_portrayal_tests bridge_render)
set_tests_properties(s100_tests PROPERTIES
  ENVIRONMENT "S100_TESTDATA=${CMAKE_SOURCE_DIR}/testdata/s101"
)

if(S100_BUILD_VIEWER)
  find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)
  qt_standard_project_setup()
  add_executable(s100_viewer src/viewer/main.cpp)
  target_link_libraries(s100_viewer PRIVATE s100_core Qt6::Widgets Qt6::Svg)
  target_compile_definitions(s100_viewer PRIVATE
    S100_DEFAULT_FC="${CMAKE_SOURCE_DIR}/catalogues/S-101/S-101_FC_2.0.0.xml"
    S100_DEFAULT_PC="${CMAKE_SOURCE_DIR}/catalogues/S-101/PC_2.0.0"
    S100_DEFAULT_S102_FC="${CMAKE_SOURCE_DIR}/catalogues/S-102/S-102_FC_3.0.0.xml"
    S100_DEFAULT_S102_PC="${CMAKE_SOURCE_DIR}/catalogues/S-102/PC_3.0.0"
    S100_DEFAULT_S111_FC="${CMAKE_SOURCE_DIR}/catalogues/S-111/S-111_FC_2.0.0.xml"
    S100_DEFAULT_S111_PC="${CMAKE_SOURCE_DIR}/catalogues/S-111/PC_2.0.0"
    S100_DEFAULT_S124_FC="${CMAKE_SOURCE_DIR}/catalogues/S-124/S-124_FC_2.0.0.xml"
    S100_DEFAULT_S124_PC="${CMAKE_SOURCE_DIR}/catalogues/S-124/PC_2.0.0")
  # Offscreen smoke: paints DrawingInstructions and writes a PNG (no display required).
  add_test(NAME viewer_snapshot
    COMMAND s100_viewer
      "${CMAKE_SOURCE_DIR}/testdata/s101/101AA00DS0003.000"
      --no-text-collision
      --snapshot "${CMAKE_BINARY_DIR}/viewer_snapshot.png")
  set_tests_properties(viewer_snapshot PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_OPENGL=software;S100_TESTDATA=${CMAKE_SOURCE_DIR}/testdata/s101"
    TIMEOUT 90)
  add_test(NAME viewer_golden
    COMMAND ${CMAKE_COMMAND}
      -DSRC=${CMAKE_SOURCE_DIR} -DBIN=${CMAKE_BINARY_DIR}
      -P ${CMAKE_SOURCE_DIR}/tests/check_viewer_golden.cmake)
  add_test(NAME viewer_palettes
    COMMAND ${CMAKE_COMMAND}
      -DSRC=${CMAKE_SOURCE_DIR} -DBIN=${CMAKE_BINARY_DIR}
      -P ${CMAKE_SOURCE_DIR}/tests/check_viewer_palettes.cmake)
  set_tests_properties(viewer_golden viewer_palettes PROPERTIES TIMEOUT 90)
  add_test(NAME viewer_text_collision
    COMMAND ${CMAKE_COMMAND}
      -DSRC=${CMAKE_SOURCE_DIR} -DBIN=${CMAKE_BINARY_DIR}
      -P ${CMAKE_SOURCE_DIR}/tests/check_text_collision.cmake)
  set_tests_properties(viewer_text_collision PROPERTIES TIMEOUT 90)
  add_test(NAME viewer_point_label_collision
    COMMAND ${CMAKE_COMMAND}
      -DSRC=${CMAKE_SOURCE_DIR} -DBIN=${CMAKE_BINARY_DIR}
      -P ${CMAKE_SOURCE_DIR}/tests/check_point_label_collision.cmake)
  set_tests_properties(viewer_point_label_collision PROPERTIES TIMEOUT 90)
  add_test(NAME viewer_s164_display_compare
    COMMAND ${CMAKE_COMMAND}
      -DSRC=${CMAKE_SOURCE_DIR} -DBIN=${CMAKE_BINARY_DIR}
      -P ${CMAKE_SOURCE_DIR}/tests/check_s164_display_compare.cmake)
  set_tests_properties(viewer_s164_display_compare PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_OPENGL=software;S100_TESTDATA=${CMAKE_SOURCE_DIR}/testdata/s101"
    TIMEOUT 120)

  if(S100_ENCDOTNET_ORACLE)
    add_test(NAME viewer_encdotnet_oracle
      COMMAND ${CMAKE_COMMAND}
        -DSRC=${CMAKE_SOURCE_DIR} -DBIN=${CMAKE_BINARY_DIR}
        -P ${CMAKE_SOURCE_DIR}/tests/check_encdotnet_oracle.cmake)
    set_tests_properties(viewer_encdotnet_oracle PROPERTIES TIMEOUT 180)
  endif()
endif()

install(TARGETS s100_core s100_security s100_cli RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(DIRECTORY include/s100 DESTINATION include)
