# Workspace Bootstrap Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Initialize the workspace with git, shared CMake modules, toolchain helpers, environment setup script, and docs scaffolding so every subsequent project can build on a consistent foundation.

**Architecture:** Single git repo at `/home/zaki/workspaces/cpp/`. Each project under `projects/` is an independent CMake project. Shared CMake helpers live in `cmake/modules/` and are imported by any project that needs them.

**Tech Stack:** git, CMake 3.25+, bash

---

### Task 1: Initialize git repository and base directory structure

**Files:**
- Create: `/home/zaki/workspaces/cpp/.gitignore`
- Create: `/home/zaki/workspaces/cpp/README.md`

- [ ] **Step 1: Initialize git**

```bash
cd /home/zaki/workspaces/cpp
git init
git config user.name "$(git config --global user.name)"
git config user.email "$(git config --global user.email)"
```

Expected: `Initialized empty Git repository in /home/zaki/workspaces/cpp/.git/`

- [ ] **Step 2: Create `.gitignore`**

Create `/home/zaki/workspaces/cpp/.gitignore`:

```gitignore
# CMake build directories
build/
build-*/
out/

# CMake generated
CMakeFiles/
CMakeCache.txt
cmake_install.cmake
CTestTestfile.cmake
_deps/

# Compiled output
*.o
*.a
*.so
*.dll
*.exe

# Coverage
*.gcno
*.gcda
*.lcov
coverage/

# IDE
.vscode/
.idea/
*.swp
*.swo
.DS_Store

# Sanitizer logs
*.log

# PGO profiles
*.profraw
*.profdata
default.profdata
```

- [ ] **Step 3: Create top-level README**

Create `/home/zaki/workspaces/cpp/README.md`:

```markdown
# C++ Senior Engineer Interview Prep

10 projects across 3 days covering the full C++ ecosystem.

## Structure

| Directory | Purpose |
|---|---|
| `projects/01-toolchain/` | CMake + compiler toolchain mastery |
| `projects/02-foundation/` | Modern C++ language features (C++11→23) |
| `projects/03-ros2/` | ROS2 robotics system |
| `projects/04-cuda/` | CUDA parallel compute |
| `projects/05-embedded/` | Embedded/RTOS simulator |
| `projects/06-ai-inference/` | C++ AI/ML inference pipeline |
| `projects/07-opengl/` | Mini 3D renderer |
| `projects/08-qt/` | Qt6 system monitor |
| `projects/09-imgui/` | DearImGui debug tools panel |
| `projects/10-crown-jewel/` | Real-time sensor fusion visualizer |
| `cmake/modules/` | Shared CMake helpers used by all projects |
| `docs/` | Specs, plans, concepts, cheatsheets |

## Prerequisites

Run `scripts/setup.sh` to verify your environment.
```

- [ ] **Step 4: Commit**

```bash
cd /home/zaki/workspaces/cpp
git add .gitignore README.md
git commit -m "chore: initialize workspace"
```

Expected: `1 file changed` or `2 files changed`

---

### Task 2: Create shared CMake modules

**Files:**
- Create: `cmake/modules/CompilerWarnings.cmake`
- Create: `cmake/modules/Sanitizers.cmake`
- Create: `cmake/modules/StaticAnalyzers.cmake`
- Create: `cmake/modules/Coverage.cmake`
- Create: `cmake/modules/StandardVersion.cmake`

- [ ] **Step 1: Create `cmake/modules/` directory**

```bash
mkdir -p /home/zaki/workspaces/cpp/cmake/modules
```

- [ ] **Step 2: Create `CompilerWarnings.cmake`**

Create `/home/zaki/workspaces/cpp/cmake/modules/CompilerWarnings.cmake`:

```cmake
# Usage: target_apply_warnings(my_target)
function(target_apply_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX)
  else()
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic -Werror
      -Wshadow -Wnon-virtual-dtor -Wold-style-cast
      -Wcast-align -Wunused -Woverloaded-virtual
      -Wconversion -Wsign-conversion -Wnull-dereference
      -Wdouble-promotion -Wformat=2
    )
  endif()
endfunction()
```

- [ ] **Step 3: Create `Sanitizers.cmake`**

Create `/home/zaki/workspaces/cpp/cmake/modules/Sanitizers.cmake`:

```cmake
# Reads ENABLE_SANITIZER_* cache variables set by CMakePresets
# Usage: target_apply_sanitizers(my_target)
function(target_apply_sanitizers target)
  set(sanitizers "")

  if(ENABLE_SANITIZER_ADDRESS)
    list(APPEND sanitizers "address")
  endif()
  if(ENABLE_SANITIZER_THREAD)
    list(APPEND sanitizers "thread")
  endif()
  if(ENABLE_SANITIZER_UNDEFINED)
    list(APPEND sanitizers "undefined")
  endif()
  if(ENABLE_SANITIZER_MEMORY)
    list(APPEND sanitizers "memory")
  endif()
  if(ENABLE_SANITIZER_LEAK)
    list(APPEND sanitizers "leak")
  endif()

  if(sanitizers)
    list(JOIN sanitizers "," sanitizer_flags)
    target_compile_options(${target} PRIVATE -fsanitize=${sanitizer_flags} -fno-omit-frame-pointer -g)
    target_link_options(${target} PRIVATE -fsanitize=${sanitizer_flags})
  endif()
endfunction()
```

- [ ] **Step 4: Create `StaticAnalyzers.cmake`**

Create `/home/zaki/workspaces/cpp/cmake/modules/StaticAnalyzers.cmake`:

```cmake
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
```

- [ ] **Step 5: Create `Coverage.cmake`**

Create `/home/zaki/workspaces/cpp/cmake/modules/Coverage.cmake`:

```cmake
# Usage: target_apply_coverage(my_target)
# Then run: cmake --build . --target coverage
function(target_apply_coverage target)
  if(ENABLE_COVERAGE)
    target_compile_options(${target} PRIVATE --coverage -O0 -g)
    target_link_options(${target} PRIVATE --coverage)
  endif()
endfunction()

function(add_coverage_target)
  if(ENABLE_COVERAGE)
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    if(LCOV AND GENHTML)
      add_custom_target(coverage
        COMMAND ${LCOV} --capture --directory . --output-file coverage.info
        COMMAND ${LCOV} --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.info
        COMMAND ${GENHTML} coverage.info --output-directory coverage_report
        COMMAND ${CMAKE_COMMAND} -E echo "Coverage report: coverage_report/index.html"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report"
      )
    endif()
  endif()
endfunction()
```

- [ ] **Step 6: Create `StandardVersion.cmake`**

Create `/home/zaki/workspaces/cpp/cmake/modules/StandardVersion.cmake`:

```cmake
# Sets C++23 as default with fallback detection
# Usage: require_cpp23(my_target)
function(require_cpp23 target)
  target_compile_features(${target} PRIVATE cxx_std_23)
  set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
endfunction()

function(require_cpp20 target)
  target_compile_features(${target} PRIVATE cxx_std_20)
  set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
endfunction()
```

- [ ] **Step 7: Commit**

```bash
cd /home/zaki/workspaces/cpp
git add cmake/
git commit -m "chore: add shared CMake modules (warnings, sanitizers, analysis, coverage)"
```

---

### Task 3: Create setup.sh environment bootstrap

**Files:**
- Create: `scripts/setup.sh`

- [ ] **Step 1: Create `scripts/setup.sh`**

```bash
mkdir -p /home/zaki/workspaces/cpp/scripts
```

Create `/home/zaki/workspaces/cpp/scripts/setup.sh`:

```bash
#!/usr/bin/env bash
set -euo pipefail

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

ok()   { echo -e "${GREEN}[OK]${NC}  $1"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
fail() { echo -e "${RED}[FAIL]${NC} $1"; }

echo "=== C++ Workspace Environment Check ==="
echo ""

# CMake
if cmake --version &>/dev/null; then
  CMAKE_VER=$(cmake --version | head -1 | awk '{print $3}')
  ok "CMake $CMAKE_VER"
else
  fail "CMake not found — install cmake >= 3.25"
fi

# GCC
if g++ --version &>/dev/null; then
  GCC_VER=$(g++ --version | head -1 | awk '{print $NF}')
  ok "GCC $GCC_VER"
else
  warn "GCC not found"
fi

# Clang
if clang++ --version &>/dev/null; then
  CLANG_VER=$(clang++ --version | head -1 | awk '{print $4}')
  ok "Clang $CLANG_VER"
else
  warn "Clang not found"
fi

# Ninja
if ninja --version &>/dev/null; then
  ok "Ninja $(ninja --version)"
else
  warn "Ninja not found — builds will use make (slower)"
fi

# clang-tidy
if clang-tidy --version &>/dev/null; then
  ok "clang-tidy found"
else
  warn "clang-tidy not found — install with: sudo apt install clang-tidy"
fi

# CUDA
if nvcc --version &>/dev/null; then
  CUDA_VER=$(nvcc --version | grep release | awk '{print $6}' | tr -d ',')
  ok "CUDA $CUDA_VER"
else
  warn "nvcc not found — project 04-cuda will be skipped"
fi

# ROS2
if [ -n "${ROS_DISTRO:-}" ]; then
  ok "ROS2 $ROS_DISTRO"
else
  warn "ROS2 not sourced — project 03-ros2 will be skipped"
fi

# Qt6
if pkg-config --exists Qt6Core 2>/dev/null; then
  ok "Qt6 found"
else
  warn "Qt6 not found — project 08-qt will be skipped"
fi

# OpenGL / GLFW
if pkg-config --exists glfw3 2>/dev/null; then
  ok "GLFW3 found"
else
  warn "GLFW3 not found — install: sudo apt install libglfw3-dev"
fi

echo ""
echo "=== Done ==="
```

- [ ] **Step 2: Make executable and test**

```bash
chmod +x /home/zaki/workspaces/cpp/scripts/setup.sh
/home/zaki/workspaces/cpp/scripts/setup.sh
```

Expected: Colored output showing which tools are installed.

- [ ] **Step 3: Commit**

```bash
cd /home/zaki/workspaces/cpp
git add scripts/setup.sh
git commit -m "chore: add environment bootstrap script"
```

---

### Task 4: Scaffold docs structure

**Files:**
- Create: `docs/roadmap.md`
- Create: `docs/concepts/.gitkeep`
- Create: `docs/cheatsheets/.gitkeep`

- [ ] **Step 1: Create docs scaffold**

```bash
mkdir -p /home/zaki/workspaces/cpp/docs/concepts
mkdir -p /home/zaki/workspaces/cpp/docs/cheatsheets
touch /home/zaki/workspaces/cpp/docs/concepts/.gitkeep
touch /home/zaki/workspaces/cpp/docs/cheatsheets/.gitkeep
```

- [ ] **Step 2: Create `docs/roadmap.md`**

Create `/home/zaki/workspaces/cpp/docs/roadmap.md`:

```markdown
# C++ Senior Interview Prep — Master Roadmap

## Timeline

| Day | Projects | Focus |
|-----|----------|-------|
| Day 1 | 01-toolchain, 02-foundation | CMake mastery + full C++ language core |
| Day 2 | 03-ros2, 04-cuda, 05-embedded, 06-ai-inference | Systems programming |
| Day 3 | 07-opengl, 08-qt, 09-imgui, 10-crown-jewel | Visual/UI + crown jewel |

## Daily Schedule (8–10 hrs)

### Day 1
- 09:00–10:30 CMake presets, sanitizers, flags
- 10:30–11:30 Cross-compilation, LTO/PGO, intrinsics
- 11:30–12:30 clang-tidy, static analysis, assembly output
- 12:30–13:00 Break
- 13:00–14:30 Memory: RAII, smart ptrs, allocators, pmr
- 14:30–16:00 OOP: Rule of 0/3/5, move semantics, CRTP
- 16:00–17:30 Templates: variadic, TMP, SFINAE, concepts
- 17:30–19:00 Rich types, patterns, concurrency, C++20/23

### Day 2
- 09:00–10:30 ROS2: lifecycle nodes, components, params
- 10:30–11:30 ROS2: custom msgs/srvs/actions, TF2, executor
- 11:30–12:00 Break
- 12:00–13:30 CUDA: memory model, thread hierarchy
- 13:30–14:30 CUDA: Thrust, streams, unified memory
- 14:30–15:00 Break
- 15:00–16:30 Embedded: RTOS sim, memory pools, ISR
- 16:30–17:30 Embedded: HSM, ring buffer, MISRA
- 17:30–19:00 AI inference: ONNX/LibTorch C++ API

### Day 3
- 09:00–10:30 OpenGL: VAO/VBO, shaders, lighting, FBO
- 10:30–12:00 Qt: MVC, signals/slots, QML, threading
- 12:00–13:30 ImGui: docking, ImPlot, ImNodes
- 13:30–14:30 Crown jewel: architecture + ECS scaffold
- 14:30–16:00 Crown jewel: CUDA pipeline + OpenGL renderer
- 16:00–17:30 Crown jewel: ImGui panel + coroutine pipeline
- 17:30–18:30 Crown jewel: polish, README, demo

## Implementation Plans

| Plan | File |
|------|------|
| 00 Workspace Bootstrap | `docs/superpowers/plans/2026-05-05-00-workspace-bootstrap.md` |
| 01 Toolchain | `docs/superpowers/plans/2026-05-05-01-toolchain.md` |
| 02 Foundation | `docs/superpowers/plans/2026-05-05-02-foundation.md` |
| 03 ROS2 | `docs/superpowers/plans/2026-05-05-03-ros2.md` |
| 04 CUDA | `docs/superpowers/plans/2026-05-05-04-cuda.md` |
| 05 Embedded | `docs/superpowers/plans/2026-05-05-05-embedded.md` |
| 06 AI Inference | `docs/superpowers/plans/2026-05-05-06-ai-inference.md` |
| 07 OpenGL | `docs/superpowers/plans/2026-05-05-07-opengl.md` |
| 08 Qt | `docs/superpowers/plans/2026-05-05-08-qt.md` |
| 09 ImGui | `docs/superpowers/plans/2026-05-05-09-imgui.md` |
| 10 Crown Jewel | `docs/superpowers/plans/2026-05-05-10-crown-jewel.md` |

## Success Criteria

- [ ] 10 independent projects built and runnable
- [ ] Every project has a README explaining what it demonstrates
- [ ] Can explain every architectural decision in `10-crown-jewel` from first principles
- [ ] Can reproduce core demos from `02-foundation` live under pressure
- [ ] Can discuss trade-offs without hesitation: virtual vs CRTP, shared_ptr vs arena, exceptions vs expected
- [ ] `01-toolchain` presets work: ASan catches injected bug, TSan catches injected race
```

- [ ] **Step 3: Commit**

```bash
cd /home/zaki/workspaces/cpp
git add docs/
git commit -m "chore: scaffold docs structure and master roadmap"
```
