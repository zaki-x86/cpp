# 01-toolchain: CMake Toolchain Mastery

Demonstrates deep knowledge of the C++ build and tooling ecosystem.

## What This Project Shows

| Topic | How to demonstrate |
|---|---|
| Memory bug detection | `./scripts/run-asan.sh` |
| Data race detection | `./scripts/run-tsan.sh` |
| Undefined behavior | `cmake --preset ubsan && ./build/ubsan/ubsan_demo overflow` |
| LTO | `cmake --preset release` (IPO enabled via `CMAKE_INTERPROCEDURAL_OPTIMIZATION`) |
| PGO end-to-end | `./scripts/pgo_workflow.sh` |
| Cross-compilation | `cmake --preset cross-arm-linux` (requires ARM toolchain) |
| SSE/AVX2 intrinsics | `cmake --build --preset release --target intrinsics_demo && ./build/release/intrinsics_demo` |
| clang-tidy | `cmake --preset clang-tidy && cmake --build --preset clang-tidy` (requires Clang) |
| Assembly output | `cmake --build --preset release --target asm_output` |

## CMake Presets

```bash
cmake --preset <name>   # Configure
cmake --build --preset <name>   # Build
```

Available presets: `debug`, `release`, `asan`, `tsan`, `ubsan`, `msan` (Clang), `coverage`, `clang-tidy` (Clang), `cross-arm-linux`, `cross-arm-baremetal`, `pgo-generate`, `pgo-use`

## Interview Talking Points

- **"How do you find memory bugs?"** → ASan/TSan/MSan as CMake presets — CI runs asan preset on every PR
- **"How do you optimize a hot path?"** → PGO measures real execution paths, LTO enables cross-TU inlining; both enabled in `release` preset
- **"How do you ship C++ to embedded?"** → CMake toolchain files separate compiler config from build logic; bare-metal toolchain disables exceptions/RTTI, sets linker specs
- **"How do you maintain code quality?"** → clang-tidy as a CMake step with `WarningsAsErrors` — lint failures are build failures
- **"What are sanitizers?"** → Compiler instrumentation added at build time; ASan adds bounds checks around every allocation; TSan adds shadow memory to track lock state per memory location; UBSan adds checks for UB like signed overflow at every operation
