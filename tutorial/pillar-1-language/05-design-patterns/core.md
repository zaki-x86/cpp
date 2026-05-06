# Design Patterns — Core

> The mental model. Read this before anything else in this chapter.

---

## C++ Makes Some Patterns Unnecessary

GoF patterns solve problems that the language cannot solve natively. C++ solves several of those problems at the language level — which means applying the GoF solution is redundant at best and harmful at worst.

**Garbage collection → RAII handles resource lifetime.**
The Dispose/Finalizer pattern from Java and C# exists because the GC does not know when a resource should be released — only when memory should be reclaimed. In C++, RAII ties resource lifetime to object scope. `unique_ptr`, `shared_ptr`, `lock_guard`, `ofstream` — all of them acquire in the constructor and release in the destructor. You do not need a separate cleanup pattern because the pattern is built into the object model. Every resource in a well-written C++ program is owned by an RAII wrapper.

**Iterator pattern → built into range-for.**
The GoF Iterator pattern defines an interface for traversing a container without exposing its representation. In C++, this is provided by the range-for loop and the iterator concept. Any type that implements `begin()` and `end()` returning objects that support `++`, `*`, and `!=` works with range-for. You do not write an Iterator class hierarchy. You write a `begin()` and an `end()`.

**Abstract factory → template policy achieves the same at compile time.**
The GoF Abstract Factory creates families of related objects. In C++, template policies compose behaviour at compile time. `std::allocator` is a policy. `std::char_traits` is a policy. The policy pattern gives you the same configurability as an abstract factory but with zero virtual dispatch overhead, evaluated entirely at compile time. When your "factory" choice is known at compile time, use a policy. Only reach for runtime polymorphism when the concrete type is chosen at runtime.

**The C++ standard library is itself a pattern library.**
Before inventing patterns, learn the stdlib. `std::function` is type erasure. `std::visit` is the visitor pattern. `std::optional` is the null object pattern. `ranges::transform` is the strategy pattern. If you find yourself implementing a pattern, ask first whether `<algorithm>`, `<functional>`, or `<ranges>` already provides it.

---

## C++ Makes Some Patterns Better

Several GoF patterns have C++ implementations that are strictly superior to the Java/C++ version described in the book.

**Type erasure outperforms vtable-based interfaces in many contexts.**
The GoF strategy pattern uses a virtual base class. Every strategy call goes through a vtable, requires heap allocation for the strategy object, and adds a layer of indirection. Type erasure achieves the same external interface — a uniform way to call different implementations — but removes the inheritance requirement on the stored type. You can store a lambda, a function pointer, a struct with `operator()`, or a class with a named method, all behind the same type-erased wrapper. `std::function` does this. In hot paths, small-buffer optimization avoids heap allocation entirely. No inheritance required. Better cache behaviour. Same external API.

**CRTP decorator composes behaviours without runtime cost.**
The GoF Decorator pattern wraps an object in another object that adds behaviour, using a shared virtual base class. The runtime cost includes a vtable lookup per method call and a heap allocation per decorator layer. CRTP decorator uses template inheritance: `Logging<Caching<Network>>`. Every combination resolves at compile time. Zero vtable lookups. Zero additional heap allocations. The chain of decorators is collapsed by the optimizer into a single inlined call. The only cost is compile time.

**Self-registering factory eliminates registration boilerplate.**
The GoF factory method requires a central switch or if-else chain that must be updated whenever a new product type is added. The self-registering factory turns this inside out: each product type registers itself into a global map via a static initializer. The factory itself never changes. You add a new type by writing the type and one registration line — the factory picks it up automatically. This is the open/closed principle in its purest C++ form.

---

## C++ Makes Some Patterns Dangerous

Some GoF patterns, applied in C++, create more problems than they solve.

**Singleton: global mutable state, initialization order fiasco, untestable code.**
The Singleton pattern ensures that only one instance of a class exists. The problem in C++ is not thread safety (which is solvable with `std::call_once` or a Meyers singleton). The problem is that singletons are global mutable state. Global mutable state creates hidden coupling between components. Tests that run in any order will interfere with each other. Components that depend on a singleton cannot be given a mock. The initialization order fiasco means that two singletons cannot safely depend on each other at startup unless you are careful with Meyers singletons. Use dependency injection instead: pass the shared object as a constructor parameter. The caller controls lifetime. Tests can substitute a mock. No hidden coupling.

**Global observer: lifetime issues when subjects outlive observers.**
The naive observer stores `vector<Observer*>`. If an observer is destroyed while still registered, the subject holds a dangling pointer. Notifying a dangling pointer is undefined behaviour. The problem is worse in multi-threaded code: an observer can be destroyed on one thread while the subject is notifying on another. Solutions exist (weak_ptr, deregistration in the observer destructor) but require deliberate design. The pattern is not banned, but the raw pointer version is a bug waiting to happen.

**Mutable global: data races in multi-threaded code.**
Any mutable global — whether a singleton, a global observer registry, or a global factory map — is a data race if accessed concurrently without synchronization. The C++ memory model does not protect you. A global `unordered_map` modified from multiple threads without a mutex is undefined behaviour. This applies to the self-registering factory: if registrations happen after `main()` starts (from multiple threads), the map must be protected. Static initializers before `main()` are safe (single-threaded). Static initializers triggered by dynamic library loading are not.

---

## The Five Patterns You Actually Use

In order of frequency in production C++ codebases:

**1. RAII — every resource.**
Not a GoF pattern. The most important C++ pattern. Every resource — memory, file handles, sockets, mutexes, GPU buffers, database connections — should be owned by an RAII wrapper. Acquire in the constructor. Release in the destructor. Never write `delete`, `fclose`, `CloseHandle` directly. Always use `unique_ptr`, `shared_ptr`, `lock_guard`, `scoped_lock`, `unique_lock`, or a custom RAII wrapper. Chapter 01-memory covers this in depth.

**2. Observer — event systems, UI, messaging.**
One subject, many observers. The subject does not know the observers' concrete types — it only knows their interface. In C++, observers are typically stored as `std::function<void(const Event&)>` callbacks (type-erased) or as `shared_ptr<Observer>` (with `weak_ptr` in the registry to handle lifetime). Used everywhere: Qt signals/slots, ROS2 topic subscriptions, game event systems, GUI frameworks.

**3. Factory — plugin systems, testing.**
Decouple construction from use. The caller says "give me a Shape" without knowing whether it gets a Circle or a Rectangle. Enables plugins (load a .so, it registers its types) and testing (inject a MockDatabase instead of a RealDatabase). The self-registering variant is the C++ idiom — it eliminates the central dispatch table.

**4. Type Erasure — heterogeneous containers, callbacks.**
Store objects of different types behind a uniform interface without requiring a common base class. `std::function<void()>` is the canonical example — it holds a lambda, a free function, or a struct with `operator()()`. Heterogeneous containers of drawables, events, commands, or tasks all use this pattern. Understand Concept+Model before your next technical interview.

**5. ECS — game engines, simulation, any system needing cache-friendly bulk updates.**
Entity Component System. Traditional OOP: each entity is an object, components are member variables, iterating entities accesses scattered memory. ECS: components stored in contiguous arrays indexed by entity ID. Iterating 10,000 position+velocity pairs accesses two arrays sequentially — all cache lines are used, zero pointer chasing. Adding or removing a component does not invalidate other components. Systems are pure functions over component views. Used in Unity (DOTS), Unreal, every AAA engine, robotics simulation. Learn it before your first game engine or simulation interview.

---

## Production Rules

**Avoid singletons — use dependency injection.**
Pass shared objects as constructor parameters. Callers control lifetime. Tests substitute mocks. No initialization order fiasco.

**Prefer type erasure over virtual when the types are known statically.**
If you can template the caller, use templates. If you must store heterogeneous objects in a container, use type erasure. Only use virtual inheritance when the concrete type must be extensible by unknown code (plugin systems, user-supplied callbacks in a library API).

**Register factories at startup via static initializers, not via if-else chains.**
The central dispatch table grows without bound and must be updated for every new type. The self-registering factory never changes — each type registers itself. Use a Meyers singleton for the registry map to avoid the initialization order fiasco.

**Use ECS when you iterate over 10,000+ entities of similar type.**
OOP entity hierarchies are fine for small scenes. At scale, cache misses dominate — OOP layouts scatter related data across heap allocations. ECS packs data in arrays. The crossover point is roughly 1,000–10,000 entities depending on component size and update frequency. Profile before refactoring.

**Prefer `std::function` over raw function pointers.**
`std::function<void()>` is type-erased: it captures lambdas, closures with state, and anything callable with the right signature. Raw function pointers cannot capture state. The cost of `std::function` is a possible heap allocation (mitigated by SBO for small lambdas) and a vtable lookup per call. In non-hot paths, the ergonomics win. In hot paths, benchmark and consider a direct template parameter instead.

---

## Lab

The foundation library implements three of these patterns:

- `projects/02-foundation/include/foundation/patterns/type_erasure.hpp` — `AnyCallable<Ret(Args...)>`: a pedagogical `std::function`. Concept+Model pattern with `clone()` for copy semantics.
- `projects/02-foundation/include/foundation/patterns/factory.hpp` — `AnimalFactory`: self-registering factory with a Meyers singleton registry. `Dog` and `Cat` register themselves via `static const bool registered_`.
- `projects/02-foundation/include/foundation/patterns/observer.hpp` — observer implementation.

Run the pattern tests: `ctest --preset debug -R test_patterns` from `projects/02-foundation/`.

Then read `examples/01_type_erasure.cpp` (simplified AnyCallable, void() only), `examples/02_self_reg_factory.cpp` (Shape factory), and `examples/03_ecs_world.cpp` (3-component ECS world with game loop).
