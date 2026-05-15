# Static Analysis with clang-tidy

This document covers the static analysis tooling in the `01-toolchain` project:
the `.clang-tidy` configuration, the intentional-issue demo file `linting_demo.cpp`,
the `StaticAnalyzers.cmake` CMake module, and how to invoke the tooling. Interview
talking points are included at the end.

---

## Table of Contents

1. [What clang-tidy Does and How It Differs from Compiler Warnings](#what-clang-tidy-does-and-how-it-differs-from-compiler-warnings)
2. [Annotated `.clang-tidy` Configuration](#annotated-clang-tidy-configuration)
3. [Annotated `linting_demo.cpp`](#annotated-linting_democpp)
4. [`StaticAnalyzers.cmake` Walkthrough](#staticanalyzerscmake-walkthrough)
5. [How to Run clang-tidy](#how-to-run-clang-tidy)
6. [Interview Talking Points](#interview-talking-points)

---

## What clang-tidy Does and How It Differs from Compiler Warnings

### Compiler Warnings (`-Wall`, `-Wextra`, ...)

Compiler warnings are generated as a **side-effect of compilation**. The compiler
parses and semantically analyses each translation unit, and warns about patterns it
considers suspicious: unused variables, signed/unsigned comparisons, unreachable code,
and so on. Warnings are:

- Fast (essentially free — they happen during normal compilation)
- Intra-procedural: the compiler only sees one function at a time during the warning
  phase
- Shallow: they operate on the AST but do not perform deep dataflow analysis

### clang-tidy

clang-tidy is a **separate program** built on top of libclang/LibTooling. It:

1. Invokes the Clang frontend to parse each source file, producing a full AST and
   (for some checks) a CFG (control-flow graph).
2. Runs a set of **pluggable check modules** over the AST, each implementing a specific
   pattern or coding guideline.
3. Reports diagnostics and, optionally, produces **automatic fix-its** that can be
   applied with `--fix`.

The key differences from compiler warnings:

| Dimension | Compiler Warnings | clang-tidy |
|-----------|------------------|------------|
| Speed | No overhead (free) | Significant: re-parses source |
| Depth | Shallow (single-pass) | Deep AST traversal + CFG analysis |
| Fix-its | Rare | Common; many checks can auto-fix |
| Configurability | Limited flag set | Hundreds of named checks, per-check options |
| Scope | Single TU, warnings only | TU-scoped, but check catalog covers architecture, style, safety |
| Integration | Always on during build | Opt-in; typically run in CI or on-demand |

clang-tidy checks are organized into **named groups** (prefixes like `bugprone-`,
`modernize-`, etc.). Each group corresponds to a specific body of knowledge: the
C++ Core Guidelines, the LLVM coding standard, performance idioms, and so on.

---

## Annotated `.clang-tidy` Configuration

**File:** `.clang-tidy` (project root of `01-toolchain`)

```yaml
Checks: >
  -*,
```
Start by **disabling all checks** (`-*`). This is the standard pattern: opt-in to
exactly the check groups you want, rather than fighting to suppress a large default set.
The `>` is YAML block scalar syntax — it continues the string on the lines that follow.

```yaml
  bugprone-*,
```
**`bugprone-*`** — Checks for patterns that are frequently the source of real bugs but
are not prohibited by the standard. Examples:
- `bugprone-use-after-move`: detects use of a moved-from object
- `bugprone-integer-overflow`: warns about expressions that can overflow
- `bugprone-sizeof-expression`: catches `sizeof(pointer)` when `sizeof(*pointer)` was
  likely intended
- `bugprone-string-constructor`: detects `std::string(n, char)` vs
  `std::string(char*, n)` confusion

```yaml
  cppcoreguidelines-*,
```
**`cppcoreguidelines-*`** — Enforces the C++ Core Guidelines (Stroustrup & Sutter).
Examples:
- `cppcoreguidelines-pro-type-cstyle-cast`: bans `(int)x`; require `static_cast<>`
- `cppcoreguidelines-pro-bounds-array-to-pointer-decay`: warns when array decays
- `cppcoreguidelines-special-member-functions`: the Rule of 5
- `cppcoreguidelines-interfaces-global-init`: warns about global initialization order

```yaml
  modernize-*,
```
**`modernize-*`** — Upgrades pre-C++11 idioms to modern C++. Examples:
- `modernize-use-nullptr`: replaces `NULL` and `0` used as null pointers with `nullptr`
- `modernize-use-override`: adds `override` to overriding virtual functions
- `modernize-use-auto`: replaces verbose type names with `auto` where unambiguous
- `modernize-loop-convert`: upgrades index-based `for` loops to range-based `for`
- `modernize-use-default-member-init`: moves member initializers to the declaration

```yaml
  performance-*,
```
**`performance-*`** — Detects common performance pitfalls. Examples:
- `performance-unnecessary-copy-initialization`: `auto x = container[i]` when `const
  auto& x = container[i]` would avoid a copy
- `performance-move-const-arg`: `std::move` on a `const` value does nothing
- `performance-for-range-copy`: range-`for` variable is copied when a reference suffices
- `performance-inefficient-string-concatenation`: repeated `+` on strings

```yaml
  readability-*,
```
**`readability-*`** — Enforces naming conventions, simplicity, and clarity. Examples:
- `readability-identifier-naming`: enforces naming rules configured in `CheckOptions`
- `readability-braces-around-statements`: requires braces even for single-statement
  `if`/`for` bodies
- `readability-const-return-type`: warns when a function returns `const T` by value
  (the qualifier is meaningless and confusing)
- `readability-qualified-auto`: requires `const auto*` instead of `auto*` for pointer
  variables pointing to `const` objects

```yaml
  -modernize-use-trailing-return-type,
```
Suppresses the check that would require every function to be rewritten as
`auto f() -> ReturnType`. That style is valid but unconventional for most codebases;
disabling it avoids noise.

```yaml
  -cppcoreguidelines-avoid-magic-numbers,
  -readability-magic-numbers,
```
Both of these checks flag numeric literals that are not named constants (`constexpr int
N = 500'000` would be fine, but `v.reserve(1000)` would not). They are disabled here
because demo and benchmark code intentionally uses bare numeric literals, and enabling
them would flood the output with low-value warnings.

```yaml
  -cppcoreguidelines-pro-bounds-pointer-arithmetic
```
This check bans all pointer arithmetic (`ptr + n`, `ptr[i]`). It is correct per the
Core Guidelines (use `std::span` instead) but is impractical in low-level code that
interacts with hardware buffers, SIMD intrinsics, or C APIs. Disabled to keep the
intrinsics demo and other low-level code clean.

---

```yaml
WarningsAsErrors: "bugprone-*,modernize-use-nullptr,modernize-use-override"
```
Escalates specific checks to **errors** (non-zero exit code). The three selected groups:

- `bugprone-*`: bug patterns should be treated as hard failures in CI
- `modernize-use-nullptr`: `NULL` in new code is never acceptable; make it an error
- `modernize-use-override`: missing `override` is a maintainability hazard (silent
  interface breakage when a virtual signature changes); treat as an error

The remaining checks (`cppcoreguidelines-*`, `readability-*`, `performance-*`,
`modernize-*` minus the two above) are warnings — reported but do not fail the build.
This is intentional: style checks should be informative, not blocking during active
development, but can be promoted to errors in a stricter CI stage.

---

```yaml
HeaderFilterRegex: ".*"
```
By default, clang-tidy only reports diagnostics in the **source file** being analyzed,
not in headers it includes. Setting this to `".*"` enables reporting in all headers
(including your own project headers). This catches issues like a missing virtual
destructor defined in a header. Be aware that this can produce noise from third-party
headers in edge cases; a more targeted filter like `"project/.*"` is common in
larger codebases.

---

```yaml
CheckOptions:
  - key: modernize-loop-convert.MinConfidence
    value: reasonable
```
`modernize-loop-convert` can suggest range-based `for` at three confidence levels:
`safe`, `reasonable`, and `risky`. `reasonable` allows some transformations that the
check is confident about but not certain are always equivalent (e.g., when the loop
modifies the container). `safe` is the most conservative; `risky` produces the most
suggestions.

```yaml
  - key: readability-identifier-naming.VariableCase
    value: camelCase
  - key: readability-identifier-naming.FunctionCase
    value: camelCase
  - key: readability-identifier-naming.ClassCase
    value: CamelCase
```
Configures the naming convention enforcement:

- Variables: `camelCase` (e.g., `myVariable`, `totalCount`)
- Functions: `camelCase` (e.g., `sumRange`, `sortChunks`)
- Classes/structs: `CamelCase` / `PascalCase` (e.g., `MyClass`, `BoundingBox`)

These are enforced only if `readability-identifier-naming` is active. Violations are
warnings (not escalated to errors in `WarningsAsErrors`).

---

## Annotated `linting_demo.cpp`

**File:** `src/linting_demo.cpp`

This file intentionally contains three canonical clang-tidy issues. It is a focused
demo — one example per major check group — rather than a realistic source file.

```cpp
// Intentional lint issues for clang-tidy demonstration.
// Run: cmake --preset clang-tidy && cmake --build --preset clang-tidy
// Expected: warnings for NULL, missing virtual destructor, C-style cast.

#include <cstdio>
```

### Issue 1: `NULL` Instead of `nullptr`

```cpp
void badNull(int* p) {
    if (p == NULL) return;      // clang-tidy: modernize-use-nullptr
    printf("%d\n", *p);         // clang-tidy: prefer std::cout (readability)
}
```

- **Check triggered:** `modernize-use-nullptr`
- **Escalation:** This is in `WarningsAsErrors` — the build fails, not just warns.
- **Why it matters:** `NULL` is typically `((void*)0)` or `0` depending on the
  platform. In C++, `nullptr` has type `std::nullptr_t`, which is unambiguous and
  cannot accidentally match an `int` parameter in an overload resolution.
- **Fix-it available:** Yes — clang-tidy will replace `NULL` with `nullptr`
  automatically when run with `--fix`.
- The `printf` line also triggers a `readability-*` suggestion to prefer `std::cout`.
  That check is a warning (not an error).

### Issue 2: Missing Virtual Destructor

```cpp
class Base {
public:
    virtual void doSomething() {}
    // Missing: virtual ~Base() = default;   <- clang-tidy flags this
};

class Derived : public Base {
public:
    void doSomething() override {}
};
```

- **Check triggered:** `cppcoreguidelines-special-member-functions` and
  `bugprone-*` (specifically the virtual destructor rule)
- **Why it matters:** If `Base*` or `Base&` is used as the type when deleting a
  `Derived` object, undefined behavior results — the `Derived` destructor is not called.
  The C++ standard is explicit: if any member function is virtual, the destructor should
  be `virtual` (or `protected` non-virtual, to prohibit base-pointer deletion).
- **The fix:** Add `virtual ~Base() = default;` to `Base`.
- **`override` demonstration:** `Derived::doSomething()` correctly uses `override`,
  which satisfies `modernize-use-override`.

### Issue 3: C-Style Cast

```cpp
void castDemo() {
    double d = 3.14;
    int i = (int)d;    // clang-tidy: cppcoreguidelines-pro-type-cstyle-cast
    (void)i;
}
```

- **Check triggered:** `cppcoreguidelines-pro-type-cstyle-cast`
- **Why it matters:** C-style casts are a "do anything" escape hatch — depending on
  context, `(T)expr` may perform a `static_cast`, `reinterpret_cast`, `const_cast`, or
  a combination. This makes them opaque and dangerous. Named casts (`static_cast<int>(d)`)
  communicate intent and are caught by code review and search tools.
- **Fix-it:** Replace with `static_cast<int>(d)`.
- **`(void)i`:** This is the idiomatic way to suppress "unused variable" warnings for a
  variable whose side effect (the cast) is the point. clang-tidy does not flag it.

---

## `StaticAnalyzers.cmake` Walkthrough

**File:** `cmake/modules/StaticAnalyzers.cmake`

This module provides two functions that activate static analyzers as part of the CMake
build. It is `include()`d from the project's `CMakeLists.txt` and the functions are
called conditionally based on cache variables.

```cmake
# Usage: enable_clang_tidy()  or  enable_cppcheck()
# Call from project CMakeLists.txt before defining targets.
```

The module must be included **before** `add_executable()` or `add_library()` calls
because `CMAKE_CXX_CLANG_TIDY` is read when a target is defined, not when the build
runs.

### `enable_clang_tidy()`

```cmake
function(enable_clang_tidy)
  find_program(CLANG_TIDY clang-tidy)
  # Search PATH for the clang-tidy executable.
  # find_program() caches the result in CMakeCache.txt so it is only searched once.

  if(CLANG_TIDY)
    message(STATUS "clang-tidy found: ${CLANG_TIDY}")
    set(CMAKE_CXX_CLANG_TIDY
        "${CLANG_TIDY};--extra-arg=-Wno-unknown-warning-option"
        CACHE STRING "" FORCE)
    # CMAKE_CXX_CLANG_TIDY is the CMake mechanism for clang-tidy integration.
    # When set, CMake passes the clang-tidy command as the "launcher" for every
    # compilation step. Each .cpp file is compiled AND analyzed in the same invocation.
    #
    # The extra arg "-Wno-unknown-warning-option" suppresses clang-tidy's diagnostic
    # when it sees GCC-specific warning flags (e.g., -Wno-maybe-uninitialized) in the
    # compile_commands.json that Clang does not recognize.
    #
    # CACHE STRING "" FORCE makes the variable visible project-wide and overrides
    # any previously cached value.
  else()
    message(WARNING "clang-tidy not found — static analysis disabled")
    # Graceful degradation: missing the tool is a warning, not an error.
    # This allows the project to build on machines without clang-tidy installed.
  endif()
endfunction()
```

### `enable_cppcheck()`

```cmake
function(enable_cppcheck)
  find_program(CPPCHECK cppcheck)

  if(CPPCHECK)
    message(STATUS "cppcheck found: ${CPPCHECK}")
    set(CMAKE_CXX_CPPCHECK
        "${CPPCHECK};--suppress=missingInclude;--enable=all;--error-exitcode=1"
        CACHE STRING "" FORCE)
    # CMAKE_CXX_CPPCHECK: analogous to CMAKE_CXX_CLANG_TIDY but for cppcheck.
    #
    # --suppress=missingInclude: cppcheck cannot always resolve all system includes
    #   (it has its own include database). This suppresses the flood of false positives
    #   that would otherwise result.
    # --enable=all: activates all cppcheck check categories including performance,
    #   portability, style, and information messages.
    # --error-exitcode=1: any finding causes cppcheck to exit with code 1, which
    #   CMake interprets as a build failure.
  else()
    message(WARNING "cppcheck not found — cppcheck analysis disabled")
  endif()
endfunction()
```

### How the Module Is Activated

In the project's `CMakeLists.txt`:

```cmake
include(StaticAnalyzers)

if(ENABLE_CLANG_TIDY)
  enable_clang_tidy()
endif()
```

The `ENABLE_CLANG_TIDY` cache variable is set by the `clang-tidy` CMake preset:

```json
{
  "name": "clang-tidy",
  "inherits": "debug",
  "cacheVariables": {
    "CMAKE_BUILD_TYPE": "Debug",
    "CMAKE_C_COMPILER": "clang",
    "CMAKE_CXX_COMPILER": "clang++",
    "ENABLE_CLANG_TIDY": "ON"
  }
}
```

Note that the `clang-tidy` preset forces `clang`/`clang++` as the compilers. This is
necessary because clang-tidy is a Clang-based tool; it uses libclang to parse source
files and must be invoked with the same flags that Clang understands. Running
clang-tidy with a GCC-generated `compile_commands.json` that contains GCC-specific
flags requires the `--extra-arg=-Wno-unknown-warning-option` workaround in the module.

---

## How to Run clang-tidy

### Method 1: CMake Preset (recommended for CI)

```bash
# Configure: sets up clang as compiler, enables clang-tidy via CMake
cmake --preset clang-tidy

# Build: compiles all sources AND runs clang-tidy on each one
cmake --build --preset clang-tidy
# clang-tidy diagnostics appear interleaved with compiler output.
# Any check in WarningsAsErrors causes a non-zero exit and stops the build.
```

The preset approach integrates analysis into the build: if clang-tidy finds an error,
the build artifact is not produced, making it impossible to ship without passing analysis.

### Method 2: Standalone clang-tidy (for interactive use)

```bash
# clang-tidy needs a compilation database to know flags for each file.
# The 'base' preset generates compile_commands.json via CMAKE_EXPORT_COMPILE_COMMANDS.
cmake --preset debug   # or any preset that writes compile_commands.json

# Run on a single file
clang-tidy src/linting_demo.cpp -p build/debug/

# Run on all project sources
find src -name '*.cpp' | xargs clang-tidy -p build/debug/

# Auto-apply fix-its (use with version control — check the diff before committing)
clang-tidy --fix src/linting_demo.cpp -p build/debug/
```

### Method 3: `run-clang-tidy` (parallel, for large codebases)

The LLVM project ships `run-clang-tidy.py` (often at
`/usr/lib/llvm-<version>/share/clang/run-clang-tidy.py`):

```bash
run-clang-tidy -p build/debug/ -j $(nproc)
# Runs clang-tidy in parallel across all files listed in compile_commands.json.
# -j controls parallelism; defaults to one thread per logical core.
```

### CI Integration Pattern

```yaml
# Example GitHub Actions step
- name: Run clang-tidy
  run: |
    cmake --preset clang-tidy
    cmake --build --preset clang-tidy 2>&1 | tee clang-tidy-report.txt
    # The build exits non-zero if any WarningsAsErrors check fires.
```

Alternatively, use the standalone form and post results as PR annotations using tools
like `clang-tidy-pr-feedback` or `reviewdog`.

---

## Interview Talking Points

### Positioning clang-tidy Against Other Tools

- **"clang-tidy is not a linter — it is an AST-based analyzer"** — Unlike simple regex
  linters, clang-tidy fully parses and type-checks the code. It understands templates,
  overload resolution, and implicit conversions. A regex linter cannot distinguish
  `NULL` the null pointer from `NULL` used as a macro in a different context.
- **"It complements compiler warnings, not replaces them"** — Run both. Compiler
  warnings catch issues during normal development at zero extra cost. clang-tidy catches
  a different and deeper category of issues, and is worth the extra CI time.
- **"Fix-its are a force multiplier"** — When modernizing a large codebase from C++11
  to C++17, `clang-tidy --fix` with `modernize-*` can perform thousands of mechanical
  transforms (range-`for`, `auto`, `nullptr`, `override`) automatically and safely.

### On Configuration Philosophy

- **"Start restrictive, open up"** — The `-*` + opt-in pattern means you only see
  checks you have explicitly decided to care about. This avoids the "warning fatigue"
  that comes from enabling everything and treating most findings as acceptable noise.
- **"`WarningsAsErrors` is the gating mechanism"** — Checks that are errors break the
  build and must be resolved before merging. Checks that are warnings inform developers
  without blocking them. Choosing what goes in each bucket is a deliberate team decision.
- **"Know what you are suppressing and why"** — Suppressions like
  `-cppcoreguidelines-pro-bounds-pointer-arithmetic` are not "ignoring safety rules" —
  they reflect that the rule is incompatible with low-level code in this project (SIMD
  intrinsics, hardware buffers). Document the rationale in the `.clang-tidy` file.

### On Static Analysis in CI Pipelines

- **"Integrate at the lowest pain point"** — `CMAKE_CXX_CLANG_TIDY` runs analysis on
  every compile, catching issues the moment they are introduced rather than in a
  separate CI job that runs later.
- **"Compile commands DB is the key artifact"** — `compile_commands.json` (generated by
  `CMAKE_EXPORT_COMPILE_COMMANDS`) is what clang-tidy uses to know the exact flags for
  each file. Without it, clang-tidy guesses at include paths and flags, producing false
  positives.
- **"Baseline the findings before enforcing"** — When introducing clang-tidy into an
  existing codebase with thousands of findings, use `clang-tidy --list-checks` to
  enumerate, run `run-clang-tidy` to get a full count, apply auto-fix for mechanical
  issues, and then gate CI on zero new findings using `--filter-files` or a baseline
  file. Do not gate on zero total findings on day one.
- **"clang-tidy and sanitizers are complementary"** — clang-tidy catches issues
  statically (before the code runs). Sanitizers (ASan, UBSan) catch issues dynamically
  (at runtime). They cover different bug classes and are both necessary in a mature
  quality workflow.
