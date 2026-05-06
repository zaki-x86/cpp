# Chapter 08: CMake — Build System Mastery

**What you'll learn:** How modern CMake works, why target-based design replaced directory-based design, how CMakePresets.json eliminates environment inconsistency, and how to integrate package managers.

**Prerequisites:** Basic C++ compilation familiarity. Understanding of what a linker does.

**Time estimate:** Core = 30 min. Deep Dive = 2 hours. Interview = 30 min.

**Reading paths:**
- Interview prep only: `interview.md`
- First-time CMake user: `core.md` → `examples/cmake_patterns.md` → Lab
- Deep reference: `core.md` → `deep-dive.md` → examples

**Lab:** `projects/01-toolchain/` — full CMakePresets.json with debug/asan/tsan/ubsan/coverage/pgo presets and shared modules in `cmake/modules/`.
