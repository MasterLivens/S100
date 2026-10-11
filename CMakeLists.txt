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
  src/core/host_adapter.cpp
  src/core/portrayal_engine.cpp
  src/core/exchange_set.cpp
  src/core/s101_update.cpp
  src/core/session.cpp
)
target_include_directories(s100_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(s100_core PRIVATE s100_pugixml s100_lua ${S100_HDF5_LIBS})  # Qt-free (contract rule 4)
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
  S100_DEFAULT_PC="${CMAKE_SOURCE_DIR}/catalogues/S-101/PC_2.0.0")

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
  tests/test_associations.cpp
)
target_link_libraries(s100_tests PRIVATE s100_core)
target_compile_definitions(s100_tests PRIVATE CMAKE_SOURCE_DIR_STR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(s100_tests PRIVATE S100_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME s100_tests COMMAND s100_tests)
add_test(NAME s102_deepen_multigrid COMMAND s100_tests s102_deepen_multigrid)
add_test(NAME s104_open_frame COMMAND s100_tests s104_open_frame)
add_test(NAME s111_open_frame COMMAND s100_tests s111_open_frame)
add_test(NAME s124_open_list COMMAND s100_tests s124_open_list)
add_test(NAME third_party_provenance COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/tests/check_third_party.cmake)
add_test(NAME catalogue_provenance COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/tests/check_catalogues.cmake)
add_executable(s100_portrayal_tests tests/test_portrayal.cpp)
target_link_libraries(s100_portrayal_tests PRIVATE s100_core s100_lua)
target_include_directories(s100_portrayal_tests PRIVATE ${CMAKE_SOURCE_DIR}/third_party/lua-5.1.5/src)
target_compile_definitions(s100_portrayal_tests PRIVATE S100_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
foreach(c portrayal_all_cells host_functions_static host_missing_function safety_contour s102_engine drawing_instruction_parser spatial_reference_resolve colour_tokens_symbolic association_host_query association_spatial_host_signatures)
  add_test(NAME portrayal.${c} COMMAND s100_portrayal_tests ${c})
endforeach()
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
    S100_DEFAULT_S102_PC="${CMAKE_SOURCE_DIR}/catalogues/S-102/PC_3.0.0")
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

install(TARGETS s100_core s100_cli RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(DIRECTORY include/s100 DESTINATION include)
