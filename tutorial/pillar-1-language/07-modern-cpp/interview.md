# Chapter 07: Interview Q&A — Modern C++ Standards

Eight questions covering C++11 through C++23, each with a common trap and a follow-up.

---

## Q1: What is the single most important addition in C++11?

**Strong answer:** Move semantics and rvalue references. Before C++11 there was no way to
transfer ownership of a heap resource without copying it. Returning a `vector<unique_ptr<T>>`
from a function required either deep-copying every element (expensive) or returning via output
parameter (awkward). Move semantics solved this at zero runtime cost: the move constructor
swaps internal pointers rather than copying data.

**Why it underpins everything:**
- `unique_ptr` *requires* move semantics — it cannot be copied.
- Standard containers became O(1) to transfer ownership.
- Algorithms that return containers (e.g. `std::sort`, `std::move` with iterators) became
  practical.

**The trap:** Answering "lambdas" or "auto". Both are important but neither changes what the
language *can express*. Lambdas are syntactic sugar for function objects (you could write
them manually before C++11). Move semantics enable code that was *impossible* before.

**Follow-up:** "What is `std::move` and does it actually move anything?"

No. `std::move` is an unconditional cast to an rvalue reference. It does not move any
data. It enables the *selection* of move constructor/assignment over copy. The actual
transfer of resources happens in the move constructor you wrote (or the compiler generated).

---

## Q2: Explain lambda capture. When is capture-by-value safe?

**Strong answer:** A lambda is a compiler-generated class with `operator()`. The capture
list determines which local variables become data members of that class.

```cpp
int x = 10;
auto f = [x](){};    // [x] — x is copied into the lambda's member at the capture point
auto g = [&x](){};   // [&x] — lambda stores a reference to x (same address as local x)
```

**The trap:** "Capture by value is always safe." It is NOT. Consider:

```cpp
class MyClass {
    int value_ = 42;
    std::function<int()> make_getter() {
        return [=]() { return value_; };  // TRAP: captures 'this', not value_
    }
};
```

`[=]` in a member function captures `this` by value (the pointer), not `value_` by value.
The lambda stores a copy of the pointer. If the `MyClass` object is destroyed before the
lambda is called, the lambda reads through a dangling pointer — undefined behaviour.

Capture-by-reference is also dangerous if the lambda is stored and the referred variable
goes out of scope.

**Safe rules:**
- For short-lived lambdas (passed to algorithms, immediately called): either capture is fine.
- For stored lambdas or lambdas passed to other threads: capture by value, and explicitly
  capture `*this` (C++17: `[*this]`) not `this`.

**Follow-up:** "What is a mutable lambda?"

By default, captured-by-value members are `const` inside `operator()`. `mutable` removes
that:

```cpp
auto counter = [n = 0]() mutable { return ++n; };  // n is modifiable per-call
```

---

## Q3: Where are C++17 structured bindings most useful? Do they create copies?

**Strong answer:** Structured bindings shine in two places:

1. **Range-for over maps:** `for (auto& [k, v] : map)` is dramatically more readable than
   `for (auto& kv : map)` followed by `kv.first`/`kv.second`.

2. **Functions returning multiple values via pair/tuple:**
   ```cpp
   auto [it, inserted] = my_set.insert(value);
   if (!inserted) { /* already present */ }
   ```

**The trap:** "Structured bindings create copies."

It depends on the form:
- `auto  [k, v] = *it` — binds to a *copy* of the map node (the pair is copied).
- `auto& [k, v] = *it` — binds by reference to the map node's `key` and `value`.
- `const auto& [k, v] = *it` — binds by const reference.

In practice, always use `auto&` or `const auto&` in range-for to avoid unnecessary copies.

**Follow-up:** "Can structured bindings decompose custom types?"

Yes. For a struct with all public non-static members, structured bindings work automatically
(aggregate decomposition). For non-aggregates you implement `get<N>()`, specialize
`std::tuple_size`, and `std::tuple_element`.

---

## Q4: What is the difference between `if constexpr` and a regular `if`?

**Strong answer:** `if constexpr` evaluates the condition at *compile time* using a constant
expression. The *discarded* branch is not instantiated for the current template arguments —
it does not need to be well-formed for the types involved.

```cpp
template<typename T>
void describe(T val) {
    if constexpr (std::is_integral_v<T>) {
        printf("int: %d\n", static_cast<int>(val));  // operator<< not required
    } else {
        printf("other\n");
        // val.integer_method();  // would be ill-formed for T=float,
                                  // but is NOT instantiated — compiles fine
    }
}
```

**The trap:** "The discarded branch never executes at runtime."

A regular `if` with a compile-time-constant condition also "never executes" at runtime
(the optimiser eliminates it). The critical difference is that with a regular `if`, *both*
branches must be *well-formed* (syntactically valid and type-correct) for every instantiation.
With `if constexpr`, the discarded branch is NOT instantiated — it may contain code that
would be a type error for the current `T`.

**Follow-up:** "When should you use `if constexpr` vs concepts/`std::enable_if`?"

- `if constexpr` inside a function body: when the function *logic* differs by type.
- Concepts / `requires`: when you want different *overloads* selected by type.
- Never use `std::enable_if` in new C++20 code — it is obsoleted by concepts.

---

## Q5: How do C++20 views differ from algorithms? Do views copy data?

**Strong answer:** Views are *lazy* — they produce elements on demand when iterated. They
do not allocate memory and do not process elements until you actually iterate. Algorithms
are *eager* — `std::sort` sorts the entire range immediately.

```cpp
auto pipeline = v
    | std::views::filter([](int x){ return x % 2 == 0; })
    | std::views::transform([](int x){ return x * x; });

// NO computation has happened yet.
for (int x : pipeline) { /* each x is computed here, one at a time */ }
```

Composing `filter | transform | take(5)` on a 1-million-element vector processes at most
5 elements from the source — stops as soon as `take(5)` is satisfied.

**The trap:** "Views copy data."

Views store only a *reference* (or iterator pair) to the underlying range plus any
captured predicates. They do not copy the source data. However, there are subtle rules:

- Mutable views (e.g. `views::filter`) cache the first element — iterating twice may
  miss changes made to the source range.
- `views::transform` returns by value from its transform function — elements are
  *computed* not *stored*, so iterating twice computes them twice.

**Follow-up:** "When should you *not* use a ranges pipeline?"

When you need to iterate the result multiple times and the transform is expensive (cache
a materialised vector instead). Also when performance is critical and the pipeline
indirection measurably hurts (profile first).

---

## Q6: What is CTAD and what are its limitations?

**Strong answer:** CTAD (Class Template Argument Deduction) was added in C++17. It lets
the compiler deduce template arguments for class templates from constructor arguments —
the same way function templates deduce from call arguments.

```cpp
std::pair p{1, 2.0};          // deduced: pair<int, double>
std::vector v{1, 2, 3};       // deduced: vector<int>
std::lock_guard lk{my_mutex}; // deduced: lock_guard<std::mutex>
```

**The trap:** "CTAD works exactly like function template deduction."

It does not. Function template deduction uses the function parameter list. CTAD uses
*deduction guides* — either compiler-generated ones (from constructor signatures) or
user-defined ones. The compiler-generated guides have restrictions:

- They are not generated for inherited constructors.
- They are not generated from `std::initializer_list` constructors in the way you
  might expect.
- `std::vector{1, 2, 3}` works because of a deduction guide for `initializer_list`.

Without a deduction guide, CTAD may not work or may deduce incorrectly.

**Follow-up:** "Write a custom deduction guide."

```cpp
template<typename T>
struct Wrapper { T value; Wrapper(T v) : value(v) {} };

// Without this, Wrapper{42} works (compiler generates it).
// With non-trivial constructors you write explicit guides:
template<typename T>
Wrapper(T) -> Wrapper<T>;  // deduction guide
```

---

## Q7: What are the advantages of C++20 modules over headers? Can you just rename `.h` to `.cppm`?

**Strong answer:** Modules provide:

1. **No macro leakage.** `#define` in a module interface does not affect importers. Headers
   pollute every translation unit that includes them.
2. **Faster builds.** The module interface is compiled once to a Binary Module Interface
   (`.gcm`, `.ifc`). Importers read the BMI — no re-parsing of headers.
3. **Proper encapsulation.** Only `export`ed names are visible to importers. Non-exported
   functions have internal linkage by default.
4. **No include-order dependencies.** Modules are order-independent.

**The trap:** "Rename `.h` to `.cppm` and you have a module."

Completely wrong. Modules require:
- An explicit `export module <name>;` declaration at the top.
- Explicit `export` on every name you want visible to importers.
- Build system support (CMake 3.28+, Ninja 1.11+, MSVC 17.5+).
- All dependencies to also be modules (or wrapped in `module;` global module fragment).
- Macros you rely on from headers must be moved to the global module fragment.

Headers included in a module interface do NOT automatically become part of the module —
they pollute the module's namespace but their symbols may or may not be exported.

**Follow-up:** "Should you convert all code to modules today (2024)?"

Probably not in production yet. Build system support is still maturing, CMake/Ninja
integration has rough edges, and mixing modules with headers requires careful handling
of the global module fragment. Monitor CMake 3.30+ for stable support.

---

## Q8: Which C++23 features would you mention in a 2026 interview?

**Strong answer:**

1. **`std::expected<T,E>`** — Monadic error handling without exceptions. Composable with
   `.and_then()`, `.transform()`, `.or_else()`. The Rust-inspired approach to typed errors.
   This is the answer to "how do I return errors without exceptions" that C++ lacked.

2. **Deducing `this`** — Enables recursive lambdas cleanly and eliminates the need for
   CRTP in some patterns (e.g. fluent builder with base class methods).

3. **`views::enumerate` and `views::zip`** — The two most practically useful range
   additions. Every Python developer expects these.

4. **`std::mdspan`** — Zero-cost multi-dimensional view. Critical for scientific/numerical
   computing without needing Eigen or Boost.MultiArray.

**The trap:** "I'd mention `std::format` as a C++23 feature."

`std::format` is **C++20**, not C++23. It appeared in C++20 but requires GCC 13+ (not
available on GCC 11). In C++23, `std::print`/`std::println` were added as convenience
wrappers. Confusing `std::format` with C++23 signals a lack of precision on standards
history — exactly the kind of detail that distinguishes prepared from unprepared candidates.

**Follow-up:** "When is `std::expected` better than exceptions?"

- When errors are *expected* (hence the name) and part of the normal control flow.
- In performance-critical paths where exception table overhead matters.
- In `noexcept` code (embedded, kernel modules, high-frequency trading).
- When you want the error type to be visible in the function signature.
