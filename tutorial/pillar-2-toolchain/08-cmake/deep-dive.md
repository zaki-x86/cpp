# CMake Deep Dive — Full Reference

## Modern Target-Based CMake

The three target types and when to use each:

```cmake
# STATIC: archived .o files. No runtime dependency.
add_library(mylib STATIC src/impl.cpp)

# SHARED: .so/.dll. Loaded at runtime. Consumers need it present.
add_library(mylib SHARED src/impl.cpp)

# INTERFACE: no compiled output. Used for header-only libraries.
add_library(mylib INTERFACE)
target_include_directories(mylib INTERFACE include/)
target_compile_features(mylib INTERFACE cxx_std_20)
```

Alias targets allow you to create namespaced references that mirror what `find_package` produces:
```cmake
add_library(MyProject::mylib ALIAS mylib)
# Now consumers can write: target_link_libraries(foo PRIVATE MyProject::mylib)
```

## CMakePresets.json v3 Schema

Version 3 supports CMake 3.22+. Key sections:

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "base",
      "hidden": true,
      "generator": "Unix Makefiles",
      "binaryDir": "${sourceDir}/build/${presetName}",
      "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
      }
    },
    {
      "name": "debug",
      "inherits": "base",
      "displayName": "Debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    }
  ],
  "buildPresets": [
    { "name": "debug", "configurePreset": "debug" }
  ],
  "testPresets": [
    {
      "name": "debug",
      "configurePreset": "debug",
      "output": { "outputOnFailure": true }
    }
  ]
}
```

`${sourceDir}` — the directory containing `CMakePresets.json`.
`${presetName}` — the preset name, useful for naming the binary dir.
`hidden: true` — preset cannot be selected directly; used as a base for inheritance only.

## Generator Expressions

Generator expressions are evaluated at build time, not configure time. Written as `$<...>`. Used in target properties, compile options, and install paths.

```cmake
# Apply flag only in Debug builds
target_compile_options(mylib PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)

# Different behavior per target type
target_compile_definitions(mylib
  INTERFACE $<INSTALL_INTERFACE:MYLIB_INSTALLED>
  PRIVATE   $<BUILD_INTERFACE:MYLIB_BUILDING>
)

# Conditional on feature support
target_compile_options(mylib PRIVATE
  $<$<CXX_COMPILER_ID:GNU>:-Wall;-Wextra>
  $<$<CXX_COMPILER_ID:Clang>:-Weverything>
)
```

Common generator expression categories:
- `$<CONFIG:Debug>` — true when build type is Debug
- `$<TARGET_FILE:tgt>` — path to built binary for target tgt
- `$<CXX_COMPILER_ID:GNU>` — true when compiler is GCC
- `$<BOOL:var>` — 0 or 1 based on variable truth value

## find_package: Config Mode vs Module Mode

CMake's `find_package(Foo)` searches in two modes:

**Config mode (preferred):** Looks for `FooConfig.cmake` or `foo-config.cmake` in known install prefixes. Modern libraries install these files. When found, you get namespaced targets: `Foo::Foo`, `Foo::bar`, etc. This is the right way.

```cmake
find_package(GTest REQUIRED)
target_link_libraries(mytests PRIVATE GTest::gtest_main)
```

**Module mode (legacy):** CMake ships with `FindFoo.cmake` files for common libraries (OpenSSL, Threads, Boost). These set variables like `FOO_INCLUDE_DIRS` and `FOO_LIBRARIES`. Used when the upstream library does not install a config file.

```cmake
find_package(Threads REQUIRED)
target_link_libraries(mylib PRIVATE Threads::Threads)
# Note: modern FindThreads.cmake does provide a target
```

Force one mode explicitly:
```cmake
find_package(Foo CONFIG REQUIRED)   # only config mode
find_package(Foo MODULE REQUIRED)   # only module mode
```

## FetchContent vs ExternalProject

**FetchContent** (CMake 3.11+): Downloads and integrates a dependency at configure time. The dependency's `CMakeLists.txt` is processed in the same CMake invocation. You get full target access.

```cmake
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
# GTest::gtest_main is now a real target
```

**ExternalProject** (CMake 2.8+): Downloads and builds a dependency as a separate CMake invocation at build time. Used when the dependency has its own incompatible build system, or when you need to cross-compile it separately.

```cmake
include(ExternalProject)
ExternalProject_Add(libsodium
  URL      https://example.com/libsodium-1.0.18.tar.gz
  URL_HASH SHA256=...
  CONFIGURE_COMMAND ./configure --prefix=<INSTALL_DIR>
  BUILD_COMMAND     make -j$(nproc)
  INSTALL_COMMAND   make install
)
```

Key difference: FetchContent gives you targets immediately; ExternalProject gives you a build step, and you must manually create imported targets pointing at the installed artifacts.

## Toolchain Files for Cross-Compilation

A toolchain file overrides compiler detection. Pass it via `CMakePresets.json` or `-DCMAKE_TOOLCHAIN_FILE=`:

```cmake
# arm-linux-gnueabihf.cmake
set(CMAKE_SYSTEM_NAME  Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

find_program(CMAKE_C_COMPILER   arm-linux-gnueabihf-gcc REQUIRED)
find_program(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++ REQUIRED)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

CMake then calls the cross compiler for all targets. `find_program` still uses the host; `find_library` uses the sysroot.

## Unity Builds and Precompiled Headers

**Unity builds** combine multiple `.cpp` files into one compilation unit, reducing compilation time by parsing common headers once.

```cmake
set_target_properties(mylib PROPERTIES UNITY_BUILD ON)
set_source_files_properties(src/a.cpp src/b.cpp
    PROPERTIES UNITY_GROUP "group1")
```

**Precompiled headers** (PCH) compile a header once and reuse the binary across translation units:

```cmake
target_precompile_headers(mylib PRIVATE
  <vector>
  <string>
  <memory>
  "include/mylib/common.hpp"
)
```

Both are compile-time optimizations. Unity builds can break code with anonymous namespaces or static variables with the same name across files. PCH is safer but provides less speedup.

## cmake --install and CPack Packaging

The install interface separates build-tree paths from install-tree paths:

```cmake
install(TARGETS mylib
  EXPORT  MyLibTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT MyLibTargets
  FILE      MyLibTargets.cmake
  NAMESPACE MyLib::
  DESTINATION lib/cmake/MyLib
)
```

Run: `cmake --install build/release --prefix /usr/local`

CPack generates installers (`.deb`, `.rpm`, `.tar.gz`, NSIS on Windows):

```cmake
set(CPACK_PACKAGE_NAME "MyLib")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_GENERATOR "TGZ;DEB")
include(CPack)
```

Run: `cd build/release && cpack`
