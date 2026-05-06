# Interview: Orientation Q&A

> Eight questions. Full answers. The wrong answer most candidates give. The follow-up that separates seniors.

---

**Q1: What is undefined behavior and why does C++ have it?**

**A:** Undefined behavior (UB) is a situation where the C++ standard places no requirements on what a program does. The compiler is free to assume UB never occurs, which allows it to perform optimizations that would be illegal if the behavior were defined. For example, because signed integer overflow is UB, the compiler can assume `i + 1 > i` is always true for signed `i`, enabling loop transformations that would otherwise require overflow checks. C++ has UB because it is a zero-cost abstraction language — defining the behavior of these edge cases would require runtime checks that would make the language slower than C. Rust and Java define these behaviors and pay a performance cost; C++ does not.

**Trap:** "UB means the program crashes." Wrong. UB means anything can happen — including appearing to work correctly. The most dangerous UB is silent misbehavior that passes all tests and then corrupts production data.

**Follow-up:** "What tools catch undefined behavior at runtime?" — ASan (AddressSanitizer) catches out-of-bounds memory access and use-after-free. UBSan (UndefinedBehaviorSanitizer) catches signed overflow, null dereferences, misaligned access, and integer type punning. Neither catches data races — TSan (ThreadSanitizer) does that.

---

**Q2: What is the difference between a declaration and a definition?**

**A:** A declaration introduces a name and its type to the compiler without allocating storage or providing an implementation. A definition provides the complete description — for a function, the body; for a variable, storage allocation; for a class, the full member list. `extern int x;` is a declaration. `int x = 0;` is a definition. `void foo(int);` is a declaration. `void foo(int n) { return; }` is a definition. You can declare something any number of times, but you can define it only once (the One Definition Rule). Declarations go in headers; definitions go in `.cpp` files (with exceptions for templates, `inline` functions, and `constexpr` variables).

**Trap:** Treating `class Foo {};` as a declaration. It is a definition. A forward declaration is `class Foo;` — it tells the compiler Foo exists but not what it contains. With a forward declaration you can declare pointers and references to Foo but cannot create instances or access members.

**Follow-up:** "When would you use a forward declaration instead of including the header?" — When you only need a pointer or reference to a type in a header. Including the full header creates a compilation dependency that forces recompilation of everything that includes your header whenever the dependency's header changes. Forward declarations minimize these cascading rebuilds.

---

**Q3: What is the One Definition Rule?**

**A:** The One Definition Rule (ODR) states that every entity in a C++ program must have exactly one definition. Functions and variables must be defined exactly once across all translation units. Class types, `inline` functions, and `constexpr` variables may be defined in multiple translation units provided every definition is identical token-for-token. Violating the ODR for functions and variables is undefined behavior — not a guaranteed link error. The linker may silently pick one definition and discard the other, leading to wrong behavior that is extremely difficult to diagnose. The most common ODR violation is defining a non-inline function in a header that is included by multiple `.cpp` files.

**Trap:** "The linker will always catch ODR violations." Not true. If both definitions have the same mangled name, the linker picks one. Only if the names differ (which they won't for true ODR violations) does the linker report a duplicate symbol error.

**Follow-up:** "How does `inline` relate to the ODR?" — `inline` on a function tells the linker that multiple identical definitions are expected (because the function is defined in a header) and that it should merge them into one. Without `inline`, defining a function in a header and including it in two `.cpp` files produces two definitions with the same symbol name, which the linker may or may not catch.

---

**Q4: What happens between `main()` returning and the process exiting?**

**A:** After `main()` returns, the C++ runtime runs global destructors in reverse order of construction, calls functions registered with `atexit()` in reverse registration order, flushes and closes all C stdio streams, and then calls `_exit()` (the OS-level process termination syscall). Importantly, destructors of objects with static storage duration (global and function-`static` objects) all run here. This is why global objects with non-trivial destructors can cause problems: if the destructor of one global depends on another global that has already been destroyed (the static destruction order fiasco), you get undefined behavior. C++ provides no guarantee on the relative order of destruction across translation units.

**Trap:** "The process exits as soon as main returns." No — a non-trivial amount of cleanup runs. If you have globals with expensive destructors (e.g., a database connection pool flushing pending writes), main returning can take measurable time.

**Follow-up:** "What is the static initialization order fiasco?" — Global objects are initialized before `main()` in an order that is guaranteed within a translation unit (in source order) but unspecified across translation units. If global A depends on global B being initialized first, and they are in different `.cpp` files, you have undefined behavior. The solution is the Meyers singleton: a function-local `static` is guaranteed to be initialized the first time the function is called, providing on-demand, thread-safe initialization (C++11 and later).

---

**Q5: Why does `static` mean different things in different contexts?**

**A:** `static` is one of the most overloaded keywords in C++. At file scope (outside any class or function), `static` gives internal linkage — the name is visible only within its translation unit and cannot be referenced by other object files. Inside a function, `static` gives the local variable static storage duration — it is initialized once and persists for the lifetime of the program. Inside a class, `static` makes the member belong to the class itself rather than to any instance — there is one copy shared by all instances. These are three completely separate language features that happen to share a keyword for historical reasons inherited from C.

**Trap:** Conflating `static` in a class (class-scope) with `static` at file scope (linkage). A `static` class member has no linkage implications — it has external linkage by default, same as any other function or variable.

**Follow-up:** "What is the modern replacement for file-scope `static`?" — An anonymous namespace: `namespace { void helper() {} }`. It achieves internal linkage for any entity (functions, variables, types), not just functions and variables. It is preferred in modern C++ because it is more explicit and applies uniformly to all entity kinds.

---

**Q6: What is name mangling and when does it matter?**

**A:** Name mangling is the process by which the C++ compiler encodes type information — parameter types, return types, namespaces, class membership — into a function's symbol name in the object file. It exists because the linker operates on symbol names and has no knowledge of C++ types, yet C++ supports function overloading (multiple functions with the same name but different parameter types). Mangling makes `add(int, int)` and `add(double, double)` produce different symbol names (`_Z3addii` and `_Z3adddd` under the Itanium ABI) so the linker can distinguish them. It matters when interfacing with C code (which has no mangling), when reading linker error messages (which show mangled names), and when writing code that must be called from C (use `extern "C"`).

**Trap:** "Mangling is compiler-specific, so you can't mix GCC and Clang object files." Actually, on Linux both GCC and Clang implement the Itanium C++ ABI, which includes a standardized mangling scheme. They produce compatible object files. On Windows, MSVC uses a different ABI and mangling scheme, so mixing is not generally safe.

**Follow-up:** "How do you decode a mangled name?" — `c++filt _Z3addii` outputs `add(int, int)`. In a core dump or linker error, pipe symbol names through `c++filt` to make them readable.

---

**Q7: What is ABI compatibility and why should you care?**

**A:** ABI (Application Binary Interface) compatibility means two pieces of compiled binary code can be linked and run together correctly without recompiling either. ABI covers symbol names (mangling), calling convention (which registers carry arguments), struct layout (member offsets and sizeof), and vtable layout (which slot corresponds to which virtual function). If you ship a shared library and a user links against it, they rely on your ABI not changing. Adding a virtual function to a public class changes the vtable layout — existing user code, which was compiled with the old vtable, will call the wrong functions. This is a silent runtime bug, not a compile error.

**Trap:** "I can add new methods without breaking ABI." True for non-virtual methods. False for virtual methods. Adding any virtual function changes the vtable, which breaks ABI for any code that was compiled against the old header.

**Follow-up:** "How do you provide a stable ABI for a shared library?" — The pimpl idiom: the public class holds a `unique_ptr<Impl>` where `Impl` is defined only in the `.cpp` file. Users never see `Impl`'s layout, so you can add members, change implementation details, and add virtual functions to `Impl` without breaking the ABI of the public class.

---

**Q8: What is the difference between `inline` and `static` for free functions?**

**A:** Both `inline` and file-scope `static` prevent linker errors when a function is defined in a header included by multiple translation units, but they do so through different mechanisms with different semantics. `static` gives the function internal linkage — each translation unit gets its own private copy of the function, and the linker never sees the name. `inline` gives the function external linkage but tells the linker that multiple identical definitions are expected and should be merged into one. The practical difference: with `static`, each translation unit has its own copy (code size multiplies), and a `static` function-local variable inside a `static` function is also per-translation-unit. With `inline`, there is conceptually one function (the linker merges them), and function-local statics are shared across all call sites. For most header-defined utility functions, `inline` is correct. For helper functions that should genuinely be private to each translation unit (unusual), `static` or an anonymous namespace is appropriate.

**Trap:** "`inline` is a hint to the compiler to inline the function at call sites." This was true in C++98. In modern C++ with LTO and compiler heuristics, `inline` has essentially no effect on whether the compiler inlines the function. Its only guaranteed effect is the ODR relaxation.

**Follow-up:** "When would you explicitly mark a function `inline` in a source file (not a header)?" — Rarely. You might use it to expose a function for cross-TU inlining without LTO, but in practice the compiler makes its own inlining decisions. The main use of `inline` in source files is `inline` variables (C++17) to define class static data members in headers without a separate `.cpp` definition.
