cmake_minimum_required(VERSION 3.22)
project(cpp_foundation VERSION 1.0.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake/modules")
include(CompilerWarnings)
include(Sanitizers)
include(Coverage)
include(StandardVersion)

include(FetchContent)
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()
include(GoogleTest)

# Header-only interface library — all modules are headers
add_library(foundation INTERFACE)
require_cpp20(foundation)
target_include_directories(foundation INTERFACE include)

# ---- Helper to add a demo executable ----
function(add_demo name src)
  add_executable(demo_${name} ${src})
  require_cpp20(demo_${name})
  target_apply_sanitizers(demo_${name})
  target_link_libraries(demo_${name} PRIVATE foundation)
  # Warnings are informational for demos — not errors (some demos show "bad" code)
  target_compile_options(demo_${name} PRIVATE -Wall -Wextra)
endfunction()

add_demo(memory      demos/memory_demo.cpp)
add_demo(oop         demos/oop_demo.cpp)
add_demo(templates   demos/templates_demo.cpp)
add_demo(types       demos/types_demo.cpp)
add_demo(patterns    demos/patterns_demo.cpp)
add_demo(concurrency demos/concurrency_demo.cpp)
add_demo(standards   demos/standards_demo.cpp)

# ---- Helper to add a test executable ----
function(add_foundation_test name src)
  add_executable(test_${name} ${src})
  require_cpp20(test_${name})
  target_apply_sanitizers(test_${name})
  target_link_libraries(test_${name} PRIVATE foundation GTest::gtest_main)
  # PRE_TEST mode: discover tests at run time, not build time.
  # Required when sanitizers (TSan) make the binary non-runnable at build time (e.g. WSL2).
  gtest_discover_tests(test_${name} DISCOVERY_MODE PRE_TEST)
endfunction()

add_foundation_test(memory      tests/test_memory.cpp)
add_foundation_test(oop         tests/test_oop.cpp)
add_foundation_test(templates   tests/test_templates.cpp)
add_foundation_test(types       tests/test_types.cpp)
add_foundation_test(patterns    tests/test_patterns.cpp)
add_foundation_test(concurrency tests/test_concurrency.cpp)
