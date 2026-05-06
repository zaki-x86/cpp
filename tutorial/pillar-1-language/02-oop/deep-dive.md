# OOP — Deep Dive

> Reference-grade. vtable layout, devirtualization, copy-and-swap, diamond inheritance, NVI, PIMPL, slicing.

---

## vtable Layout

Every class with at least one virtual function has exactly one **vtable** — a static array of function pointers, shared by all instances of the class. Each instance stores a `vptr` (a hidden pointer to its class's vtable) at offset 0 of the object.

```
Object layout (for class B with two virtual functions):
[ vptr ] ──────────────────────────────────────────► vtable for B:
[ member1 ]                                           [ &B::~B        ]   slot 0 (destructor)
[ member2 ]                                           [ &B::virtual1  ]   slot 1
                                                      [ &B::virtual2  ]   slot 2

Object layout for class D : B (overrides virtual1):
[ vptr ] ──────────────────────────────────────────► vtable for D:
[ B::member1 ]                                        [ &D::~D        ]   slot 0 (overrides)
[ B::member2 ]                                        [ &D::virtual1  ]   slot 1 (overrides)
[ D::member3 ]                                        [ &B::virtual2  ]   slot 2 (inherited)
```

The `vptr` is set by the constructor of each class in the hierarchy. When `B`'s constructor runs, `vptr` points to B's vtable. When `D`'s constructor runs, `vptr` is updated to point to D's vtable. This is why calling a virtual function from a constructor does not dispatch to the derived class — the vptr has not been updated yet.

With multiple inheritance, a derived class may have multiple `vptr`s — one per base class that has virtual functions. The derived class's vtable may have multiple sections (one per base), and pointer adjustments (`this` adjustments stored in the vtable) are needed when calling through a secondary base pointer.

---

## Devirtualization by the Compiler

The compiler can replace an indirect vtable call with a direct call (or inline it entirely) when it knows the dynamic type at the call site. This is **devirtualization**.

When it happens:
- **`final` classes:** `class D final : B`. The compiler knows no further derivation is possible.
- **Local variable of a known type:** `D obj; obj.virtual_fn();` — the compiler knows `obj` is `D`.
- **`final` virtual functions:** `virtual void foo() final` — cannot be overridden, compiler can devirtualize.
- **Whole-program optimization (LTO):** at link time, the optimizer can see all call sites and may devirtualize even without `final`.

Devirtualization matters most in tight inner loops. A virtual call in a loop body that processes 10,000 elements is 10,000 indirect calls. Devirtualization turns these into 10,000 direct calls (or one inlined loop body).

Verify with `g++ -O2 -fdump-tree-all` and inspect the `.optimized` dump, or use Compiler Explorer (godbolt.org) and look for `call` vs `callq *%rax`.

---

## Copy-and-Swap Idiom

The copy-and-swap idiom implements copy assignment with strong exception safety and self-assignment safety in three lines:

```cpp
Buffer& Buffer::operator=(Buffer other) {  // pass by value: copy or move
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    return *this;  // other.~Buffer() runs here, releasing the old data
}
```

Why it works:
1. **Exception safety:** The copy (or move) happens when constructing `other` before the function body runs. If the copy constructor throws, `*this` is unchanged — strong exception safety.
2. **Self-assignment safe:** `b = b` copies `b` into `other`, swaps, then destroys the copy. Correct.
3. **Move-aware:** If called with an rvalue (`b = std::move(c)`), `other` is move-constructed — O(1), no copy.

The trade-off: a self-assignment check (`if (this == &other) return *this`) would be faster for the pathological self-assign case, but self-assignment is so rare that the savings are not worth complicating the code.

---

## The Diamond Problem and Virtual Inheritance

```
    A
   / \
  B   C
   \ /
    D
```

Without virtual inheritance, `D` contains two `A` subobjects: one from `B` and one from `C`. Any access to `A`'s members from `D` is ambiguous. The fix: declare `B` and `C` as virtual bases of `A`:

```cpp
struct A { int x; };
struct B : virtual A {};
struct C : virtual A {};
struct D : B, C {};  // D has exactly one A::x
```

With virtual inheritance, the single shared `A` subobject is at a runtime-determined offset from the `B` and `C` subobjects. This offset is stored in a **vbtable** (virtual base table), separate from the vtable. Accessing `A::x` through `B*` requires loading the vbtable to find `A`'s offset — one indirection.

Construction rule: the **most-derived class** in the hierarchy is responsible for constructing all virtual bases directly, bypassing the intermediate constructors. `D()` must call `A()` explicitly if `A` has a non-default constructor, because neither `B` nor `C` can guarantee they will agree on how to construct the shared `A`.

Virtual inheritance is rarely the right answer. If you find yourself reaching for it, consider whether composition or a cleaner hierarchy would serve better.

---

## Non-Virtual Interface Pattern

The NVI pattern separates the public API from the customization interface:

```cpp
class Reader {
public:
    // Public non-virtual: base class controls pre/post conditions
    std::string read(size_t n) {
        validate_n(n);              // pre: base enforces this always
        auto result = do_read(n);   // delegate to derived implementation
        update_stats(n);            // post: base enforces this always
        return result;
    }
    virtual ~Reader() = default;

private:
    virtual std::string do_read(size_t n) = 0;  // derived customizes here
    void validate_n(size_t n) { if (n == 0) throw std::invalid_argument("n=0"); }
    void update_stats(size_t n) { bytes_read_ += n; }
    size_t bytes_read_ = 0;
};
```

Advantages:
- Derived classes cannot bypass `validate_n` or `update_stats` — they only override `do_read`.
- You can add logging, locking, or tracing to every call site by modifying the base class.
- The virtual interface is private — external callers cannot call `do_read` directly.

This is the C++ implementation of the Template Method design pattern.

---

## PIMPL for ABI Stability

The pimpl idiom shields downstream code from changes to your implementation details:

```cpp
// header: widget.hpp (public interface — stable ABI)
class Widget {
public:
    Widget();
    ~Widget();  // must be defined in .cpp where Impl is complete
    void render();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// source: widget.cpp (private implementation — free to change)
struct Widget::Impl {
    int x, y, width, height;
    std::vector<std::string> labels;
    void do_render() { /* ... */ }
};

Widget::Widget() : impl_(std::make_unique<Impl>()) {}
Widget::~Widget() = default;  // unique_ptr<Impl> destructor needs complete Impl
void Widget::render() { impl_->do_render(); }
```

Important: `Widget`'s destructor must be defined in the `.cpp` file, not defaulted in the header. If you default it in the header, `unique_ptr<Impl>` will attempt to call `Impl`'s destructor there, where `Impl` is incomplete — a compile error. The destructor must be defined after `Impl` is complete.

Cost: one heap allocation per `Widget`, one pointer indirection per call. For widgets that are not hot-path objects, this is acceptable. For hot-path objects, prefer composition with a `Facade` pattern.

---

## Object Slicing

Object slicing happens when a derived object is copied into a base-type value:

```cpp
struct Animal { virtual std::string speak() { return "..."; } };
struct Dog : Animal { std::string speak() override { return "Woof"; } };

Dog dog;
Animal a = dog;         // sliced: Dog's speak() and members gone
a.speak();              // calls Animal::speak() — "..."
```

The assignment `Animal a = dog` copies only the `Animal` subobject. `Dog`'s data members are discarded. The virtual dispatch is resolved through the copy's static type (`Animal`) — the vtable says `Animal::speak`, not `Dog::speak`.

This does not happen with pointers or references:
```cpp
Animal* pa = &dog;      // no slicing: pa points to the full Dog object
pa->speak();            // calls Dog::speak() via vtable — "Woof"
```

Prevention: delete the copy constructor in abstract bases to make slicing a compile error:
```cpp
struct Animal {
    Animal(const Animal&) = delete;
    Animal& operator=(const Animal&) = delete;
    virtual ~Animal() = default;
    virtual std::string speak() = 0;
};
```

Now `Animal a = dog;` fails to compile.

---

## Abstract Base Class vs Concept

An ABC (abstract base class with pure virtual functions) provides runtime polymorphism: you can store `Dog*` and `Cat*` in a `vector<Animal*>` and dispatch at runtime. Cost: heap allocation, virtual dispatch overhead, pointer semantics.

A Concept (C++20) constrains template parameters at compile time: `template<Animal T> void speak(T& a)` works with any type that satisfies the `Animal` concept, without inheritance, without heap allocation, with full inlining. Cost: code bloat (a new instantiation per concrete type).

Use ABCs when you need:
- Runtime-polymorphic containers (heterogeneous collections)
- Plugin systems (types not known at compile time)
- Stable binary interfaces (shared library ABIs)

Use Concepts when you need:
- Compile-time polymorphism in templates
- Value semantics (no heap, no pointers)
- Maximum performance with zero dispatch overhead

For the best of both: define a Concept for compile-time use, and write a type-erased wrapper (like `std::function` or a custom `AnyShape`) for the runtime polymorphism case.
