cmake_minimum_required(VERSION 3.22)
project(toolchain_mastery VERSION 1.0.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake/modules")

include(CompilerWarnings)
include(Sanitizers)
include(StaticAnalyzers)
include(Coverage)
include(StandardVersion)

if(ENABLE_CLANG_TIDY)
  enable_clang_tidy()
endif()
if(ENABLE_CPPCHECK)
  enable_cppcheck()
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()
include(GoogleTest)

# ---- Demo executables ----

add_executable(asan_demo src/asan_demo.cpp)
require_cpp20(asan_demo)
target_apply_sanitizers(asan_demo)

add_executable(tsan_demo src/tsan_demo.cpp)
require_cpp20(tsan_demo)
target_apply_sanitizers(tsan_demo)

add_executable(ubsan_demo src/ubsan_demo.cpp)
require_cpp20(ubsan_demo)
target_apply_sanitizers(ubsan_demo)

add_executable(intrinsics_demo src/intrinsics_demo.cpp)
require_cpp20(intrinsics_demo)
# -march=native enables AVX2 if the CPU supports it
target_compile_options(intrinsics_demo PRIVATE -march=native)

add_executable(pgo_workload src/pgo_workload.cpp)
require_cpp20(pgo_workload)

# Assembly + LLVM IR output (custom target, not built by default)
find_program(CXX_COMPILER_FOR_ASM ${CMAKE_CXX_COMPILER})
add_custom_target(asm_output
  COMMAND ${CMAKE_CXX_COMPILER} -std=c++20 -O2 -S
          ${CMAKE_SOURCE_DIR}/src/pgo_workload.cpp
          -o ${CMAKE_BINARY_DIR}/pgo_workload.s
  COMMENT "Generating assembly output"
)

# ---- Tests ----
add_executable(toolchain_tests tests/test_basic.cpp)
require_cpp20(toolchain_tests)
target_apply_sanitizers(toolchain_tests)
target_apply_coverage(toolchain_tests)
target_link_libraries(toolchain_tests PRIVATE GTest::gtest_main)
gtest_discover_tests(toolchain_tests)

add_coverage_target()
