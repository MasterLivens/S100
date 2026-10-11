# Vendored dependencies, built from source so Linux and Windows CI need no system packages.
# Provenance + hashes: third_party/*/SOURCE.md, checked by CTest third_party_provenance.

# Lua 5.1.5 (S-100 Part 13 clause 13-7.1 requires a Lua 5.1 engine).
set(LUA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lua-5.1.5)
add_library(s100_lua STATIC
  ${LUA_DIR}/src/lapi.c ${LUA_DIR}/src/lcode.c ${LUA_DIR}/src/ldebug.c ${LUA_DIR}/src/ldo.c
  ${LUA_DIR}/src/ldump.c ${LUA_DIR}/src/lfunc.c ${LUA_DIR}/src/lgc.c ${LUA_DIR}/src/llex.c
  ${LUA_DIR}/src/lmem.c ${LUA_DIR}/src/lobject.c ${LUA_DIR}/src/lopcodes.c ${LUA_DIR}/src/lparser.c
  ${LUA_DIR}/src/lstate.c ${LUA_DIR}/src/lstring.c ${LUA_DIR}/src/ltable.c ${LUA_DIR}/src/ltm.c
  ${LUA_DIR}/src/lundump.c ${LUA_DIR}/src/lvm.c ${LUA_DIR}/src/lzio.c
  ${LUA_DIR}/src/lauxlib.c ${LUA_DIR}/src/lbaselib.c ${LUA_DIR}/src/ldblib.c ${LUA_DIR}/src/liolib.c
  ${LUA_DIR}/src/lmathlib.c ${LUA_DIR}/src/loslib.c ${LUA_DIR}/src/ltablib.c ${LUA_DIR}/src/lstrlib.c
  ${LUA_DIR}/src/loadlib.c ${LUA_DIR}/src/linit.c)
# Compiled as C++ (supported by Lua 5.1, see luaconf.h LUAI_THROW): lua_error then unwinds with
# a C++ exception instead of longjmp, so host functions written in C++ run their destructors.
# Consequence: include lua.h / lauxlib.h / lualib.h directly, NOT etc/lua.hpp (extern "C").
get_target_property(_lua_srcs s100_lua SOURCES)
set_source_files_properties(${_lua_srcs} PROPERTIES LANGUAGE CXX)
target_include_directories(s100_lua SYSTEM PUBLIC ${LUA_DIR}/src)
set_target_properties(s100_lua PROPERTIES LINKER_LANGUAGE CXX POSITION_INDEPENDENT_CODE ON)
if(UNIX)
  target_compile_definitions(s100_lua PRIVATE LUA_USE_POSIX)  # mkstemp etc.; no dlopen/readline
endif()
if(MSVC)
  target_compile_options(s100_lua PRIVATE /w)
  target_compile_definitions(s100_lua PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(s100_lua PRIVATE -w)
endif()
if(UNIX)
  target_link_libraries(s100_lua PUBLIC m)
endif()

# pugixml 1.14
add_library(s100_pugixml STATIC ${CMAKE_CURRENT_SOURCE_DIR}/pugixml-1.14/src/pugixml.cpp)
target_include_directories(s100_pugixml SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/pugixml-1.14/src)
set_target_properties(s100_pugixml PROPERTIES POSITION_INDEPENDENT_CODE ON)
