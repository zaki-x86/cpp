---
description: 
alwaysApply: true
---

# C++ Interview Workspace

## Toolchain
- Compiler: GCC 11.4.0 (`g++`). No `std::expected` (needs GCC 12+), no `std::format` (needs GCC 13+).
- CMake 4.3.2 installed via `pip3 install cmake`. Add to PATH: `export PATH="$(python3 -c 'import cmake; print(cmake.CMAKE_BIN_DIR)'):$PATH"`
- Generator: "Unix Makefiles" (Ninja not installed; install with `sudo apt-get install ninja-build`)
- All CMakePresets.json files use `"version": 3` (compatible with CMake 3.22+)

## CMake Patterns
- `require_cpp20(target)` in `cmake/modules/StandardVersion.cmake` detects INTERFACE libraries and switches to INTERFACE visibility automatically.
- `gtest_discover_tests(target DISCOVERY_MODE PRE_TEST)` — always use PRE_TEST; sanitized binaries cannot run at build time.
- Shared CMake modules live in `cmake/modules/`; each project adds them with `list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake/modules")`.

## Sanitizers on WSL2
- ASan/LeakSan: work correctly. Run via `cmake --preset asan && ctest --preset asan`.
- TSan: builds fine but **cannot execute** in WSL2 — `FATAL: ThreadSanitizer: unexpected memory mapping`. Native Linux or Docker required.
- UBSan: works correctly.

## Project Structure
- Each project: `projects/<NN-name>/` with its own CMakeLists.txt + CMakePresets.json.
- Tutorial: `tutorial/` — 24-chapter C++ bible authored by Forge (5 pillars: language, toolchain, systems, domain-systems, visual). Sub-plans: `docs/superpowers/plans/2026-05-06-cpp-bible-NN-*.md`.
- Specs: `docs/superpowers/specs/` | Plans: `docs/superpowers/plans/`
- All docs/diagrams as `.md` with Mermaid blocks — no browser tools.

## Planning Conventions
- Sub-plans: split large implementation plans by pillar/phase when content exceeds ~1000 lines. Naming: `YYYY-MM-DD-<project>-NN-<phase>.md`.
- Background agent transcripts are JSONL (`/tmp/claude-1000/.../tasks/*.output`) — do not Read directly; check file existence with `ls` instead.

## Multi-Agent System
- Two AI agents collaborate on this workspace. Each owns `.agents/<name>/` with an `identity.md` and a `log.md`.
- **Atlas** (`claude-sonnet-4-6`) — lead C++ infrastructure agent. Log: `.agents/atlas/log.md`.
- **Forge** (`claude-sonnet-4-6`) — tutorial authoring agent. Log: `.agents/forge/log.md`. Owns `tutorial/`.
- Before starting any task, read all `.agents/*/log.md` files to see what is claimed or in progress.
- After completing any task, append a timestamped entry to your own log (format in `.agents/README.md`).
- Cross-agent handoffs go in `.agents/incoming/`. Log files are gitignored; identity files are committed.

## foundation library (02-foundation) API notes
- `Buffer::data()` returns `std::byte*` — use `std::byte{val}` to assign and `std::to_integer<int>()` to read.
- `Sorter<Policy>::sort(first, last)` takes iterators, not a container.
- `TypeList<...>::size` is a static constexpr member; use `L::size`, not a separate `type_list_size` trait.
- TSan test preset exists but is expected to fail in WSL2 (build-only verification).
