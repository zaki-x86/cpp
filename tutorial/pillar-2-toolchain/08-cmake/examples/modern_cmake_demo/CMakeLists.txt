cmake_minimum_required(VERSION 3.22)
project(modern_cmake_demo VERSION 1.0.0 LANGUAGES CXX)

# -----------------------------------------------------------------------
# Header-only math library: INTERFACE target (no compiled source)
# -----------------------------------------------------------------------
add_library(demo_math INTERFACE)
target_include_directories(demo_math INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(demo_math INTERFACE cxx_std_20)

# -----------------------------------------------------------------------
# Executable that uses the library
# -----------------------------------------------------------------------
add_executable(demo_main src/main.cpp)
target_link_libraries(demo_main PRIVATE demo_math)

# -----------------------------------------------------------------------
# GoogleTest via FetchContent
# -----------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# -----------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------
enable_testing()
include(GoogleTest)

add_executable(demo_tests tests/test_math.cpp)
target_link_libraries(demo_tests PRIVATE demo_math GTest::gtest_main)
# DISCOVERY_MODE PRE_TEST: sanitized binaries cannot run at configure time
gtest_discover_tests(demo_tests DISCOVERY_MODE PRE_TEST)
