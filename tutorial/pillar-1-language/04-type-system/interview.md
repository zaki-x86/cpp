# Type System — Interview Q&A

> Eight Q&A pairs. Full answers, common traps, senior follow-ups.

---

**Q1: What is a strong typedef and why use one over a plain type alias?**

**A:** A type alias (`using Meters = double`) creates a new name for the same type — the compiler treats `Meters` and `Seconds` as identical because both are `double`. Passing a `Seconds` where `Meters` is expected compiles without error. A strong typedef wraps the underlying type in a struct, creating a genuinely distinct type that requires explicit construction. `Meters{5.0}` cannot be passed where `Seconds` is expected — the compiler enforces the distinction at zero runtime cost. This prevents entire categories of unit-confusion bugs that a type alias cannot catch.

**Trap:** "A `typedef` or `using` alias is good enough for safety." Not for distinct domain types. `using UserId = int` and `using PostId = int` are the same type — you can pass one where the other is expected with no diagnostic.

**Follow-up:** How do you give a strong type arithmetic operators without repeating the implementation? CRTP mixins: `template<typename D> struct Addable { D operator+(const D& o) const { return D{static_cast<const D*>(this)->value() + o.value()}; } };`. The strong type inherits from `Addable<Self>` to get `+` for free.

---

**Q2: When do you use `std::optional` vs a raw pointer?**

**A:** Use `std::optional<T>` for values that may not exist but are not independently heap-allocated — the classic "not found," "not set," or "might fail silently" cases in return values. Use a raw pointer when the pointee has independent lifetime (it is allocated elsewhere and you are observing it) or when you need pointer arithmetic or polymorphism. `optional` makes optionality explicit in the type: callers must check `has_value()` or dereference with `value()` (which throws on empty). A nullable pointer requires discipline and relies on the caller remembering to check `nullptr`.

**Trap:** "I use a pointer and check for `nullptr`." The caller may forget to check. With `optional`, the type signature announces the value may be absent; with a pointer, the interface contract must be communicated through documentation, which can be ignored.

**Follow-up:** Can `optional<T&>` hold a reference? No — the standard does not allow `optional` to hold references. Use `std::reference_wrapper<T>` (`std::ref(x)`) or a raw pointer (`T*`) for optional references.

---

**Q3: What is `std::variant` and how is it different from a C `union`?**

**A:** `std::variant<Ts...>` is a type-safe discriminated union. It tracks which alternative is active (the discriminant), runs the correct constructor and destructor when switching, and throws `std::bad_variant_access` (or UB without assertions) when you access the wrong alternative. `std::visit` provides exhaustive dispatch — if you add an alternative and forget to update the visitor, it is a compile error. A C `union` provides none of this: you must manually track the active member, the compiler does not call constructors or destructors, and accessing the wrong member is silent undefined behavior.

**Trap:** "Variant is just a tagged union implementation detail." The key distinction is `std::visit` exhaustiveness. A `switch` on a discriminant enum compiles even if you miss a case. `std::visit` with `overloaded` refuses to compile unless all alternatives are handled.

**Follow-up:** What is `std::monostate` and when would you use it? `std::monostate` is a trivially default-constructible, comparable type used as the first alternative in a variant when no other alternative is default-constructible. `std::variant<std::monostate, NonDefault>` can be default-constructed in the `monostate` state, then transitioned to `NonDefault` when data is available.

---

**Q4: What is the `overloaded` pattern for `std::visit`?**

**A:** `overloaded<Fs...>` is a struct that inherits from multiple callable types and exposes all their `operator()` overloads: `struct overloaded : Fs... { using Fs::operator()...; }`. With a deduction guide, you can construct it from lambdas: `overloaded{[](int i){...}, [](double d){...}}`. When passed to `std::visit`, overload resolution selects the correct lambda based on the active alternative's type. If any alternative is unhandled, overload resolution fails — compile error. This is the standard idiom for exhaustive inline dispatch on variants.

**Trap:** Forgetting to handle all variant alternatives — this gives a compile error with `overloaded`, which is exactly the desired behavior. The alternative, `if constexpr` chains, can miss cases silently.

**Follow-up:** What if you want a default case for unhandled types? Add a catch-all at the end: `overloaded{[](int i){...}, [](auto&&){}}` — the generic lambda `auto&&` matches anything that the more specific overloads do not.

---

**Q5: What is `std::expected` (or a manual equivalent) and how does monadic error handling work?**

**A:** `Expected<T, E>` (C++23's `std::expected`) holds either a success value `T` or an error `E` — an error-aware alternative to `std::optional`. Monadic chaining: `and_then(f)` applies `f` to the success value and propagates errors unchanged, `transform(f)` maps the success value wrapping the result, `or_else(f)` handles the error case. A chain like `parse(s).and_then(validate).and_then(process)` propagates the first error without explicit `if`-checks at each step. This is composable error handling that avoids both exceptions (for performance/no-exception environments) and explicit error-code checks at every call site.

**Trap:** "This is just a fancier error code return." Monadic chaining is the difference. Without it, every step needs `if (result.is_error()) return result.error();`. With `and_then`, the plumbing disappears and the happy path reads linearly.

**Follow-up:** When would you use `expected` over exceptions? Performance-critical code (exceptions add binary overhead and have unpredictable throw/catch cost), no-exception environments (embedded, `-fno-exceptions`), and when errors are frequent and expected (parsing user input), not truly exceptional.

---

**Q6: What is a phantom type and what problem does it solve?**

**A:** A phantom type is a template parameter that appears in the type tag but not in the data representation. `Handle<Open>` and `Handle<Closed>` have identical binary layouts but are different types — functions requiring `Handle<Open>` refuse `Handle<Closed>` at compile time. State transitions are expressed as functions returning a different phantom instantiation: `Handle<Closed> close(Handle<Open>)`. The C++ type system enforces the state machine transitions. Incorrect sequences (calling `read()` on a closed handle, closing an already-closed handle) are compile errors, not runtime errors.

**Trap:** "I would just use a runtime bool flag to track open/closed." A bool flag requires a runtime check; the program can still call `read()` with the flag set to `false` — the compiler will not stop it. Phantom types make the incorrect state impossible to construct.

**Follow-up:** How do you implement state transitions with phantom types? Return a different phantom instantiation from the transition function: `Handle<Closed> close_file(Handle<Open> h)`. The old `Handle<Open>` is consumed (either by passing by value or by design), and the new `Handle<Closed>` is the only valid handle after the transition.

---

**Q7: What are user-defined literals and how do you implement one?**

**A:** User-defined literals attach a type to a literal value: `1.5_km` creates a `Kilometers{1.5}`. Implement with `operator""_suffix(...)`. The parameter type must be one of: `unsigned long long` (for integer literals), `long double` (for floating-point literals), `const char*` or `const char*, size_t` (for string literals), or `char` (for character literals). The suffix must start with `_` — suffixes without `_` are reserved for the standard library. User-defined literals are zero-overhead: they are just function calls resolved at compile time.

**Trap:** "Literal operators can take any type." Only the four allowed parameter types. No `int`, no `double` — you must use `long double` for floating-point and cast down inside the operator if needed.

**Follow-up:** What is a raw literal operator and when is it useful? `operator""_bin(const char* str)` receives the raw character sequence of the literal before any numeric parsing — `0b1010_bin` passes the string `"0b1010"`. Useful for implementing custom-base numeric literals, binary literals (pre-C++14), or custom encoding formats.

---

**Q8: How does the spaceship operator (`<=>`) work?**

**A:** `auto operator<=>(const T&) const = default` generates all six comparison operators by performing lexicographic member-wise three-way comparison. The return type determines the comparison category: `std::strong_ordering` (equal implies substitutable), `std::weak_ordering` (equivalent does not imply substitutable, e.g., case-insensitive comparison), `std::partial_ordering` (some values unordered, e.g., NaN). In C++20, if you define `<=>`, the compiler also generates `operator==` via rewriting — `a == b` becomes `(a <=> b) == 0` unless you provide `==` explicitly. Defaulted `<=>` gives you all comparison operators for free with correct semantics for well-behaved types.

**Trap:** "Defaulting `<=>` automatically gives me `==`." In C++20, it does via expression rewriting. But if you define a non-defaulted `<=>`, you still need to define `==` explicitly (or also default `==`). The rewriting rules for `==` prioritize any explicitly declared `==` over the rewritten form.

**Follow-up:** What is the difference between `strong_ordering` and `weak_ordering`? `strong_ordering::equal` guarantees the two values are substitutable — any operation on them produces the same result. `weak_ordering::equivalent` only guarantees the ordering is consistent, not substitutability. Example: case-insensitive string comparison — "Hello" and "hello" are `equivalent` in weak ordering but not `equal` (they differ when case matters).
