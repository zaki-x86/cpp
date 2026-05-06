# C++ Bible — Phase 5b: Cheatsheets + Interview Master

> **For agentic workers:** Use superpowers:subagent-driven-development or superpowers:executing-plans. Steps use `- [ ]` checkbox syntax.

**Compiler:** GCC 11.4.0, `g++ -std=c++17`. No `std::expected`, no `std::format`.
**Author voice:** Forge — direct, mentor-tone, real-world grounded.

---

## Task 1: Cheatsheets

### Step 1.1 — Create directory

- [ ] `mkdir -p tutorial/cheatsheets`

---

### Step 1.2 — Write `memory-management.md`

- [ ] Create `tutorial/cheatsheets/memory-management.md` with the following content:

````markdown
# Memory Management Cheatsheet

*Forge's quick reference — know this cold before any systems interview.*

---

## RAII — The Golden Rule

> Acquire in constructor. Release in destructor. Never call `delete` in user code.

```cpp
class FileHandle {
    FILE* fp_;
public:
    explicit FileHandle(const char* path, const char* mode)
        : fp_(fopen(path, mode)) {
        if (!fp_) throw std::runtime_error("cannot open file");
    }
    ~FileHandle() { if (fp_) fclose(fp_); }  // always runs, even on exception
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;
    FILE* get() const { return fp_; }
};
```

The destructor runs whether the scope exits normally or via exception. That's the magic.

---

## Smart Pointer Comparison

| | `unique_ptr<T>` | `shared_ptr<T>` | `weak_ptr<T>` |
|---|---|---|---|
| Ownership | Sole | Shared (ref-counted) | Non-owning observer |
| Overhead | Zero (same as raw ptr) | ~2× (control block, atomic ref count) | None beyond `shared_ptr` |
| Copyable | No — move only | Yes | Yes (from `weak_ptr` or `shared_ptr`) |
| Thread-safe ref count | N/A | Yes (atomic) | Checked via `lock()` |
| Use when | Default choice | Shared ownership needed | Breaking cycles / observer |

### Smart Pointer Syntax

```cpp
// unique_ptr
auto p = std::make_unique<MyClass>(arg1, arg2);
auto raw = p.get();           // borrow raw pointer — p still owns
auto raw = p.release();       // transfer ownership to caller — p becomes null
p.reset();                    // destroy managed object now

// shared_ptr
auto sp = std::make_shared<MyClass>(arg1, arg2);  // one allocation (control block + T)
std::shared_ptr<MyClass> sp2 = sp;                // ref count: 2
sp2.use_count();                                  // 2

// weak_ptr
std::weak_ptr<MyClass> wp = sp;
if (auto locked = wp.lock()) {  // returns shared_ptr or null — never throws
    locked->doSomething();
}

// shared_from_this — T must inherit enable_shared_from_this<T>
struct Node : std::enable_shared_from_this<Node> {
    std::shared_ptr<Node> self() { return shared_from_this(); }
};
// TRAP: only valid if object is already managed by a shared_ptr
```

---

## When to Use Raw Pointers

Raw pointers are not evil — they just don't own anything.

```cpp
// OK: non-owning reference into already-managed storage
void process(const MyClass* obj) { /* observes, does not delete */ }

// OK: C API interop
int result = some_c_api(ptr.get());

// OK: intrusive data structures (e.g., linked list with external lifetime)
struct Node { Node* next; };  // pool manages lifetime

// NOT OK:
delete rawPtr;  // if rawPtr came from make_unique, you have a double-free
```

---

## Allocator Types

| Type | Best for | Alloc cost | Free cost | Fragmentation |
|---|---|---|---|---|
| Global heap (`malloc`/`new`) | General purpose | O(1) amortized | O(1) amortized | Yes |
| Arena / monotonic buffer | Bulk alloc, bulk free (frame data) | O(1) bump ptr | O(1) reset() | None |
| Pool (fixed-size blocks) | Many same-size objects (nodes, tasks) | O(1) free-list pop | O(1) free-list push | None |
| Stack (alloca) | Very short-lived small allocations | ~zero (SP adjust) | ~zero | None |

### PMR (Polymorphic Memory Resources — C++17)

```cpp
#include <memory_resource>

// Stack-backed arena — zero heap allocation if fits
alignas(std::max_align_t) std::byte buffer[4096];
std::pmr::monotonic_buffer_resource pool(buffer, sizeof(buffer));

std::pmr::vector<int> v(&pool);   // vector's internal alloc uses pool
std::pmr::string s("hello", &pool);

// pool.release() resets — all objects become invalid
// pool goes out of scope → stack memory freed automatically
```

---

## Placement New

```cpp
// Separate allocation from construction
alignas(MyClass) std::byte storage[sizeof(MyClass)];

MyClass* obj = new(storage) MyClass(arg1, arg2);  // constructs at &storage[0]

// MUST manually destroy — delete is wrong (storage is not heap)
obj->~MyClass();

// Typical use: pool allocator, variant-like storage, in-place construction
```

---

## Memory Layout (Process Address Space)

```
High address
+------------------+
|      Stack       |  grows DOWN — local variables, return addresses
+--------+---------+
         | (unmapped gap — stack overflow lands here)
+--------+---------+
|       Heap       |  grows UP — malloc/new
+------------------+
|   BSS segment    |  zero-initialized global/static variables
+------------------+
|   Data segment   |  initialized global/static variables
+------------------+
|   Text segment   |  executable code (read-only)
+------------------+
Low address
```

Key implication: stack and heap grow toward each other. Stack overflow = unmapped page between them → SIGSEGV.
````

---

### Step 1.3 — Write `oop-idioms.md`

- [ ] Create `tutorial/cheatsheets/oop-idioms.md` with the following content:

````markdown
# OOP Idioms Cheatsheet

*Forge's OOP reference — know which rule applies before touching copy/move.*

---

## Rule of 0 / 3 / 5 — Decision Tree

```
Does the class directly manage a resource?
(raw ptr, file descriptor, mutex, socket, mmapped region)
    |
   NO ──→ Rule of 0: let compiler generate everything.
          Use unique_ptr/shared_ptr to own resources.
    |
   YES
    |
    └──→ Can you wrap it in unique_ptr or another RAII type?
              |
             YES ──→ Do so, then apply Rule of 0.
              |
             NO ──→ Rule of 5: declare all five.
```

Rule of 3 (pre-C++11): if you need destructor, copy ctor, or copy assign → you need all three.
Rule of 5 (C++11+): add move ctor and move assign.

---

## Rule of 5 Skeleton

```cpp
class Buffer {
    std::byte* data_;
    std::size_t size_;
public:
    // Constructor
    explicit Buffer(std::size_t n)
        : data_(new std::byte[n]), size_(n) {}

    // Destructor
    ~Buffer() { delete[] data_; }

    // Copy constructor
    Buffer(const Buffer& other)
        : data_(new std::byte[other.size_]), size_(other.size_) {
        std::memcpy(data_, other.data_, size_);
    }

    // Copy assignment — implemented via copy-and-swap
    Buffer& operator=(Buffer other) {  // takes by value (copy ctor runs)
        swap(*this, other);
        return *this;
    }

    // Move constructor
    Buffer(Buffer&& other) noexcept
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    // Move assignment
    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    friend void swap(Buffer& a, Buffer& b) noexcept {
        using std::swap;
        swap(a.data_, b.data_);
        swap(a.size_, b.size_);
    }
};
```

---

## Copy-and-Swap Idiom

```cpp
// One assignment operator handles both copy and move,
// is exception-safe, and self-assignment safe.

Buffer& operator=(Buffer other) {  // 'other' is a copy (or moved-into) instance
    swap(*this, other);            // swap internals
    return *this;
    // 'other' destructor releases what *this used to hold
}

// Why exception-safe: if copy ctor of 'other' throws, *this is unchanged.
// Why self-assignment safe: a=a → copies 'a' into 'other', swaps, releases copy.
```

---

## Move Semantics

```cpp
std::string a = "hello";
std::string b = std::move(a);  // 'a' is now in "valid but unspecified" state

// std::move does NOT move. It casts to rvalue reference (T&&).
// The move constructor/assign does the actual transfer.

// "Valid but unspecified": you can still call a.clear(), a.size(), destroy a.
// You cannot assume a.empty() is true — but in practice most STL types are.

// rvalue reference — binds to temporaries and std::move() results:
void consume(MyClass&& r);

// forwarding reference — binds to anything, preserves value category:
template<typename T>
void forward_to(T&& t) { consume(std::forward<T>(t)); }
```

---

## Virtual Dispatch vs CRTP

| | Virtual Dispatch | CRTP |
|---|---|---|
| Resolution time | Runtime | Compile time |
| Overhead | vtable ptr (8 bytes/object), indirect call (~2-10ns branch misprediction) | Zero — direct call inlined |
| Runtime polymorphism | Yes — heterogeneous containers possible | No — types must be known at compile time |
| Code size | One vtable per class | One instantiation per Derived type |
| Use when | Type not known at compile time, plugin systems | Performance-critical, mix-in behavior |

```cpp
// Virtual dispatch
struct Shape {
    virtual double area() const = 0;
    virtual ~Shape() = default;
};
struct Circle : Shape {
    double r;
    double area() const override { return 3.14159 * r * r; }
};

// CRTP — zero overhead
template<typename Derived>
struct Shape {
    double area() const {
        return static_cast<const Derived*>(this)->area_impl();
    }
};
struct Circle : Shape<Circle> {
    double r;
    double area_impl() const { return 3.14159 * r * r; }
};
```

---

## Non-Virtual Interface (NVI)

```cpp
// Public non-virtual method is the stable API contract.
// Private virtual method is the extension point.

class Logger {
public:
    void log(const std::string& msg) {  // non-virtual: controls pre/post
        if (!enabled_) return;
        timestamp_prefix(msg);          // common logic before
        do_log(msg);                    // virtual dispatch here
    }
private:
    virtual void do_log(const std::string& msg) = 0;  // derived classes customize this
    bool enabled_ = true;
};
```

Benefits: base class controls invariants, derived classes cannot bypass them.

---

## Virtual Inheritance (Diamond Problem)

```cpp
struct Animal { int age; };
struct Mammal : virtual Animal {};   // virtual base
struct WingedAnimal : virtual Animal {};
struct Bat : Mammal, WingedAnimal {
    // Only one 'age' subobject — shared by both bases
};

Bat b;
b.age = 5;  // unambiguous — virtual inheritance ensures single Animal subobject

// Cost: adds a vptr for the virtual base offset table.
// The layout uses an offset stored in the vtable to find Animal's subobject.
```

---

## Abstract Class Rules

```cpp
struct Interface {
    virtual void do_work() = 0;  // pure virtual — must override
    virtual ~Interface() = default;  // virtual destructor: ALWAYS on abstract classes

    // Even pure virtual functions CAN have a body:
    virtual void optional_hook() {}  // derived may override or call Interface::optional_hook()
};

// Pure virtual destructor: allowed, but must provide a body
struct AbstractBase {
    virtual ~AbstractBase() = 0;
};
AbstractBase::~AbstractBase() {}  // body required — derived dtors call it implicitly
```

**TRAP:** Deleting a derived object through a base pointer without a virtual destructor is undefined behavior. Always make destructors virtual in polymorphic hierarchies.

---

## Why You Cannot Call Virtual Functions in Constructors

During construction, the vtable pointer points to the *current class being constructed*, not the final derived class. The object is not yet fully constructed, so calling a virtual function would dispatch to the base's version — not the derived override. This is a frequent source of bugs in Java-style "call virtual in ctor" patterns.
````

---

### Step 1.4 — Write `templates-metaprogramming.md`

- [ ] Create `tutorial/cheatsheets/templates-metaprogramming.md` with the following content:

````markdown
# Templates & Metaprogramming Cheatsheet

*Forge's TMP reference — SFINAE, concepts, fold expressions, two-phase lookup.*

---

## SFINAE Quick Reference

> Substitution Failure Is Not An Error — an invalid substitution removes a candidate from overload set instead of causing a compile error.

```cpp
// Enable function only for integral types
template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
void f(T x) { /* only matches integers */ }

// Return type SFINAE
template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, T>
sqrt_safe(T x) { return std::sqrt(x); }

// With multiple constraints (conjunction)
template<typename T,
    typename = std::enable_if_t<
        std::is_integral_v<T> && !std::is_same_v<T, bool>
    >>
void integer_only(T x) {}
```

---

## void_t Pattern — Detecting Member Existence

```cpp
// Primary template — default to false
template<typename T, typename = void>
struct has_size : std::false_type {};

// Specialization — only valid when T has .size()
template<typename T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>>
    : std::true_type {};

// Usage
static_assert(has_size<std::vector<int>>::value);
static_assert(!has_size<int>::value);

// std::declval<T>() creates a T value in unevaluated context
// even if T has no default constructor
```

---

## C++20 Concepts Syntax (reference — GCC 10+)

```cpp
// Concept definition
template<typename T>
concept Hashable = requires(T a) {
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

// Three ways to constrain a function:
template<Hashable T>     void f1(T x);   // directly as type-constraint
void f2(Hashable auto x);               // abbreviated function template
template<typename T> requires Hashable<T>
void f3(T x);                           // requires clause

// Standard library concepts: std::integral, std::floating_point,
// std::same_as<T,U>, std::derived_from<D,B>, std::convertible_to<F,T>,
// std::invocable<F,Args...>, std::ranges::range<R>
```

---

## std::type_traits Quick Table

| Trait | Returns true if | Notes |
|---|---|---|
| `is_same<A,B>` | A and B are identical types | |
| `is_base_of<Base,Derived>` | Base is a base class of Derived | |
| `is_integral<T>` | T is int, char, bool, etc. | |
| `is_floating_point<T>` | T is float, double, long double | |
| `is_pointer<T>` | T is a pointer type | |
| `is_const<T>` | T is const-qualified | |
| `is_trivially_copyable<T>` | Safe for memcpy | |
| `is_standard_layout<T>` | Compatible with C structs | |
| `decay_t<T>` | Remove ref, const, array→ptr, fn→fptr | |
| `remove_reference_t<T>` | Strip & or && | |
| `add_pointer_t<T>` | T* | |
| `enable_if_t<cond,T>` | T if cond, else substitution failure | |
| `conditional_t<cond,A,B>` | A if cond, B otherwise | |
| `invoke_result_t<F,Args...>` | Return type of F(Args...) | |
| `common_type_t<A,B>` | Common arithmetic type of A,B | |

---

## Fold Expressions (C++17)

```cpp
// Unary left fold:  (... op args)   →  ((a0 op a1) op a2) ...
template<typename... Args>
auto sum_left(Args... args) { return (... + args); }

// Unary right fold: (args op ...)   →  a0 op (a1 op (a2 ...))
template<typename... Args>
auto sum_right(Args... args) { return (args + ...); }

// Binary left fold: (init op ... op args) — init is left-most
template<typename... Args>
auto sum_from_100(Args... args) { return (100 + ... + args); }

// Practical: print all args
template<typename... Args>
void print_all(Args&&... args) {
    ((std::cout << args << ' '), ...);  // comma fold
}

// Practical: all conditions must be true
template<typename... Bs>
bool all_true(Bs... bs) { return (... && bs); }
```

---

## TypeList Operations

```cpp
template<typename... Ts>
struct TypeList {
    static constexpr std::size_t size = sizeof...(Ts);
};

// type_at<List, I> — get Ith type
template<typename List, std::size_t I>
struct type_at;

template<typename T, typename... Ts>
struct type_at<TypeList<T, Ts...>, 0> {
    using type = T;
};

template<typename T, typename... Ts, std::size_t I>
struct type_at<TypeList<T, Ts...>, I> {
    using type = typename type_at<TypeList<Ts...>, I-1>::type;
};

template<typename List, std::size_t I>
using type_at_t = typename type_at<List, I>::type;

// contains<List, T>
template<typename List, typename T>
struct contains;

template<typename T>
struct contains<TypeList<>, T> : std::false_type {};

template<typename T, typename... Ts>
struct contains<TypeList<T, Ts...>, T> : std::true_type {};

template<typename U, typename T, typename... Ts>
struct contains<TypeList<U, Ts...>, T>
    : contains<TypeList<Ts...>, T> {};

// push_front
template<typename List, typename T>
struct push_front;

template<typename... Ts, typename T>
struct push_front<TypeList<Ts...>, T> {
    using type = TypeList<T, Ts...>;
};
```

---

## Two-Phase Lookup

In a template, names are looked up at two times:

1. **Definition time** — non-dependent names (no template parameters) are resolved immediately. Must be visible at definition.
2. **Instantiation time** — dependent names (depend on template params) are resolved at instantiation, using ADL + the instantiation context.

```cpp
template<typename T>
struct Derived : Base<T> {
    void foo() {
        bar();           // ERROR: non-dependent, looked up at definition time
                         // Base<T>::bar is dependent, not visible yet
        this->bar();     // OK: 'this->' makes it dependent
        Base<T>::bar();  // OK: explicit qualification
    }
};
```

---

## Template Argument Deduction & CTAD

```cpp
// CTAD — Class Template Argument Deduction (C++17)
std::pair p{1, 2.0};           // deduces pair<int, double>
std::vector v{1, 2, 3};        // deduces vector<int>
std::tuple t{1, 'a', 3.14};   // deduces tuple<int, char, double>

// Deduction guide (custom)
template<typename T>
struct Wrapper { T val; };

template<typename T>
Wrapper(T) -> Wrapper<T>;  // explicit deduction guide

Wrapper w{42};  // deduces Wrapper<int>
```

---

## Explicit Instantiation

```cpp
// In header (suppress implicit instantiation in all TUs):
extern template class MyContainer<int>;  // declaration

// In exactly one .cpp (force instantiation here):
template class MyContainer<int>;         // definition

// Why: reduces compile time and object file size when a template
// is used with the same arguments across many translation units.
```

---

## `if constexpr` — Compile-Time Branching

```cpp
template<typename T>
void serialize(T value) {
    if constexpr (std::is_integral_v<T>) {
        write_int(value);           // only compiled for integral T
    } else if constexpr (std::is_floating_point_v<T>) {
        write_float(value);         // only compiled for float T
    } else {
        static_assert(std::is_same_v<T,void>,
            "Unsupported type");    // always false — causes compile error
    }
}
// Unlike regular if: the discarded branch is not compiled.
// Allows branches that would be ill-formed for other T.
```
````

---

### Step 1.5 — Write `type-system.md`

- [ ] Create `tutorial/cheatsheets/type-system.md` with the following content:

````markdown
# Type System Cheatsheet

*Forge's guide to making the type system work for you — strong types, variants, optionals.*

---

## StrongType Pattern

Prevent implicit unit confusion at compile time:

```cpp
template<typename T, typename Tag>
struct StrongType {
    explicit StrongType(T v) : value(v) {}
    T value;
    // Spaceship operator gives ==, !=, <, >, <=, >= for free (C++20 / GCC 10+)
    // For C++17: manually define or use a helper base
    bool operator==(const StrongType& o) const { return value == o.value; }
    bool operator<(const StrongType& o) const { return value < o.value; }
};

using Meters  = StrongType<double, struct MetersTag>;
using Seconds = StrongType<double, struct SecondsTag>;

Meters  dist{100.0};
Seconds time{9.58};

// dist + time;  // COMPILE ERROR: different types — no operator+ defined
// double d = dist;  // COMPILE ERROR: explicit ctor, no implicit conversion

// Define arithmetic inside the strong type if needed:
template<typename T, typename Tag>
StrongType<T,Tag> operator+(StrongType<T,Tag> a, StrongType<T,Tag> b) {
    return StrongType<T,Tag>{a.value + b.value};
}
```

---

## std::variant

```cpp
#include <variant>

std::variant<int, double, std::string> v;
v = 42;                     // holds int
v = std::string("hello");   // holds string

// Access — throws std::bad_variant_access if wrong type
int i = std::get<int>(v);   // throws because v holds string

// Safe access — returns pointer or null
if (auto* s = std::get_if<std::string>(&v)) {
    std::cout << *s;
}

// Check which type is active
if (std::holds_alternative<std::string>(v)) { ... }

// Pattern: index-based access
std::cout << v.index();  // 2 (string is third alternative)
```

---

## Overloaded Visitor Pattern

```cpp
// Helper struct — inherits call operator from all lambdas
template<typename... Fs>
struct overloaded : Fs... {
    using Fs::operator()...;
};
template<typename... Fs>
overloaded(Fs...) -> overloaded<Fs...>;  // deduction guide (C++17)

// Usage with std::visit
std::variant<int, double, std::string> v = 3.14;
std::visit(overloaded{
    [](int i)               { std::cout << "int: " << i; },
    [](double d)            { std::cout << "double: " << d; },
    [](const std::string& s){ std::cout << "str: " << s; }
}, v);
```

---

## std::optional

```cpp
#include <optional>

std::optional<int> find_index(const std::vector<int>& v, int target) {
    for (int i = 0; i < (int)v.size(); ++i)
        if (v[i] == target) return i;
    return std::nullopt;  // empty optional
}

auto idx = find_index(vec, 42);

idx.has_value();        // false if empty
*idx;                   // UB if empty — no check
idx.value();            // throws std::bad_optional_access if empty
idx.value_or(-1);       // returns -1 if empty — safe

idx.emplace(99);        // construct in-place, replacing previous value
idx.reset();            // make empty
```

---

## Custom Expected<T,E> (C++17 — std::expected needs C++23)

```cpp
template<typename T, typename E>
class Expected {
    std::variant<T, E> data_;
public:
    Expected(T val) : data_(std::move(val)) {}
    static Expected error(E err) { Expected r; r.data_ = std::move(err); return r; }

    bool has_value() const { return std::holds_alternative<T>(data_); }
    T& value() { return std::get<T>(data_); }
    E& error() { return std::get<E>(data_); }

    // Monadic operations
    template<typename F>
    auto and_then(F&& f) {   // apply f to value if success, propagate error
        if (has_value()) return f(value());
        return Expected<typename std::invoke_result_t<F,T>::value_type, E>::error(error());
    }

    template<typename F>
    Expected or_else(F&& f) {  // apply f to error if failure, propagate success
        if (!has_value()) return f(error());
        return *this;
    }
};

// Usage:
Expected<int, std::string> parse_int(const std::string& s) {
    try { return std::stoi(s); }
    catch (...) { return Expected<int,std::string>::error("parse failed"); }
}
parse_int("42").and_then([](int x){ return x * 2; });
```

---

## User-Defined Literals (UDLs)

```cpp
// Must start with _ in user code (names without _ are reserved)
constexpr Meters operator""_m(long double d)  { return Meters{(double)d}; }
constexpr Meters operator""_km(long double d) { return Meters{(double)d * 1000.0}; }

auto dist = 5.2_km;   // Meters{5200.0}
auto dist2 = 100.0_m; // Meters{100.0}

// Integer UDL
constexpr long long operator""_KB(unsigned long long n) { return n * 1024; }
std::size_t buf_size = 64_KB;
```

---

## Phantom Types

```cpp
// Tags carry semantic meaning without runtime cost
struct Unvalidated {};
struct Validated {};

template<typename State>
struct UserInput {
    explicit UserInput(std::string s) : data(std::move(s)) {}
    std::string data;
};

using RawInput       = UserInput<Unvalidated>;
using VerifiedInput  = UserInput<Validated>;

VerifiedInput validate(RawInput raw) {
    if (raw.data.empty()) throw std::invalid_argument("empty");
    return VerifiedInput{raw.data};
}

void process(VerifiedInput input);  // only accepts validated input
// process(raw_input);  // COMPILE ERROR — wrong phantom type
```
````

---

### Step 1.6 — Write `design-patterns.md`

- [ ] Create `tutorial/cheatsheets/design-patterns.md` with the following content:

````markdown
# Design Patterns Cheatsheet

*Forge's C++ patterns — each with a working sketch. No GoF boilerplate, just the essential form.*

---

## Factory with Self-Registration

```cpp
// Each type registers itself — no central switch statement needed
using Creator = std::function<std::unique_ptr<Shape>()>;
std::map<std::string, Creator>& registry() {
    static std::map<std::string, Creator> r;
    return r;
}
template<typename T>
struct Registrar {
    explicit Registrar(const std::string& name) {
        registry()[name] = []() { return std::make_unique<T>(); };
    }
};
struct Circle : Shape {
    static inline Registrar<Circle> reg{"Circle"};  // C++17 inline
    double area() const override { return 3.14159 * r * r; }
    double r = 1.0;
};
// Usage: auto s = registry()["Circle"]();
```

---

## Observer / Event Emitter

```cpp
template<typename... Args>
class Signal {
    using Slot = std::function<void(Args...)>;
    std::vector<Slot> slots_;
public:
    void connect(Slot s) { slots_.push_back(std::move(s)); }
    void emit(Args... args) {
        for (auto& s : slots_) s(args...);
    }
};

// Usage:
Signal<int, std::string> on_event;
on_event.connect([](int code, std::string msg) { /* handler */ });
on_event.emit(200, "OK");
```

---

## Strategy Pattern (Policy Template)

```cpp
// Compile-time strategy via template parameter
struct AscendingSort {
    template<typename T>
    bool operator()(const T& a, const T& b) const { return a < b; }
};

template<typename Policy>
class Sorter {
    Policy policy_;
public:
    template<typename Iter>
    void sort(Iter first, Iter last) {
        std::sort(first, last, policy_);
    }
};

// Runtime strategy via std::function
class Logger {
    std::function<void(const std::string&)> sink_;
public:
    explicit Logger(std::function<void(const std::string&)> sink)
        : sink_(std::move(sink)) {}
    void log(const std::string& msg) { sink_(msg); }
};
```

---

## Decorator (Template Wrapping)

```cpp
template<typename Inner>
struct TimedDecorator : Inner {
    using Inner::Inner;  // inherit constructors
    void execute() {
        auto t0 = std::chrono::steady_clock::now();
        Inner::execute();
        auto dt = std::chrono::steady_clock::now() - t0;
        std::cout << "took " << dt.count() << " ns\n";
    }
};

struct NetworkFetch {
    void execute() { /* fetch data */ }
};

TimedDecorator<NetworkFetch> timed_fetch;
timed_fetch.execute();  // prints timing
```

---

## Command Pattern with Undo

```cpp
struct Command {
    std::function<void()> execute;
    std::function<void()> undo;
};

class CommandHistory {
    std::vector<Command> history_;
public:
    void run(Command cmd) {
        cmd.execute();
        history_.push_back(std::move(cmd));
    }
    void undo_last() {
        if (!history_.empty()) {
            history_.back().undo();
            history_.pop_back();
        }
    }
};
```

---

## Template Method (NVI Pattern)

```cpp
class DataProcessor {
public:
    // Template method — final algorithm, non-virtual
    void process(const Data& d) {
        validate(d);      // hook point
        do_transform(d);  // hook point
        emit_result(d);   // hook point
    }
private:
    virtual void validate(const Data&) {}    // optional override
    virtual void do_transform(const Data&) = 0;  // required override
    virtual void emit_result(const Data&) {}
};
```

---

## CRTP Mixin

```cpp
template<typename Derived>
struct Printable {
    void print() const {
        std::cout << static_cast<const Derived*>(this)->to_string() << '\n';
    }
};

template<typename Derived>
struct Comparable {
    bool operator!=(const Derived& o) const {
        return !(static_cast<const Derived*>(this)->operator==(o));
    }
};

struct Point : Printable<Point>, Comparable<Point> {
    int x, y;
    std::string to_string() const { return "(" + std::to_string(x) + "," + std::to_string(y) + ")"; }
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
};
```

---

## Type Erasure (AnyCallable)

```cpp
// Store any callable behind a stable interface — without virtual overhead per call
struct AnyCallable {
    void* obj = nullptr;
    void (*invoke)(void*, int) = nullptr;
    void (*destroy)(void*) = nullptr;

    template<typename F>
    explicit AnyCallable(F f) {
        auto* heap_f = new F(std::move(f));
        obj = heap_f;
        invoke  = [](void* p, int x){ (*static_cast<F*>(p))(x); };
        destroy = [](void* p){ delete static_cast<F*>(p); };
    }
    ~AnyCallable() { if (destroy) destroy(obj); }
    void operator()(int x) { invoke(obj, x); }
};
```

---

## Flat State Machine

```cpp
enum class State { Idle, Running, Paused, Stopped };
enum class Event { Start, Pause, Resume, Stop };

using Transition = std::function<State(State)>;
const std::size_t N_STATES = 4, N_EVENTS = 4;

// Transition table: table[state][event] → new state (or same if invalid)
std::array<std::array<State, N_EVENTS>, N_STATES> transitions = {{
    //                Start            Pause           Resume          Stop
    /* Idle    */ {{ State::Running,  State::Idle,    State::Idle,    State::Stopped }},
    /* Running */ {{ State::Running,  State::Paused,  State::Running, State::Stopped }},
    /* Paused  */ {{ State::Running,  State::Paused,  State::Running, State::Stopped }},
    /* Stopped */ {{ State::Stopped,  State::Stopped, State::Stopped, State::Stopped }},
}};

State current = State::Idle;
void dispatch(Event e) {
    current = transitions[(int)current][(int)e];
}
```

---

## Singleton (Meyer's — Thread-Safe Since C++11)

```cpp
class Config {
public:
    static Config& instance() {
        static Config cfg;  // initialized exactly once, thread-safe
        return cfg;
    }
    // Delete copy/move
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    std::string get(const std::string& key) const { /* ... */ }
private:
    Config() { /* load config */ }
};
// Usage: Config::instance().get("db.host");
```

**TRAP:** Singletons are global state. They make unit testing harder. Prefer dependency injection in testable code. Use Meyer's singleton only for truly global resources (logger, config, allocator).
````

---

### Step 1.7 — Write `concurrency.md`

- [ ] Create `tutorial/cheatsheets/concurrency.md` with the following content:

````markdown
# Concurrency Cheatsheet

*Forge's concurrency reference — memory model, atomics, patterns, and pitfalls.*

---

## Memory Order Table

| Order | Guarantee | Use case |
|---|---|---|
| `memory_order_relaxed` | Atomic operation, no ordering relative to other ops | Counters where only the final value matters |
| `memory_order_acquire` | No reads/writes after this can be reordered before it. Pairs with release stores. | Load a flag to "acquire" everything published before the release |
| `memory_order_release` | No reads/writes before this can be reordered after it. Pairs with acquire loads. | Store a flag to "release" all prior writes to another thread |
| `memory_order_acq_rel` | Both acquire and release semantics | Read-modify-write (fetch_add on shared counter) |
| `memory_order_seq_cst` | Total sequential consistency across all threads. Default. | When in doubt — most expensive but always correct |

---

## Mutex vs Atomic — Decision Tree

```
Is the shared data a single scalar (int, bool, ptr) that fits in one word?
    YES → std::atomic<T>
    NO  → mutex / shared_mutex

Is the operation a single read or write (no compound operations)?
    YES → atomic with appropriate ordering
    NO  → mutex (compound read-modify-write needs atomicity over multiple fields)

Are reads far more frequent than writes?
    YES → std::shared_mutex (reader-writer lock)
    NO  → std::mutex
```

---

## Happens-Before Relationship

Thread A:
```cpp
x = 1;                                    // (1)
flag.store(true, std::memory_order_release); // (2)
```

Thread B:
```cpp
while (!flag.load(std::memory_order_acquire)) {} // (3)
int val = x;                              // (4) — guaranteed to see x == 1
```

Rule: if (2) synchronizes-with (3) (B's acquire sees A's release), then everything A did before (2) happens-before everything B does after (3). So (4) sees the value written at (1).

**Without the acquire/release:** the compiler or CPU could reorder (1) after (2), or B could read a stale cached x. Undefined behavior.

---

## False Sharing

```cpp
// BAD: head and tail share a cache line (both in same 64-byte chunk)
struct Queue {
    std::atomic<std::size_t> head;  // producer reads, consumer writes
    std::atomic<std::size_t> tail;  // producer writes, consumer reads
};
// Every write to tail invalidates consumer's cached head — cache ping-pong

// GOOD: each in its own cache line
struct Queue {
    alignas(64) std::atomic<std::size_t> head;
    alignas(64) std::atomic<std::size_t> tail;
};
// No false sharing — independent cache lines
```

Detection: `perf c2c` or Intel VTune HITM events show high "hit-in-modified" counts on the struct.

---

## SPSC Lock-Free Queue Pattern

```cpp
// Single Producer Single Consumer — no locks needed
template<typename T, std::size_t N>
struct SPSCQueue {
    static_assert((N & (N-1)) == 0, "N must be power of 2");
    std::array<T, N> buf;
    alignas(64) std::atomic<std::size_t> head{0};
    alignas(64) std::atomic<std::size_t> tail{0};

    bool push(T val) {
        std::size_t t = tail.load(std::memory_order_relaxed);
        std::size_t next = (t + 1) & (N - 1);
        if (next == head.load(std::memory_order_acquire)) return false; // full
        buf[t] = std::move(val);
        tail.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T& val) {
        std::size_t h = head.load(std::memory_order_relaxed);
        if (h == tail.load(std::memory_order_acquire)) return false; // empty
        val = std::move(buf[h]);
        head.store((h + 1) & (N - 1), std::memory_order_release);
        return true;
    }
};
```

---

## Thread Pool Pattern

```cpp
class ThreadPool {
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mu_;
    std::condition_variable cv_;
    bool stop_ = false;

public:
    explicit ThreadPool(std::size_t n) {
        for (std::size_t i = 0; i < n; ++i)
            workers_.emplace_back([this]{ worker_loop(); });
    }
    ~ThreadPool() {
        { std::lock_guard<std::mutex> lk(mu_); stop_ = true; }
        cv_.notify_all();
        for (auto& w : workers_) w.join();
    }
    template<typename F>
    std::future<std::invoke_result_t<F>> submit(F f) {
        auto task = std::make_shared<std::packaged_task<std::invoke_result_t<F>()>>(std::move(f));
        auto fut = task->get_future();
        { std::lock_guard<std::mutex> lk(mu_);
          tasks_.emplace([task]{ (*task)(); }); }
        cv_.notify_one();
        return fut;
    }
private:
    void worker_loop() {
        while (true) {
            std::function<void()> task;
            { std::unique_lock<std::mutex> lk(mu_);
              cv_.wait(lk, [this]{ return stop_ || !tasks_.empty(); });
              if (stop_ && tasks_.empty()) return;
              task = std::move(tasks_.front()); tasks_.pop(); }
            task();
        }
    }
};
```

---

## ABA Problem

```
Thread A: load pointer P → value X (address 0x100)
Thread B: pop X from stack, processes it, frees it
Thread B: allocates new Y — gets address 0x100 (same address, different object!)
Thread B: pushes Y to stack
Thread A: CAS(head, 0x100, A->next) — SUCCEEDS even though head object changed!
```

Result: stack corruption.

Fix options:
1. **Tagged pointer**: pack a version counter into high bits of the 64-bit pointer, increment on each push. CAS on 128-bit struct (ptr + version) via `cmpxchg16b`.
2. **Hazard pointers**: before dereferencing a pointer, announce it. Freeing thread checks all hazard pointers before reclaiming.
3. **Epoch-based reclamation**: threads declare their epoch; objects are only freed when all threads have advanced.

---

## Lock Types Comparison

| | `lock_guard` | `unique_lock` | `scoped_lock` |
|---|---|---|---|
| RAII | Yes | Yes | Yes |
| Unlockable mid-scope | No | Yes | No |
| Movable | No | Yes | No |
| Multi-mutex deadlock avoidance | No | Manual (std::lock) | Yes (C++17) |
| Use with condition_variable | No | Yes (required) | No |
| Overhead | Minimal | Tiny (flag for lock state) | Minimal |

```cpp
// unique_lock required for condition_variable
std::unique_lock<std::mutex> lk(mu);
cv.wait(lk, []{ return ready; });  // atomically releases lock and waits

// scoped_lock — acquire two mutexes without deadlock risk
std::scoped_lock<std::mutex, std::mutex> lk(mu_a, mu_b);
```
````

---

### Step 1.8 — Write `cmake-build.md`

- [ ] Create `tutorial/cheatsheets/cmake-build.md` with the following content:

````markdown
# CMake Build System Cheatsheet

*Forge's CMake reference — presets, targets, FetchContent, generator expressions.*

---

## CMakePresets.json Skeleton (version 3)

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "default",
      "generator": "Unix Makefiles",
      "binaryDir": "${sourceDir}/build/${presetName}",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_CXX_STANDARD": "17",
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
      }
    },
    {
      "name": "release",
      "inherits": "default",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "CMAKE_INTERPROCEDURAL_OPTIMIZATION": "ON"
      }
    },
    {
      "name": "asan",
      "inherits": "default",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_CXX_FLAGS": "-fsanitize=address -fno-omit-frame-pointer",
        "CMAKE_EXE_LINKER_FLAGS": "-fsanitize=address"
      }
    }
  ],
  "buildPresets": [
    { "name": "default",  "configurePreset": "default" },
    { "name": "release",  "configurePreset": "release" },
    { "name": "asan",     "configurePreset": "asan" }
  ],
  "testPresets": [
    { "name": "default", "configurePreset": "default", "output": { "verbosity": "verbose" } },
    { "name": "asan",    "configurePreset": "asan" }
  ]
}
```

---

## Essential CMake Commands

| Command | Purpose |
|---|---|
| `add_executable(name src1 src2)` | Create executable target |
| `add_library(name [STATIC\|SHARED\|INTERFACE] srcs)` | Create library target |
| `target_link_libraries(name [PUBLIC\|PRIVATE\|INTERFACE] deps)` | Link dependencies |
| `target_include_directories(name [PUBLIC\|...] dirs)` | Add include paths |
| `target_compile_options(name [PRIVATE\|...] flags)` | Add compiler flags |
| `target_compile_definitions(name [PRIVATE\|...] DEFS)` | Add preprocessor defines |
| `target_sources(name [PRIVATE\|...] srcs)` | Append sources to target |
| `set_target_properties(name PROPERTIES prop val)` | Set target properties |
| `add_custom_command(OUTPUT ... COMMAND ... DEPENDS ...)` | Run command during build |
| `add_custom_target(name COMMAND ... DEPENDS ...)` | Always-run custom target |
| `install(TARGETS name DESTINATION lib)` | Declare install rules |

**Visibility keywords:**
- `PRIVATE` — used by this target only
- `PUBLIC` — used by this target AND propagated to dependents
- `INTERFACE` — NOT used by this target, only propagated to dependents (header-only libs)

---

## find_package

```cmake
# Basic usage
find_package(OpenGL REQUIRED)
target_link_libraries(myapp PRIVATE OpenGL::GL)

# With components
find_package(Boost 1.74 REQUIRED COMPONENTS filesystem system)
target_link_libraries(myapp PRIVATE Boost::filesystem Boost::system)

# Optional
find_package(fmt QUIET)
if(fmt_FOUND)
    target_link_libraries(myapp PRIVATE fmt::fmt)
endif()
```

---

## FetchContent (C++17 CMake 3.14+)

```cmake
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)  # downloads and adds subdirectory

target_link_libraries(my_tests PRIVATE GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(my_tests DISCOVERY_MODE PRE_TEST)
```

---

## Generator Expressions

```cmake
# Configuration-based
target_compile_options(myapp PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Target file path
$<TARGET_FILE:myapp>         # full path to executable
$<TARGET_FILE_DIR:myapp>     # directory only

# Install vs build interface — critical for installed packages
target_include_directories(mylib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Negate
$<NOT:$<CONFIG:Release>>
```

---

## Common CMake Variables

| Variable | Purpose |
|---|---|
| `CMAKE_BUILD_TYPE` | Debug / Release / RelWithDebInfo / MinSizeRel |
| `CMAKE_INSTALL_PREFIX` | Install root (default: /usr/local) |
| `CMAKE_CXX_STANDARD` | 11, 14, 17, 20, 23 |
| `CMAKE_CXX_STANDARD_REQUIRED` | ON = error if standard not supported |
| `CMAKE_EXPORT_COMPILE_COMMANDS` | ON = generates compile_commands.json for clangd |
| `CMAKE_INTERPROCEDURAL_OPTIMIZATION` | ON = link-time optimization (LTO) |
| `BUILD_SHARED_LIBS` | ON = default to SHARED for add_library |
| `CMAKE_VERBOSE_MAKEFILE` | ON = show full compiler commands |

---

## Toolchain File (Cross-Compilation Skeleton)

```cmake
# toolchain-arm.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER   arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabihf)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

Use: `cmake -DCMAKE_TOOLCHAIN_FILE=toolchain-arm.cmake ..`

---

## Testing with GTest

```cmake
enable_testing()

add_executable(my_tests test_foo.cpp test_bar.cpp)
target_link_libraries(my_tests PRIVATE GTest::gtest_main mylib)

include(GoogleTest)
gtest_discover_tests(my_tests
    DISCOVERY_MODE PRE_TEST  # REQUIRED for sanitized builds — binary can't run at cmake time
    TEST_PREFIX "MyLib."
    PROPERTIES TIMEOUT 30
)
```

Run: `ctest --preset asan -V`
````

---

### Step 1.9 — Write `sanitizers-debugging.md`

- [ ] Create `tutorial/cheatsheets/sanitizers-debugging.md` with the following content:

````markdown
# Sanitizers & Debugging Cheatsheet

*Forge's debugging toolbox — catch bugs in CI, not production.*

---

## Sanitizer Flags Table

| Sanitizer | Flags | Catches |
|---|---|---|
| ASan (AddressSanitizer) | `-fsanitize=address -fno-omit-frame-pointer` | Heap use-after-free, buffer overflows, stack overflows, use-after-scope |
| UBSan (UndefinedBehaviorSanitizer) | `-fsanitize=undefined` | Integer overflow, null deref, misaligned access, out-of-bounds, invalid cast |
| TSan (ThreadSanitizer) | `-fsanitize=thread` | Data races, mutex order violations |
| LeakSan | `-fsanitize=leak` | Memory leaks (subset of ASan — ASan enables it too) |
| Combined | `-fsanitize=address,undefined` | ASan + UBSan together (common for CI) |

**Important:** do NOT link with `-O2` for ASan — use `-O1` or `-O0` with `-g` for best stack traces.

**WSL2 note:** TSan builds but **cannot execute** — throws `FATAL: ThreadSanitizer: unexpected memory mapping`. Use Docker or native Linux for TSan.

---

## Common ASan Error Messages

```
ERROR: AddressSanitizer: heap-use-after-free on address 0x... at pc ...
  READ of size 4 at 0x... thread T0
    #0 main /path/file.cpp:42
```

| Error | Meaning |
|---|---|
| `heap-use-after-free` | Accessed memory after `delete` |
| `heap-buffer-overflow` | Wrote/read past the end of heap allocation |
| `stack-buffer-overflow` | Wrote/read past the end of local array |
| `global-buffer-overflow` | Overflow on global array |
| `stack-use-after-scope` | Returned pointer to local variable |
| `detected memory leaks` | LeakSan triggered — shows allocation stack |
| `double-free` | Called `delete` twice on same pointer |

---

## GDB Command Reference

```bash
# Start and control
gdb ./myapp                 # start gdb
run [args]                  # run program
run < input.txt             # run with stdin redirect
attach <pid>                # attach to running process
kill                        # kill inferior

# Breakpoints
break file.cpp:42           # line breakpoint
break ClassName::method     # function breakpoint
break *0xdeadbeef           # address breakpoint
info breakpoints            # list all breakpoints
delete 2                    # delete breakpoint #2
condition 3 x > 5           # conditional breakpoint

# Stepping
next   (n)                  # step over (don't enter calls)
step   (s)                  # step into function calls
finish                      # run until current function returns
continue (c)                # run until next breakpoint

# Inspection
print expr          (p)     # print expression value
print *ptr                  # dereference pointer
info locals                 # all local variables
info args                   # function arguments
x/10xb 0xaddr              # examine 10 bytes in hex at address

# Stack
backtrace           (bt)    # show call stack
frame N                     # switch to frame N
info frame                  # current frame details

# Threads
info threads                # list all threads
thread N                    # switch to thread N
thread apply all bt         # backtrace all threads

# Watchpoints
watch var                   # stop when var changes
rwatch var                  # stop when var is read
awatch var                  # stop on read or write

# Signals / Exceptions
catch throw                 # stop on any thrown exception
catch throw std::runtime_error
handle SIGSEGV stop         # stop on signal

# TUI mode
layout src                  # show source code window
layout asm                  # show disassembly
```

---

## Valgrind

```bash
# Full memory check with origin tracking
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         ./myapp [args]

# Outputs:
# definitely lost: memory you leaked
# indirectly lost: memory pointed to by leaked pointers
# possibly lost: ambiguous (interior pointer)
# still reachable: memory held by globals at exit (usually OK)
```

**Note:** Valgrind is much slower (~10-50x) than ASan. Use ASan first; Valgrind for stubborn leaks.

---

## Core Dumps

```bash
# Enable core dumps
ulimit -c unlimited
# Or set in /etc/security/limits.conf

# Run crashing program — generates 'core' file
./myapp

# Debug with gdb
gdb ./myapp core

# In gdb after loading core:
bt                   # see where it crashed
info registers       # CPU registers at crash
frame N              # inspect frames
```

---

## Sanitizer + CMake Integration

```cmake
# In CMakePresets.json:
{
  "name": "asan",
  "cacheVariables": {
    "CMAKE_CXX_FLAGS": "-fsanitize=address -fno-omit-frame-pointer -g",
    "CMAKE_EXE_LINKER_FLAGS": "-fsanitize=address"
  }
}

# gtest_discover_tests: always use PRE_TEST
gtest_discover_tests(my_tests DISCOVERY_MODE PRE_TEST)
# Reason: sanitized binaries set up shadow memory at startup.
# Running them at cmake configure time (default) causes false positives.
```

---

## Quick Debugging Checklist

```
Crash / SIGSEGV:
  □ Run under ASan first
  □ If no ASan info: gdb + bt
  □ Check for null ptr dereference (print ptr in gdb)
  □ Check for array out-of-bounds

Memory leak:
  □ ASan with LSAN_OPTIONS=detect_leaks=1
  □ valgrind --leak-check=full for stubborn cases
  □ Check for containers holding shared_ptr cycles

Data race:
  □ TSan (on native Linux or Docker — not WSL2)
  □ Check all shared state accesses for mutex protection
  □ Check atomic ordering (use seq_cst to rule out ordering issues)

Undefined behavior:
  □ UBSan: -fsanitize=undefined
  □ Check integer overflow (-fsanitize=signed-integer-overflow)
  □ Check for uninitialized reads (valgrind --tool=memcheck)
```
````

---

### Step 1.10 — Write `systems-programming.md`

- [ ] Create `tutorial/cheatsheets/systems-programming.md` with the following content:

````markdown
# Systems Programming Cheatsheet

*Forge's POSIX reference — syscalls, signals, epoll, IPC. Know this for any systems role.*

---

## POSIX Syscall Quick Reference

### Process Management
| Syscall | Signature | Purpose |
|---|---|---|
| `fork` | `pid_t fork()` | Create child process — copy-on-write clone |
| `exec` family | `execvp(path, argv)` | Replace process image with new program |
| `waitpid` | `waitpid(pid, &status, flags)` | Wait for child; `WNOHANG` = non-blocking |
| `_exit` | `_exit(status)` | Exit without calling atexit/destructors |
| `getpid/getppid` | `pid_t getpid()` | Get own or parent process ID |
| `kill` | `kill(pid, sig)` | Send signal to process |
| `signal/sigaction` | `sigaction(signum, &act, &oldact)` | Install signal handler |

### Memory
| Syscall | Purpose |
|---|---|
| `mmap(addr, size, prot, flags, fd, offset)` | Map file or anonymous memory |
| `munmap(ptr, size)` | Unmap |
| `mprotect(ptr, size, prot)` | Change page permissions (`PROT_READ|PROT_WRITE|PROT_EXEC`) |

### File I/O
| Syscall | Purpose |
|---|---|
| `open(path, flags, mode)` | Open or create file. Common flags: `O_RDONLY`, `O_WRONLY`, `O_RDWR`, `O_CREAT`, `O_TRUNC`, `O_NONBLOCK`, `O_CLOEXEC` |
| `read(fd, buf, count)` | Read up to count bytes. Returns 0 on EOF, -1 on error. |
| `write(fd, buf, count)` | Write count bytes. May write fewer (partial write). |
| `close(fd)` | Release file descriptor |
| `lseek(fd, offset, whence)` | Move file offset (`SEEK_SET`, `SEEK_CUR`, `SEEK_END`) |
| `ioctl(fd, request, arg)` | Device-specific control |
| `fcntl(fd, cmd, arg)` | `F_SETFL, O_NONBLOCK` to set non-blocking; `F_SETFD, FD_CLOEXEC` |
| `stat/fstat` | Get file metadata (size, permissions, times) |

### I/O Multiplexing
| Syscall | Purpose |
|---|---|
| `pipe(fds[2])` | Create anonymous pipe — fds[0]=read, fds[1]=write |
| `dup(fd)` / `dup2(fd, fd2)` | Duplicate file descriptor |
| `select(nfds, readfds, writefds, errorfds, timeout)` | Wait for events — O(nfds) scan, max 1024 fds |
| `poll(fds, nfds, timeout)` | Wait for events — O(nfds) scan, no fd limit |
| `epoll_create1(flags)` | Create epoll instance |
| `epoll_ctl(epfd, op, fd, &event)` | Add/modify/remove fd (`EPOLL_CTL_ADD/MOD/DEL`) |
| `epoll_wait(epfd, events, maxevents, timeout)` | Wait — returns only ready fds, O(ready) |

### Sockets
| Syscall | Purpose |
|---|---|
| `socket(domain, type, proto)` | Create socket (`AF_INET, SOCK_STREAM, 0`) |
| `bind(fd, addr, addrlen)` | Bind to local address |
| `listen(fd, backlog)` | Mark as passive (server) |
| `accept(fd, addr, addrlen)` | Accept incoming connection |
| `connect(fd, addr, addrlen)` | Connect to server |
| `send(fd, buf, len, flags)` | Send data |
| `recv(fd, buf, len, flags)` | Receive data |
| `sendmsg/recvmsg` | Scatter-gather I/O, ancillary data |
| `socketpair(AF_UNIX, SOCK_STREAM, 0, fds)` | Bidirectional pipe |

### IPC
| Syscall | Purpose |
|---|---|
| `mq_open/mq_send/mq_receive` | POSIX message queues |
| `sem_open/sem_wait/sem_post` | Named semaphores |
| `shm_open + mmap` | Shared memory between processes |
| `pthread_create/join/mutex/cond` | POSIX threads |

---

## errno Quick Reference

| errno | Meaning | Common cause |
|---|---|---|
| `EINTR` | Interrupted by signal | Retry the syscall in a loop |
| `EAGAIN` / `EWOULDBLOCK` | No data ready (non-blocking fd) | Use select/poll/epoll before read/write |
| `ENOENT` | No such file or directory | Check path |
| `EACCES` | Permission denied | Check file permissions or socket port < 1024 |
| `EINVAL` | Invalid argument | Check argument ranges/types |
| `EBADF` | Bad file descriptor | fd was closed or never opened |
| `EPIPE` | Broken pipe | Reader end of pipe closed; also triggers SIGPIPE |
| `ECONNREFUSED` | Connection refused | Server not listening on port |
| `ETIMEDOUT` | Connection timed out | Network unreachable or firewall |
| `EADDRINUSE` | Address already in use | Use `SO_REUSEADDR` on server socket |

**Retry pattern for EINTR:**
```cpp
ssize_t n;
do {
    n = read(fd, buf, size);
} while (n == -1 && errno == EINTR);
```

---

## Signal Names

| Signal | Number | Default action | Catchable | Meaning |
|---|---|---|---|---|
| `SIGINT` | 2 | Terminate | Yes | Ctrl+C |
| `SIGTERM` | 15 | Terminate | Yes | Graceful shutdown request |
| `SIGKILL` | 9 | Terminate | **No** | Force kill |
| `SIGSEGV` | 11 | Core dump | Yes* | Segmentation fault |
| `SIGBUS` | 7 | Core dump | Yes* | Bus error (misaligned access, mmap truncation) |
| `SIGALRM` | 14 | Terminate | Yes | Timer (alarm()) |
| `SIGCHLD` | 17 | Ignore | Yes | Child process exited/stopped |
| `SIGHUP` | 1 | Terminate | Yes | Hangup — conventionally: reload config |
| `SIGUSR1` | 10 | Terminate | Yes | User-defined |
| `SIGUSR2` | 12 | Terminate | Yes | User-defined |
| `SIGPIPE` | 13 | Terminate | Yes | Write to closed pipe/socket — ignore with `signal(SIGPIPE, SIG_IGN)` |

*Catching SIGSEGV/SIGBUS is rarely useful — clean up and exit only.

---

## epoll Pattern (Edge-Triggered Non-Blocking Server)

```cpp
int epfd = epoll_create1(0);

// Add listening socket
struct epoll_event ev;
ev.events = EPOLLIN;
ev.data.fd = listen_fd;
epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

struct epoll_event events[64];
while (true) {
    int n = epoll_wait(epfd, events, 64, -1);  // -1 = block forever
    for (int i = 0; i < n; ++i) {
        if (events[i].data.fd == listen_fd) {
            int client = accept4(listen_fd, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
            ev.events = EPOLLIN | EPOLLET;  // edge-triggered
            ev.data.fd = client;
            epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev);
        } else {
            // read all available data (edge-triggered: read until EAGAIN)
            handle_client(events[i].data.fd);
        }
    }
}
```

Edge-triggered (`EPOLLET`): only notified on state change (new data arrives), not on level (data present). Must read until EAGAIN.

---

## Non-Blocking I/O

```cpp
// Set existing fd to non-blocking
int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);

// Or set at creation (socket):
int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

// Non-blocking read:
ssize_t n = read(fd, buf, size);
if (n == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // No data available — try again later
    } else {
        // Real error
    }
}
```

---

## O_CLOEXEC — Close on exec

```cpp
int fd = open("file.txt", O_RDONLY | O_CLOEXEC);
// fd is automatically closed when this process calls exec()
// Without O_CLOEXEC: child process inherits all open fds — security and resource leak

// For sockets:
int sock = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
// accept4 also takes SOCK_CLOEXEC flag
```
````

---

### Step 1.11 — Write `cuda-reference.md`

- [ ] Create `tutorial/cheatsheets/cuda-reference.md` with the following content:

````markdown
# CUDA Reference Cheatsheet

*Forge's CUDA quick reference — kernel launch, memory hierarchy, and the error-check macro you always need.*

---

## Kernel Launch Syntax

```cpp
// kernel<<<gridDim, blockDim, sharedMemBytes, stream>>>(args);

kernel<<<1024, 256>>>(d_data, n);
// 1024 blocks, 256 threads/block = 262144 threads total

kernel<<<dim3(32,32,1), dim3(16,16,1)>>>(d_image, width, height);
// 2D grid: 32x32 blocks, each 16x16 threads
```

---

## Index Math

```cpp
// 1D grid of 1D blocks
int i = blockIdx.x * blockDim.x + threadIdx.x;
// Stride loop (handles N > total threads):
for (int i = blockIdx.x * blockDim.x + threadIdx.x;
         i < N;
         i += gridDim.x * blockDim.x) { /* ... */ }

// 2D grid of 2D blocks (image processing)
int x = blockIdx.x * blockDim.x + threadIdx.x;  // column
int y = blockIdx.y * blockDim.y + threadIdx.y;  // row
int idx = y * width + x;                         // row-major index

// 1D within 2D block
int tid = threadIdx.y * blockDim.x + threadIdx.x;  // local flat index
```

---

## Function Qualifiers

| Qualifier | Callable from | Runs on | Notes |
|---|---|---|---|
| `__global__` | Host | Device | Entry point; must return void |
| `__device__` | Device | Device | Cannot be called from host |
| `__host__` | Host | Host | Default — same as regular function |
| `__host__ __device__` | Host or Device | Respective | Compiled twice — handy for math utilities |
| `__forceinline__ __device__` | Device | Device | Force inline even if compiler disagrees |
| `__noinline__ __device__` | Device | Device | Prevent inlining (for register pressure) |

---

## Memory Qualifiers

| Qualifier | Scope | Lifetime | Notes |
|---|---|---|---|
| `__shared__` | Block (all threads) | Kernel | Declared inside kernel; ~48KB typical |
| `__constant__` | All threads (read-only) | Program | 64KB; broadcast to all threads = fast when same index |
| `__restrict__` | Local hint | — | Tell compiler no aliasing; optimizer hint |
| `__device__` (variable) | All threads | Program | Global device variable |

```cpp
__global__ void kernel(float* data) {
    __shared__ float tile[256];    // shared memory declaration
    tile[threadIdx.x] = data[...]; // load into shared
    __syncthreads();               // synchronize before use
    // ... use tile
}

__constant__ float filter[16];    // constant memory — set from host via cudaMemcpyToSymbol
```

---

## Memory API

```cpp
// Device memory
float* d_ptr;
cudaMalloc(&d_ptr, N * sizeof(float));
cudaFree(d_ptr);

// Host ↔ Device transfer
cudaMemcpy(d_ptr, h_ptr, N*sizeof(float), cudaMemcpyHostToDevice);
cudaMemcpy(h_ptr, d_ptr, N*sizeof(float), cudaMemcpyDeviceToHost);
cudaMemcpy(d_dst, d_src, N*sizeof(float), cudaMemcpyDeviceToDevice);

// Async (overlaps with compute in separate stream)
cudaMemcpyAsync(d_ptr, h_ptr, N*sizeof(float), cudaMemcpyHostToDevice, stream);

// Pinned host memory (page-locked — faster H↔D transfer, enables async)
float* h_pinned;
cudaMallocHost(&h_pinned, N * sizeof(float));
cudaFreeHost(h_pinned);

// Unified memory (accessible from host and device)
float* u_ptr;
cudaMallocManaged(&u_ptr, N * sizeof(float));
cudaMemPrefetchAsync(u_ptr, N*sizeof(float), device_id, stream);  // hint migration
cudaFree(u_ptr);
```

---

## Synchronization

```cpp
__syncthreads();             // Block barrier — all threads in block reach this point
__syncwarp(mask);            // Warp barrier — all lanes in mask

cudaDeviceSynchronize();     // Host blocks until ALL kernels complete
cudaStreamSynchronize(s);    // Host blocks until stream s completes
cudaStreamWaitEvent(s, event, 0);  // stream s waits for event (cross-stream dependency)

// Events for timing
cudaEvent_t start, stop;
cudaEventCreate(&start);
cudaEventCreate(&stop);
cudaEventRecord(start, stream);
kernel<<<...>>>(args);
cudaEventRecord(stop, stream);
cudaEventSynchronize(stop);
float ms;
cudaEventElapsedTime(&ms, start, stop);  // elapsed in milliseconds
```

---

## Atomic Functions

```cpp
// All atomicX functions return the OLD value

atomicAdd(&x, val);    // x += val
atomicSub(&x, val);    // x -= val
atomicExch(&x, val);   // x = val (exchange)
atomicMax(&x, val);    // x = max(x, val)
atomicMin(&x, val);    // x = min(x, val)
atomicAnd(&x, val);    // x &= val
atomicOr (&x, val);    // x |= val
atomicXor(&x, val);    // x ^= val
atomicCAS(&x, compare, val);  // if x == compare: x = val. Returns old x.

// Warp-level reduction using shuffle (no shared memory needed)
float val = data[threadIdx.x];
for (int offset = 16; offset > 0; offset >>= 1)
    val += __shfl_down_sync(0xffffffff, val, offset);
// Thread 0 now holds sum of all 32 values
```

---

## Warp Functions

```cpp
// All require a mask of participating lanes (0xffffffff = all)

__shfl_sync(mask, var, srcLane);        // broadcast srcLane's var to all lanes
__shfl_up_sync(mask, var, delta);       // shift down: lane i gets lane i-delta's value
__shfl_down_sync(mask, var, delta);     // shift up: lane i gets lane i+delta's value
__shfl_xor_sync(mask, var, lane_mask);  // butterfly reduction

__ballot_sync(mask, predicate);  // bitmask of lanes where predicate is true
__any_sync(mask, predicate);     // 1 if any lane has predicate true
__all_sync(mask, predicate);     // 1 if all lanes have predicate true

__popc(x);       // population count (number of 1 bits in 32-bit x)
__clz(x);        // count leading zeros
__ffs(x);        // find first set bit (1-indexed, 0 if x==0)
```

---

## Error Check Macro

```cpp
#define CUDA_CHECK(call)                                                    \
    do {                                                                    \
        cudaError_t err = (call);                                           \
        if (err != cudaSuccess) {                                           \
            fprintf(stderr, "CUDA error at %s:%d — %s\n",                  \
                    __FILE__, __LINE__, cudaGetErrorString(err));           \
            exit(EXIT_FAILURE);                                             \
        }                                                                   \
    } while (0)

// Usage
CUDA_CHECK(cudaMalloc(&d_ptr, size));
CUDA_CHECK(cudaMemcpy(d_ptr, h_ptr, size, cudaMemcpyHostToDevice));

// Check after kernel launch (kernels don't return errors synchronously)
kernel<<<grid, block>>>(args);
CUDA_CHECK(cudaGetLastError());    // check for launch errors
CUDA_CHECK(cudaDeviceSynchronize()); // ensure kernel completed
```

---

## CUDA Memory Hierarchy (Approximate Latencies)

| Memory | Scope | Size | Latency | Bandwidth |
|---|---|---|---|---|
| Registers | Thread | ~255/thread | ~1 cycle | — |
| Shared memory | Block | 48–96 KB | ~5–10 cycles | ~5 TB/s |
| L1 cache | SM | 32–128 KB | ~20 cycles | ~5 TB/s |
| L2 cache | GPU | 4–32 MB | ~200 cycles | ~3 TB/s |
| Global memory (HBM) | GPU | 8–80 GB | ~500 cycles | 700–2000 GB/s |
| Pinned host memory | Host+GPU | System RAM | ~3000 cycles | PCIe: ~25 GB/s |
| Unified memory | Host+GPU | System RAM | ~6000+ cycles (page fault) | PCIe: ~25 GB/s |
````

---

### Step 1.12 — Write `ros2-reference.md`

- [ ] Create `tutorial/cheatsheets/ros2-reference.md` with the following content:

````markdown
# ROS2 Reference Cheatsheet

*Forge's ROS2 quick reference — CLI, QoS, lifecycle, launch files.*

---

## ros2 CLI Cheat Table

### Topics
```bash
ros2 topic list                            # list all topics
ros2 topic list -t                         # include type names
ros2 topic echo /chatter                   # print messages to console
ros2 topic pub /chatter std_msgs/msg/String '{data: "hello"}' --rate 10
ros2 topic hz /scan                        # measure publish rate
ros2 topic bw /camera/image_raw            # measure bandwidth
ros2 topic info /scan --verbose            # show publishers, subscribers, QoS
ros2 topic find std_msgs/msg/String        # find topics of given type
```

### Services
```bash
ros2 service list
ros2 service list -t                       # with type names
ros2 service call /add_ints example_interfaces/srv/AddTwoInts '{a: 1, b: 2}'
ros2 service type /add_ints
ros2 service find example_interfaces/srv/AddTwoInts
```

### Actions
```bash
ros2 action list
ros2 action list -t
ros2 action send_goal /fibonacci \
    action_tutorials_interfaces/action/Fibonacci '{order: 5}'
ros2 action send_goal --feedback /fibonacci \
    action_tutorials_interfaces/action/Fibonacci '{order: 10}'
ros2 action info /fibonacci
```

### Parameters
```bash
ros2 param list --node /my_node            # list params for a node
ros2 param get /my_node use_sim_time       # get a param value
ros2 param set /my_node log_level DEBUG    # set a param value
ros2 param dump /my_node                   # dump all params as YAML
ros2 param load /my_node params.yaml       # load params from file
```

### Nodes
```bash
ros2 node list
ros2 node info /my_node                    # show pubs, subs, services, params
```

### Bags
```bash
ros2 bag record -o my_bag /scan /odom     # record topics to bag
ros2 bag record -a                         # record all topics
ros2 bag play my_bag                       # replay bag
ros2 bag play --rate 0.5 my_bag           # half speed replay
ros2 bag info my_bag                       # show bag metadata
ros2 bag filter my_bag filtered -i '/scan' # filter to specific topics (foxy+)
```

### Lifecycle
```bash
ros2 lifecycle list /my_lifecycle_node     # list available transitions
ros2 lifecycle get /my_lifecycle_node      # current state
ros2 lifecycle set /my_lifecycle_node configure
ros2 lifecycle set /my_lifecycle_node activate
ros2 lifecycle set /my_lifecycle_node deactivate
ros2 lifecycle set /my_lifecycle_node cleanup
ros2 lifecycle set /my_lifecycle_node shutdown
```

---

## QoS Profile Table

| Profile | Reliability | Durability | History Depth | Use case |
|---|---|---|---|---|
| `sensor_data` | BEST_EFFORT | VOLATILE | 5 | High-rate sensor streams (LiDAR, camera) — drop ok |
| `system_default` | RELIABLE | VOLATILE | 10 | General purpose |
| `parameters` | RELIABLE | VOLATILE | 1000 | Parameter events |
| `services_default` | RELIABLE | VOLATILE | 10 | Service clients/servers |
| `parameter_events` | RELIABLE | VOLATILE | 1000 | Parameter event topic |
| `transient_local` | RELIABLE | TRANSIENT_LOCAL | varies | Late-joining subscribers receive last N messages (map, static transforms) |

**QoS Mismatch:** if publisher and subscriber have incompatible QoS (e.g., publisher is BEST_EFFORT, subscriber requires RELIABLE), they will not connect. No error is thrown — the connection is silently skipped. Check with `ros2 topic info --verbose`.

---

## Lifecycle Node State Machine

```
                  +---------------+
                  |  Unconfigured |  ← initial state after creation
                  +------+--------+
                         | configure()
                         ↓
                  +------+--------+
                  |   Configured  |
                  +------+--------+
                         | activate()
                         ↓
      deactivate() ←  +--+-----+  → on_error()
                      |  Active |
      deactivate() →  +--+-----+
                         |
                         ↓ deactivate()
                  +------+--------+
                  |   Inactive    |
                  +------+--------+
                         | cleanup()
                         ↓
                  +------+--------+
                  |  Unconfigured |  (back to start)
                  +---------------+

Any state → shutdown() → Finalized
```

**When to use lifecycle nodes:** drivers, hardware interfaces, components that need ordered bring-up and safe shut-down (e.g., motor controller: configure before activating).

---

## colcon Build Options

```bash
# Build everything
colcon build

# Build specific packages only
colcon build --packages-select my_pkg other_pkg

# Build package and all its dependencies
colcon build --packages-up-to my_pkg

# No file copy for Python (symlink — changes take effect immediately)
colcon build --symlink-install

# Set CMake build type
colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release

# Show live build output
colcon build --event-handlers console_direct+

# Parallel jobs
colcon build --parallel-workers 4

# After build: source the overlay
source install/setup.bash  # or setup.zsh

# Test
colcon test --packages-select my_pkg
colcon test-result --verbose
```

---

## Launch File Skeleton (Python)

```python
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():
    # Declare arguments
    use_sim_time_arg = DeclareLaunchArgument(
        'use_sim_time', default_value='false',
        description='Use simulation clock')

    return LaunchDescription([
        use_sim_time_arg,

        # Basic node
        Node(
            package='my_pkg',
            executable='my_node',
            name='my_node',                         # override default node name
            namespace='robot1',                      # namespace prefix
            parameters=[
                {'param_name': 'value'},
                {'use_sim_time': LaunchConfiguration('use_sim_time')},
                PathJoinSubstitution([FindPackageShare('my_pkg'), 'config', 'params.yaml']),
            ],
            remappings=[
                ('/cmd_vel', '/robot1/cmd_vel'),     # remap topics
                ('/scan', '/robot1/scan'),
            ],
            output='screen',
        ),

        # Include another launch file
        IncludeLaunchDescription(
            PathJoinSubstitution([FindPackageShare('nav2_bringup'), 'launch', 'navigation_launch.py']),
            launch_arguments={'use_sim_time': LaunchConfiguration('use_sim_time')}.items(),
        ),
    ])
```

---

## package.xml Dependencies

```xml
<?xml version="1.0"?>
<package format="3">
  <name>my_pkg</name>
  <version>0.0.1</version>
  <description>My ROS2 package</description>
  <maintainer email="me@example.com">Me</maintainer>
  <license>Apache-2.0</license>

  <!-- Build + exec (most C++ deps) -->
  <depend>rclcpp</depend>
  <depend>std_msgs</depend>
  <depend>geometry_msgs</depend>

  <!-- Build only -->
  <build_depend>ament_cmake</build_depend>
  <build_depend>rosidl_default_generators</build_depend>

  <!-- Exec only (generated interfaces) -->
  <exec_depend>rosidl_default_runtime</exec_depend>

  <!-- Testing -->
  <test_depend>ament_lint_auto</test_depend>
  <test_depend>ament_gtest</test_depend>

  <!-- Interface group membership -->
  <member_of_group>rosidl_interface_packages</member_of_group>

  <export>
    <build_type>ament_cmake</build_type>
  </export>
</package>
```

---

## Topics vs Services vs Actions

| | Topic | Service | Action |
|---|---|---|---|
| Pattern | Pub/sub (one-to-many) | Request/Response | Goal/Feedback/Result |
| Blocking | No | Yes (until response) | No (async, cancel supported) |
| Feedback | No | No | Yes (periodic updates) |
| Cancel | No | No | Yes |
| Use when | Continuous sensor data, state | Query / compute on demand | Long-running tasks (navigation, manipulation) |
````

---

### Step 1.13 — Commit cheatsheets

- [ ] `git add tutorial/cheatsheets/ && git commit -m "docs(tutorial): add 11 cheatsheets"`

---

## Task 2: Interview Master

### Step 2.1 — Create directory

- [ ] `mkdir -p tutorial/interview-master`

---

### Step 2.2 — Write `mega-qa.md`

- [ ] Create `tutorial/interview-master/mega-qa.md` with the following content:

````markdown
# C++ Interview Mega Q&A

*Forge's complete Q&A bank — 95 questions across 8 domains. Real answers. No filler.*

---

## Memory & Ownership

**1. What is RAII and why does it matter?**

RAII (Resource Acquisition Is Initialization) means acquiring a resource in a constructor and releasing it in the destructor. Because C++ guarantees destructors run when objects go out of scope — even during exception unwinding — RAII makes resource management exception-safe without try/finally blocks. Every `unique_ptr`, `lock_guard`, and `fstream` in the standard library uses RAII. If you have a bare `delete` in your code, you have a potential leak.

**2. When would you use `shared_ptr` over `unique_ptr`?**

Use `shared_ptr` only when two or more owners must share lifetime — for example, a cache that hands out references to entries while the entries must stay alive as long as any reference exists. `unique_ptr` should be the default because it has zero overhead. `shared_ptr` adds an atomic reference count (two pointer-size increments on construction), which is ~2× slower and can become a bottleneck in high-frequency allocation paths.

> TRAP: `shared_ptr` is not a garbage collector. Cycles between `shared_ptr`s leak forever. Break cycles with `weak_ptr`.

**3. What is the difference between `weak_ptr::lock()` returning null vs throwing?**

`weak_ptr::lock()` never throws. It returns an empty `shared_ptr` (which is falsy) if the managed object has already been destroyed. The alternative, `weak_ptr::expired()`, is a racy check — between `expired()` returning false and your next dereference, the object could be destroyed. Always use `lock()` and check the result. The only thing that throws in weak_ptr is the constructor `shared_ptr(weak_ptr)` — it throws `std::bad_weak_ptr` if expired.

**4. What is false sharing and how do you fix it?**

False sharing occurs when two threads modify different variables that happen to reside on the same CPU cache line (64 bytes). Even though the variables are logically independent, the cache coherence protocol (MESI) forces the owning CPU to invalidate and re-read the entire cache line on every remote write. Fix: use `alignas(64)` to ensure each hot variable has its own cache line. Detect with `perf c2c` (HITM events).

**5. Explain the difference between stack allocation and heap allocation for performance.**

Stack allocation is a single stack-pointer decrement — essentially free, ~1 cycle. It is also cache-hot because recent allocations are adjacent in memory. Heap allocation (`new`/`malloc`) must find free memory of the right size, potentially traverse free lists, and may call into the OS for more virtual memory pages; this costs hundreds of nanoseconds on a cold path. Additionally, heap objects from different allocation sites are scattered in memory, causing cache misses on traversal. Prefer stack, local arenas, or pools for hot paths.

**6. What is a dangling reference and how does ASan catch it?**

A dangling reference (or pointer) points to memory that has been freed. Accessing it is undefined behavior — the memory may have been reused by another allocation, zeroed, or remain unchanged. ASan catches it by filling freed memory with a poison pattern and adding a "shadow memory" layer that tracks allocation status for every 8 bytes. On any access to poisoned memory, ASan intercepts the load/store and reports the exact location and the original free/allocation call stacks.

**7. What is a double-free and how does it arise?**

A double-free occurs when `delete` or `free()` is called twice on the same pointer. It corrupts the allocator's free list (which typically stores bookkeeping data at the freed location), leading to crashes or exploitable heap corruption. It arises from manual ownership — two code paths both believe they own the pointer. Prevention: use `unique_ptr`, set raw pointer to `nullptr` after `delete` (though this only prevents the double-free via that same pointer variable), or use RAII consistently.

**8. When is placement new appropriate?**

Placement new is appropriate when you want to separate memory acquisition from object construction: pool allocators, in-place construction inside variant-like storage, embedded-systems contexts where you manage a fixed buffer, and when constructing into shared memory. After use, you must call the destructor explicitly (`obj->~T()`) because you cannot use `delete` on a pointer that was not returned by `::operator new`.

**9. What is `std::pmr::monotonic_buffer_resource` useful for?**

It provides a bump-pointer allocator backed by a user-supplied buffer (stack or a larger heap block). Allocation is O(1) — just increment a pointer. There is no free-per-object: `release()` resets the entire buffer. It is ideal for per-frame allocations in game loops, per-request allocations in servers (allocate freely during the request, release everything at the end), and anywhere bulk free semantics apply.

**10. What is NRVO (Named Return Value Optimization)?**

NRVO is a compiler optimization that constructs the return value directly in the caller's storage, avoiding a copy or move of the return object. When a function has a single `return local_var;` and `local_var` is of the return type, the compiler may construct it in-place. Unlike copy elision (RVO for temporaries), NRVO is optional but universally implemented by GCC, Clang, and MSVC at -O1+. Mark your return type as movable just in case, but do not `std::move` the return — that defeats NRVO.

**11. Why is `std::vector` resizing O(1) amortized?**

When `push_back` exceeds capacity, `vector` allocates a new buffer at typically 2× the current capacity, moves all elements, and frees the old buffer. This is O(n) for that one operation. But consider n pushes from an empty vector: total copy work is 1 + 2 + 4 + ... + n ≈ 2n. So n pushes do O(2n) total work = O(1) amortized per push. The doubling factor is critical — any constant factor > 1 gives O(1) amortized.

**12. What does `std::move` actually do?**

`std::move` is an unconditional cast to an rvalue reference (`T&&`). It does not move anything. It signals to the compiler that the named object may be treated as a temporary — enabling the move constructor or move assignment operator to steal its resources. The actual resource transfer happens in the move constructor/assign of the receiving type. After `std::move`, the source is in a "valid but unspecified" state.

**13. When does the compiler elide copies (copy elision / RVO)?**

Since C++17, copy elision is mandatory for prvalues (temporaries) — this is guaranteed RVO. NRVO (named return value optimization) is permitted but not required. Cases where elision applies: `return T(args);` (mandatory), `T obj = T(args);` (mandatory), `return named_local;` (permitted NRVO). Even if move semantics are available, mandatory elision constructs the object directly in its final location — no move, no copy.

**14. What is a memory pool and when would you implement one?**

A memory pool pre-allocates a large chunk of memory and carves it into fixed-size blocks. Allocation is O(1) — pop from a free list. Deallocation is O(1) — push back to free list. No fragmentation for fixed-size objects. Use it when: (a) you allocate/deallocate thousands of same-size objects per second (e.g., network packets, task objects, tree nodes), (b) you need predictable latency (no malloc jitter), or (c) you need contiguous layout for cache performance.

**15. Explain the difference between `new T` and `::operator new(sizeof(T)); new(ptr) T;`**

`new T` does two things: call `::operator new(sizeof(T))` to allocate raw memory, then call `T`'s constructor on that memory. `::operator new(sizeof(T))` alone allocates raw memory without constructing anything. `new(ptr) T` (placement new) constructs `T` at an already-obtained address `ptr` without any allocation. Separating them is what pool allocators do: allocate blocks wholesale with `::operator new`, then construct objects with placement new, then destroy with explicit destructor calls, then return blocks to the pool.

---

## OOP & Design

**1. What is the virtual dispatch mechanism?**

Each polymorphic class has a virtual table (vtable) — an array of function pointers, one per virtual function. Every instance carries a hidden vtable pointer (vptr, typically 8 bytes) initialized by the constructor. A virtual call `obj->foo()` compiles to: load vptr from object, index into vtable at foo's slot, call through the pointer. Cost: one extra memory load for the vptr, one indirect branch (which branch predictors struggle with for highly polymorphic call sites), and inability to inline.

**2. Why is CRTP faster than virtual dispatch?**

With CRTP, the derived type is known at compile time as a template argument. `static_cast<Derived*>(this)->method()` compiles to a direct call that the compiler can inline, removing the vtable lookup and indirect branch entirely. There is no vptr — CRTP objects are smaller by 8 bytes. The cost: no runtime polymorphism (you cannot store `Shape<Circle>` and `Shape<Square>` in the same container without a type-erased wrapper).

**3. What is the diamond problem and how does virtual inheritance solve it?**

When class `D` inherits from both `B` and `C`, which both inherit from `A`, `D` gets two subobjects of `A` — `B::A` and `C::A`. Accessing `A`'s members from `D` is ambiguous. Virtual inheritance (`class B : virtual public A`) ensures `D` shares a single `A` subobject between all virtual bases. The cost: each virtual base path adds a vtable pointer for the offset to the shared `A` subobject; construction order is more complex (the most-derived class must initialize virtual bases).

**4. What does `= delete` do? When would you use it?**

`= delete` makes a function explicitly unavailable. The compiler will emit an error if that function is selected by overload resolution, with a message pointing to the deletion. Uses: disable copy for move-only types (`unique_ptr`), prevent narrowing conversions (`void f(int) = delete; void f(double);` — forces caller to use double), suppress implicit conversions, or remove dangerous base class methods in derived types.

**5. Explain the Non-Virtual Interface (NVI) pattern.**

The public interface consists of non-virtual functions that implement the invariant-preserving "template method" — they call private virtual functions as customization points. Derived classes override the private virtual functions but cannot bypass the invariant checks in the public function. This separates what the class does (public API) from how it does it (virtual extension points). It also lets the base class add pre/post-conditions without derived classes needing to know.

**6. What is object slicing and how do you prevent it?**

Object slicing occurs when a derived class object is assigned to a base class object by value: the derived-class-specific data is silently discarded. `Base b = derived_obj;` copies only the `Base` subobject. This is rarely intended. Prevention: use pointers or references to base (`Base&`, `Base*`, `unique_ptr<Base>`), make the base class non-copyable (delete copy ctor/assign), or use a `clone()` virtual method for polymorphic copying.

**7. When should a class have a virtual destructor?**

Whenever you might delete a derived class object through a base class pointer. If `Base* p = new Derived; delete p;` is possible and `Base::~Base` is not virtual, only `Base`'s destructor runs — `Derived`'s resources leak, undefined behavior. Rule of thumb: if a class has any virtual function, give it a virtual destructor. Pure interfaces (no data, all pure virtual) should have a virtual destructor too, even if it has an empty body.

**8. What is the Liskov Substitution Principle?**

Objects of a derived class must be usable in place of objects of the base class without changing the correctness of the program. Formally: for any precondition the base establishes, the derived class must accept it (cannot strengthen preconditions); for any postcondition the base guarantees, the derived class must guarantee it too (cannot weaken postconditions). LSP violation example: a `Square` deriving from `Rectangle` where `setWidth` also sets `setHeight` — code that calls `setWidth(5); setHeight(3);` on a `Rectangle*` gets width=3 from a Square, violating the caller's expectation.

**9. Explain SOLID principles with C++ examples.**

- **S**ingle Responsibility: a `FileParser` class parses; it does not log or connect to a database.
- **O**pen/Closed: `Sorter<Policy>` — open for extension via new policy types, closed for modification.
- **L**iskov: derived types must satisfy base contracts (see Q8).
- **I**nterface Segregation: split `IAnimal` into `IFlyable` and `ISwimmable` — clients depend only on what they use.
- **D**ependency Inversion: `DataProcessor` depends on `IDataSink` interface, not on `ConsoleSink` directly — inject the sink.

**10. What is the pimpl idiom and why use it?**

Pimpl (pointer to implementation) hides private implementation details behind a forward-declared pointer: `class Widget { struct Impl; unique_ptr<Impl> pimpl_; };`. The `Impl` struct is defined in the .cpp file. Benefits: (a) changes to implementation don't trigger recompilation of headers that include Widget, (b) private data members are hidden from users of the header (binary compatibility), (c) compile-time firewall for large dependencies (e.g., including boost only in the .cpp).

**11. How does CRTP enable static polymorphism?**

```cpp
template<typename Derived>
struct Base {
    void interface() {
        static_cast<Derived*>(this)->implementation();
    }
};
struct Concrete : Base<Concrete> {
    void implementation() { /* actual work */ }
};
```

The compiler knows `Derived` at instantiation time, so `static_cast` is resolved statically. No vtable, no vptr, fully inlineable. The trade-off: `Base<Circle>` and `Base<Square>` are different types — you cannot store them in the same `vector<Base>`.

**12. What is a mixin?**

A mixin is a class (often CRTP-based) that adds a capability to any class that inherits from it, without knowing what that class is. Example: `struct Comparable<Derived>` adds `!=`, `>`, `>=`, `<=` given that `Derived` provides `==` and `<`. Multiple mixins can be combined: `struct MyClass : Comparable<MyClass>, Printable<MyClass>, Serializable<MyClass>`. This avoids code duplication while achieving compile-time, zero-overhead trait composition.

**13. What is the difference between composition and inheritance?**

Inheritance models "is-a" relationships and enables polymorphism. Composition models "has-a" relationships. Prefer composition: it is more flexible, avoids fragile base class problems, and limits coupling. A `Car` that has an `Engine` field can swap engines; a `Car` that inherits `Engine` cannot. Inheritance for public polymorphic interfaces is appropriate. Inheritance for code reuse alone (private or protected inheritance) is usually better replaced with composition.

**14. What is a pure abstract interface in C++ and how do you implement one?**

```cpp
struct IDataSource {
    virtual std::vector<std::byte> read(std::size_t n) = 0;
    virtual bool eof() const = 0;
    virtual ~IDataSource() = default;  // virtual destructor: mandatory
};
```

All methods are pure virtual (`= 0`). No data members. The virtual destructor ensures correct cleanup through a base pointer. Implement by inheriting and overriding all pure virtuals. Callers depend on `IDataSource*` or `IDataSource&` — the concrete type is injected (dependency inversion).

**15. Why can't you call a virtual function in a constructor?**

During construction of `Base`, the vptr points to `Base`'s vtable. The `Derived` part has not yet been constructed. If `Base`'s constructor calls a virtual function, it dispatches to `Base::foo()` — not `Derived::foo()`. This is correct behavior (calling a Derived method when Derived's members are uninitialized would be worse), but it surprises programmers expecting polymorphic dispatch. The same applies to destructors — by the time `Base::~Base()` runs, `Derived::~Derived()` has already run, so the vptr is again pointed at `Base`'s vtable.

---

## Templates

**1. What is two-phase template lookup?**

Template parsing happens in two phases. Phase 1 (definition time): non-dependent names (those not involving template parameters) are looked up immediately. They must be visible at the point of template definition. Phase 2 (instantiation time): dependent names (involving template parameters) are looked up at the point of instantiation using the instantiation context plus ADL. Implication: if a base class member depends on `T`, it is a dependent name and requires `this->member` or `Base<T>::member` to be found.

**2. Explain SFINAE with an example.**

```cpp
template<typename T>
std::enable_if_t<std::is_integral_v<T>, std::string>
to_string(T val) { return std::to_string(val); }

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, std::string>
to_string(T val) { return std::to_string(val) + "f"; }
```

When instantiated with `int`, the floating-point overload's return type becomes `enable_if_t<false, string>` — a substitution failure, which is not an error. It is removed from the overload set silently. The integral overload succeeds.

> DEEP CUT: SFINAE only works in the "immediate context" of template argument substitution. Errors in function bodies are still errors.

**3. What is the difference between a concept and enable_if?**

Both constrain template arguments but differ in: (a) readability — `template<std::integral T>` is self-documenting; `enable_if` is cryptic; (b) error messages — concepts produce "constraint not satisfied" with the specific requirement that failed; enable_if produces a substitution failure dump; (c) expressiveness — concepts can express compound requirements, associated types, and return type constraints cleanly; (d) overload resolution — concepts participate directly in overload ranking (more constrained = higher priority).

**4. How does template argument deduction work?**

The compiler deduces template arguments from the function call arguments. For `template<typename T> void f(T x)` called as `f(42)`, deduces `T=int`. Rules: array arguments decay to pointers, function arguments decay to function pointers, `const`/`volatile` are stripped for top-level cv-qualification. For forwarding references (`T&&`), special rules apply: lvalue of type `X` deduces `T=X&` (reference collapsing gives `X& &&` → `X&`), rvalue of type `X` deduces `T=X`.

**5. What is a variadic template? Show a recursive sum.**

```cpp
template<typename T>
T sum(T t) { return t; }  // base case

template<typename T, typename... Rest>
T sum(T first, Rest... rest) {
    return first + sum(rest...);
}
// sum(1, 2, 3, 4) → 1 + sum(2,3,4) → 1+2+sum(3,4) → 1+2+3+sum(4) → 10

// C++17 fold expression (simpler):
template<typename... Args>
auto sum(Args... args) { return (args + ...); }
```

**6. What is a fold expression? How does `(args + ...)` differ from `(... + args)`?**

Both sum a parameter pack. `(args + ...)` is a unary right fold — evaluates right-to-left: `a0 + (a1 + (a2 + a3))`. `(... + args)` is a unary left fold — evaluates left-to-right: `((a0 + a1) + a2) + a3`. For addition they give the same result (commutative), but for non-commutative operations like string concatenation or subtraction, the order matters. Binary forms: `(init + ... + args)` starts with `init` on the left; `(args + ... + init)` has `init` on the right.

**7. What is tag dispatch and when is it used?**

Tag dispatch selects function overloads at compile time using empty tag types as extra arguments. Example: choosing between random-access and forward-iterator implementations of `advance`:

```cpp
template<typename Iter>
void advance_impl(Iter& it, int n, std::random_access_iterator_tag) {
    it += n;  // O(1)
}
template<typename Iter>
void advance_impl(Iter& it, int n, std::forward_iterator_tag) {
    while (n--) ++it;  // O(n)
}
template<typename Iter>
void advance(Iter& it, int n) {
    advance_impl(it, n, typename std::iterator_traits<Iter>::iterator_category{});
}
```

Used before `if constexpr` and concepts were available; still useful for ADL-extensible dispatch.

**8. What is CTAD (Class Template Argument Deduction)?**

Since C++17, class templates can deduce their template arguments from constructor arguments, just like function templates. `std::pair p{1, 2.0}` deduces `pair<int,double>`. `std::vector v{1,2,3}` deduces `vector<int>`. Libraries provide explicit deduction guides (`template<class T> vector(initializer_list<T>) -> vector<T>;`) to control deduction for non-obvious cases. This eliminates the need for `make_pair`, `make_tuple`, etc.

**9. What is explicit template instantiation and why use it?**

```cpp
// In header:
extern template class MyTemplate<int>;  // suppress implicit instantiation

// In exactly one .cpp:
template class MyTemplate<int>;         // force instantiation here
```

Without this, every translation unit that uses `MyTemplate<int>` generates its own instantiation, which the linker then deduplicates. Explicit instantiation moves the instantiation to one place, reducing compile time (instantiation happens once) and reducing object file bloat. Critical for large template libraries that are instantiated with a few common types.

**10. What is `if constexpr`?**

`if constexpr (condition)` evaluates the condition at compile time. The branch not taken is not compiled — it may contain code that would be ill-formed for other template arguments. Unlike regular `if`, the compiler does not even attempt to parse the discarded branch for type correctness in the context of the current instantiation. This enables writing template functions with branches that only make sense for certain types:

```cpp
template<typename T>
void debug_print(T val) {
    if constexpr (std::is_pointer_v<T>)
        std::cout << "ptr: " << *val;
    else
        std::cout << "val: " << val;
}
```

**11. Explain the difference between `decltype(expr)` and `auto`.**

`auto` deduces type from an initializer expression, applying decay (strips references, cv-qualifiers). `decltype(expr)` yields the exact type of the expression, preserving references and cv-qualifiers. `decltype((x))` (double parentheses) gives a reference if `x` is an lvalue. Used together in trailing return types: `auto f(T a, U b) -> decltype(a + b)` returns the type of `a+b` preserving all qualifications. In C++14, `decltype(auto)` as a return type or placeholder deduces like `decltype` instead of like `auto`.

**12. What is `std::void_t` used for?**

`void_t<Exprs...>` is always `void` if all expressions in `Exprs` are well-formed, substitution failure otherwise. It is the standard idiom for detecting whether a type has a member, member function, or nested type. Combined with partial template specialization, it selects the specialization only when the probed expression compiles. Used to implement `has_size`, `has_begin`, `is_detected`, etc.

**13. What is the curiously recurring template pattern?**

CRTP: a class `D` inherits from `Base<D>`. The base has the interface; the derived provides the implementation. Because `D` is a complete type by the time `Base<D>`'s member functions are instantiated, `Base` can `static_cast<D*>(this)` and call `D`'s methods with zero overhead. Common uses: static polymorphism, mixins, enabling shared utility via the Barton-Nackman trick, `enable_shared_from_this`.

**14. How do expression templates avoid temporaries?**

In naive `Vector a = b + c + d`, each `+` creates a temporary. Expression templates represent lazy arithmetic: `b + c` returns an `Add<Vec,Vec>` node object (not a Vector), and `(b+c) + d` returns `Add<Add<Vec,Vec>,Vec>`. The assignment operator evaluates the entire expression element-by-element into the result, with no temporaries. Each element computation is: `result[i] = b[i] + c[i] + d[i]`. Libraries like Eigen use this extensively.

**15. What is a non-type template parameter?**

A template parameter that is a value rather than a type. Examples: `template<int N> struct Array { int data[N]; };`, `template<std::size_t Capacity> class RingBuffer`, `template<auto Val>` (C++17 — deduces type). Non-type template parameters must be compile-time constants. Since C++20, floating-point values and class types with constexpr constructors are also allowed. They enable zero-overhead sized containers and compile-time configuration without runtime overhead.

---

## Concurrency

**1. What is the C++ memory model?**

The C++ memory model defines the rules for how memory accesses in one thread are visible to other threads. It specifies: (a) that accesses to the same location from multiple threads are a data race (UB) unless at least one is atomic or protected by a mutex; (b) an ordering hierarchy from `relaxed` (only atomicity) to `seq_cst` (global total order); (c) the happens-before and synchronizes-with relations that establish causality between threads. The model abstracts over hardware by allowing compilers and CPUs to reorder instructions within the defined rules.

**2. What is a data race and why is it undefined behavior?**

A data race occurs when two threads access the same memory location concurrently, at least one access is a write, and there is no synchronization (atomic, mutex, or happens-before) between them. It is undefined behavior in C++ — not merely "might get a torn read." The compiler is permitted to assume data races do not occur, enabling optimizations that are incorrect if they do. For example, a load can be hoisted out of a loop, or a value can be cached in a register, producing a stale read even if another thread writes it.

**3. Explain the happens-before relationship.**

Happens-before (HB) is a transitive partial order over memory operations. If A happens-before B, then B observes all memory writes done by A. It is established by: (a) sequenced-before (single-thread program order), (b) synchronizes-with (a release store that is observed by an acquire load), (c) transitivity (A HB B, B HB C → A HB C). Data races are precisely two conflicting accesses where neither happens-before the other.

**4. What is `memory_order_acquire` and `memory_order_release`?**

A release store makes all preceding writes (in program order) visible to any thread that performs an acquire load of the same atomic variable and sees the stored value. Acquire: "I see everything that happened before the corresponding release." Release: "I'm publishing everything I've done so far." Together they form a synchronization point. Cost: on x86 they compile to no extra instructions (TSO gives acquire/release semantics for regular loads/stores); on ARM they add `dmb` memory barrier instructions.

**5. When would you use `memory_order_relaxed`?**

For operations where you need only atomicity (no tearing), not ordering relative to other operations. Example: a thread-safe hit counter where you only care about the total value at the end, not about sequencing relative to other actions. Another example: a relaxed load of a flag that was published with release, when you're using a separate acquire operation for the actual synchronization. Relaxed is the cheapest atomic op — no fences, just an atomic instruction.

**6. What is false sharing and how do you detect it?**

False sharing: two threads modify different variables that share a 64-byte cache line. The cache coherence protocol (MESI) forces the entire cache line to be invalidated on every write, causing expensive cross-core traffic even though the data is logically independent. Detection: `perf c2c report` shows "HITM" (hit-in-modified) events — a core read that found the line in Modified state in another core's cache. Also visible as high L3 miss rates correlated with thread count.

**7. What is the ABA problem in lock-free programming?**

In a CAS loop, thread A reads pointer value X (pointing to object at address 0x100). Thread B pops 0x100, processes it, frees it, allocates a new object — which happens to land at address 0x100 — and pushes it. Thread A's CAS sees head == 0x100, same as it read, and succeeds — but the object has changed. The pointer value A compared against is correct, but the object it points to is not the one A prepared its operation for. Fix: version/tag the pointer — pack a generation counter into spare bits of the address, increment on every push.

**8. Explain the difference between a mutex and a semaphore.**

A mutex is binary — locked or unlocked — and is owned by the thread that locked it (only that thread can unlock it, enforcing mutual exclusion). A semaphore is an integer counter — `sem_wait` decrements (blocks if 0), `sem_post` increments. Any thread can call either operation. A semaphore with initial value 1 approximates a mutex but without ownership semantics. Semaphores are better for producer-consumer signaling (producer posts, consumer waits) while mutexes are better for protecting critical sections.

**9. What is a condition variable and how does spurious wakeup affect it?**

A condition variable allows threads to wait for a condition to become true. `wait(lock, predicate)` atomically releases the mutex and suspends the thread; when notified, it reacquires the mutex and checks the predicate. Spurious wakeup: a thread may wake up without being notified — this is permitted by POSIX and the C++ standard to simplify OS-level implementation. Always check the predicate in a loop: `cv.wait(lk, [&]{ return !queue.empty(); })` — the lambda form of `wait` does this automatically.

**10. What is `std::atomic` and when is it not sufficient?**

`std::atomic<T>` guarantees that individual load, store, or read-modify-write operations on `T` are atomic (no tearing, no interleaving). It is not sufficient when: (a) you need an atomic operation across multiple variables (update balance and transaction count atomically), (b) you need a conditional check-then-act sequence beyond what CAS provides, (c) `T` is not lock-free (check `is_lock_free()`; large or non-trivially-copyable types use an internal mutex). For these cases, use `std::mutex`.

**11. What is a deadlock? Give three conditions and how to avoid them.**

A deadlock is a cycle of threads where each waits for a resource held by the next. Coffman conditions: (1) mutual exclusion — resources are not sharable; (2) hold-and-wait — threads hold resources while waiting for more; (3) no preemption — resources cannot be taken away; (4) circular wait — circular chain of waits. Avoidance: (a) impose a global lock ordering — always acquire mutex A before B; (b) use `std::scoped_lock<MutexA, MutexB>` which uses `std::lock` under the hood (try-lock with backoff); (c) use try-lock with timeout and retry; (d) eliminate the need to hold two locks simultaneously via design.

**12. What is a thread pool and how does `std::packaged_task` help?**

A thread pool maintains a fixed set of worker threads that process tasks from a queue, amortizing thread creation overhead. `std::packaged_task<F(Args...)>` wraps a callable so that calling it stores the result (or exception) in a shared state, which a `std::future<F>` can retrieve asynchronously. In a pool: `auto task = make_shared<packaged_task<int()>>(f); future = task->get_future(); queue.push([task]{ (*task)(); });`. Workers drain the queue; callers await futures.

**13. How do C++20 coroutines differ from threads?**

Coroutines are stackless — their state (local variables, suspension point) is stored in a heap-allocated frame, not a dedicated OS stack. Suspending a coroutine is a cheap `co_await` expression — no context switch, no syscall. Thousands of coroutines can be live simultaneously with much less memory than threads (no 8MB stack per coroutine). Threads are preemptively scheduled by the OS; coroutines cooperatively yield. Coroutines require a scheduler (executor) to resume them; threads run independently. Coroutines are excellent for high-concurrency I/O; threads better for CPU-bound parallelism.

**14. What is the difference between `std::async(launch::async, ...)` and `std::thread`?**

`std::async(launch::async, f)` launches `f` in a new thread (or thread pool, implementation-defined) and returns a `std::future` that holds the result. When the future is destroyed, its destructor blocks until the thread completes — preventing detached threads. `std::thread` gives you more control: you choose when to `join` or `detach`. `async` simplifies result retrieval and exception propagation. `thread` is better when you need fine-grained lifetime control, or when you are building your own task system.

**15. What is the SPSC queue and why is it faster than a mutex-based queue?**

Single Producer Single Consumer queue: two atomic indices (head and tail), a fixed-size ring buffer. Producer writes to `buf[tail]` and increments `tail` with a release store. Consumer reads from `buf[head]` and increments `head` with a release store. No mutex, no condition variable, no lock contention. The SPSC discipline (one writer, one reader) means head and tail never need to synchronize with each other beyond the simple acquire/release pair. Each access is one atomic operation vs the mutex path of: atomic CAS (lock), memory fence, check, modify, fence, atomic CAS (unlock), potential futex syscall.

---

## Systems Programming

**1. What is virtual memory and what happens on a page fault?**

Virtual memory gives each process a private address space where addresses are translated by the MMU via page tables. Pages (4KB typically) map virtual addresses to physical frames. On first access to an unmapped page, the CPU raises a page fault exception. The OS kernel handles it: allocates a physical frame, zeros it (for anonymous pages) or reads it from disk/file (for file-backed mappings), updates the page table, and resumes the faulting instruction. Page faults cost microseconds to milliseconds depending on whether the page is in memory or on disk.

**2. What is the cost of a system call?**

A syscall costs roughly 100–300ns on modern Linux for a minimal syscall like `getpid()`. The cost components: SYSCALL instruction (privilege mode switch, trap), kernel validates arguments, executes the syscall, IRET returns to user mode. The Spectre/Meltdown mitigations (KPTI) doubled many syscall costs on affected CPUs. For I/O syscalls, the cost is dominated by the actual I/O. Minimize syscalls in hot paths — batch them (write large buffers, use scatter-gather I/O with `writev`).

**3. What is VDSO and which system calls does it optimize?**

The Virtual Dynamic Shared Object (vDSO) is a small shared library that the kernel maps into every process's address space. It contains implementations of certain syscalls that don't actually need kernel privilege — they read kernel data structures directly from the mapped vDSO page. `gettimeofday()`, `clock_gettime()`, `getcpu()`, `time()` are served from vDSO without the SYSCALL trap. This reduces `clock_gettime(CLOCK_REALTIME)` from ~100ns to ~4ns on modern systems.

**4. What is copy-on-write (COW) in the context of fork()?**

After `fork()`, parent and child share all physical pages — the page tables point to the same frames, marked read-only. When either process writes to a page, the CPU raises a protection fault. The kernel copies the page, updates the writer's page table to point to the new copy, marks it writable, and resumes execution. The other process still sees the original. This makes `fork()` extremely fast even for large processes — you only pay for pages you actually modify. `exec()` immediately after `fork()` (the "fork-exec" idiom) copies almost nothing.

**5. What is epoll and how does it differ from select/poll?**

`epoll` is Linux's scalable I/O event notification mechanism. `select` and `poll` both require passing the entire set of file descriptors on every call and scan through them in the kernel — O(n) per call. `epoll` maintains a kernel-side set of monitored fds (`epoll_ctl` to add/modify/remove); `epoll_wait` returns only the ready fds, O(ready). For 10,000 connections with 10 active at any moment, `epoll_wait` returns 10 events — `poll` would scan all 10,000 every time. `epoll` also supports edge-triggered mode (notify only on state change) and one-shot mode.

**6. Explain the difference between blocking and non-blocking I/O.**

Blocking I/O: `read()` does not return until at least one byte is available (or EOF/error). The thread is suspended by the kernel in an interruptible sleep, consuming no CPU. Non-blocking I/O (`O_NONBLOCK`): `read()` returns immediately with `EAGAIN` if no data is ready. The application must use `poll`/`epoll` to know when to retry. Non-blocking enables single-threaded concurrent I/O — one thread can manage many connections by checking readiness and only reading when data is available. Blocking is simpler; non-blocking scales better.

**7. What is mmap and when is it faster than read()?**

`mmap` maps a file into the process's virtual address space. Access to mapped pages triggers page faults that load file pages on demand via the page cache. Advantages over `read()`: (a) zero-copy — data is in the page cache and directly accessible, no kernel→userspace copy; (b) random access is O(1) pointer arithmetic instead of `lseek`+`read`; (c) the OS's readahead and page eviction policies apply automatically. Faster for: large files with random access patterns, multiple processes reading the same file (they share page cache pages). Slower for: sequential streaming of small files (read() with large buffers is competitive) or network files (page faults are expensive over NFS).

**8. What is a zombie process and how do you prevent it?**

A zombie is a process that has exited but whose entry in the process table has not been reaped by the parent via `waitpid()`. The exit status must be stored somewhere until the parent collects it. Zombies consume a process table slot. Prevention: (a) call `waitpid()` (or `wait()`) in the parent after `fork()`; (b) install a `SIGCHLD` handler that calls `waitpid(-1, nullptr, WNOHANG)` in a loop to reap all exited children; (c) double-fork: grandchild is reparented to init, which auto-reaps.

**9. What is the difference between POSIX shared memory and pipes for IPC?**

Pipes are unidirectional byte streams with a kernel buffer (~64KB). They require a fork or pre-arrangement because both ends must be held simultaneously. Good for parent-child communication or shell pipelines. POSIX shared memory (`shm_open` + `mmap`) maps the same physical pages into multiple processes' address spaces. It is bidirectional, zero-copy once set up, and can be much larger. It requires explicit synchronization (semaphores, mutexes in shared memory). Choose pipes for simple streaming; shared memory for high-throughput, low-latency communication between coordinating processes.

**10. What is SO_REUSEADDR for sockets?**

Without `SO_REUSEADDR`, after a server closes a listening socket, the port stays in `TIME_WAIT` for ~2 minutes (the TCP MSL — Maximum Segment Lifetime). A new server process cannot bind the same port until `TIME_WAIT` expires. `SO_REUSEADDR` allows binding to a port still in `TIME_WAIT`, enabling fast server restarts. Set it before `bind()`. `SO_REUSEPORT` (different option) allows multiple sockets to bind the same port simultaneously — useful for multi-process servers with per-core accept loops.

**11. What is the difference between TCP and UDP at the socket level?**

TCP: `SOCK_STREAM`, connection-oriented. `connect()`/`accept()` before data. Guarantees ordered, reliable, flow-controlled byte stream. `send()`/`recv()` may transfer partial data — must loop. UDP: `SOCK_DGRAM`, connectionless. No `connect()` required; use `sendto()`/`recvfrom()` with explicit addresses. Each `recvfrom()` receives exactly one datagram. No delivery guarantee, no ordering, no flow control. UDP is faster for real-time traffic (audio, gaming, DNS) where retransmission would be worse than dropping.

**12. What is a signal handler and what functions are async-signal-safe?**

A signal handler is a function registered with `sigaction()` that the kernel calls asynchronously in the context of the signaled thread. The thread's state is interrupted at an arbitrary point — potentially in the middle of a library call holding an internal lock. Async-signal-safe functions are guaranteed safe to call from signal handlers: `write()`, `read()`, `_exit()`, `kill()`, `getpid()`, `sigprocmask()`, and a few others defined by POSIX. Unsafe (never call from handlers): `malloc()`, `free()`, `printf()`, `fopen()`, any `std::*`, `pthread_mutex_lock()`.

**13. What is io_uring and how does it differ from epoll?**

`io_uring` (Linux 5.1+) is an asynchronous I/O interface based on two shared ring buffers between kernel and userspace: submission queue (SQE) and completion queue (CQE). You post I/O operations to the SQ without syscalls; the kernel drains them asynchronously and posts results to the CQ. A single `io_uring_enter()` syscall can submit batches and wait for completions. Benefits over epoll: (a) true async — no read-after-poll roundtrip; (b) batch submissions reduce syscall count; (c) `IORING_SETUP_SQPOLL` can eliminate syscalls entirely. Used in high-performance servers and databases (PostgreSQL async work).

**14. What is memory-mapped I/O vs DMA?**

Memory-mapped I/O (MMIO): device registers are mapped into the CPU's address space. The CPU reads/writes device registers using regular load/store instructions at specific physical addresses. Volatile reads/writes prevent compiler optimization. DMA (Direct Memory Access): the CPU programs a DMA controller with source/destination addresses and size, then the DMA engine transfers data between device and RAM independently, interrupting the CPU when done. CPU doesn't touch each byte — it's freed for other work. Modern devices use DMA for data transfers and MMIO for control registers.

**15. What is the difference between process-level and thread-level address space?**

A process has its own virtual address space — page tables, heap, stack, code, file descriptor table. Forking creates a copy of all these (page tables COW). Threads within a process share the same address space — same heap, same code, same file descriptors, same global variables. Each thread has its own stack (typically 8MB) and CPU registers. Thread creation (`pthread_create`) is faster than `fork` (no page table clone). The trade-off: thread bugs (data races, use-after-free of shared state) can corrupt the entire process; process bugs are isolated.

---

## CUDA

**1. Explain warp divergence and its cost.**

A warp is 32 threads that execute the same instruction simultaneously (SIMT). When threads in a warp take different branches of an `if` statement, the GPU executes both branches — disabling (masking out) the threads not taking each branch. If half the warp takes the true branch and half the false branch, execution width is halved for each branch — effectively serializing them. Cost: proportional to the number of distinct paths taken by threads in the warp. Fix: ensure all threads in a warp (consecutive thread indices for 1D data) take the same path — align branch conditions to warp boundaries.

**2. How do you choose block size?**

Rules of thumb: (a) must be a multiple of 32 (warp size) — avoid partial warps; (b) 128–256 is a common starting point; (c) use enough threads to hide latency (latency hiding needs enough active warps per SM — target > 50% occupancy); (d) consider shared memory: more threads per block means more shared memory usage per SM, possibly limiting how many blocks fit; (e) use `cudaOccupancyMaxPotentialBlockSize()` to let the runtime compute the optimal size for a given kernel. Profile with Nsight Compute.

**3. What is a bank conflict in shared memory?**

Shared memory is divided into 32 banks (for 32-bit words), each serving one thread per cycle. If multiple threads in a warp access different addresses that map to the same bank, their accesses are serialized — each conflict adds one cycle. Worst case: all 32 threads access the same bank — 32-way conflict, 32× slower. Fix: pad shared memory arrays to avoid stride = multiple of 32. Common pitfall: `__shared__ float tile[32][32]` with column-major access — every row access hits bank 0. Fix: `__shared__ float tile[32][33]` — padding shifts columns to different banks.

**4. What is occupancy in CUDA?**

Occupancy is the ratio of active warps per SM to the maximum possible active warps per SM. Higher occupancy helps the GPU hide memory latency (when one warp stalls on a memory access, the SM executes another warp). It is limited by: (a) registers per thread (more registers → fewer concurrent threads → fewer warps per SM), (b) shared memory per block (more shmem → fewer blocks per SM), (c) block size (very large blocks limit how many blocks fit per SM). 100% occupancy is not always optimal — sometimes fewer threads with more registers and better cache reuse wins.

**5. When does unified memory hurt performance?**

Unified memory (`cudaMallocManaged`) migrates pages on demand between CPU and GPU. Page faults on the GPU are expensive — ~microseconds each. Hurts when: (a) random access pattern causes many small page faults instead of a few large coalesced transfers; (b) CPU and GPU access the same data simultaneously, causing page-fault ping-pong; (c) the first kernel run after CPU writes is slow due to migration. Fix: `cudaMemPrefetchAsync` to proactively migrate data before the kernel runs. Or use explicit `cudaMemcpy` with pinned memory for predictable performance.

**6. What are CUDA streams?**

A CUDA stream is a queue of GPU operations (kernels, memcpies) that execute in order relative to each other. Operations in different streams can overlap — compute and data transfer, or multiple kernels. Enables pipelining: while kernel N runs on GPU, transfer for kernel N+1 happens on a separate DMA engine. The default stream (stream 0) is synchronizing — it waits for all other streams. Use multiple streams with `cudaMemcpyAsync` and pinned host memory to overlap H↔D transfers with computation.

**7. Explain CUDA's memory hierarchy with latencies.**

Registers (~1 cycle) → Shared memory / L1 cache (~5–10 cycles, ~5 TB/s) → L2 cache (~200 cycles, ~3 TB/s) → Global memory / HBM (~500 cycles, 700–2000 GB/s) → Pinned host memory via PCIe (~3000 cycles, ~25 GB/s). Use shared memory as a manually managed L1 for data reused within a block. Load a tile into shared memory with a coalesced read, synchronize, process from shared memory. This is the tiled matrix multiply pattern.

**8. What is the difference between `__global__`, `__device__`, and `__host__`?**

`__global__`: entry point for GPU kernels. Called from the host using `<<<>>>` syntax. Returns `void`. Runs on the device. `__device__`: called from device code only (from within a kernel or another `__device__` function). Cannot be called from host. Inlined by default. `__host__`: called from host code only. This is the default for any C++ function — the specifier is mostly used in combination: `__host__ __device__` compiles the function for both host and device (compiled twice, different code paths possible with `#ifdef __CUDA_ARCH__`).

**9. What is coalesced memory access?**

When consecutive threads in a warp access consecutive addresses in global memory, the hardware combines them into one (or a few) wide memory transaction — a coalesced access. For 32 threads accessing consecutive 4-byte floats, this is one 128-byte transaction (one cache line). Non-coalesced: threads access scattered addresses — each may require a separate transaction — potentially 32 transactions instead of 1. 32× bandwidth waste. Rule: thread index should correspond to the innermost (fastest-varying) dimension of your data.

**10. What is a CUDA graph?**

A CUDA graph captures a sequence of GPU operations (kernels, memcpies, events) and their dependencies as a graph, then submits the whole graph as a single unit. Benefits: (a) eliminates per-operation CPU launch overhead (reduces kernel launch latency from ~5μs to ~1μs for repeated launches); (b) the driver can optimize the whole graph at "instantiation" time (DMA scheduling, kernel merging); (c) ideal for fixed-structure pipelines that run thousands of times (inference server). Capture: record operations between `cudaStreamBeginCapture` and `cudaStreamEndCapture`, then `cudaGraphInstantiate` + `cudaGraphLaunch`.

---

## ROS2

**1. Topics vs services vs actions — when to use each?**

Topics (pub/sub) for continuous, one-directional data streams — sensor readings, state estimates, control commands. No response expected. Services (request/response) for discrete, synchronous queries — configuration changes, status requests, one-shot computations. Blocks the caller until response. Actions for long-running tasks where you want periodic feedback, cancellation, and a final result — navigation goals, arm trajectories, task execution. Actions are built on topics and services internally.

**2. What is DDS and why does ROS2 use it instead of ROS1's rosmaster?**

DDS (Data Distribution Service) is an industry-standard publish-subscribe middleware. ROS2 replaced the centralized rosmaster with DDS because: (a) no single point of failure — nodes discover each other peer-to-peer via multicast; (b) built-in QoS — reliability, durability, deadline, liveliness; (c) real-time support — DDS implementations (FastDDS, CycloneDDS, Connext) are certified for real-time use; (d) industry adoption — already used in aerospace, defense, automotive; (e) security — DDS-Security standard supports authentication, encryption.

**3. Transient_local durability — what problem does it solve?**

When a publisher has `DURABILITY = TRANSIENT_LOCAL`, it stores the last N published messages (history depth). A late-joining subscriber with matching `TRANSIENT_LOCAL` durability receives those stored messages immediately upon connection. This solves the problem of nodes that publish once at startup (static map, initial transforms, configuration) — late subscribers get the data even if they started after the publisher. Without it, late subscribers miss the initial publication.

**4. How does intra-process communication work in ROS2?**

When publisher and subscriber are in the same process, ROS2 can bypass serialization and DDS entirely. The publisher publishes a `unique_ptr<MsgType>` — ownership is transferred directly to the subscriber callback without copying or serializing. The subscriber receives the message by pointer. This requires: both nodes in the same executor, matching QoS, `create_publisher` and `create_subscription` with the same topic and the correct `rclcpp::IntraProcessCommunicationPolicy`. Latency drops from microseconds (inter-process DDS) to nanoseconds (pointer hand-off).

**5. What is a lifecycle node?**

A lifecycle node (`rclcpp_lifecycle::LifecycleNode`) has managed transitions between states: Unconfigured → Configured → Active → Inactive → Unconfigured, plus Finalized. Transition callbacks (`on_configure`, `on_activate`, `on_deactivate`, `on_cleanup`, `on_shutdown`) let the node allocate/deallocate resources at the right time. Benefits: ordered system bring-up (configure all nodes, then activate them in order), controlled shutdown, ability to deactivate without destroying (useful for hot reconfiguration). Required by `ros2_control` and `nav2`.

**6. SingleThreadedExecutor vs MultiThreadedExecutor — when to use each?**

SingleThreadedExecutor runs all callbacks sequentially in one thread — simple, no concurrency issues within callbacks, but a slow callback blocks others. MultiThreadedExecutor uses a thread pool — callbacks can run concurrently, but you must protect shared state with mutexes. Use MultiThreaded when: you have independent callbacks that can safely run in parallel, or when one callback is slow (e.g., image processing) and should not block navigation callbacks. Use SingleThreaded when callbacks share state (simpler) or when real-time determinism is needed.

**7. What is a composable node?**

A composable node is a ROS2 node compiled as a shared library that can be loaded into a container process at runtime. Instead of spawning a separate process per node (OS overhead, IPC latency), multiple composable nodes share one process. Intra-process communication becomes available between all composable nodes in the same container. The container manages the executor. Composable nodes can also be unloaded and reloaded without restarting the container.

**8. What is a QoS mismatch and why is it silent?**

A QoS mismatch occurs when a publisher's and subscriber's QoS settings are incompatible — e.g., publisher is BEST_EFFORT, subscriber requires RELIABLE. DDS cannot guarantee delivery from a best-effort source to a reliable sink, so the connection is simply not made. No error is thrown; no warning is printed by default. The subscriber receives nothing and the user wonders why. Detection: `ros2 topic info /topic --verbose` shows the QoS of each publisher and subscriber, and `offered_incompatible_qos` / `requested_incompatible_qos` events in rmw callbacks.

**9. How does AMCL work?**

Adaptive Monte Carlo Localization uses a particle filter. Particles represent hypotheses about the robot's pose. At each step: (1) Prediction — move each particle according to the odometry model with added noise; (2) Update — weight each particle by how well its expected laser scan (from the map at that pose) matches the actual scan; (3) Resample — draw new particles proportional to their weight (higher-weight poses survive). "Adaptive" means the number of particles adjusts based on how confident the filter is — fewer when localized well, more when uncertain.

**10. What is ros2_control?**

`ros2_control` is a hardware abstraction and controller management framework. It defines: `HardwareInterface` (reads state, writes commands to hardware), `Controller` (runs at fixed rate, reads state, computes commands), and `ControllerManager` (loads/unloads controllers, manages the real-time loop). Controllers and hardware interfaces are composable plugins. Examples: `JointTrajectoryController`, `DiffDriveController`, `ForwardCommandController`. Lifecycle-managed — activate/deactivate controllers without stopping the hardware interface.

---

## AI Inference

**1. ONNX Runtime vs TensorRT — when to choose each?**

Choose ONNX Runtime when: you need portability (runs on CPU, GPU, various hardware with ExecutionProvider plugins), you want framework-agnostic deployment, or build time is a constraint. Choose TensorRT when: you target NVIDIA GPUs in production and need maximum throughput/minimum latency. TensorRT's offline build phase generates a highly optimized engine (kernel fusion, precision calibration, layer elimination). TensorRT is typically 2–5× faster than ONNX Runtime on GPU for the same model.

**2. What is INT8 quantization?**

INT8 quantization converts model weights and activations from FP32 (or FP16) to signed 8-bit integers. Each tensor is represented as `float = int8 * scale + zero_point`. This reduces memory footprint 4× vs FP32, enables vectorized 8-bit integer arithmetic (256-bit AVX2 processes 32 int8s vs 8 floats), and fits more data in cache. Modern GPU tensor cores execute INT8 at 2× or 4× the throughput of FP16. Accuracy impact: typically <1% top-1 accuracy drop for classification if properly calibrated.

**3. FP16 vs INT8 trade-offs?**

FP16: dynamic range of float (crucial for attention layers, batch normalization), straightforward to use (most models run correctly with `--fp16`), 2× memory vs FP32, ~2× throughput on tensor cores. INT8: 4× memory reduction vs FP32, ~4× throughput on tensor cores, but requires calibration to determine per-tensor scales. Some layers (especially transformers) are sensitive to INT8 — may require keeping certain layers in FP16 (mixed precision). INT8 is preferred for inference throughput; FP16 is the safer default.

**4. What is calibration in post-training quantization?**

INT8 quantization maps the full float range of a tensor to 128 discrete values. If the float range is too large (outliers), most values cluster in a few INT8 bins — losing precision. Calibration determines the optimal scale: run representative input data through the model, collect activation statistics (min/max, or KL-divergence of histograms), choose the float range that minimizes quantization error. TensorRT supports `EntropyCalibrator2` (KL divergence) and `MinMaxCalibrator`. A calibration dataset of ~500–1000 images is typically sufficient.

**5. What is kernel fusion in TensorRT?**

Kernel fusion combines multiple GPU operations (e.g., convolution + bias add + ReLU) into a single CUDA kernel. Without fusion, each operation launches a separate kernel — data is written to HBM between operations. With fusion, intermediate results stay in registers or L1 cache. Benefits: fewer kernel launch overheads, reduced HBM bandwidth. TensorRT's engine builder analyzes the computational graph and automatically fuses eligible subgraphs. Fusion is why TensorRT engines are substantially faster than layer-by-layer execution.

**6. How does dynamic batching work?**

Dynamic batching accumulates requests from multiple clients until either a timeout or a batch size limit is reached, then runs inference on the batch. Since GPU compute is parallelized across the batch dimension, a batch of 8 requests takes only slightly longer than a batch of 1. Triton Inference Server implements dynamic batching: `max_queue_delay_microseconds` controls the timeout, `preferred_batch_size` the target. Important: the model must be exported with dynamic batch dimension (ONNX `dynamic_axes`, TensorRT `OptimizationProfile`).

**7. TorchScript vs ONNX export — which to prefer?**

TorchScript: serializes a PyTorch model as a graph of PyTorch ops. Self-contained — no Python needed at runtime. Integrates with `torch::jit::load()` in C++. Limitation: can only run via LibTorch; less portable. ONNX: open standard representing the model as a graph of standardized operators. Portable to TensorRT, ONNX Runtime, CoreML, OpenVINO. Use ONNX when: you need multi-framework deployment or TensorRT acceleration. Use TorchScript when: you want to stay in the PyTorch ecosystem (C++ LibTorch) without conversion overhead.

**8. What is the TensorRT build phase?**

The TensorRT build phase (offline) takes an ONNX model and a target GPU's capabilities, runs layer profiling (benchmarks all candidate kernel implementations for each layer), performs fusion and optimization, selects the fastest kernels for the calibrated precision, and serializes the result as an opaque engine file. This takes minutes to hours for large models. The runtime phase (online) deserializes the engine and runs inference — extremely fast, no overhead from the build. Engines are GPU-specific and must be rebuilt for different GPU architectures.

**9. How do you validate accuracy after quantization?**

Compare outputs of the FP32 model and quantized model on a validation dataset. Metrics: (a) task-level (classification: top-1/top-5 accuracy; detection: mAP; segmentation: mIoU) — should degrade by less than 1%; (b) output-level (cosine similarity, max absolute difference of output tensors) — sanity check; (c) per-layer comparison using TensorRT's `INetworkDefinition` layer profiling to identify which layer contributes most quantization error. If degradation is too high: keep problematic layers in FP16 with `setLayerPrecision`, or use QAT.

**10. What is the difference between PTQ and QAT?**

Post-Training Quantization (PTQ): quantize an already-trained FP32 model without retraining. Fast and easy. Calibrate with a small dataset. Accuracy may degrade for sensitive models. Quantization-Aware Training (QAT): simulate quantization during training by inserting fake quantization nodes (fake quantize ops) into the training graph. The model learns to be robust to quantization error. Substantially better accuracy for models where PTQ degrades noticeably (transformers, segmentation). Cost: requires retraining (or fine-tuning) for hundreds of epochs. Use PTQ first; move to QAT if accuracy is insufficient.
````

---

### Step 2.3 — Write `system-design.md`

- [ ] Create `tutorial/interview-master/system-design.md` with the following content:

````markdown
# C++ System Design

*Forge's 15 system design scenarios — each with design decisions, implementation sketch, and trade-offs.*

---

## 1. Lock-Free SPSC Queue

**Problem:** Design a single-producer, single-consumer queue with no mutexes, suitable for inter-thread communication in a real-time system. Maximum throughput, minimum latency.

**Key Design Decisions:**
- Ring buffer with power-of-2 capacity — modulo via bitwise AND (`& (N-1)`)
- `head` owned by consumer, `tail` owned by producer — no sharing of the same atomic
- `alignas(64)` on head and tail — prevent false sharing
- Producer: load `head` with acquire, write element, store `tail` with release
- Consumer: load `tail` with acquire, read element, store `head` with release
- Fixed capacity — no dynamic allocation in hot path

**Implementation Sketch:**
```cpp
template<typename T, std::size_t N>
class SPSCQueue {
    static_assert((N & (N-1)) == 0);
    std::array<T, N> buf_;
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
public:
    bool push(T val) {
        auto t = tail_.load(std::memory_order_relaxed);
        auto next = (t + 1) & (N - 1);
        if (next == head_.load(std::memory_order_acquire)) return false;
        buf_[t] = std::move(val);
        tail_.store(next, std::memory_order_release);
        return true;
    }
    bool pop(T& out) {
        auto h = head_.load(std::memory_order_relaxed);
        if (h == tail_.load(std::memory_order_acquire)) return false;
        out = std::move(buf_[h]);
        head_.store((h + 1) & (N - 1), std::memory_order_release);
        return true;
    }
};
```

**Trade-offs:**
- Power-of-2 capacity wastes memory if exact size is needed; use modulo for arbitrary sizes at a small cost.
- SPSC only — adding a second producer or consumer requires a different algorithm (MPMC with CAS loops).
- Spin-waiting on empty/full burns CPU — add exponential backoff or parking/futex signaling for low-throughput cases.

---

## 2. C++ Plugin System

**Problem:** Design a plugin system that loads shared libraries at runtime, instantiates typed plugin objects, and handles versioning and ABI stability.

**Key Design Decisions:**
- `dlopen` / `dlsym` for loading; extern "C" factory functions to avoid name mangling
- Type erasure behind a stable C-compatible interface struct (function pointers)
- Version field in the interface struct — check at load time
- Separate plugin API header from implementation (only the header is ABI-stable)
- `RTLD_LOCAL` to prevent symbol conflicts between plugins

**Implementation Sketch:**
```cpp
// plugin_api.h — stable ABI
struct PluginInterface {
    uint32_t version;                // must match PLUGIN_API_VERSION
    const char* name;
    void* (*create)();               // factory
    void  (*destroy)(void* instance);
    void  (*process)(void*, const uint8_t*, size_t);
};
extern "C" PluginInterface* get_plugin_interface();

// plugin_loader.cpp
struct Plugin {
    void* handle;
    PluginInterface* iface;
    void* instance;
};

Plugin load_plugin(const std::string& path) {
    void* h = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!h) throw std::runtime_error(dlerror());
    auto* get = reinterpret_cast<PluginInterface*(*)()>(dlsym(h, "get_plugin_interface"));
    auto* iface = get();
    if (iface->version != PLUGIN_API_VERSION) throw std::runtime_error("version mismatch");
    return {h, iface, iface->create()};
}
void unload_plugin(Plugin& p) {
    p.iface->destroy(p.instance);
    dlclose(p.handle);
}
```

**Trade-offs:**
- `extern "C"` factories break with templates/overloads — plugin types must be identified by strings, not C++ types.
- `dlclose` is dangerous if any code still holds a pointer into the library — set pointers to null first.
- Version numbers must be bumped on any ABI-breaking change (adding a field to the interface struct breaks binary compatibility).

---

## 3. Real-Time Robot Controller at 1kHz

**Problem:** Design a 1kHz control loop for a robot joint controller with <500μs worst-case deadline, no dynamic allocation, and no priority inversion.

**Key Design Decisions:**
- Dedicated RT thread with `SCHED_FIFO` at high priority (`pthread_setschedparam`)
- Lock-free SPSC queue for state from sensor thread to controller, commands from controller to driver
- All memory pre-allocated at init — no `new`/`malloc` in loop; use object pools
- `mlockall(MCL_CURRENT | MCL_FUTURE)` to prevent page faults
- `clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_wakeup)` for precise timing

**Implementation Sketch:**
```cpp
void control_loop(SharedState* state) {
    mlockall(MCL_CURRENT | MCL_FUTURE);
    set_thread_priority(SCHED_FIFO, 90);

    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);
    const long period_ns = 1'000'000;  // 1ms

    while (running) {
        // Read sensor state (lock-free SPSC pop)
        SensorData sensors;
        sensor_queue.pop(sensors);

        // Compute control output (no allocations)
        ControlOutput cmd = pid_compute(sensors, setpoint_);

        // Write to actuator (lock-free SPSC push)
        cmd_queue.push(cmd);

        // Advance next wakeup
        next.tv_nsec += period_ns;
        if (next.tv_nsec >= 1'000'000'000L) { next.tv_sec++; next.tv_nsec -= 1'000'000'000L; }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, nullptr);
    }
}
```

**Trade-offs:**
- `SCHED_FIFO` can starve non-RT threads — assign carefully; use `SCHED_RR` or watchdog if needed.
- SPSC queues decouple the RT thread from non-RT code; if the sensor thread misses, the controller uses stale data — detect with timestamps.
- On Linux, avoid `std::cout`, `malloc`, file I/O in the RT thread — use async logging.

---

## 4. GPU Inference Pipeline for 10ms Latency

**Problem:** Deploy a neural network for object detection with end-to-end latency < 10ms at 30 FPS, on a single NVIDIA GPU, using TensorRT.

**Key Design Decisions:**
- Three CUDA streams: transfer-in, compute, transfer-out (triple-buffering)
- Pinned (page-locked) host memory for H2D and D2H transfers
- TensorRT engine built with FP16 (or INT8 for 4× throughput) and dynamic batch size
- Dynamic batching: accumulate up to max_batch over max_delay_ms, then flush
- CUDA events for cross-stream synchronization and timing

**Implementation Sketch:**
```cpp
// Pipeline stage per frame
struct InferContext {
    float* pinned_input;   // cudaMallocHost
    float* d_input;        // cudaMalloc
    float* d_output;
    float* pinned_output;
    cudaStream_t stream;
    cudaEvent_t input_ready, infer_done;
};

// Frame N:
// stream[0]: cudaMemcpyAsync(host→d_input)  → record input_ready
// stream[1]: cudaStreamWaitEvent(input_ready) → enqueueV2() → record infer_done
// stream[2]: cudaStreamWaitEvent(infer_done) → cudaMemcpyAsync(d_output→host) → process results
// While stream[1] infers frame N, stream[0] transfers frame N+1
```

**Trade-offs:**
- Three-buffer pipeline adds 2 frames of latency (2 × 33ms at 30fps) — unacceptable for control; use single-buffer for minimum latency at the cost of throughput.
- TensorRT engine build takes minutes — serialize and cache engine files; rebuild only on GPU capability change or model update.
- Dynamic batch size requires `OptimizationProfile` with min/opt/max shapes — profile to find the sweet spot.

---

## 5. Memory Allocator for a Game Engine

**Problem:** Design a multi-tier memory allocator for a game engine: per-frame scratch memory, per-system pool allocators, and a general heap with minimal fragmentation.

**Key Design Decisions:**
- Frame allocator (monotonic): bump pointer, reset at frame end. Zero deallocation cost.
- Pool allocator: fixed-size blocks per type (e.g., 64-byte for particles, 256-byte for entities). O(1) alloc/free via free list.
- General heap: `dlmalloc` / `jemalloc` for variable-size, rare allocations.
- All allocators are aware of alignment requirements (SIMD needs 16/32-byte alignment).

**Implementation Sketch:**
```cpp
struct FrameAllocator {
    std::byte* base;
    std::size_t offset = 0, capacity;

    void* allocate(std::size_t size, std::size_t align) {
        std::size_t aligned = (offset + align - 1) & ~(align - 1);
        if (aligned + size > capacity) throw std::bad_alloc();
        offset = aligned + size;
        return base + aligned;
    }
    void reset() { offset = 0; }
};

struct PoolAllocator {
    struct Block { Block* next; };
    Block* free_list = nullptr;
    std::size_t block_size;

    void* allocate() {
        if (!free_list) grow();  // allocate a slab from system
        Block* b = free_list;
        free_list = b->next;
        return b;
    }
    void deallocate(void* p) {
        auto* b = static_cast<Block*>(p);
        b->next = free_list;
        free_list = b;
    }
};
```

**Trade-offs:**
- Frame allocator requires all frame allocations to have lifetimes shorter than one frame — cross-frame references cause dangling pointers.
- Pool allocator wastes memory if block sizes are poorly chosen — profile allocation sizes before setting pool sizes.
- Mixing allocators: objects from a pool must be returned to the same pool — avoid accidental deletion via global `delete`.

---

## 6. Event-Driven Server for 100k Connections

**Problem:** Design a TCP server handling 100,000 simultaneous connections with low latency and high throughput on a Linux server.

**Key Design Decisions:**
- `epoll` in edge-triggered mode — O(1) per ready event regardless of connection count
- Non-blocking sockets (`O_NONBLOCK | O_CLOEXEC` at accept4)
- Thread pool: N threads (= CPU count), each runs its own epoll loop (per-thread epoll + `SO_REUSEPORT`)
- Zero-copy send via `sendfile()` for file serving
- Fixed-size receive buffers per connection — pre-allocated slab, no per-connection malloc on each read

**Implementation Sketch:**
```cpp
// Per thread:
int epfd = epoll_create1(0);
// SO_REUSEPORT: each thread has its own listen socket on same port
int lfd = create_listen_socket(port);
epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &listen_ev);

while (true) {
    int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
    for (int i = 0; i < n; i++) {
        if (events[i].data.fd == lfd) {
            int cfd = accept4(lfd, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
            connections[cfd] = new_connection(cfd);
            epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &client_ev(EPOLLIN | EPOLLET));
        } else {
            handle_client(events[i].data.fd, events[i].events);
        }
    }
}

void handle_client(int fd, uint32_t events) {
    // Edge-triggered: read until EAGAIN
    while (true) {
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n <= 0) { if (errno == EAGAIN) break; close_connection(fd); return; }
        process_data(fd, buf, n);
    }
}
```

**Trade-offs:**
- `SO_REUSEPORT` distributes connections across threads at the kernel level — no user-space load balancing needed, but connections to one thread can queue up if imbalanced.
- Edge-triggered requires reading until EAGAIN — partial protocol frames must be buffered per connection.
- For very high connection counts, memory for per-connection state dominates — pre-allocate a connection pool.

---

## 7. ROS2 Autonomous Vehicle System

**Problem:** Design the ROS2 node graph and architecture for an autonomous vehicle with LiDAR, camera, GPS, IMU, and a path planner.

**Key Design Decisions:**
- Sensor nodes: `sensor_data` QoS (BEST_EFFORT, volatile) for LiDAR/camera; `reliable` for GPS/IMU
- Sensor fusion node: subscribes to all sensors, publishes `/odometry` and `/localization`
- Nav2 stack: map server (TRANSIENT_LOCAL for map), AMCL for localization, planner, controller
- ros2_control for joint/velocity control of actuators
- Lifecycle ordering: hardware interfaces → sensor nodes → fusion → planner → controller

**Architecture Sketch:**
```
[LiDAR Node]  →  /scan  (sensor_data QoS)
[Camera Node] →  /camera/image (sensor_data QoS)
[IMU Node]    →  /imu/data (reliable QoS)
[GPS Node]    →  /gps/fix (reliable QoS)
      ↓
[Sensor Fusion Node] → /odometry, /tf (odom→base_link)
      ↓
[AMCL Node]   → /tf (map→odom), /particle_cloud
      ↓
[Nav2 Planner] → /plan
      ↓
[Nav2 Controller] → /cmd_vel
      ↓
[ros2_control DiffDrive] → /wheel_velocities
```

**Trade-offs:**
- Sensor QoS BEST_EFFORT risks dropped packets — for safety-critical systems, use RELIABLE with higher depth; profile bandwidth first.
- Multi-node latency from perception to control can exceed 100ms — use composable nodes in one process to eliminate inter-process DDS serialization for the critical path.
- tf2 broadcasting: high-rate transforms (100Hz) should use a dedicated broadcaster, not topic pub/sub — avoid QoS incompatibility with existing tf2 listeners.

---

## 8. Safe Embedded RTOS Task

**Problem:** Design a safe RTOS task for a safety-critical embedded system (FreeRTOS / Zephyr) that reads CAN bus messages, runs a state machine, and commands actuators.

**Key Design Decisions:**
- Task stack size computed from worst-case call depth: `configSTACK_DEPTH_TYPE` analysis + 20% margin
- ISR to task communication via RTOS queue (FreeRTOS xQueueSendFromISR) — no shared globals
- Task priorities: ISR handler task highest, state machine mid, logging lowest (avoid priority inversion)
- Watchdog task pings hardware watchdog — if any critical task stops feeding, system resets
- All allocations at init time — no dynamic allocation after boot (MPU region locks)

**Implementation Sketch:**
```c
// ISR: receives CAN frame, sends to queue
void CAN_IRQHandler(void) {
    CANFrame frame;
    CAN_receive(&frame);
    BaseType_t woken = pdFALSE;
    xQueueSendFromISR(can_rx_queue, &frame, &woken);
    portYIELD_FROM_ISR(woken);
}

// Control task: fixed rate via vTaskDelayUntil
void control_task(void* pvParam) {
    TickType_t last_wake = xTaskGetTickCount();
    for (;;) {
        CANFrame f;
        if (xQueueReceive(can_rx_queue, &f, 0) == pdTRUE) {
            state_machine_update(&sm, &f);
        }
        actuator_command_t cmd = state_machine_get_output(&sm);
        actuator_set(cmd);
        watchdog_feed();
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));  // 100Hz
    }
}
```

**Trade-offs:**
- ISR queues decouple ISR and task — queue overflow means dropped frames; size queue for worst-case burst.
- `vTaskDelayUntil` gives fixed-rate scheduling; jitter depends on task scheduling and interrupt latency.
- No C++ exceptions or RTTI in embedded — compile with `-fno-exceptions -fno-rtti`.

---

## 9. High-Performance Lock-Free Logger

**Problem:** Design a logger for a high-frequency trading system: submitting a log entry must take < 100ns, must not allocate heap memory, and must not block on disk I/O.

**Key Design Decisions:**
- SPSC ring buffer (or MPSC for multi-thread): producers write log records; background thread drains to file
- Log records are fixed-size structs (no strings — use an enum code + numeric fields, decode offline)
- Producers: claim a slot with an atomic fetch_add on tail, write fields, mark slot "ready" with a release store
- Background flusher: reads ready slots, formats to file with a single `write()` call per batch
- No heap allocation in hot path: records are stack-local, copied into ring buffer

**Implementation Sketch:**
```cpp
struct LogRecord {
    uint64_t timestamp_ns;
    uint32_t event_code;
    uint64_t value1, value2;
    bool ready;  // producer marks true when fields written
};

static std::array<LogRecord, 65536> ring;
static std::atomic<uint64_t> prod_tail{0};

void log_event(uint32_t code, uint64_t v1, uint64_t v2) {
    uint64_t slot = prod_tail.fetch_add(1, std::memory_order_relaxed) & 0xffff;
    ring[slot].timestamp_ns = rdtsc();  // or clock_gettime
    ring[slot].event_code = code;
    ring[slot].value1 = v1;
    ring[slot].value2 = v2;
    std::atomic_thread_fence(std::memory_order_release);
    ring[slot].ready = true;  // release — flusher sees complete record
}
// Flusher thread: scans from cons_head until !ready, writes batch to file
```

**Trade-offs:**
- Ring buffer overflow (producer laps consumer) drops oldest records silently — monitor lag.
- Fixed-size records limit expressiveness — embed a uint8_t[32] payload for variable-length data if needed.
- Single-consumer flusher is simple; for MPSC, use a lock or a per-thread ring with a aggregator.

---

## 10. Type-Safe Unit System

**Problem:** Design a compile-time unit system (meters, seconds, newtons, etc.) that prevents adding apples to oranges, enables automatic unit conversion, and has zero runtime overhead.

**Key Design Decisions:**
- StrongType template parameterized by quantity tag and scale (as std::ratio)
- Operator overloading: `Meters + Meters = Meters`, `Meters / Seconds = MetersPerSecond`
- Compile-time conversion: `to<NewUnit>(value)` checks ratio compatibility, applies scale factor constexpr
- UDLs: `5.0_km`, `9.81_m_s2`

**Implementation Sketch:**
```cpp
template<typename Tag, typename Ratio = std::ratio<1>>
struct Quantity {
    double value;
    explicit constexpr Quantity(double v) : value(v) {}
    constexpr Quantity operator+(Quantity o) const { return Quantity{value + o.value}; }
    constexpr bool operator==(Quantity o) const { return value == o.value; }
};

struct MetersTag {};
struct SecondsTag {};
using Meters = Quantity<MetersTag, std::ratio<1>>;
using Kilometers = Quantity<MetersTag, std::kilo>;  // 1km = 1000m

template<typename To, typename From>
constexpr To unit_cast(From f) {
    static_assert(std::is_same_v<typename To::tag, typename From::tag>,
        "Cannot convert between different physical quantities");
    constexpr double factor = (double)From::ratio::num / From::ratio::den
                            * (double)To::ratio::den   / To::ratio::num;
    return To{f.value * factor};
}
// unit_cast<Meters>(Kilometers{5.0}).value == 5000.0
```

**Trade-offs:**
- Floating-point scales introduce rounding errors for fractional ratios — use integer arithmetic or rational arithmetic for exact conversions.
- Derived units (m/s, kg⋅m/s²) require dimensional analysis tracking (Boost.Units does this) — a full implementation needs rational exponent vectors per base dimension.
- Zero overhead: all computations are constexpr or inlined; no vtable, no dynamic dispatch.

---

## 11. Hot-Reload Plugin System

**Problem:** Design a plugin system where plugins can be updated and reloaded at runtime without restarting the host application.

**Key Design Decisions:**
- Plugins compiled as shared libraries (`.so`) with extern "C" factory/destroy
- Hot-reload: detect file change (inotify or timestamp poll), quiesce plugin (drain in-flight requests), unload, reload new version
- Reference counting on plugin instances — do not `dlclose` while any instance is alive
- Symbol stability: use versioned interface structs; add fields at the end only
- ABI: use C-compatible types in plugin API; no C++ stdlib types across the boundary

**Implementation Sketch:**
```cpp
struct PluginHandle {
    void* dl_handle;
    std::atomic<int> ref_count{0};
    PluginInterface* iface;
    std::string path;

    void retain()  { ref_count.fetch_add(1, std::memory_order_relaxed); }
    void release() {
        if (ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            iface->shutdown();
            dlclose(dl_handle);
            delete this;
        }
    }
};

void reload_plugin(PluginHandle*& handle) {
    auto* old = handle;
    // Load new version
    auto* fresh = load_plugin(old->path);
    // Swap atomically — new requests get fresh, old completes
    handle = fresh;
    old->release();  // decrement; dlclose when last user done
}
```

**Trade-offs:**
- `dlclose` is not always safe — static destructors in the plugin run, which may access stale memory if threads still run plugin code.
- ABI instability: adding a virtual function to a base class breaks all plugins compiled against the old header.
- Two-version coexistence: briefly both old and new plugin code is loaded — ensure they do not share writable state (global variables in the .so).

---

## 12. Scatter-Gather I/O

**Problem:** Design a network send path that assembles multi-part protocol frames (header, payload, checksum) without copying all parts into a contiguous buffer.

**Key Design Decisions:**
- `struct iovec { void* iov_base; size_t iov_len; }` — array of buffer descriptors
- `writev(fd, iov, iovcnt)` — single syscall sends all parts; kernel assembles into TCP segments
- For sockets: `sendmsg(fd, &msghdr, flags)` — also carries ancillary data (file descriptors, credentials)
- Respect `IOV_MAX` (typically 1024) — split if more parts needed

**Implementation Sketch:**
```cpp
bool send_frame(int sock, const FrameHeader& hdr, const uint8_t* payload, size_t plen) {
    uint32_t checksum = compute_crc32(payload, plen);

    struct iovec iov[3];
    iov[0].iov_base = const_cast<FrameHeader*>(&hdr);
    iov[0].iov_len  = sizeof(hdr);
    iov[1].iov_base = const_cast<uint8_t*>(payload);
    iov[1].iov_len  = plen;
    iov[2].iov_base = &checksum;
    iov[2].iov_len  = sizeof(checksum);

    ssize_t total = sizeof(hdr) + plen + sizeof(checksum);
    ssize_t sent = writev(sock, iov, 3);
    return sent == total;  // handle partial sends in production
}
```

**Trade-offs:**
- Partial write: `writev` may return less than the total — must reconstruct an `iovec` array from the remaining bytes and retry.
- For performance-critical paths, a single `sendmsg` with MSG_ZEROCOPY can avoid copying data to the kernel buffer (Linux 4.14+, requires pinned user pages).
- `IOV_MAX` limit: split large iovec arrays into multiple calls, paying one syscall per batch.

---

## 13. Entity Component System (ECS)

**Problem:** Design an ECS for a game engine: entities are IDs, components are data, systems iterate over components with maximum cache efficiency.

**Key Design Decisions:**
- Archetype-based storage: entities with the same component signature share a contiguous array of component structs
- Component lookup: sparse set mapping entity ID → archetype + index within archetype
- System iteration: linear scan over a single archetype's component arrays — CPU prefetcher friendly
- No virtual dispatch in hot path — systems are templated functors or lambdas

**Implementation Sketch:**
```cpp
// Archetype: contiguous arrays per component type
struct Archetype {
    std::vector<Position> positions;
    std::vector<Velocity> velocities;
    std::size_t size = 0;
};

// World
struct World {
    std::unordered_map<ComponentMask, Archetype> archetypes;
    std::unordered_map<EntityID, std::pair<ComponentMask, std::size_t>> entity_index;

    EntityID create(ComponentMask mask) {
        auto& arch = archetypes[mask];
        EntityID id = next_id_++;
        entity_index[id] = {mask, arch.size++};
        arch.positions.push_back({});
        arch.velocities.push_back({});
        return id;
    }
};

// System: iterate archetypes with Position+Velocity
void physics_system(World& w, float dt) {
    for (auto& [mask, arch] : w.archetypes) {
        if (!(mask & HAS_POSITION && mask & HAS_VELOCITY)) continue;
        for (std::size_t i = 0; i < arch.size; ++i)
            arch.positions[i].x += arch.velocities[i].vx * dt;
    }
}
```

**Trade-offs:**
- Archetype fragmentation: too many unique component combinations creates many small archetypes — poor cache utilization. Group common component sets.
- Adding/removing components requires moving the entity to a different archetype — copy all components. Expensive for frequent structural changes.
- Sparse set approach (no archetypes) has better add/remove performance but worse iteration locality.

---

## 14. C++20 Coroutine Async Framework

**Problem:** Design a minimal async task framework using C++20 coroutines: `Task<T>` type, `co_await` on I/O, a scheduler, and cancellation via `stop_token`.

**Key Design Decisions:**
- `Task<T>` is the coroutine return type — holds a `coroutine_handle`, non-copyable, movable
- `promise_type` stores result or exception; `final_suspend` resumes the awaiting coroutine
- Scheduler: `post(coro_handle)` adds to a run queue; worker calls `resume()`
- `co_await io_op` suspends, registers completion callback that posts handle back to scheduler
- Cancellation: pass `std::stop_token` through task chain; I/O operations check it before suspending

**Implementation Sketch:**
```cpp
template<typename T>
struct Task {
    struct promise_type {
        T result_;
        std::exception_ptr exc_;
        std::coroutine_handle<> continuation_;  // awaiter's handle

        Task get_return_object() { return Task{std::coroutine_handle<promise_type>::from_promise(*this)}; }
        std::suspend_never initial_suspend() { return {}; }  // eager execution
        auto final_suspend() noexcept {
            struct Awaiter {
                std::coroutine_handle<> cont;
                bool await_ready() const noexcept { return false; }
                std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept { return cont ? cont : std::noop_coroutine(); }
                void await_resume() noexcept {}
            };
            return Awaiter{continuation_};
        }
        void return_value(T v) { result_ = std::move(v); }
        void unhandled_exception() { exc_ = std::current_exception(); }
    };
    std::coroutine_handle<promise_type> handle_;
    T get() { if (handle_.promise().exc_) std::rethrow_exception(handle_.promise().exc_); return handle_.promise().result_; }
    ~Task() { if (handle_) handle_.destroy(); }
};
```

**Trade-offs:**
- Eager vs lazy start: `suspend_never` at `initial_suspend` means the coroutine runs until its first `co_await` synchronously on the caller's stack. `suspend_always` makes it lazy — must be explicitly scheduled. Each has different performance implications.
- The framework must handle the case where `co_await task` resumes on a different thread than the awaiter was on — ensure thread-safe continuation chaining.
- Cancellation via `stop_token` is cooperative — I/O operations must periodically check and throw/return.

---

## 15. Multi-GPU Matrix Multiply

**Problem:** Design a distributed matrix multiply C = A × B across 4 GPUs on a single node with NVLink, minimizing inter-GPU communication overhead.

**Key Design Decisions:**
- Partition A by rows across GPUs — each GPU computes a horizontal slice of C
- B is replicated to all GPUs (or broadcast using NVLink via NCCL AllReduce)
- Enable P2P access: `cudaDeviceEnablePeerAccess(other_gpu, 0)` for direct NVLink reads
- Use NCCL `ncclBcast` for B, `ncclAllReduce` if using row × column partition
- Use cuBLAS `cublasGemmEx` per GPU for the local matrix multiply

**Implementation Sketch:**
```cpp
// On each GPU 'g' (thread per GPU):
cudaSetDevice(g);
cudaStream_t stream;  cudaStreamCreate(&stream);

// Each GPU owns rows [g*rows_per_gpu .. (g+1)*rows_per_gpu) of A and C
float* d_A_slice;  cudaMalloc(&d_A_slice, rows_per_gpu * K * sizeof(float));
float* d_B;        cudaMalloc(&d_B, K * N * sizeof(float));
float* d_C_slice;  cudaMalloc(&d_C_slice, rows_per_gpu * N * sizeof(float));

// Broadcast B to all GPUs using NCCL
ncclBcast(d_B, K * N, ncclFloat, root_gpu, comm, stream);
cudaStreamSynchronize(stream);

// Local GEMM: C_slice = A_slice × B
float alpha = 1.0f, beta = 0.0f;
cublasGemmEx(handle, CUBLAS_OP_N, CUBLAS_OP_N,
    N, rows_per_gpu, K,
    &alpha, d_B, CUDA_R_32F, N, d_A_slice, CUDA_R_32F, K,
    &beta,  d_C_slice, CUDA_R_32F, N,
    CUBLAS_COMPUTE_32F, CUBLAS_GEMM_DEFAULT_TENSOR_OP);

// Gather C slices to GPU 0 (or AllGather if all GPUs need full C)
ncclGather(d_C_slice, d_C_full, rows_per_gpu * N, ncclFloat, root_gpu, comm, stream);
```

**Trade-offs:**
- Row partition requires broadcasting full B — for large B (huge N), this is the bottleneck. Alternative: column partition (each GPU gets a column slice of B and produces a column slice of C) with a final AllReduce.
- NVLink bandwidth (600 GB/s on NVLink 3.0) dwarfs PCIe (64 GB/s) — exploit it with peer access and NCCL, not cudaMemcpy through host.
- For very large matrices that do not fit in GPU memory, use streaming: tile A and B, process tile pairs, accumulate into C.
````

---

### Step 2.4 — Write `live-coding.md`

- [ ] Create `tutorial/interview-master/live-coding.md` with the following content:

````markdown
# Live Coding Patterns

*Forge's 25 patterns — write clean code, explain as you go, watch the gotchas.*

---

## 1. Scope Guard

**Problem:** Execute cleanup code on scope exit, with ability to cancel.

```cpp
#include <functional>
#include <utility>

template<typename F>
class ScopeGuard {
    F func_;
    bool active_;
public:
    explicit ScopeGuard(F f) : func_(std::move(f)), active_(true) {}
    ~ScopeGuard() { if (active_) func_(); }
    void dismiss() { active_ = false; }

    ScopeGuard(ScopeGuard&&) = delete;
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
};

// Usage:
void write_file(const char* path) {
    int fd = open(path, O_WRONLY | O_CREAT, 0644);
    ScopeGuard guard([fd]{ close(fd); });

    write(fd, "hello", 5);   // if this throws, fd is still closed
    guard.dismiss();         // success — don't close twice if we hand fd elsewhere
}
```

**Gotchas:**
- Do not move or copy the guard — the lambda captures state that may be invalidated.
- The cleanup function must be noexcept if it runs in a destructor during stack unwinding; a throw in a destructor during exception handling calls `std::terminate`.
- `dismiss()` is crucial for success paths where cleanup should not happen.

---

## 2. Move-Only Type (UniqueResource)

**Problem:** Implement a move-only RAII wrapper for an arbitrary handle.

```cpp
class UniqueResource {
    int handle_ = -1;
public:
    explicit UniqueResource(int h) : handle_(h) {}
    ~UniqueResource() { if (handle_ >= 0) release(handle_); }

    // Move constructor: steal handle, leave source empty
    UniqueResource(UniqueResource&& other) noexcept
        : handle_(other.handle_) {
        other.handle_ = -1;
    }

    // Move assignment: release own, steal other's
    UniqueResource& operator=(UniqueResource&& other) noexcept {
        if (this != &other) {
            if (handle_ >= 0) release(handle_);
            handle_ = other.handle_;
            other.handle_ = -1;
        }
        return *this;
    }

    // Delete copy
    UniqueResource(const UniqueResource&) = delete;
    UniqueResource& operator=(const UniqueResource&) = delete;

    int get() const { return handle_; }
    bool valid() const { return handle_ >= 0; }

private:
    static void release(int h) { /* close/free h */ }
};
```

**Gotchas:**
- Check `this != &other` in move assign, or use move-and-check pattern (it is UB to access `other` after self-move).
- Leave the moved-from object in a "valid but empty" state (handle_ = -1) — the destructor must handle that state safely.
- `noexcept` on move operations is important: STL containers (vector reallocation) prefer noexcept moves; without it they copy.

---

## 3. Copy-and-Swap (Buffer)

**Problem:** Implement a `Buffer` class with a correct copy-assignment operator.

```cpp
#include <cstring>
#include <utility>

class Buffer {
    std::byte* data_ = nullptr;
    std::size_t size_ = 0;
public:
    explicit Buffer(std::size_t n)
        : data_(new std::byte[n]), size_(n) {}

    ~Buffer() { delete[] data_; }

    Buffer(const Buffer& o)
        : data_(new std::byte[o.size_]), size_(o.size_) {
        std::memcpy(data_, o.data_, size_);
    }

    // Copy-and-swap: parameter is by value (copy/move ctor runs)
    Buffer& operator=(Buffer other) {
        swap(*this, other);
        return *this;
        // 'other' destructor cleans up old data
    }

    Buffer(Buffer&& o) noexcept : data_(o.data_), size_(o.size_) {
        o.data_ = nullptr; o.size_ = 0;
    }

    friend void swap(Buffer& a, Buffer& b) noexcept {
        using std::swap;
        swap(a.data_, b.data_);
        swap(a.size_, b.size_);
    }

    std::byte* data() const { return data_; }
    std::size_t size() const { return size_; }
};
```

**Gotchas:**
- The copy assignment parameter is taken by value — this handles self-assignment (a = a → copies 'a' into 'other', swaps, releases copy of 'a').
- `friend swap` should be in the same namespace for ADL to find it.
- `noexcept` on `swap` is essential — the move assignment uses it and callers expect no-throw swap.

---

## 4. Thread-Safe Meyer's Singleton

**Problem:** Implement a singleton that is lazy-initialized, thread-safe, and not copyable.

```cpp
#include <mutex>

class Config {
public:
    static Config& instance() {
        // C++11 guarantees local static initialization is thread-safe
        // exactly once, even with concurrent callers
        static Config cfg;
        return cfg;
    }

    std::string get(const std::string& key) {
        std::lock_guard<std::mutex> lk(mu_);
        auto it = map_.find(key);
        return it != map_.end() ? it->second : "";
    }

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

private:
    Config() { /* load config from file */ }
    std::mutex mu_;
    std::unordered_map<std::string, std::string> map_;
};
```

**Gotchas:**
- Static local initialization: thread-safe in C++11 and later. Pre-C++11 compilers need a double-checked lock.
- Destruction order: singletons are destroyed in reverse order of construction at program exit — if singleton A's destructor uses singleton B, ensure B outlives A.
- Testing: singletons are notoriously hard to reset between tests — consider a `reset_for_testing()` method or inject via dependency injection in production code.

---

## 5. Arena Allocator

**Problem:** Implement a bump-pointer arena allocator.

```cpp
#include <cstddef>
#include <stdexcept>

class Arena {
    std::byte* base_;
    std::size_t capacity_;
    std::size_t offset_ = 0;
public:
    explicit Arena(std::size_t cap)
        : base_(new std::byte[cap]), capacity_(cap) {}
    ~Arena() { delete[] base_; }

    void* allocate(std::size_t size, std::size_t align = alignof(std::max_align_t)) {
        std::size_t aligned = (offset_ + align - 1) & ~(align - 1);
        if (aligned + size > capacity_)
            throw std::bad_alloc();
        void* ptr = base_ + aligned;
        offset_ = aligned + size;
        return ptr;
    }

    void reset() { offset_ = 0; }  // free everything at once

    // Placement helper
    template<typename T, typename... Args>
    T* construct(Args&&... args) {
        void* mem = allocate(sizeof(T), alignof(T));
        return new(mem) T(std::forward<Args>(args)...);
    }

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
};
```

**Gotchas:**
- Objects constructed in the arena must have their destructors called manually if they are non-trivial; `reset()` just reclaims memory.
- Alignment: always align to at least the type's `alignof(T)` — misaligned access is UB.
- Thread safety: this arena is not thread-safe — each thread should have its own arena.

---

## 6. Type-Erased Callable (AnyCallable)

**Problem:** Implement a type-erased callable like `std::function<void(int)>` without using `std::function`.

```cpp
class AnyCallable {
    struct VTable {
        void (*invoke)(void*, int);
        void (*destroy)(void*);
        void* (*clone)(const void*);
    };

    void* obj_ = nullptr;
    const VTable* vtable_ = nullptr;

    template<typename F>
    static VTable make_vtable() {
        return VTable{
            [](void* p, int x) { (*static_cast<F*>(p))(x); },
            [](void* p) { delete static_cast<F*>(p); },
            [](const void* p) -> void* { return new F(*static_cast<const F*>(p)); }
        };
    }

public:
    AnyCallable() = default;

    template<typename F>
    AnyCallable(F f) {
        static const VTable vt = make_vtable<F>();
        obj_ = new F(std::move(f));
        vtable_ = &vt;
    }

    ~AnyCallable() { if (vtable_) vtable_->destroy(obj_); }

    AnyCallable(const AnyCallable& o) {
        if (o.vtable_) { obj_ = o.vtable_->clone(o.obj_); vtable_ = o.vtable_; }
    }
    AnyCallable& operator=(AnyCallable o) {
        std::swap(obj_, o.obj_); std::swap(vtable_, o.vtable_); return *this;
    }

    void operator()(int x) const {
        if (!vtable_) throw std::bad_function_call();
        vtable_->invoke(obj_, x);
    }
    explicit operator bool() const { return vtable_ != nullptr; }
};
```

**Gotchas:**
- Heap allocates for every callable — production implementations use SBO (small buffer optimization) to avoid allocation for small lambdas.
- The VTable static local is per-instantiation of `F` — different callable types get different vtables.
- Thread safety: `AnyCallable` is not thread-safe — concurrent copies require external synchronization.

---

## 7. Compile-Time Type List

**Problem:** Implement a `TypeList` with `type_at` and `contains`.

```cpp
#include <type_traits>

template<typename... Ts>
struct TypeList {
    static constexpr std::size_t size = sizeof...(Ts);
};

// type_at<List, I>
template<typename List, std::size_t I>
struct type_at_impl;

template<typename T, typename... Ts>
struct type_at_impl<TypeList<T, Ts...>, 0> { using type = T; };

template<typename T, typename... Ts, std::size_t I>
struct type_at_impl<TypeList<T, Ts...>, I>
    : type_at_impl<TypeList<Ts...>, I - 1> {};

template<typename List, std::size_t I>
using type_at = typename type_at_impl<List, I>::type;

// contains<List, T>
template<typename List, typename T>
struct contains_impl : std::false_type {};

template<typename T, typename... Ts>
struct contains_impl<TypeList<T, Ts...>, T> : std::true_type {};

template<typename U, typename T, typename... Ts>
struct contains_impl<TypeList<U, Ts...>, T> : contains_impl<TypeList<Ts...>, T> {};

template<typename List, typename T>
inline constexpr bool contains = contains_impl<List, T>::value;

// Tests (compile-time)
using L = TypeList<int, double, char>;
static_assert(L::size == 3);
static_assert(std::is_same_v<type_at<L, 1>, double>);
static_assert(contains<L, char>);
static_assert(!contains<L, float>);
```

**Gotchas:**
- `type_at` with `I >= size` causes a hard compile error (incomplete instantiation) — add a `static_assert` for bounds checking.
- `contains_impl` stops at the first match — more efficient than folding all.
- Index out of bounds during recursion unwinds as a template instantiation depth error — `static_assert(I < sizeof...(Ts), "index out of bounds")` in the primary template helps.

---

## 8. Policy-Based Sort

**Problem:** Implement a `Sorter` with compile-time ascending/descending policies.

```cpp
#include <algorithm>

struct Ascending {
    template<typename T>
    bool operator()(const T& a, const T& b) const { return a < b; }
};

struct Descending {
    template<typename T>
    bool operator()(const T& a, const T& b) const { return a > b; }
};

template<typename Policy = Ascending>
class Sorter {
    Policy cmp_;
public:
    explicit Sorter(Policy p = {}) : cmp_(std::move(p)) {}

    template<typename Iter>
    void sort(Iter first, Iter last) {
        std::sort(first, last, cmp_);
    }

    template<typename Container>
    void sort(Container& c) {
        std::sort(c.begin(), c.end(), cmp_);
    }
};

// Usage:
Sorter<Ascending>  asc;
Sorter<Descending> desc;
std::vector<int> v = {3, 1, 4, 1, 5};
asc.sort(v.begin(), v.end());   // {1, 1, 3, 4, 5}
desc.sort(v.begin(), v.end());  // {5, 4, 3, 1, 1}
```

**Gotchas:**
- Policy must be a callable with two arguments returning bool — document this concept requirement.
- EBO (Empty Base Optimization): `cmp_` as a data member wastes no space if Policy is empty (stateless functor) thanks to `[[no_unique_address]]` (C++20) or by inheriting from Policy.
- `std::sort` requires RandomAccessIterators — a runtime check or static_assert on iterator category is good practice.

---

## 9. `make_overloaded` for std::variant

**Problem:** Create a visitor from a set of lambdas using the overloaded pattern.

```cpp
#include <variant>
#include <iostream>

template<typename... Fs>
struct overloaded : Fs... {
    using Fs::operator()...;
};

// Deduction guide (C++17): lets us write overloaded{f1, f2} without template args
template<typename... Fs>
overloaded(Fs...) -> overloaded<Fs...>;

// Usage:
int main() {
    std::variant<int, double, std::string> v = 3.14;

    std::visit(overloaded{
        [](int i)               { std::cout << "int: "    << i << '\n'; },
        [](double d)            { std::cout << "double: " << d << '\n'; },
        [](const std::string& s){ std::cout << "str: "   << s << '\n'; }
    }, v);
    // prints: double: 3.14
}
```

**Gotchas:**
- All variant alternatives must be handled — `std::visit` is a compile error if any alternative is unhandled (no implicit fallthrough).
- `using Fs::operator()...` is pack expansion — requires C++17.
- If two overloads are ambiguous for a given variant alternative, it is a compile error — make overloads non-overlapping.

---

## 10. Expected<T,E> with Monadic Operations

**Problem:** Implement a minimal `Expected<T,E>` with `and_then` and `or_else`.

```cpp
#include <variant>
#include <functional>
#include <stdexcept>

template<typename T, typename E>
class Expected {
    std::variant<T, E> data_;
    struct ErrorTag {};

    Expected(ErrorTag, E e) : data_(std::move(e)) {}
public:
    Expected(T v) : data_(std::move(v)) {}
    static Expected error(E e) { return Expected(ErrorTag{}, std::move(e)); }

    bool has_value() const { return std::holds_alternative<T>(data_); }
    T&       value()       { return std::get<T>(data_); }
    const T& value() const { return std::get<T>(data_); }
    E&       error()       { return std::get<E>(data_); }

    // and_then: f(T) → Expected<U,E>
    template<typename F>
    auto and_then(F&& f) -> std::invoke_result_t<F, T> {
        if (has_value()) return std::forward<F>(f)(value());
        return std::invoke_result_t<F,T>::error(error());
    }

    // or_else: f(E) → Expected<T,F2>
    template<typename F>
    Expected or_else(F&& f) {
        if (!has_value()) return std::forward<F>(f)(error());
        return *this;
    }

    // transform: f(T) → U, wraps in Expected<U,E>
    template<typename F>
    auto transform(F&& f) -> Expected<std::invoke_result_t<F,T>, E> {
        using U = std::invoke_result_t<F, T>;
        if (has_value()) return Expected<U,E>{std::forward<F>(f)(value())};
        return Expected<U,E>::error(error());
    }
};

// Usage:
Expected<int, std::string> parse(const char* s) {
    try { return std::stoi(s); }
    catch (...) { return Expected<int,std::string>::error("bad parse"); }
}
auto result = parse("42")
    .transform([](int x) { return x * 2; })
    .and_then([](int x) -> Expected<std::string, std::string> {
        return "value=" + std::to_string(x);
    });
```

**Gotchas:**
- `and_then` requires `F` to return an `Expected<U,E>` — the E type must match. Use `transform` when `F` returns a plain value.
- Chaining works because each operation returns a new `Expected` — no mutation of the original.
- `std::variant` requires all alternatives to be different types — `Expected<T,T>` would fail; use a tagged wrapper.

---

## 11. SPSC Lock-Free Queue

**Problem:** Implement a lock-free single-producer, single-consumer queue with power-of-2 capacity.

```cpp
#include <atomic>
#include <array>
#include <optional>

template<typename T, std::size_t N>
class SPSCQueue {
    static_assert((N & (N - 1)) == 0, "N must be power of 2");
    std::array<T, N> buf_{};
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
public:
    // Producer
    bool push(T val) {
        const auto t = tail_.load(std::memory_order_relaxed);
        const auto next = (t + 1) & (N - 1);
        if (next == head_.load(std::memory_order_acquire)) return false; // full
        buf_[t] = std::move(val);
        tail_.store(next, std::memory_order_release);
        return true;
    }

    // Consumer
    std::optional<T> pop() {
        const auto h = head_.load(std::memory_order_relaxed);
        if (h == tail_.load(std::memory_order_acquire)) return std::nullopt; // empty
        T val = std::move(buf_[h]);
        head_.store((h + 1) & (N - 1), std::memory_order_release);
        return val;
    }

    bool empty() const {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }
};
```

**Gotchas:**
- `alignas(64)` prevents false sharing between head and tail — without it, each write to tail invalidates the cache line holding head.
- The queue is full when `next == head` (not when `tail == head`) — this wastes one slot to distinguish full from empty.
- Do not add a second producer or consumer — the algorithm is only correct for exactly 1P+1C.

---

## 12. Thread Pool

**Problem:** Implement a thread pool with `submit()` returning a `std::future`.

```cpp
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>
#include <functional>
#include <vector>

class ThreadPool {
    std::vector<std::thread>          workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex                        mu_;
    std::condition_variable           cv_;
    bool                              stop_ = false;

public:
    explicit ThreadPool(std::size_t n = std::thread::hardware_concurrency()) {
        for (std::size_t i = 0; i < n; ++i)
            workers_.emplace_back([this] { loop(); });
    }

    ~ThreadPool() {
        { std::lock_guard<std::mutex> lk(mu_); stop_ = true; }
        cv_.notify_all();
        for (auto& w : workers_) w.join();
    }

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using R = std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<R()>>(
            [f = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable {
                return f(args...);
            }
        );
        std::future<R> fut = task->get_future();
        { std::lock_guard<std::mutex> lk(mu_);
          if (stop_) throw std::runtime_error("pool stopped");
          tasks_.emplace([task] { (*task)(); }); }
        cv_.notify_one();
        return fut;
    }

private:
    void loop() {
        while (true) {
            std::function<void()> task;
            { std::unique_lock<std::mutex> lk(mu_);
              cv_.wait(lk, [this] { return stop_ || !tasks_.empty(); });
              if (stop_ && tasks_.empty()) return;
              task = std::move(tasks_.front()); tasks_.pop(); }
            task();
        }
    }
};
```

**Gotchas:**
- The lambda capture `[...args = std::forward<Args>(args)]` is C++20. In C++17, bind the args into a tuple.
- `packaged_task` is non-copyable — store via `shared_ptr` and capture by value in the lambda.
- Exceptions in submitted tasks are stored in the `future` and rethrown on `get()` — callers must handle them.

---

## 13. Minimal C++20 Task Coroutine

**Problem:** Implement a basic `Task<T>` coroutine type that executes eagerly.

```cpp
#include <coroutine>
#include <exception>
#include <optional>

template<typename T>
struct Task {
    struct promise_type {
        std::optional<T>    result;
        std::exception_ptr  exc;
        std::coroutine_handle<> continuation;

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() noexcept { return {}; }
        auto final_suspend() noexcept {
            struct Awaiter {
                std::coroutine_handle<> cont;
                bool await_ready() const noexcept { return !cont; }
                std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept {
                    return cont;
                }
                void await_resume() noexcept {}
            };
            return Awaiter{continuation};
        }
        void return_value(T v) { result = std::move(v); }
        void unhandled_exception() { exc = std::current_exception(); }
    };

    std::coroutine_handle<promise_type> coro;

    explicit Task(std::coroutine_handle<promise_type> h) : coro(h) {}
    Task(Task&& o) noexcept : coro(o.coro) { o.coro = nullptr; }
    ~Task() { if (coro) coro.destroy(); }
    Task(const Task&) = delete;

    bool await_ready() const noexcept { return coro.done(); }
    void await_suspend(std::coroutine_handle<> h) noexcept {
        coro.promise().continuation = h;
    }
    T await_resume() {
        if (coro.promise().exc) std::rethrow_exception(coro.promise().exc);
        return std::move(*coro.promise().result);
    }
};

// Usage:
Task<int> compute() { co_return 42; }
Task<int> main_coro() {
    int x = co_await compute();
    co_return x * 2;
}
```

**Gotchas:**
- `initial_suspend() = suspend_never` makes the coroutine run eagerly (synchronously until first `co_await`) — this can cause re-entrancy if the coroutine immediately completes and resumes its awaiter.
- `final_suspend` must return an awaitable (not `suspend_never`) if there's a continuation to resume — otherwise the coroutine is destroyed before the awaiter reads the result.
- The `coro.destroy()` in the destructor should only be called if the coroutine has not yet been awaited and resumed to completion.

---

## 14. Signal/Slot System

**Problem:** Implement a type-safe signal/slot (event) system with connection handles.

```cpp
#include <functional>
#include <map>

template<typename... Args>
class Signal {
    using Slot = std::function<void(Args...)>;
    std::map<int, Slot> slots_;
    int next_id_ = 0;
public:
    struct Connection {
        Signal* sig;
        int id;
        void disconnect() { if (sig) { sig->slots_.erase(id); sig = nullptr; } }
    };

    Connection connect(Slot s) {
        int id = next_id_++;
        slots_[id] = std::move(s);
        return Connection{this, id};
    }

    void emit(Args... args) {
        // Copy to avoid invalidation if a slot disconnects during emit
        auto snapshot = slots_;
        for (auto& [id, slot] : snapshot) slot(args...);
    }

    void disconnect_all() { slots_.clear(); }
};

// Usage:
Signal<int, std::string> on_event;
auto conn = on_event.connect([](int code, std::string msg) {
    printf("event %d: %s\n", code, msg.c_str());
});
on_event.emit(200, "OK");
conn.disconnect();
on_event.emit(404, "Not Found");  // no output — disconnected
```

**Gotchas:**
- Disconnecting within an emit callback: snapshot the map before iteration, otherwise erasing an element invalidates the iterator.
- Thread safety: this implementation is not thread-safe — add a mutex if signals cross threads.
- Connection handle invalidation: if the Signal is destroyed while a Connection object exists, the Connection holds a dangling pointer — add a `weak_ptr` to the Signal or a validity flag.

---

## 15. Self-Registering Factory

**Problem:** Implement a factory where each type registers itself via a static initializer.

```cpp
#include <memory>
#include <map>
#include <string>
#include <functional>
#include <stdexcept>

struct Shape { virtual double area() const = 0; virtual ~Shape() = default; };

// Registry is a function to avoid static initialization order issues
using Creator = std::function<std::unique_ptr<Shape>()>;
std::map<std::string, Creator>& shape_registry() {
    static std::map<std::string, Creator> reg;
    return reg;
}

template<typename T>
struct Registrar {
    explicit Registrar(const std::string& name) {
        shape_registry()[name] = [] { return std::make_unique<T>(); };
    }
};

std::unique_ptr<Shape> create_shape(const std::string& name) {
    auto& reg = shape_registry();
    auto it = reg.find(name);
    if (it == reg.end()) throw std::out_of_range("unknown shape: " + name);
    return it->second();
}

// Concrete type — registers itself
struct Circle : Shape {
    double r = 1.0;
    double area() const override { return 3.14159 * r * r; }
    // C++17 inline static — initialized when the TU is loaded
    static inline Registrar<Circle> reg{"Circle"};
};

struct Square : Shape {
    double side = 1.0;
    double area() const override { return side * side; }
    static inline Registrar<Square> reg{"Square"};
};

// Usage: auto s = create_shape("Circle");
```

**Gotchas:**
- `inline static` requires C++17 — in C++14, define the static in a .cpp file to ensure exactly one definition.
- Static initialization order: the registry must be initialized before any `Registrar` — using a function-local static (as above) guarantees this.
- Registration happens at library load time — if the library is dynamically loaded with `RTLD_LAZY`, types may not be registered yet when `create_shape` is called.

---

## 16. CRTP Zero-Cost Static Polymorphism

**Problem:** Use CRTP to implement `area()` and `perimeter()` for multiple shapes without virtual dispatch.

```cpp
#include <cmath>

template<typename Derived>
struct Shape {
    double area()      const { return static_cast<const Derived*>(this)->area_impl(); }
    double perimeter() const { return static_cast<const Derived*>(this)->perimeter_impl(); }

    void describe() const {
        printf("area=%.2f perimeter=%.2f\n", area(), perimeter());
    }
};

struct Circle : Shape<Circle> {
    double r;
    explicit Circle(double radius) : r(radius) {}
    double area_impl()      const { return 3.14159 * r * r; }
    double perimeter_impl() const { return 2 * 3.14159 * r; }
};

struct Rectangle : Shape<Rectangle> {
    double w, h;
    Rectangle(double width, double height) : w(width), h(height) {}
    double area_impl()      const { return w * h; }
    double perimeter_impl() const { return 2 * (w + h); }
};

// Template function works with any Shape<Derived>
template<typename S>
void print_shape(const Shape<S>& s) {
    s.describe();  // fully inlined — no virtual call
}

// print_shape(Circle{5.0});    → area=78.54 perimeter=31.42
// print_shape(Rectangle{3,4}); → area=12.00 perimeter=14.00
```

**Gotchas:**
- No runtime polymorphism: `Shape<Circle>*` and `Shape<Rectangle>*` are unrelated types — cannot store in the same container without a wrapper.
- Forgetting to override `area_impl()` in Derived causes infinite recursion — add a `static_assert` in the base template.
- `static_cast` is safe here because the base is constructed as part of the derived; it is UB only if the dynamic type is wrong (which it cannot be here).

---

## 17. Ring Buffer

**Problem:** Implement a ring buffer with `push`, `pop`, `size`, and `empty`.

```cpp
#include <stdexcept>
#include <vector>

template<typename T>
class RingBuffer {
    std::vector<T> buf_;
    std::size_t head_ = 0, tail_ = 0, count_ = 0;
public:
    explicit RingBuffer(std::size_t cap) : buf_(cap) {}

    void push(T val) {
        if (count_ == buf_.size()) throw std::overflow_error("ring buffer full");
        buf_[tail_] = std::move(val);
        tail_ = (tail_ + 1) % buf_.size();
        ++count_;
    }

    T pop() {
        if (empty()) throw std::underflow_error("ring buffer empty");
        T val = std::move(buf_[head_]);
        head_ = (head_ + 1) % buf_.size();
        --count_;
        return val;
    }

    T& front()       { if (empty()) throw std::underflow_error("empty"); return buf_[head_]; }
    const T& front() const { return const_cast<RingBuffer*>(this)->front(); }

    bool        empty() const { return count_ == 0; }
    bool        full()  const { return count_ == buf_.size(); }
    std::size_t size()  const { return count_; }
    std::size_t capacity() const { return buf_.size(); }
};
```

**Gotchas:**
- Using a separate `count_` rather than distinguishing full/empty by `head == tail` avoids the off-by-one problem at the cost of one extra field.
- Power-of-2 size allows `& (N-1)` instead of `%` — faster but constrains capacity.
- This ring buffer is not thread-safe — the SPSC pattern (pattern 11) uses atomics for lock-free concurrent access.

---

## 18. Flat State Machine

**Problem:** Implement a flat (table-driven) state machine with enum states and events.

```cpp
#include <array>
#include <functional>
#include <stdexcept>

enum class State : int { Idle = 0, Running = 1, Paused = 2, Stopped = 3 };
enum class Event : int { Start = 0, Pause = 1, Resume = 2, Stop = 3 };

constexpr int NS = 4, NE = 4;

// Transition table: [state][event] → next state
constexpr std::array<std::array<State, NE>, NS> TRANSITIONS = {{
    // State::Idle
    {{ State::Running, State::Idle,    State::Idle,    State::Stopped }},
    // State::Running
    {{ State::Running, State::Paused,  State::Running, State::Stopped }},
    // State::Paused
    {{ State::Running, State::Paused,  State::Running, State::Stopped }},
    // State::Stopped
    {{ State::Stopped, State::Stopped, State::Stopped, State::Stopped }},
}};

struct StateMachine {
    State current = State::Idle;
    std::function<void(State, State)> on_transition;  // optional hook

    void dispatch(Event e) {
        State next = TRANSITIONS[(int)current][(int)e];
        if (next != current && on_transition)
            on_transition(current, next);
        current = next;
    }
};

// Usage:
StateMachine sm;
sm.on_transition = [](State from, State to) {
    printf("transition %d → %d\n", (int)from, (int)to);
};
sm.dispatch(Event::Start);   // Idle → Running
sm.dispatch(Event::Pause);   // Running → Paused
sm.dispatch(Event::Resume);  // Paused → Running
sm.dispatch(Event::Stop);    // Running → Stopped
```

**Gotchas:**
- `constexpr` table: all state/event pairs must be handled explicitly — a missing cell is a compile error (array size mismatch) or a logic bug (wrong default).
- The transition hook fires only on actual state changes — check `next != current` or fire unconditionally if the caller needs to know about no-ops.
- For entry/exit actions per state, add `on_enter[NS]` and `on_exit[NS]` arrays of callbacks.

---

## 19. Compile-Time Fibonacci with consteval

**Problem:** Compute Fibonacci numbers at compile time.

```cpp
// consteval: guaranteed compile-time evaluation (C++20)
consteval int fib(int n) {
    if (n < 0) throw "n must be non-negative";  // consteval can throw (compile error)
    if (n <= 1) return n;
    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i) {
        int c = a + b;
        a = b;
        b = c;
    }
    return b;
}

// All computed at compile time — no runtime cost
static_assert(fib(0)  == 0);
static_assert(fib(1)  == 1);
static_assert(fib(10) == 55);
static_assert(fib(20) == 6765);

// In C++17 without consteval, use constexpr:
constexpr int fib17(int n) {
    if (n <= 1) return n;
    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i) { int c = a + b; a = b; b = c; }
    return b;
}
```

**Gotchas:**
- Recursive `constexpr fib(n) = fib(n-1) + fib(n-2)` hits the constexpr evaluation step limit for n > ~40. The iterative form has no such limit.
- `consteval` (C++20) forces compile-time only; `constexpr` can also be called at runtime. Use `consteval` when runtime calls should be a compile error.
- `int` overflows for `fib(47)` on 32-bit int (> 2^31). Use `long long` for n > 46.

---

## 20. Variadic Tuple-Apply

**Problem:** Apply a function to each element of a `std::tuple`.

```cpp
#include <tuple>
#include <utility>
#include <iostream>

// Apply f to each element of tuple t (in order)
template<typename F, typename Tuple, std::size_t... Is>
void apply_each_impl(F&& f, Tuple&& t, std::index_sequence<Is...>) {
    (f(std::get<Is>(std::forward<Tuple>(t))), ...);  // fold expression
}

template<typename F, typename Tuple>
void apply_each(F&& f, Tuple&& t) {
    apply_each_impl(
        std::forward<F>(f),
        std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{}
    );
}

// Usage:
auto tup = std::make_tuple(1, 3.14, std::string("hello"));
apply_each([](auto&& x) { std::cout << x << ' '; }, tup);
// prints: 1 3.14 hello

// Transform: build a new tuple by applying f to each element
template<typename F, typename Tuple, std::size_t... Is>
auto transform_tuple_impl(F&& f, Tuple&& t, std::index_sequence<Is...>) {
    return std::make_tuple(f(std::get<Is>(t))...);
}
template<typename F, typename Tuple>
auto transform_tuple(F&& f, Tuple&& t) {
    return transform_tuple_impl(f, std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>{});
}
```

**Gotchas:**
- The comma fold `(f(args), ...)` evaluates left-to-right (comma has left-to-right sequencing). Safe for side effects.
- `std::make_index_sequence` is a zero-overhead compile-time integer sequence — no runtime cost.
- `F` must accept every element type in the tuple — if types differ, use a generic lambda or overloaded visitor.

---

## 21. Parallel Transform

**Problem:** Implement `parallel_transform` that applies `f` to each element in parallel across N threads.

```cpp
#include <thread>
#include <vector>
#include <iterator>
#include <algorithm>

template<typename InputIt, typename OutputIt, typename F>
void parallel_transform(InputIt first, InputIt last, OutputIt out, F f,
    std::size_t nthreads = std::thread::hardware_concurrency())
{
    const std::size_t n = std::distance(first, last);
    if (n == 0) return;
    nthreads = std::min(nthreads, n);

    std::vector<std::thread> threads;
    threads.reserve(nthreads);

    const std::size_t chunk = n / nthreads;
    std::size_t start = 0;

    for (std::size_t t = 0; t < nthreads; ++t) {
        std::size_t end = (t == nthreads - 1) ? n : start + chunk;
        threads.emplace_back([=, &f]() {
            auto in_it  = std::next(first, start);
            auto out_it = std::next(out, start);
            for (std::size_t i = start; i < end; ++i, ++in_it, ++out_it)
                *out_it = f(*in_it);
        });
        start = end;
    }
    for (auto& th : threads) th.join();
}

// Usage:
std::vector<int> in = {1, 2, 3, 4, 5, 6, 7, 8};
std::vector<int> out(in.size());
parallel_transform(in.begin(), in.end(), out.begin(), [](int x) { return x * x; });
// out = {1, 4, 9, 16, 25, 36, 49, 64}
```

**Gotchas:**
- Each thread writes to non-overlapping output ranges — no synchronization needed for writes.
- If `f` throws, the exception is lost (thread terminates). Use `std::exception_ptr` + `future` for exception propagation.
- `std::distance` is O(n) for non-random-access iterators — use random-access iterators or accept indices instead.

---

## 22. Memory Pool Allocator (STL-Compatible)

**Problem:** Implement an STL-compatible fixed-size-block pool allocator.

```cpp
#include <memory>
#include <vector>

template<typename T>
class PoolAllocator {
    struct Block { Block* next; };
    static Block* free_list_;
    static std::vector<std::unique_ptr<char[]>> slabs_;

    static void grow() {
        const int SLAB = 64;
        auto slab = std::make_unique<char[]>(SLAB * sizeof(T));
        auto* begin = reinterpret_cast<Block*>(slab.get());
        for (int i = 0; i < SLAB - 1; ++i)
            begin[i].next = &begin[i + 1];
        begin[SLAB - 1].next = free_list_;
        free_list_ = begin;
        slabs_.push_back(std::move(slab));
    }

public:
    using value_type = T;

    T* allocate(std::size_t n) {
        if (n != 1) return static_cast<T*>(::operator new(n * sizeof(T)));
        if (!free_list_) grow();
        Block* b = free_list_;
        free_list_ = b->next;
        return reinterpret_cast<T*>(b);
    }
    void deallocate(T* p, std::size_t n) noexcept {
        if (n != 1) { ::operator delete(p); return; }
        auto* b = reinterpret_cast<Block*>(p);
        b->next = free_list_;
        free_list_ = b;
    }
    template<typename U> struct rebind { using other = PoolAllocator<U>; };
    template<typename U> bool operator==(const PoolAllocator<U>&) const noexcept { return true; }
    template<typename U> bool operator!=(const PoolAllocator<U>&) const noexcept { return false; }
};
template<typename T> typename PoolAllocator<T>::Block* PoolAllocator<T>::free_list_ = nullptr;
template<typename T> std::vector<std::unique_ptr<char[]>> PoolAllocator<T>::slabs_;

// Usage with STL:
std::list<int, PoolAllocator<int>> pool_list;
pool_list.push_back(1);
pool_list.push_back(2);
```

**Gotchas:**
- STL containers call `rebind` to get an allocator for internal node types — the `rebind` struct is mandatory.
- Equality operator: two `PoolAllocator` instances of the same `T` share the global pool, so they are equal — this is important for container move/swap.
- Thread safety: this pool is not thread-safe (global free list). Add a mutex or make it thread-local.

---

## 23. Kalman Filter Step

**Problem:** Implement one predict + update step of a scalar Kalman filter.

```cpp
struct KalmanFilter {
    double x;      // state estimate
    double p;      // estimate error covariance
    double q;      // process noise covariance
    double r;      // measurement noise covariance

    explicit KalmanFilter(double initial_x, double initial_p, double q, double r)
        : x(initial_x), p(initial_p), q(q), r(r) {}

    // Predict: advance state by dt using constant-velocity model
    void predict(double dt = 1.0) {
        // State: x = x (position, simple model — no velocity)
        p = p + q;  // uncertainty grows
    }

    // Update: incorporate measurement z
    double update(double z) {
        // Kalman gain: how much to trust the measurement vs prediction
        double k = p / (p + r);
        // Correct estimate
        x = x + k * (z - x);
        // Update covariance
        p = (1.0 - k) * p;
        return x;
    }

    // Combined step:
    double step(double z, double dt = 1.0) {
        predict(dt);
        return update(z);
    }
};

// Usage:
KalmanFilter kf(0.0, 1.0, 0.01, 0.1);  // initial_x=0, p=1, q=0.01, r=0.1
for (double meas : {1.1, 1.9, 3.0, 4.1, 5.0}) {
    double est = kf.step(meas);
    printf("meas=%.1f est=%.3f\n", meas, est);
}
```

**Gotchas:**
- This is a scalar 1D filter — the 2D+ version uses matrices (state vector, transition matrix, observation matrix).
- `q` too small → filter is slow to respond to real change; `q` too large → filter is noisy.
- `r` is the measurement noise variance — must be estimated from sensor datasheet or empirical noise measurements.

---

## 24. A* on 2D Grid

**Problem:** Implement A* pathfinding on a 2D grid.

```cpp
#include <queue>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <optional>

struct Pos { int x, y; bool operator==(Pos o) const { return x==o.x && y==o.y; } };

struct PosHash {
    std::size_t operator()(Pos p) const {
        return std::hash<int>()(p.x * 10007 + p.y);
    }
};

std::optional<std::vector<Pos>> astar(
    const std::vector<std::string>& grid, Pos start, Pos goal)
{
    const int rows = grid.size(), cols = grid[0].size();
    auto valid = [&](Pos p) {
        return p.x >= 0 && p.y >= 0 && p.x < cols && p.y < rows && grid[p.y][p.x] != '#';
    };
    auto h = [&](Pos p) {
        return std::abs(p.x - goal.x) + std::abs(p.y - goal.y);  // Manhattan heuristic
    };

    struct Node { int f, g; Pos pos; bool operator>(const Node& o) const { return f > o.f; } };
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
    std::unordered_map<Pos, Pos,    PosHash> came_from;
    std::unordered_map<Pos, int,    PosHash> g_score;

    g_score[start] = 0;
    open.push({h(start), 0, start});

    while (!open.empty()) {
        auto [f, g, cur] = open.top(); open.pop();
        if (cur == goal) {
            std::vector<Pos> path;
            for (Pos p = goal; p != start; p = came_from[p]) path.push_back(p);
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }
        for (auto [dx, dy] : std::initializer_list<std::pair<int,int>>{{1,0},{-1,0},{0,1},{0,-1}}) {
            Pos nb{cur.x + dx, cur.y + dy};
            if (!valid(nb)) continue;
            int ng = g + 1;
            if (!g_score.count(nb) || ng < g_score[nb]) {
                g_score[nb] = ng;
                came_from[nb] = cur;
                open.push({ng + h(nb), ng, nb});
            }
        }
    }
    return std::nullopt;  // no path found
}
```

**Gotchas:**
- A priority queue does not support decrease-key — duplicate nodes may be in the queue. Check `g > g_score[cur]` at pop to skip stale entries.
- Manhattan heuristic is only admissible for 4-directional movement — use Chebyshev distance for 8-directional.
- The `came_from` map tracks the path but not all intermediates — reconstruct by following `came_from` pointers from goal to start, then reverse.

---

## 25. PID Controller with Anti-Windup

**Problem:** Implement a PID controller with clamped integral (anti-windup) and derivative filter.

```cpp
class PID {
    double kp_, ki_, kd_;
    double integral_ = 0.0;
    double prev_error_ = 0.0;
    double integral_limit_;
    double output_limit_;
    double alpha_;   // derivative filter coefficient (0=no filter, 1=max filter)

public:
    PID(double kp, double ki, double kd,
        double integral_limit = 100.0,
        double output_limit   = 1000.0,
        double derivative_filter_alpha = 0.1)
        : kp_(kp), ki_(ki), kd_(kd)
        , integral_limit_(integral_limit)
        , output_limit_(output_limit)
        , alpha_(derivative_filter_alpha)
    {}

    double step(double setpoint, double measurement, double dt) {
        double error = setpoint - measurement;

        // Proportional
        double p = kp_ * error;

        // Integral with anti-windup (clamp)
        integral_ += ki_ * error * dt;
        integral_ = std::clamp(integral_, -integral_limit_, integral_limit_);

        // Derivative with low-pass filter
        double raw_d = (error - prev_error_) / dt;
        double d = kd_ * (alpha_ * prev_d_ + (1.0 - alpha_) * raw_d);
        prev_d_ = raw_d;
        prev_error_ = error;

        double output = p + integral_ + d;
        return std::clamp(output, -output_limit_, output_limit_);
    }

    void reset() { integral_ = 0.0; prev_error_ = 0.0; prev_d_ = 0.0; }

private:
    double prev_d_ = 0.0;
};

// Usage:
PID pid(1.0, 0.1, 0.05, 50.0, 100.0, 0.2);
double position = 0.0, setpoint = 10.0, dt = 0.01;
for (int i = 0; i < 1000; ++i) {
    double force = pid.step(setpoint, position, dt);
    position += force * dt;  // simplified plant
}
```

**Gotchas:**
- Anti-windup: clamping the integral prevents it from accumulating indefinitely when the output is saturated (e.g., motor already at max speed). Without it, the system overshoots severely when setpoint changes.
- Derivative filter: raw derivative amplifies measurement noise — the low-pass filter (`alpha_`) smooths it. Higher alpha = more filtering, more phase lag.
- `dt` must be the actual elapsed time per step — use `clock_gettime` or equivalent; never assume it equals the nominal period.
````

---

### Step 2.5 — Write `war-stories.md`

- [ ] Create `tutorial/interview-master/war-stories.md` with the following content:

````markdown
# War Stories

*Forge's field reports — five real debugging battles. Learn from the scars.*

---

## 1. The False Sharing Bug That Ate 40% of Our Throughput

We were building a lock-free order routing system. The core was a queue — each instrument had its own queue, and we had a producer thread receiving orders from the network, a consumer thread sending them to the exchange gateway. Textbook SPSC queue. Clean. Correct. And somehow, on our 48-core production box, it was running at 60% of the single-threaded baseline.

I spent three days staring at the code before a colleague mentioned `perf c2c`. I had never used it before. It analyzes cache line contention — specifically, HITM events: Hit-in-Modified, meaning a core tried to read a cache line that was in another core's cache in Modified state. The data came back immediately, embarrassingly clear: our `Queue` struct had `head` and `tail` as adjacent `std::atomic<size_t>` fields, laid out sequentially in memory. Eight bytes each. They fit in the same 64-byte cache line.

The producer wrote to `tail` at thousands of operations per second. The consumer wrote to `head` at thousands of operations per second. Even though they were touching completely different memory locations, the CPU had to transfer the entire 64-byte cache line back and forth between the producer's core and the consumer's core on every operation. MESI protocol: Modified state means exclusive ownership. One core modifies → invalidates all other copies → other core must fetch from L3 or from the remote core's cache. On our NUMA system, that fetch crossed a QPI link. Three hundred nanoseconds. Per operation.

The fix was one line: `alignas(64)`. We split `head` and `tail` into their own cache lines. Throughput went from 60% to 101% of single-threaded baseline (the 1% improvement came from better cache locality for each thread's private data). Deployment to production: forty percent throughput increase overnight, zero code logic changes.

The lesson that stayed with me: cache coherence traffic is completely invisible in the code. Two threads touching different variables — different `size_t` fields — and they are contending as badly as if they were sharing the same mutex. `perf c2c` should be the first tool you reach for in any unexplained throughput regression on a multi-core system.

---

## 2. The ABA Problem That Corrupted Our Order Book

This one took four weeks. Four weeks of staring at core dumps, adding logging, scratching our heads in postmortems. The symptom: about once every two to three days of production load, our order book would produce an obviously wrong state — a bid higher than the ask, or an order attributed to the wrong instrument. We could never reproduce it in testing.

The system: we had a lock-free stack for recycling `Order` objects. Allocating and freeing an `Order` per message was too slow at 500k messages per second, so we recycled them through a CAS-based free stack. Push and pop looked correct — the standard ABA-proof pattern... except it wasn't. We were using a single 64-bit pointer for the stack head with a CAS loop.

The ABA sequence: Thread A pops `Order* X` from the free stack — address 0xCAFE0000. It's about to process X. Before it does, thread B pops X, fills it with a new order's data, pushes it back, then the allocator gives that slot to someone else, who happens to free a different order that the OS gives the same address 0xCAFE0000 (or the pool reuses it). Thread A's CAS: is head still 0xCAFE0000? Yes. CAS succeeds. Thread A believes it safely popped X. But X's contents are now from a different order. Thread A reads X's instrument ID, price, side — all wrong.

The discovery came from adding a generation counter. We changed the stack head from a raw `Order*` to a tagged pointer: a 128-bit struct `{Order* ptr; uint64_t tag}`, incremented `tag` on every push, and used `cmpxchg16b` for the CAS. Suddenly the bug stopped reproducing. We confirmed it in a stress test: with tagging, after 72 hours and 800 billion operations, zero corruptions. Without tagging: corruption within 30 minutes.

The lesson: CAS is atomic, but it only compares the value you loaded. It does not protect you from the address being reused with a different object. Every lock-free data structure that recycles memory must version its pointers. The generation counter wastes 8 bytes per pointer. It saved us from a production incident that was costing $200k per occurrence in bad trades.

---

## 3. The Signal Handler That Deadlocked Our Server

We were deploying a new version of our pricing server to production. The deployment process sent SIGTERM to the old process. Graceful shutdown. The old process was supposed to flush its last prices, close connections, and exit cleanly. Instead, it hung. Every single time. The monitoring team would wait 30 seconds, then send SIGKILL. Not ideal, but workable — until we needed a truly zero-downtime deployment.

The code looked fine to me. Our SIGTERM handler:
```cpp
void sigterm_handler(int) {
    fprintf(stderr, "Received SIGTERM — shutting down\n");
    shutdown_requested = true;
}
```

I attached gdb to a hung process. `thread apply all bt`. The main thread was in `__lll_lock_wait` inside `malloc`. Thread 2 (the signal handler) was also in `malloc`. Via `fprintf`.

I had to read the glibc source to understand. `fprintf` internally calls `fwrite`, which calls `malloc` (for the formatting buffer). `malloc` has an internal lock (`__malloc_mutex`). The main thread had called `malloc` for some other allocation, held `__malloc_mutex`, was blocked waiting for a condition. The signal arrived while the main thread held the lock. The signal handler ran (preempting the main thread), called `fprintf`, tried to acquire `__malloc_mutex` — deadlock. Main thread never woke up. Signal handler never returned.

POSIX's async-signal-safe function list is very short. `write()` is on it. `fprintf` is not. `malloc` is not. Even `strlen` is not guaranteed safe in a signal handler. The only safe pattern: set a `volatile sig_atomic_t` flag in the handler, check it in the main loop.

We rewrote the handler:
```cpp
volatile sig_atomic_t shutdown_requested = 0;
void sigterm_handler(int) { shutdown_requested = 1; }
```

And checked `shutdown_requested` in the main loop. Worked immediately. We also switched to `signalfd` — turn signal delivery into readable file events that the epoll loop can handle, eliminating async signal handler issues entirely.

The lesson: every function you are tempted to call in a signal handler is probably unsafe. Printf, cout, mutex, condition variable, new, delete — all forbidden. When in doubt, check the POSIX async-signal-safe list. It is shorter than you think.

---

## 4. The CUDA Kernel That Was Slower Than the CPU

We had written a new image processing kernel to run on a Tesla V100. The algorithm was a custom per-column contrast enhancement — for each column of the image, compute statistics over the full column, then apply a normalization. Straightforward. We launched one thread per column. Simple.

The benchmark came back. CPU (single thread): 45ms. GPU with 4096 threads: 280ms.

That cannot be right. The GPU has thousands of cores. The data is 4096×4096 floats. How is it slower?

Nsight Compute. First metric I checked: `dram__sectors_read.avg_per_request`. On a perfectly coalesced access, 32 threads access 32 consecutive floats — one 128-byte sector, one memory transaction. My number: 32 sectors per request.

I had assigned one thread per column. The image was stored in row-major order. Thread 0 accessed `image[0][threadIdx.x]`, thread 1 accessed `image[1][threadIdx.x]`, ..., thread 31 accessed `image[31][threadIdx.x]`. In memory, `image[0][threadIdx.x]` and `image[1][threadIdx.x]` are 4096 floats apart — 16KB apart. Each of the 32 threads in a warp was accessing a different 128-byte sector of global memory. 32 memory transactions instead of 1. The GPU's 900 GB/s of HBM bandwidth was being used at about 1/32nd efficiency.

The fix was two-fold. First, I tried transposing the image (converting from row-major to column-major) so that column data was contiguous. That alone brought it to 12ms. Then I tiled the kernel — load a 32×32 block into shared memory with coalesced reads (threads loading consecutive floats from a row), then process the tile from shared memory. That got it to 4ms.

CPU: 45ms. GPU final: 4ms. Eleven times faster. But the naive GPU version was six times slower than the CPU. The GPU is not magic — it rewards memory access patterns that respect the cache line and warp architecture. Strided access patterns turn thousands of cores into a liability.

The lesson: always open Nsight Compute before claiming a GPU implementation is optimized. `dram__sectors_read.avg_per_request` should be close to 1 for good coalescing. If it's 32, you have a strided access problem, not a kernel logic problem.

---

## 5. The Memory Leak That Only Appeared After 3 Days

We had a microservice — order state cache — that ran fine in our 30-minute smoke tests. It ran fine in our one-hour load tests. It passed a 24-hour soak test. We deployed it to production and two days later the ops team paged us: memory usage was growing at 100MB per hour. By day 3, processes were OOM-killed.

ASan found nothing. Valgrind on the test binary found nothing. We added tcmalloc's heap profiler — nothing visible in 30-minute runs. The leak was too slow to appear in any CI run.

The architecture: an async task system. Each incoming order kicked off a `Task` object that captured a lambda. The lambda captured a raw `Context*` — a pointer to a per-connection context object. Tasks were stored in a `std::deque<Task>` pending work queue. A background thread drained the queue and executed tasks.

The bug: one specific shutdown path — triggered only when a client disconnected during the processing of a particular message type — skipped the "drain pending tasks" step. It just cleared the connection's metadata and moved on. The tasks in the deque were never executed and never freed. The `Context*` they captured pointed to memory that the connection manager had already freed — but the tasks (and their captured lambdas, and the lambda's closure storage on the heap) were never cleaned up.

Each leaked task was small — 200 bytes. But at 20,000 orders per second with a 1-in-500,000 hit rate on the buggy path, that's 40 tasks per second × 200 bytes = 8KB/s. Over 3 days: 2GB. Spread across a fleet of 50 processes: 100MB/hour per process.

We caught it with a long-running stress test that specifically triggered the shutdown path. Not a 30-minute test — a 72-hour test with `LSAN_OPTIONS=detect_leaks=1:report_objects=1`. It finally caught it on day 2, showing the allocation stack perfectly.

The fix: replace the raw `Context*` capture with `std::shared_ptr<Context>`. Now even if the task is never executed, its destructor (when the deque is cleared) releases the shared_ptr reference. The connection object's lifetime is tied to whoever holds a reference, not to whoever remembered to call cleanup.

The lesson: leaks that don't manifest at exit are not caught by standard sanitizer runs. Production-representative soak tests with explicit LeakSanitizer are the only reliable way to catch slow leaks. "It doesn't leak in our test suite" means nothing unless your test suite runs for days with the same workload distribution as production. Treat memory growth graphs in production monitoring as seriously as latency graphs — add them if they're missing.
````

---

### Step 2.6 — Commit interview-master

- [ ] `git add tutorial/interview-master/ && git commit -m "docs(tutorial): add interview master"`
