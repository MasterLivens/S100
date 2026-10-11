cmake_minimum_required(VERSION 3.16)
project(s100-chart VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(S100_BUILD_VIEWER "Build Qt6 map viewer" ON)

add_library(s100_core
  src/core/iso8211.cpp
  src/core/layer.cpp
  src/core/pick.cpp
  src/core/load.cpp
  src/core/hdf5_product.cpp
  src/core/feature_catalogue.cpp
)
target_include_directories(s100_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(s100_cli src/cli/main.cpp)
target_link_libraries(s100_cli PRIVATE s100_core)
target_compile_definitions(s100_cli PRIVATE
  S100_DEFAULT_FC="${CMAKE_SOURCE_DIR}/catalogues/S-101/S-101_FC_2.0.0.xml")

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
)
target_link_libraries(s100_tests PRIVATE s100_core)
target_compile_definitions(s100_tests PRIVATE CMAKE_SOURCE_DIR_STR="${CMAKE_SOURCE_DIR}")
target_compile_definitions(s100_tests PRIVATE S100_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME s100_tests COMMAND s100_tests)
add_test(NAME catalogue_provenance COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/tests/check_catalogues.cmake)
set_tests_properties(s100_tests PROPERTIES
  ENVIRONMENT "S100_TESTDATA=${CMAKE_SOURCE_DIR}/testdata/s101"
)

if(S100_BUILD_VIEWER)
  find_package(Qt6 REQUIRED COMPONENTS Widgets)
  qt_standard_project_setup()
  add_executable(s100_viewer src/viewer/main.cpp)
  target_link_libraries(s100_viewer PRIVATE s100_core Qt6::Widgets)
  target_compile_definitions(s100_viewer PRIVATE
    S100_DEFAULT_FC="${CMAKE_SOURCE_DIR}/catalogues/S-101/S-101_FC_2.0.0.xml")
endif()

install(TARGETS s100_core s100_cli RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(DIRECTORY include/s100 DESTINATION include)
