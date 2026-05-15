# Templates and Concepts (Gentle Introduction)

**Templates** let you write one piece of code that works for many types. They are the main reason C++ can be **fast** (monomorphization — the compiler generates specialized code per type) and also why error messages can be **long**.

## 1. Function templates

```cpp
template<typename T>
T max_value(T a, T b) {
    return (a < b) ? b : a;
}

int x = max_value(1, 2);           // T is int
double y = max_value(1.5, 2.5);    // T is double
```

The compiler **deduces** `T` from arguments when possible.

---

## 2. Class templates

```cpp
template<typename T>
struct Box {
    T value;
};

Box<int> b{42};
```

`std::vector<int>` is a class template instance.

---

## 3. SFINAE (say “ess-if-in-ay”) — intuition only

**Substitution Failure Is Not An Error:** when the compiler tries a template and some type requirement fails, it **discards** that overload instead of erroring — unless nothing matches.

You will see this in advanced metaprogramming (`std::enable_if`, `requires` clauses). Beginners: know the **name** and that it explains weird overload resolution.

---

## 4. Concepts (C++20) — “compile-time type requirements”

Instead of cryptic errors deep inside a template body, you state constraints:

```cpp
#include <concepts>

template<std::integral T>
T double_it(T x) { return x * 2; }

// double_it(3);      // OK
// double_it(3.14);  // clearer error: not integral
```

**Mental model:** concepts are **named requirements** (`integral`, `totally_ordered`, custom ones you write).

**Connect:** `projects/02-foundation/include/foundation/templates/concepts.hpp`.

---

## 5. Variadic templates — one sentence

Templates that accept **any number** of type parameters; C++17 **fold expressions** help expand them.

**Connect:** `variadic.hpp` in foundation.

---

## 6. Policy-based design — cartoon version

A class template parameter chooses **behavior** (sorting policy, allocator) without virtual calls.

```cpp
template<typename SortPolicy>
struct Sorter {
    template<typename It>
    void sort(It first, It last) {
        SortPolicy::sort(first, last);
    }
};
```

**Connect:** `policy.hpp` — includes expression-template examples; read slowly.

---

## 7. When to use templates (practical)

| Situation | Template? |
|-----------|-----------|
| Same algorithm, many numeric types | Yes |
| Containers / generic utilities | Yes |
| Plugin-like runtime swapping | Often **virtuals** or **type erasure** instead |

---

## 8. Best practices

1. **Start concrete**, then templatize when you have **two real use cases**.
2. **Concepts** to constrain inputs early.
3. **Keep template definitions** in headers (or explicit instantiations in `.cpp` for controlled builds).
4. Read errors **from the first line** that mentions *your* code — ignore the middle noise at first.

## Connect to this repo

- `projects/02-foundation/docs/templates.md`
- `demo_templates.cpp`

---

*Next:* [06-patterns-concurrency-and-modern-types.md](06-patterns-concurrency-and-modern-types.md)
