# Design Patterns — Interview Q&A

> 8 questions with traps, correct answers, and follow-ups. Practice saying these aloud — interviewers watch for hesitation on the traps.

---

## Q1: What is type erasure, and how is it different from virtual dispatch?

**What they're testing:** Whether you can distinguish the external API from the internal mechanism.

**The trap:** "Type erasure uses vtables internally — so it is just virtual dispatch with extra steps."

**Correct answer:**

Type erasure and virtual dispatch solve the same surface problem — calling different concrete implementations through a uniform interface — but the constraint they impose on the stored type is different.

Virtual dispatch requires the stored type to **inherit** from a base class. The base class and the derived class must be designed to work together. You cannot store a lambda or a third-party type that was not written to inherit from your base.

Type erasure imposes **no inheritance requirement** on the stored type. The Concept+Model pattern wraps any conforming type in a `Model<T>` at the point of construction. The outer type — `std::function`, `std::any`, or your custom wrapper — is stable and copyable. The stored type only needs to satisfy a syntactic constraint (e.g., callable with the right signature).

Yes, the `Model<T>` class does inherit from `Concept` and uses a vtable internally. But that inheritance is an implementation detail of the wrapper, not a requirement on the stored type. The caller never sees it.

The practical difference: `std::function<void()>` can store a lambda, a free function pointer, a struct with `operator()()`, or a `std::bind` result. A virtual base cannot.

**Follow-up:** "What does the clone() method do in the Concept+Model pattern?"

It enables value semantics on the type-erased wrapper. Copying a `std::function` copies the stored callable — the copy constructor calls `clone()` on the current model, which creates a new `Model<F>` with a copy of the stored `F`. Without `clone()`, the outer class can only be moved, not copied.

---

## Q2: Explain the self-registering factory. What is the initialization order fiasco, and how does the Meyers singleton fix it?

**What they're testing:** Whether you understand static initialization and can reason about pre-main execution order.

**The trap:** "Put the factory map in a namespace-level static variable — it will be initialized before main."

**Correct answer:**

The self-registering factory uses static initializers to register product types. Each product type has a static object (e.g., a `Registrar`) whose constructor adds the product to a global registry. This registration happens before `main()`.

The problem: two translation units (TUs) that both contain static objects in namespace scope have **undefined initialization order** relative to each other across TUs (within a TU, order is declaration order). If the `Circle` registrar in `circle.cpp` runs before the factory map in `factory.cpp` is initialized, the registrar writes to a not-yet-constructed map — undefined behaviour.

The Meyers singleton (local static) fixes this because **local statics are initialized on first call**, not at startup. The factory function is:

```cpp
auto& registry() {
    static std::unordered_map<std::string, ...> reg;  // initialized on first call
    return reg;
}
```

When `Circle`'s registrar constructor calls `registry()["circle"] = ...`, `registry()` is called for the first time, which initializes `reg`, then performs the insertion. The map is always initialized before the insertion because the call is inside the function. No cross-TU ordering dependency.

**Follow-up:** "Is this thread-safe?"

Static local initialization is thread-safe since C++11 (the compiler emits locking around the initialization). But **writes to the map after initialization** (e.g., from `dlopen`-loaded plugins after main starts) are not thread-safe. You need a `shared_mutex` for that scenario.

---

## Q3: Why is the Singleton pattern harmful in C++? Is a thread-safe Singleton acceptable?

**What they're testing:** Whether you understand the real problem with Singleton, which is not thread safety.

**The trap:** "Double-checked locking (DCLP) or `std::call_once` makes Singleton thread-safe, so the problems are solved."

**Correct answer:**

Thread safety is only one problem with Singleton, and it is the easiest one to fix. The deeper problems remain:

**1. Global mutable state creates hidden coupling.** Any component that calls `Database::instance()` is coupled to the global database. The coupling is not visible in the component's constructor or interface — it is hidden in the implementation. When two components that both use the singleton interact, their behaviour depends on shared state in ways that are not obvious from reading either component alone.

**2. Untestable by default.** Tests that use the singleton share state between test cases. Tests that run in a different order produce different results. The singleton cannot be replaced with a mock for isolated unit testing — you cannot inject a `FakeDatabase` in place of `Database::instance()`.

**3. Initialization order fiasco (between singletons).** If `Database::instance()` calls `Logger::instance()` during initialization, and `Logger::instance()` calls `Database::instance()`, you have a circular dependency that causes a crash or undefined behaviour.

**The fix:** dependency injection. Pass the shared object as a constructor parameter. The caller controls the lifetime. Tests can inject a mock. Components declare their dependencies explicitly in their constructors.

A thread-safe Singleton using `std::call_once` or a Meyers singleton fixes the data race. It does not fix the coupling, testability, or initialization order problems.

**Follow-up:** "When is Singleton acceptable?"

A read-only singleton with no mutable state — e.g., a compiled regex pattern or a static configuration loaded once — is acceptable. The risk is coupling and testability, not thread safety; both risks are low when the singleton is immutable.

---

## Q4: How do you implement a thread-safe Observer? What is the deadlock risk?

**What they're testing:** Whether you know the snapshot-notification pattern and can reason about lock ordering.

**The trap:** "Hold the mutex during notification to prevent concurrent modifications."

**Correct answer:**

Holding the mutex during notification causes a deadlock if any observer callback attempts to subscribe or unsubscribe — those operations also need the mutex.

The correct approach is **snapshot-notify**:

```
1. Acquire mutex
2. Copy the observer list to a local snapshot
3. Release mutex
4. Iterate the snapshot, calling each observer
```

Notification happens outside the lock. The observer list may change between the copy and the notification — new subscribers miss this notification, deregistered observers appear in the snapshot but the `weak_ptr::lock()` call returns null, so they are skipped safely.

Additional requirements:
- Store observers as `weak_ptr` to handle lifetime — subjects must not outlive observers without the observer explicitly deregistering.
- Compact expired `weak_ptr`s periodically (on subscribe, or on a background task) to prevent the list from growing unboundedly.

**C++20 alternative for read-heavy workloads:** use `std::atomic<std::shared_ptr<vector<Observer*>>>`. Readers load the shared_ptr atomically (no lock). Writers create a new vector with the modification applied and atomically store it. Notification is lock-free. Subscription requires an atomic compare-exchange loop but is infrequent.

**Follow-up:** "What happens if an observer throws during notification?"

With the naive design, an exception from one observer prevents later observers from being notified. The standard fix: catch exceptions per-observer and either log them or accumulate them for re-throw after all observers have been notified.

---

## Q5: Explain why ECS is more cache-friendly than OOP for large entity counts. Is ECS just a database pattern?

**What they're testing:** Whether you understand cache effects and can distinguish data layout from data access pattern.

**The trap:** "ECS is just a database pattern — it stores entities as rows with component columns, same as a relational table."

**Correct answer:**

ECS is motivated by cache behaviour, which is distinct from relational database design.

In OOP, each entity is an object. All its components are stored as member variables inside the object. Iterating N entities for a physics update loads each entity's full object into cache — position, velocity, health, name, model pointer, AI state — even though physics only needs position and velocity. Every cache line is partially wasted on irrelevant data.

In ECS, each component type has its own contiguous array indexed by entity ID. Position[0..N] is packed in memory. Velocity[0..N] is packed in memory. The physics system reads Position[] and Velocity[] sequentially — every cache line loaded is fully used. The CPU prefetcher fills the cache ahead of the iteration. No pointer chasing. At 10,000+ entities, the result is 10–100× faster iteration.

The relational database comparison is superficially apt — ECS does store entities as IDs with associated components, similar to rows with columns. But a relational database is optimized for ad-hoc queries, transactions, and joins with rich type systems. ECS is optimized for **bulk iteration over fixed component combinations** in hot loops at 60 Hz. The similarities are in structure; the design goals are different.

**Follow-up:** "What is a sparse set, and why do production ECS implementations use it?"

A hash map for component storage has O(1) average insert/erase but poor cache performance because hash maps scatter memory. A sparse set pairs a dense array (for fast iteration) with a sparse array (for O(1) lookup by entity ID). Iteration is as fast as a plain array. Lookup and modification are O(1). Insert/remove is O(1) amortized. This gives production ECS (EnTT, Flecs) both fast bulk iteration and fast entity modification.

---

## Q6: Does `std::function` always heap-allocate? What is small buffer optimization?

**What they're testing:** Whether you know that `std::function` has a fast path for small callables.

**The trap:** "std::function always heap-allocates — use raw function pointers in performance-critical code."

**Correct answer:**

`std::function` uses small buffer optimization (SBO). libstdc++ embeds a small fixed-size buffer (16 bytes on most implementations) inside the `std::function` object. If the stored callable — including its captures — fits in the buffer and satisfies alignment, it is constructed in-place using placement new. No heap allocation. No memory management overhead.

The common cases that avoid heap allocation:
- A free function pointer (fits in a pointer-size buffer)
- A lambda with no captures (zero-size closure)
- A lambda capturing a single pointer or reference

The cases that require heap allocation:
- A lambda capturing a `std::string`, a `std::vector`, or multiple values whose total size exceeds the SBO buffer

The test: check `sizeof` of the closure. If `sizeof(closure) <= 16` (platform-dependent), it fits. If not, it heap-allocates.

**Practical rule:** use `std::function` freely in non-hot paths. In hot paths where `std::function` shows up in profiling, either use a template parameter (zero overhead, compile-time dispatch) or ensure your closures are small enough for SBO.

**Follow-up:** "What is `std::launder` and when is it needed with placement new?"

When you construct a new object at a memory address using placement new, the compiler may have cached the value of pointers to that memory region based on the old object's type. `std::launder` tells the compiler to treat the pointer as pointing to a new live object of the new type, preventing it from using stale cached values. Required when reusing aligned storage.

---

## Q7: Implement a CRTP decorator. How is it different from the GoF runtime decorator?

**What they're testing:** Whether you can produce correct CRTP code and articulate the trade-off.

**The trap:** "CRTP decorator is the same as GoF decorator — just a different syntax."

**Correct answer:**

CRTP decorator:

```cpp
template<typename Base>
class Logging : public Base {
public:
    template<typename... Args>
    explicit Logging(Args&&... args) : Base(std::forward<Args>(args)...) {}

    void send(Packet p) {
        std::cout << "LOG: sending\n";
        Base::send(p);     // not virtual — resolved at compile time
    }
};

template<typename Base>
class Caching : public Base {
    std::unordered_set<Packet> cache_;
public:
    void send(Packet p) {
        if (cache_.count(p)) return;
        cache_.insert(p);
        Base::send(p);
    }
};

using Stack = Logging<Caching<Network>>;
Stack s;
s.send(packet);   // Logging::send → Caching::send → Network::send — all inlined
```

GoF runtime decorator:

```cpp
class LoggingDecorator : public NetworkBase {
    std::unique_ptr<NetworkBase> inner_;
public:
    void send(Packet p) override {
        std::cout << "LOG: sending\n";
        inner_->send(p);  // virtual call — pointer indirection
    }
};
```

**The differences:**

| Aspect | CRTP Decorator | GoF Runtime Decorator |
|--------|---------------|----------------------|
| Stack chosen | Compile time | Runtime |
| Overhead | Zero (inlined) | vtable call per layer |
| Heap allocation | No | Yes (each decorator) |
| Can switch stack at runtime | No | Yes |
| Supports unknown base types | No (template) | Yes (virtual base) |

Use CRTP when the stack is known at compile time and performance matters. Use GoF runtime decorator when you need to compose decorators dynamically (e.g., from user config at startup).

**Follow-up:** "What is the diamond problem with CRTP decorator, and how do you avoid it?"

If two decorators inherit from the same base, and you try to compose them, you get two copies of the base. Avoid by ensuring each decorator inherits only from its `Base` template parameter — no additional base classes. The chain is linear: `A<B<C>>` never has a diamond.

---

## Q8: When do you choose compile-time polymorphism over runtime polymorphism?

**What they're testing:** Whether you can reason about trade-offs rather than applying a dogmatic rule.

**The trap:** "Virtual dispatch is always slower — always prefer templates."

**Correct answer:**

Compile-time polymorphism (templates, CRTP) and runtime polymorphism (virtual dispatch) have different applicability conditions. The performance difference is often irrelevant outside hot loops.

**Choose compile-time polymorphism when:**
- The concrete type is known at compile time and will not change
- You are in a hot loop with 10,000+ iterations (vtable overhead becomes measurable)
- You want zero-overhead abstractions — no heap allocation, no indirection
- You are writing a library where the caller supplies a policy type

**Choose runtime polymorphism when:**
- The concrete type is determined at runtime (user input, config file, plugin)
- You need a heterogeneous collection where the types are not fixed at compile time
- The concrete type may be extended by code you do not know about (library users, plugin authors)
- Compile times are already long and additional template instantiations are costly

**The measurement rule:** a vtable dispatch costs roughly 1–3 ns on modern hardware. It is measurable only when called millions of times per second in a tight loop. For most application code — event handlers, UI callbacks, service layer calls — the difference is in the noise. Profile before optimizing.

**Follow-up:** "Can you mix compile-time and runtime polymorphism?"

Yes. Type erasure is the bridge: the outer interface is runtime (you can store any conforming type), but the stored model is generated by compile-time template instantiation. `std::function` is runtime from the caller's perspective (different callables are interchangeable at runtime), but the dispatch from the outer wrapper to the model IS a single virtual call (through the `Concept` vtable). Inside the model, the call to the stored callable `f_()` is direct and non-virtual — no additional vtable lookup.
