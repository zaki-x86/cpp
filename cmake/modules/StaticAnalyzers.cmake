# Usage: enable_clang_tidy()  or  enable_cppcheck()
# Call from project CMakeLists.txt before defining targets.

function(enable_clang_tidy)
  find_program(CLANG_TIDY clang-tidy)
  if(CLANG_TIDY)
    message(STATUS "clang-tidy found: ${CLANG_TIDY}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY};--extra-arg=-Wno-unknown-warning-option"
        CACHE STRING "" FORCE)
  else()
    message(WARNING "clang-tidy not found — static analysis disabled")
  endif()
endfunction()

function(enable_cppcheck)
  find_program(CPPCHECK cppcheck)
  if(CPPCHECK)
    message(STATUS "cppcheck found: ${CPPCHECK}")
    set(CMAKE_CXX_CPPCHECK
        "${CPPCHECK};--suppress=missingInclude;--enable=all;--error-exitcode=1"
        CACHE STRING "" FORCE)
  else()
    message(WARNING "cppcheck not found — cppcheck analysis disabled")
  endif()
endfunction()
