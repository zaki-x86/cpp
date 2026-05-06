# CMake Interview Questions

---

**Q: What is the difference between PRIVATE, PUBLIC, and INTERFACE in target_link_libraries?**

**A:** These visibility keywords control transitive dependency propagation. PRIVATE means the dependency is an implementation detail — consumers of this target do not inherit it. PUBLIC means the dependency is part of this target's public interface — consumers automatically inherit it. INTERFACE applies only to consumers, not to this target's own build — used for header-only libraries that pass requirements through without compiling anything themselves. Getting this right prevents accidental transitive dependencies from polluting downstream targets.

**Trap:** Many candidates say "PRIVATE is for private headers and PUBLIC is for public headers." This is backwards thinking — the keyword controls what *consumers inherit*, not where headers live.

**Follow-up:** If library A links B with PUBLIC, and executable C links A, does C get B? Yes — PUBLIC propagates transitively.

---

**Q: What is the difference between find_package config mode and module mode?**

**A:** Config mode looks for a `FooConfig.cmake` file installed by the library itself — modern libraries provide this, and it gives you properly namespaced imported targets like `Foo::Foo`. Module mode runs a `FindFoo.cmake` file bundled with CMake, which sets legacy variables like `FOO_INCLUDE_DIRS` and `FOO_LIBRARIES`. Config mode is always preferred when available because it gives you real targets with correct transitive dependencies. Module mode exists for older libraries that predate the config-file convention.

**Trap:** Candidates often forget that `find_package` tries config mode first by default and falls back to module mode. You can force one with `CONFIG` or `MODULE` keywords.

**Follow-up:** How do you write your own config file for a library? You use `install(EXPORT ...)` and `configure_package_config_file()`.

---

**Q: What do generator expressions solve, and give an example?**

**A:** Generator expressions are evaluated at build time rather than configure time. They solve the problem of needing different values depending on the build configuration, compiler, or target type — information not fully known at configure time. A common example: `$<$<CONFIG:Debug>:-fsanitize=address>` applies the sanitizer flag only in Debug builds without needing separate if/else logic. Another: `$<TARGET_FILE:mytarget>` gives the full path to the built binary, used in custom commands that depend on the output.

**Trap:** Candidates try to use generator expressions in `if()` conditions, which does not work — they evaluate to the literal string `$<...>` at configure time.

**Follow-up:** Can you use generator expressions in `add_custom_command`? Yes — `COMMAND` and `DEPENDS` accept them.

---

**Q: What is the difference between FetchContent and ExternalProject?**

**A:** FetchContent downloads and integrates a dependency at configure time, processing its CMakeLists.txt as part of the current build. You get real CMake targets immediately and can use them with `target_link_libraries`. ExternalProject builds a dependency as a separate CMake invocation at build time, which is necessary when the dependency has an incompatible build system (Autotools, plain make) or needs cross-compilation with different flags. ExternalProject does not give you real targets — you must manually create imported targets pointing at the installed artifacts.

**Trap:** Using ExternalProject when FetchContent would work, then being confused why the targets are not available at configure time.

**Follow-up:** What happens if two FetchContent dependencies require different versions of the same library? The first one wins (first-to-populate wins). This is a real problem that package managers solve better.

---

**Q: What problem does CMakePresets.json solve?**

**A:** It standardizes configure, build, and test invocations across all machines. Before presets, developers would use custom shell scripts, README instructions, or just type long cmake commands with many -D flags — each person's environment was slightly different. CMakePresets.json is committed to the repository and encodes all presets with their generator, binary directory, and cache variables. CI, Docker containers, and every developer use identical commands: `cmake --preset asan && cmake --build --preset asan && ctest --preset asan`. It also supports inheritance so a dozen presets can share a common base without repetition.

**Trap:** Candidates confuse CMakePresets.json (committed, shared) with CMakeUserPresets.json (gitignored, personal overrides).

**Follow-up:** What CMake version does preset schema version 3 require? CMake 3.22.

---

**Q: What is a unity build and what are its tradeoffs?**

**A:** A unity build combines multiple `.cpp` files into a single compilation unit. This reduces compilation time because the compiler parses common headers (like `<string>`, `<vector>`, system headers) only once instead of once per file. The speedup can be dramatic — 2–5x — for large projects with many headers. The tradeoffs: anonymous namespaces and static variables with the same name in different files will conflict. Some macro definitions in one file can accidentally affect another. Unity builds can also hide missing includes (a file that should include X works because another file in the unit included it). Enable with `set_target_properties(target PROPERTIES UNITY_BUILD ON)`.

**Trap:** Candidates say unity builds are always better. They are build-time optimizations that can break code.

**Follow-up:** Is unity build the same as precompiled headers? No — PCH compiles a header once and reuses the binary; unity build merges source files together.

---

**Q: Why is GLOB dangerous for listing source files in CMake?**

**A:** `file(GLOB SOURCES src/*.cpp)` captures all matching files at configure time. If you add a new `.cpp` file to the directory, CMake does not know to re-run — it only re-runs when `CMakeLists.txt` changes. Your new file silently never gets compiled. The fix is to list source files explicitly in `CMakeLists.txt`. This is a real source of confusion where "I added the file but my changes are not running." CMake 3.12 added `CONFIGURE_DEPENDS` to work around this, but it forces a slow glob check on every build.

**Trap:** Candidates defend GLOB for convenience. The correct answer is: explicit file lists, or `CONFIGURE_DEPENDS` as a compromise.

**Follow-up:** Is there a tool to help manage explicit source lists? Some teams use scripts or IDE integrations. The real answer is that the pain of explicit lists drives adoption of modular libraries where each library is small.

---

**Q: What is the difference between add_custom_command and add_custom_target?**

**A:** `add_custom_command` generates a file as a build step and participates in dependency tracking — if its output file is out of date relative to its inputs, it re-runs. It must be associated with a target (via `DEPENDS`) to be built. `add_custom_target` is always out of date and always runs when built — it has no output file tracking. Use `add_custom_command` for code generation (protobuf, GLSL compilation) where you want incremental builds. Use `add_custom_target` for utility commands like running tests, formatting, or generating documentation where you always want the action to run.

**Trap:** Using `add_custom_target` for code generation and being confused why it re-runs even when inputs have not changed, or using `add_custom_command` for a utility target and being confused why it never runs.

**Follow-up:** How do you make a custom command run before a specific target builds? Use the `PRE_BUILD` or `PRE_LINK` step in `add_custom_command(TARGET ...)`.
