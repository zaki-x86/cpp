# OOP — Interview Q&A

> Eight Q&A pairs. Full answers, common traps, senior follow-ups.

---

**Q1: What is the Rule of Five and when do you need it?**

**A:** The Rule of Five says: if you define any one of the five special member functions (destructor, copy constructor, copy assignment, move constructor, move assignment), you should define all five. You need it when your class directly manages a raw resource — a raw pointer, a file descriptor, a socket, a GPU allocation. The compiler-generated versions will be wrong: the default copy constructor will copy the pointer (two owners for one resource), and the default destructor will free it — leading to double-free on destruction. Writing all five ensures you control exactly how the resource is duplicated, transferred, and released.

**Trap:** "I just need to define a destructor and the compiler handles the rest." A user-defined destructor suppresses the compiler-generated move operations (they are not generated). Copy operations are still generated as deprecated. The result: your class silently loses move semantics and falls back to copy everywhere.

**Follow-up:** What is the Rule of Zero? Prefer designing classes so compiler-generated members are correct — use smart pointers and value types as members, and the compiler generates correct copy/move/destroy automatically.

---

**Q2: What is the difference between copy and move semantics?**

**A:** Copy duplicates the resource. For a `std::vector`, copying allocates new heap memory and copies all elements — O(n). Move transfers ownership without duplication. Moving a `std::vector` copies three internal values (pointer, size, capacity) and sets the source's pointer to null — O(1). The source is left in a valid but empty state. The compiler automatically selects move when the source is an rvalue (temporary, or a value cast with `std::move`). Move semantics allow returning large objects from functions by value without copying.

**Trap:** "`std::move` moves the object." `std::move` is a cast — it converts an lvalue to an rvalue reference (`T&&`). The actual resource transfer is done by the move constructor or move assignment operator. Calling `std::move` on an object that has no move constructor still copies.

**Follow-up:** What happens if you `std::move` a `const` object? A `const T&&` does not bind to the `T&&` parameter of a move constructor — there is no way to steal from a const object (you cannot null out its source). The overload resolution falls back to the copy constructor, which accepts `const T&`. You get a copy.

---

**Q3: How does virtual dispatch work internally?**

**A:** Every polymorphic object stores a `vptr` (pointer to the class's vtable) at offset 0. A virtual call `obj->foo()` loads `vptr` from the object, indexes into the vtable at `foo`'s slot, loads the function pointer, and calls it. The vtable is a static array of function pointers, one per virtual function, laid out at compile time. When a derived class overrides a virtual function, its vtable has the derived function's pointer in that slot. The base vtable retains the base function's pointer.

**Trap:** "Virtual dispatch is slow." A virtual call is one indirect branch — approximately 1–3 ns on a modern CPU with a warm instruction cache. It is only expensive when it prevents the compiler from inlining a hot inner loop, or when cache misses on the vtable pointer are frequent (cold cache, many small objects).

**Follow-up:** How can the compiler devirtualize a virtual call? With the `final` keyword on the class or function (compiler knows no override exists), or when the dynamic type is known at the call site (local variable of concrete type, or inlining a factory that returns a known type). Link-time optimization (LTO) can also devirtualize across translation units.

---

**Q4: What is CRTP and what advantage does it have over virtual dispatch?**

**A:** CRTP (Curiously Recurring Template Pattern) is a pattern where a base class template takes the derived class as its template parameter: `template<typename D> class Base`. The base calls derived methods via `static_cast<D*>(this)->method()`. Because `D` is known at compile time, the call is a direct function call — not an indirect vtable lookup. This enables full inlining and eliminates the `vptr` overhead per object. CRTP is the idiom for zero-overhead mixins: add behavior to a derived class without adding data or vtable overhead.

**Trap:** "CRTP is always faster than virtual." Often true in microbenchmarks, but devirtualization (via `final`, LTO, or known types) can make virtual calls equally fast. Profile before choosing CRTP for performance — the trade-off is loss of runtime polymorphism (you cannot mix `Base<Dog>` and `Base<Cat>` in the same container).

**Follow-up:** What problem does CRTP solve for mixins? You can add methods to `Derived` without it inheriting any data or virtual functions. `Printable<D>` adds `print()` to `D` by calling `D::to_string()` — pure compile-time delegation. Multiple CRTP bases stack without vtable overhead.

---

**Q5: What is the diamond problem and how does C++ solve it?**

**A:** The diamond problem arises when `D` inherits from `B` and `C`, both of which inherit from `A`. Without `virtual` inheritance, `D` contains two copies of `A`'s data members — one via `B` and one via `C`. Accessing `A::x` from `D` is ambiguous: `D::B::A::x` or `D::C::A::x`? The fix: declare `B : virtual A` and `C : virtual A`. The compiler ensures exactly one shared `A` subobject exists in `D`, accessed via a vbtable indirection.

**Trap:** "Just use virtual inheritance everywhere as a safety measure." Virtual inheritance has real costs: the shared base offset is stored in a vbtable (one extra indirection per virtual base access), and the most-derived class must construct virtual bases directly (complex initialization rules). Use it only when you have an actual diamond.

**Follow-up:** Who is responsible for constructing a virtual base? The most-derived concrete class in the hierarchy. If `A` has a non-default constructor, `D()` must call it explicitly — `B` and `C` constructors' calls to `A()` are silently skipped for virtual bases, to avoid double-construction.

---

**Q6: What is PIMPL and why would you use it?**

**A:** PIMPL (Pointer to Implementation) places all private data members and implementation details in a `struct Impl` defined only in the `.cpp` file. The public class holds only a `unique_ptr<Impl>`. This makes the binary ABI stable: adding, removing, or reordering private members does not change the header, so downstream code does not need to recompile. It also acts as a compilation firewall: changes to `Impl`'s dependencies do not propagate to the header's includers.

**Trap:** "PIMPL is just about information hiding." Information hiding is a side effect. The primary benefit is ABI stability for shared libraries and compilation speed in large codebases. The cost — one heap allocation per object and one pointer indirection per method call — is real and matters for hot-path objects.

**Follow-up:** How does PIMPL interact with the Rule of Five? You must define the destructor in the `.cpp` file (not defaulted in the header), because `unique_ptr<Impl>`'s destructor requires `Impl` to be complete. If you put `~MyClass() = default` in the header where `Impl` is incomplete, you get a compile error. Define it in `.cpp` after the full `Impl` definition.

---

**Q7: What is object slicing and how do you prevent it?**

**A:** Object slicing happens when a derived object is assigned or passed by value to a base-type variable. Only the base subobject is copied; derived-specific data members and overrides are silently discarded. The virtual dispatch through the sliced copy calls the base class's functions, not the derived class's. This is a silent bug — the program compiles and runs without error.

**Trap:** "Slicing only happens with assignment." It also happens when passing derived objects by value to functions with base-type parameters: `void process(Animal a)` called with a `Dog` argument slices the `Dog` silently.

**Follow-up:** What does the compiler do with a virtual function call on a sliced object? The call resolves at compile time to the base class version — there is no vptr in a value (only in heap/stack-allocated objects accessed via pointer or reference). The static type determines the function called.

---

**Q8: What is the Non-Virtual Interface pattern and why is it useful?**

**A:** The NVI pattern makes the public API non-virtual. The public method handles invariants (validation, logging, locking) and delegates to a private virtual `do_*` method for the customizable behavior. Derived classes override `do_*` but cannot bypass the base-class invariants enforced in the public method. This prevents a common bug where a derived class override forgets to call the base implementation and silently drops required behavior.

**Trap:** "You could just make the public method virtual and let derived classes call `super::method()`." Then derived classes can forget the `super` call or call it at the wrong time. NVI makes the invariants structurally impossible to bypass.

**Follow-up:** What is the Template Method design pattern and how does NVI relate to it? They are the same pattern. Template Method (from the GoF book) defines an algorithm skeleton in the base class and lets derived classes fill in specific steps. NVI is the idiomatic C++ implementation of Template Method using a public non-virtual method that calls private virtual hooks.
