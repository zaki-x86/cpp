# Chapter 07: Core — Standards Summary Tables

> One page per standard. Five most impactful features each. Memorise these and you can
> hold an intelligent conversation about any era of C++ code.

---

## C++11 — The Modern C++ Foundation

C++11 was the rebirth of C++. The committee acknowledged that the language had become
too painful and made sweeping changes. Everything after C++11 builds on this foundation.

| Feature | Why It Matters |
|---------|----------------|
| **Move semantics + rvalue references** | Eliminates copies of expensive resources — `vector` of `unique_ptr` becomes practical |
| **`auto` type deduction** | Reduces boilerplate and prevents type errors in generic code |
| **Lambda expressions** | First-class functions — algorithms become usable without separate functor classes |
| **`nullptr`** | Replaces `NULL` (which is `0`) — eliminates ambiguity between pointer and integer overloads |
| **`std::thread` + `std::mutex`** | First standardized portable threading — no more platform-specific API calls |

**Honourable mentions:** `unique_ptr`/`shared_ptr`, `constexpr`, range-based for, initializer lists,
variadic templates, `enum class`, `static_assert`, `noexcept`, `std::atomic`.

---

## C++14 — Polishing C++11

C++14 is a small but important release. It fixed the rough edges in C++11 that the
committee couldn't squeeze in before the deadline. Treat it as "C++11 done right."

| Feature | Why It Matters |
|---------|----------------|
| **Generic lambdas (`auto` params)** | Lambdas become templates — one lambda works for any type |
| **`make_unique<T>()`** | The missing C++11 companion to `make_shared` — now the preferred way to create `unique_ptr` |
| **`std::exchange`** | Atomic get-and-set for move operations — simplifies move constructors |
| **`constexpr` relaxation** | `constexpr` functions can now contain loops and local variables |
| **Return type deduction** | `auto` return type deduced from `return` statements — useful in generic code |

**Honourable mentions:** variable templates, binary literals (`0b1010`), digit separators (`1'000'000`),
`decltype(auto)`, `std::integer_sequence`.

---

## C++17 — Mainstream Features

C++17 landed the features that make C++ readable at a glance. Structured bindings and
`std::optional` alone changed how experienced developers write everyday code.

| Feature | Why It Matters |
|---------|----------------|
| **Structured bindings** | `auto [key, val] = *it` — range-for over maps becomes readable |
| **`if constexpr`** | Replaces SFINAE for conditional code paths — readable, maintainable |
| **`std::optional<T>`** | Explicit nullable values — no more sentinel values or output parameters |
| **`std::variant<Ts...>`** | Type-safe discriminated union — replaces tagged unions |
| **`std::string_view`** | Non-owning string reference — zero-copy string passing |

**Honourable mentions:** guaranteed copy elision (mandatory RVO), fold expressions, CTAD,
`std::any`, parallel algorithms, `std::filesystem`, `std::byte`, `[[nodiscard]]`.

---

## C++20 — The Biggest Revision Since C++11

C++20 delivered four headline features (Concepts, Ranges, Coroutines, Modules) plus dozens
of smaller but significant improvements. It is the most ambitious C++ release ever shipped.

| Feature | Why It Matters |
|---------|----------------|
| **Concepts** | Named constraints on templates — readable error messages and subsumption-based overloading |
| **Ranges + Views** | Lazy composable algorithms — `views::filter \| views::transform` replaces loops |
| **Coroutines** | Language-level suspend/resume — enables async I/O and generators without callbacks |
| **`std::jthread`** | RAII threads with cooperative cancellation — no more forgetting to join |
| **Modules** | Replaces `#include` — faster build times and no macro leakage across translation units |

**Honourable mentions:** three-way comparison (`<=>`), `consteval`/`constinit`, `std::span<T>`,
`std::latch`/`std::barrier`/`counting_semaphore`, designated initializers, `std::bit_cast<T>`,
abbreviated function templates.

> **Compiler note:** `std::format` is part of C++20 but requires GCC 13+. On GCC 11.4 use
> `printf`/`ostringstream` instead.

---

## C++23 — Incremental Quality of Life

C++23 is smaller in scope than C++20 but refines the features that landed rough. The
committee focused on filling gaps (monadic `optional`/`expected`) and range additions.

| Feature | Why It Matters |
|---------|----------------|
| **`std::expected<T,E>`** | Monadic error handling without exceptions — composable, typed error values |
| **`std::format`-era: `std::print`/`std::println`** | Direct-to-stdout formatting without constructing a string |
| **`std::flat_map`** | Sorted vector-backed map — cache-friendly for read-heavy maps |
| **Deducing `this` (explicit object parameter)** | Enables CRTP-style patterns without CRTP — simpler recursive lambdas |
| **`std::mdspan`** | Multi-dimensional array view — zero-cost abstraction over contiguous memory |

**Honourable mentions:** `std::generator<T>`, `std::stacktrace`, `views::zip`, `views::enumerate`,
`views::chunk`, `if consteval`, `[[assume(expr)]]`, monadic `optional` (`.and_then`, `.transform`).

> **Compiler note:** GCC 11.4 does not support C++23 features. They are described in
> [deep-dive.md](deep-dive.md) but the examples only use C++20.

---

## Production Rules

These rules apply when writing new C++ code in 2024+:

1. **Write C++20 by default.** Concepts, ranges, `jthread`, `span` — use them.
2. **`make_unique` over `new`.** Never write `new` outside of placement new or custom allocators.
3. **`[[nodiscard]]` on anything that returns an error or resource.** Always.
4. **Prefer `std::string_view` for read-only string parameters.** Never `const std::string&` for a sink parameter.
5. **Prefer structured bindings over `.first`/`.second`.** `auto [k, v]` is readable; `it->second` is archaeology.
6. **`if constexpr` over `std::enable_if`.** The former is readable. The latter is not.
7. **Use `std::variant` + `std::visit` for discriminated unions.** Never raw tagged `union`.
8. **Know your `constexpr` vs `consteval` distinction.** `constexpr` *may* run at compile time; `consteval` *must*.
9. **`std::optional` for nullable return values.** No `nullptr`, no `-1`, no output parameters.
10. **Ranges pipelines over hand-written loops** where clarity wins and performance is not critical.

---

## Lab

| Project | Features Exercised |
|---------|--------------------|
| [01-toolchain](../../../projects/01-toolchain/) | `constexpr`, `auto`, lambdas, modern CMake |
| [02-foundation](../../../projects/02-foundation/) | Concepts, ranges, `span`, variadic templates, smart pointers |
