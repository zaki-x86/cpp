# Templates — Core

> The mental model. Read this before anything else in this chapter.

---

## Templates as Code Generation

When you write a template, you are writing a pattern that the compiler fills in for each type you use it with. This happens entirely at compile time — the resulting binary contains specialized machine code for each instantiation, identical to what you would write by hand.

```cpp
template<typename T>
T max(T a, T b) { return a < b ? b : a; }

max(3, 5);        // compiler generates: int max(int, int)
max(3.0, 5.0);    // compiler generates: double max(double, double)
```

Both generated functions are in the binary. Each is as efficient as a hand-written version. The template itself generates no code — it is a blueprint, not an object.

This is zero-cost abstraction: the abstraction (the template) exists only at compile time. At runtime, you pay only for the specific types you use.

---

## The Three Kinds of Templates

**Function templates:** Generate functions for specific type arguments. Argument deduction lets you call without explicit `<>` in most cases.

```cpp
template<typename T>
void swap(T& a, T& b) { T tmp = std::move(a); a = std::move(b); b = std::move(tmp); }
```

**Class templates:** Generate classes for specific type arguments. The type argument is part of the class's identity — `Stack<int>` and `Stack<double>` are unrelated types.

```cpp
template<typename T>
class Stack {
    std::vector<T> data_;
public:
    void push(T val) { data_.push_back(std::move(val)); }
    T pop() { T v = std::move(data_.back()); data_.pop_back(); return v; }
    bool empty() const { return data_.empty(); }
};
```

**Variable templates (C++14):** A template that generates a variable (typically a compile-time constant) for each type.

```cpp
template<typename T>
constexpr T pi = T(3.14159265358979323846);

double area = pi<double> * r * r;
float  area = pi<float>  * r * r;
```

---

## Concepts: SFINAE for Humans

Before C++20, the way to constrain templates was SFINAE — Substitution Failure Is Not An Error. It worked, but the code was unreadable and error messages were incomprehensible walls of template noise. C++20 concepts are the replacement.

```cpp
// Pre-C++20: SFINAE with enable_if (do not write this for new code)
template<typename T,
         typename = std::enable_if_t<std::is_arithmetic_v<T>>>
T square(T x) { return x * x; }

// C++20: concepts (write this instead)
template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template<Arithmetic T>
T square(T x) { return x * x; }

// Or with abbreviated template syntax (cleaner):
Arithmetic auto square(Arithmetic auto x) { return x * x; }
```

When a concept requirement is violated, the error message names the concept: "constraint 'Arithmetic' not satisfied" instead of fifteen lines of `enable_if` substitution failure.

Write your own concepts when a type must support a specific set of operations:

```cpp
template<typename T>
concept Sortable = requires(T& c) {
    { c.begin() } -> std::forward_iterator;
    { c.end()   } -> std::forward_iterator;
    { c.size()  } -> std::convertible_to<std::size_t>;
};
```

The `requires` expression tests that each operation is well-formed for type `T`. If any operation fails, `Sortable<T>` is false.

---

## When to Template vs When to Use virtual

**Use templates when:**
- The type is known at compile time
- Performance is critical (template = no virtual overhead, full inlining)
- You want value semantics (no heap allocation for polymorphism)
- You are implementing generic algorithms (sort, find, transform)

**Use virtual when:**
- You need a heterogeneous collection (`vector<Animal*>`)
- The concrete type is not known until runtime (plugins, factory patterns)
- You want a stable binary ABI (shared library)
- The polymorphism is at architectural boundaries, not hot inner loops

When in doubt, start with virtual. It is simpler, produces better error messages, and is easier to test. Optimize to templates when profiling shows the virtual overhead matters.

---

## Production Rules

- Keep template definitions in headers (`.hpp` or `.h`) — the compiler needs the definition to instantiate.
- Use `static_assert(condition, "message")` for early, readable errors instead of cryptic SFINAE failures.
- Prefer `if constexpr(condition)` over SFINAE for conditional logic inside function bodies.
- Write a named `concept` before writing a bare `requires` expression — names make the intent clear.
- Prefer fold expressions over recursive variadic templates — clearer and faster to compile.
- Use `typename` and `template` disambiguators when dependent names are types or templates.

---

## Lab

Atlas's `projects/02-foundation/include/foundation/templates/` implements:
- `type_traits.hpp` — `TypeList<Ts...>`, `has_size<T>`, SFINAE detection idioms
- `variadic.hpp` — fold expression demos
- `concepts.hpp` — C++20 concept constraints, `consteval`
- `policy.hpp` — `Sorter<Policy>` and `VecAdd<L,R>` expression templates

Run: `ctest --preset debug -R test_templates` from `projects/02-foundation/`.
