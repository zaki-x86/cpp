# CMake Patterns Cheat-Sheet

The 10 patterns every senior C++ developer uses daily.

## 1. Header-only INTERFACE library

```cmake
add_library(myheaders INTERFACE)
target_include_directories(myheaders INTERFACE include/)
target_compile_features(myheaders INTERFACE cxx_std_20)
```

## 2. Static library with correct visibility

```cmake
add_library(mylib STATIC src/impl.cpp)
target_include_directories(mylib
    PUBLIC  include/          # propagates to consumers
    PRIVATE src/              # internal only
)
target_link_libraries(mylib PUBLIC  some_public_dep
                             PRIVATE some_internal_dep)
```

## 3. Executable with feature requirement

```cmake
add_executable(myapp src/main.cpp)
target_link_libraries(myapp PRIVATE mylib)
target_compile_features(myapp PRIVATE cxx_std_20)
```

## 4. GoogleTest with FetchContent and PRE_TEST discovery

```cmake
include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0)
FetchContent_MakeAvailable(googletest)

add_executable(mytests tests/test_foo.cpp)
target_link_libraries(mytests PRIVATE GTest::gtest_main mylib)
enable_testing()
include(GoogleTest)
gtest_discover_tests(mytests DISCOVERY_MODE PRE_TEST)
```

## 5. Sanitizer flags via generator expression

```cmake
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    target_compile_options(mylib PRIVATE
        -fsanitize=address,leak -fno-omit-frame-pointer -g)
    target_link_options(mylib PRIVATE -fsanitize=address,leak)
endif()
```

## 6. Platform-specific compile options

```cmake
target_compile_options(mylib PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall;-Wextra;-Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Weverything;-Wno-c++98-compat>
    $<$<PLATFORM_ID:Windows>:/W4;/WX>
)
```

## 7. install() with config-file package support

```cmake
install(TARGETS mylib EXPORT MyLibTargets
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT MyLibTargets
    FILE      MyLibTargets.cmake
    NAMESPACE MyLib::
    DESTINATION lib/cmake/MyLib)
```

## 8. Custom code-generation command (protobuf style)

```cmake
add_custom_command(
    OUTPUT  ${CMAKE_BINARY_DIR}/generated/foo.pb.cc
            ${CMAKE_BINARY_DIR}/generated/foo.pb.h
    COMMAND protoc --cpp_out=${CMAKE_BINARY_DIR}/generated
                   ${CMAKE_SOURCE_DIR}/proto/foo.proto
    DEPENDS ${CMAKE_SOURCE_DIR}/proto/foo.proto
    COMMENT "Compiling protobuf schema"
)
add_library(foo_proto STATIC ${CMAKE_BINARY_DIR}/generated/foo.pb.cc)
```

## 9. clang-tidy integration

```cmake
find_program(CLANG_TIDY_EXE clang-tidy)
if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--warnings-as-errors=*")
endif()
```

Or per-target only:
```cmake
set_target_properties(mylib PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
```

## 10. Precompiled headers for faster builds

```cmake
target_precompile_headers(mylib PRIVATE
    <vector>
    <string>
    <memory>
    <unordered_map>
    "include/mylib/common.hpp"
)
# Share PCH between targets (avoids recompiling the PCH):
target_precompile_headers(myapp REUSE_FROM mylib)
```

---

Build commands for the modern_cmake_demo:
```bash
export PATH="$(python3 -c 'import cmake; print(cmake.CMAKE_BIN_DIR)'):$PATH"
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure

# Run under AddressSanitizer:
cmake --preset asan
cmake --build --preset asan
ctest --preset asan --output-on-failure
```
