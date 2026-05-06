# Templates — Deep Dive

> Reference-grade. Two-phase lookup, SFINAE mechanics, specialization, variadic templates, expression templates.

---

## Two-Phase Name Lookup

Templates in C++ are parsed in two phases:

**Phase 1 — at template definition:** Non-dependent names (names that do not depend on any template parameter) are looked up immediately. If the name is not visible at the template definition point, it is an error — you cannot fix it by adding a declaration later in the translation unit.

**Phase 2 — at instantiation:** Dependent names (names that depend on a template parameter `T`) are looked up when the template is instantiated with a concrete type. The lookup searches the template definition context plus argument-dependent lookup (ADL) in the namespaces of the template arguments.

```cpp
template<typename T>
struct Derived : Base<T> {
    void foo() {
        bar();          // Phase 1: non-dependent. Error if bar() not visible here.
        this->bar();    // Phase 2: dependent (through this which depends on T).
                        // Finds Base<T>::bar at instantiation time.
    }
};
```

The `this->` idiom makes a name dependent, deferring lookup to phase 2 where the base class specialization is visible. Without it, base-class names in templates are invisible.

---

## Dependent Names and typename/template Disambiguators

When a name inside a template depends on a template parameter, the compiler cannot know whether it is a type, a value, or a template until the parameter is known. The standard requires explicit disambiguation:

```cpp
template<typename T>
void example() {
    T::iterator it;                          // Error: is iterator a type or a value?
    typename T::iterator it;                 // OK: explicitly a type

    T::template make<int>();                 // Error: is make a template?
    T::template make<int>();                 // OK: explicitly a template

    typename T::template rebind<int>::type;  // combined: type + template disambiguator
}
```

The rule: use `typename` before any dependent qualified name that names a type. Use `template` before any dependent name that is used as a template. Omitting either is a compile error on conforming compilers (GCC 11 enforces this).

---

## Full and Partial Specialization

**Full specialization** provides a specific implementation for a complete set of template arguments:

```cpp
template<typename T>
class TypeName { static constexpr const char* value = "unknown"; };

template<>
class TypeName<int> { static constexpr const char* value = "int"; };

template<>
class TypeName<double> { static constexpr const char* value = "double"; };
```

**Partial specialization** provides an implementation for a pattern of arguments, leaving some still templated:

```cpp
template<typename T>
class IsPointer { static constexpr bool value = false; };

template<typename T>
class IsPointer<T*> { static constexpr bool value = true; };  // matches any pointer type
```

Function templates cannot be partially specialized — only class and variable templates can. For function templates, use overloading to achieve similar effects.

**Explicit instantiation** forces the compiler to generate a template instance in a specific `.cpp` file:

```cpp
// sorter.cpp
template class Sorter<int>;     // explicit instantiation
template void sort<double>(double*, double*);
```

Explicit instantiation reduces link time in large codebases by ensuring each template is instantiated once, in a known translation unit, rather than redundantly in every translation unit that uses it.

---

## Variadic Templates: Recursive vs Fold Expressions

**Recursive unpacking** (pre-C++17 style):

```cpp
// Base case: zero arguments
void print() {}

// Recursive case: print head, recurse on tail
template<typename T, typename... Rest>
void print(T head, Rest... rest) {
    std::cout << head;
    if constexpr (sizeof...(rest) > 0) std::cout << ", ";
    print(rest...);
}
```

This generates N+1 function instantiations for N arguments. For large N, this inflates compile time and binary size.

**Fold expressions (C++17):** Apply a binary operator to a parameter pack in one expression:

```cpp
template<typename... Ts>
auto sum(Ts... args) { return (args + ...); }   // right fold: a + (b + (c + ...))

template<typename... Ts>
auto sum(Ts... args) { return (... + args); }   // left fold:  ((a + b) + c) + ...

// With initial value (binary fold):
template<typename... Ts>
auto sum(Ts... args) { return (0 + ... + args); }  // left fold with init
```

Prefer fold expressions for arithmetic, logical operations, and comma-operator packs. Use recursion only when you need per-element processing with stateful logic that fold expressions cannot express.

---

## SFINAE: enable_if and void_t

SFINAE — Substitution Failure Is Not An Error — is the mechanism by which template overloads are silently removed from the candidate set when their type expressions are invalid:

```cpp
// Overload 1: enabled only when T is arithmetic
template<typename T>
std::enable_if_t<std::is_arithmetic_v<T>, std::string>
describe(T x) { return "arithmetic: " + std::to_string(x); }

// Overload 2: enabled when T is NOT arithmetic
template<typename T>
std::enable_if_t<!std::is_arithmetic_v<T>, std::string>
describe(T) { return "non-arithmetic"; }
```

`std::enable_if_t<B, T>` is `T` when `B` is true. When `B` is false, `::type` does not exist — substitution failure removes the overload.

`std::void_t<Expr...>` (C++17) is always `void` if all expressions are valid, and substitution failure otherwise. Used to detect member existence:

```cpp
template<typename T, typename = void>
struct has_size : std::false_type {};

template<typename T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>>
    : std::true_type {};
```

Both are superseded by C++20 concepts in new code, but appear throughout all pre-C++20 library code.

---

## Tag Dispatch

Tag dispatch uses overloaded functions with a dummy tag argument to select algorithm variants based on type properties, without virtual dispatch or SFINAE:

```cpp
struct slow_tag {};
struct fast_tag {};

template<typename T>
using algo_tag = std::conditional_t<std::is_arithmetic_v<T>, fast_tag, slow_tag>;

template<typename T>
void process_impl(T x, fast_tag) { /* SIMD path */ }

template<typename T>
void process_impl(T x, slow_tag) { /* generic path */ }

template<typename T>
void process(T x) { process_impl(x, algo_tag<T>{}); }
```

The tag carries zero runtime information — it is always optimized away. The selection happens entirely at compile time through overload resolution. The standard library uses this for iterator category dispatch (`std::random_access_iterator_tag`, `std::forward_iterator_tag`, etc.).

---

## Type Lists

A TypeList carries a sequence of types as a compile-time value:

```cpp
template<typename... Ts> struct TypeList {};

using List = TypeList<int, double, char>;
```

Operations implemented via specialization:

```cpp
// Head: first element
template<typename L> struct Head;
template<typename T, typename... Ts>
struct Head<TypeList<T, Ts...>> { using type = T; };

// Size: element count
template<typename L> struct Size;
template<typename... Ts>
struct Size<TypeList<Ts...>> { static constexpr size_t value = sizeof...(Ts); };

// At: N-th element
template<size_t N, typename L> struct At;
template<size_t N, typename T, typename... Ts>
struct At<N, TypeList<T, Ts...>> : At<N-1, TypeList<Ts...>> {};
template<typename T, typename... Ts>
struct At<0, TypeList<T, Ts...>> { using type = T; };
```

TypeLists are the compile-time equivalent of `std::vector<std::type_info>`. They are used in tuple implementations, type-safe heterogeneous containers, and static dispatch tables.

---

## constexpr if vs SFINAE

`if constexpr` discards the false branch at compile time — not just optimizes it away, but removes it from the abstract machine entirely. The discarded branch does not need to be well-formed for the current type:

```cpp
template<typename T>
void serialize(T val) {
    if constexpr (std::is_arithmetic_v<T>) {
        write_binary(val);                    // only valid for arithmetic types
    } else if constexpr (has_to_string<T>::value) {
        write_string(val.to_string());        // only valid when to_string() exists
    } else {
        static_assert(false, "T is not serializable");
    }
}
```

With regular `if`, both branches must compile for any `T`. With `if constexpr`, only the taken branch is compiled.

Limitation: `if constexpr` works only inside function templates. For conditional class members or class structures, SFINAE or explicit specialization is still required.

---

## Policy-Based Design

A policy class is a small type that provides a specific behavior as a static function or type alias, passed as a template parameter to a host class. The host class delegates to the policy at compile time — full inlining, zero virtual overhead:

```cpp
struct AscendingPolicy  { template<typename T> static bool cmp(T a, T b) { return a < b; } };
struct DescendingPolicy { template<typename T> static bool cmp(T a, T b) { return a > b; } };

template<typename SortPolicy>
class Sorter {
public:
    template<typename It>
    void sort(It first, It last) {
        std::sort(first, last, [](const auto& a, const auto& b){
            return SortPolicy::cmp(a, b);
        });
    }
};
```

`Sorter<AscendingPolicy>` and `Sorter<DescendingPolicy>` are separate types. Each can be inlined completely — no function pointer, no virtual call. Policies compose: `Sorter<Cached<AscendingPolicy>>` wraps the policy in a caching layer without changing `Sorter`.

---

## Expression Templates: Zero-Cost Arithmetic

Expression templates replace intermediate temporaries in arithmetic expressions with a lazy evaluation tree:

```
Naive:       a = b + c + d
  Step 1:    tmp1 = b + c   (allocates N floats)
  Step 2:    tmp2 = tmp1 + d (allocates N floats)
  Step 3:    a = tmp2        (copies N floats)

Expression template:
  a[i] = b[i] + c[i] + d[i]  for each i  (no temporaries, one pass)
```

The `+` operator returns a proxy type that stores references to its operands. Assignment evaluates the entire expression tree element-by-element:

```cpp
template<typename L, typename R>
struct VecAdd {
    const L& l; const R& r;
    float operator[](size_t i) const { return l[i] + r[i]; }
    size_t size() const { return l.size(); }
};

template<typename L, typename R>
VecAdd<L,R> operator+(const L& l, const R& r) { return {l, r}; }
```

The expression `b + c + d` has type `VecAdd<VecAdd<Vec,Vec>, Vec>` — no allocation. Assignment evaluates it lazily. This pattern is the foundation of Eigen, Blaze, and every high-performance C++ numerical library.

Danger: the proxy stores references. If you store the expression object and the operands go out of scope, you have dangling references. Expression templates must be evaluated immediately on assignment.

---

## Concept Subsumption Rules

A concept A **subsumes** concept B if every type satisfying A also satisfies B. In overload resolution, a more constrained template is preferred over a less constrained one:

```cpp
template<typename T>
concept Comparable = requires(T a, T b) { { a < b } -> std::convertible_to<bool>; };

template<typename T>
concept Sortable = Comparable<T> && requires(T c) { c.begin(); c.end(); c.size(); };

template<Comparable T> void process(T x) { /* version 1 */ }
template<Sortable T>   void process(T x) { /* version 2 */ }
```

When `process` is called with a type satisfying `Sortable`, version 2 is preferred because `Sortable` subsumes `Comparable`. Subsumption only applies to **atomic constraints** that are identical expression-wise — semantically equivalent but syntactically different constraints are treated as unrelated.
