# Vendored third-party source provenance (run: cmake -DSRC=<repo> -P check_third_party.cmake).
# Tree hash = sha256 of the text "<sha256>  ./<relpath>\n" for every file, sorted by path
# (same as: cd third_party/<lib> && find . -type f | LC_ALL=C sort | xargs sha256sum | sha256sum).
# Values must equal those recorded in third_party/*/SOURCE.md.
function(expect_tree dir want)
  file(GLOB_RECURSE files RELATIVE "${dir}" "${dir}/*")
  list(FILTER files EXCLUDE REGEX "^SOURCE\\.md$")
  list(SORT files)
  set(text "")
  foreach(f IN LISTS files)
    file(SHA256 "${dir}/${f}" h)
    string(APPEND text "${h}  ./${f}\n")
  endforeach()
  string(SHA256 got "${text}")
  if(NOT got STREQUAL want)
    message(FATAL_ERROR "vendored tree hash mismatch: ${dir}\n  got  ${got}\n  want ${want}")
  endif()
  message(STATUS "tree sha256 ok ${dir}")
endfunction()
set(T "${SRC}/third_party")
expect_tree("${T}/lua-5.1.5" 062f4c2a4a414834e5eb6172354bf507139a38271bb1efa623cffd22f110b414)
expect_tree("${T}/pugixml-1.14" 3ab88061c12321f21b108505097a534d0f44f1acf3e0e9909419cc4ac1d115c8)
