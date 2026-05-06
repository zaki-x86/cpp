# Design Patterns — Deep Dive

> Reference-grade. Internals, implementation details, and architecture diagrams. Return here when you need to implement, not just use.

---

## Type Erasure: The Concept + Model Pattern

Type erasure solves a specific problem: you want a container or function parameter that accepts objects of many unrelated types, without requiring those types to share a common base class. The caller side is uniform. The stored types are unrestricted at the definition site.

The three-part implementation:

```
┌─────────────────────────────────────────────────────────┐
│                   AnyCallable                           │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │  unique_ptr<Concept>  self_                     │   │
│  └────────────────────────┬────────────────────────┘   │
│                           │ points to                   │
└───────────────────────────┼─────────────────────────────┘
                            │
               ┌────────────▼────────────────┐
               │     Concept  (abstract)     │
               │  + call() = 0              │
               │  + clone() = 0             │
               └────────────┬────────────────┘
                            │ inherits
               ┌────────────▼────────────────┐
               │    Model<F>  (concrete)     │
               │  F f_                       │
               │  call() { f_(); }           │
               │  clone() { new Model(f_); } │
               └─────────────────────────────┘
```

**Component 1: Concept** — a pure virtual inner class defining the interface.

```cpp
struct Concept {
    virtual ~Concept() = default;
    virtual void call() = 0;
    virtual Concept* clone() const = 0;
};
```

**Component 2: Model<F>** — a class template that wraps any `F` conforming to the interface via virtual inheritance of Concept. The constraint on `F` is only checked at instantiation time, not at the definition of `AnyCallable`.

```cpp
template<typename F>
struct Model final : Concept {
    F f_;
    explicit Model(F f) : f_(std::move(f)) {}
    void call() override { f_(); }
    Concept* clone() const override { return new Model(f_); }
};
```

**Component 3: AnyCallable** — the outer class that owns a `unique_ptr<Concept>`. The constructor is templated and creates the right `Model<F>`. `operator()` delegates. Copy constructor calls `clone()`.

```cpp
class AnyCallable {
    std::unique_ptr<Concept> self_;
public:
    template<typename F>
    AnyCallable(F f) : self_(new Model<std::decay_t<F>>(std::move(f))) {}

    void operator()() const { self_->call(); }

    AnyCallable(const AnyCallable& other)
        : self_(other.self_ ? other.self_->clone() : nullptr) {}
};
```

This is how `std::function`, `std::any`, and C++20 `std::ranges::owning_view` are implemented internally. The outer type is stable and copyable. The inner model is invisible to the caller. No inheritance required on the stored type.

---

## Small Buffer Optimization for Type-Erased Objects

The Concept+Model pattern has one cost: every stored callable requires a heap allocation for the `Model<F>` object. In code that creates many short-lived `std::function` objects (event dispatchers, task queues), this becomes significant.

**The SBO idea:** embed a small fixed-size buffer inside the outer class. If the model fits, construct it in-place (no heap). If it does not fit, fall back to heap allocation.

```
┌─────────────────────────────────────────────────┐
│               AnyCallable (SBO)                 │
│                                                 │
│  aligned_storage<32> buf_                       │
│  Concept*            ptr_     ──► buf_ (small)  │
│  bool                on_heap_     heap  (large) │
│                                                 │
└─────────────────────────────────────────────────┘
```

Implementation sketch:

```cpp
static constexpr std::size_t kBufSize = 32;
alignas(std::max_align_t) char buf_[kBufSize];
Concept* ptr_;
bool on_heap_;

template<typename F>
AnyCallable(F f) {
    using M = Model<std::decay_t<F>>;
    if constexpr (sizeof(M) <= kBufSize &&
                  alignof(M) <= alignof(std::max_align_t)) {
        ptr_ = new (buf_) M(std::move(f));  // placement new
        on_heap_ = false;
    } else {
        ptr_ = new M(std::move(f));         // heap
        on_heap_ = true;
    }
}

~AnyCallable() {
    if (on_heap_) {
        delete ptr_;
    } else {
        ptr_->~Concept();  // explicit destructor for placement-new object
    }
}
```

After placement new, access through the pointer requires `std::launder` if the buffer is reused, because the compiler must be told that the buffer now holds a live object of a different type:

```cpp
ptr_ = std::launder(reinterpret_cast<Concept*>(buf_));
```

`std::function` in libstdc++ uses SBO with a 16-byte buffer. Lambdas with no captures or a single pointer capture fit; lambdas with multiple captures do not. This is why capturing `[this, &ctx, &log]` in a hot loop is slower than `[this]`.

---

## Self-Registering Factory with Static Initializers

The goal: adding a new product type to the factory requires touching only one file — the product type's own file. The factory itself never changes.

**The registry:** a function returning a Meyers singleton `unordered_map`. The local static is initialized on first call, thread-safe since C++11.

```cpp
auto& registry() {
    static std::unordered_map<std::string,
                              std::function<std::unique_ptr<Shape>()>> reg;
    return reg;
}
```

The Meyers singleton (local static) avoids the initialization order fiasco. If you had used a namespace-level static, the order of initialization relative to other translation units would be undefined. With a local static, initialization happens on the first call to `registry()` — which is guaranteed to happen before any use.

**The registrar:** a small struct whose constructor performs registration.

```cpp
struct Registrar {
    Registrar(const std::string& name,
              std::function<std::unique_ptr<Shape>()> fn) {
        registry()[name] = std::move(fn);
    }
};
```

**The macro:** reduces boilerplate.

```cpp
#define REGISTER_SHAPE(name, type)                           \
    static Registrar reg_##type{                             \
        name, []{ return std::make_unique<type>(); }         \
    }
```

The static variable `reg_##type` is initialized before `main()` (for objects with static storage duration). Its constructor runs `registry()[name] = ...`. By the time `main()` calls `registry()["circle"]()`, the registration is already present.

```
Static initialization timeline:

[before main()]
  reg_Circle constructor → registry()["circle"] = create_Circle
  reg_Rectangle constructor → registry()["rectangle"] = create_Rectangle

[main() runs]
  registry()["circle"]()   → Circle::make_unique()
  registry()["rectangle"]() → Rectangle::make_unique()
```

**Risk:** if a plugin `.so` is loaded after `main()` starts (via `dlopen`), the plugin's static initializers run on the loading thread. If another thread is simultaneously reading the registry map, that is a data race. Protect the registry with a `shared_mutex` for dynamic plugin scenarios.

---

## Thread-Safe Observer

The naive observer pattern:

```cpp
class Subject {
    std::vector<Observer*> observers_;
public:
    void notify(const Event& e) {
        for (auto* obs : observers_) obs->on_event(e);  // BUG: modifying during iteration
    }
};
```

Two classes of bugs:

1. **Modification during iteration.** If `obs->on_event(e)` triggers a call to `unsubscribe()`, `observers_` is modified while being iterated — undefined behaviour.
2. **Concurrent access.** If `subscribe()`, `unsubscribe()`, and `notify()` are called from different threads, there is a data race on `observers_`.

**Thread-safe design with snapshot notification:**

```cpp
class Subject {
    std::mutex mu_;
    std::vector<std::weak_ptr<Observer>> observers_;
public:
    void notify(const Event& e) {
        // 1. Take a snapshot under lock
        std::vector<std::weak_ptr<Observer>> snapshot;
        {
            std::lock_guard lock(mu_);
            snapshot = observers_;
        }
        // 2. Notify outside the lock — no deadlock risk
        for (auto& wk : snapshot) {
            if (auto obs = wk.lock()) obs->on_event(e);
        }
    }
};
```

Key decisions:
- **Copy the list under lock, notify outside the lock.** This means the lock is not held while calling back into user code. User code may call `subscribe()` or `unsubscribe()` without deadlocking.
- **Use `weak_ptr` for observer storage.** If the observer is destroyed, `wk.lock()` returns `nullptr`. No dangling pointers. No explicit deregistration required (though it is still courteous to deregister to keep the list compact).
- **Periodically compact.** Expired `weak_ptr`s accumulate. Compact the list on `subscribe()` calls or on a periodic background task.

**C++20 lock-free read path:**

```cpp
std::atomic<std::shared_ptr<std::vector<Observer*>>> observers_;
```

Readers load the shared_ptr atomically (lock-free on most platforms). Writers create a new vector, copy the old contents plus the new observer, then atomically store the new shared_ptr. Notification reads the shared_ptr once — no lock, no blocking. Suitable for systems where notification dominates subscription in frequency.

---

## CRTP Decorator for Mixin Composition

GoF Decorator at runtime:

```cpp
class LoggingNetwork : public Network {    // virtual overhead
    Network* inner_;                       // heap allocation
public:
    void send(Packet p) override {
        log("sending"); inner_->send(p);
    }
};
```

CRTP Decorator at compile time:

```cpp
template<typename Base>
class Logging : public Base {
public:
    void send(Packet p) {
        log("sending");
        Base::send(p);          // non-virtual call — inlined
    }
};

template<typename Base>
class Caching : public Base {
public:
    void send(Packet p) {
        if (cache_.has(p)) return;
        Base::send(p);
        cache_.insert(p);
    }
};

using Stack = Logging<Caching<Network>>;
Stack s;
s.send(packet);   // calls Logging::send → Caching::send → Network::send
                  // all inlined by the compiler — no vtable lookups
```

Order matters: `Logging<Caching<Network>>` logs before checking cache. `Caching<Logging<Network>>` checks cache before logging — nothing is logged for cache hits.

```
Call chain for Logging<Caching<Network>>:

Logging::send(p)
  │  log("sending")
  └─► Caching::send(p)          ← Base::send(p) in Logging
        │  cache check
        └─► Network::send(p)    ← Base::send(p) in Caching
```

Each decorator template has zero runtime overhead compared to calling through one or more vtable pointers. The optimizer sees through all the `Base::send` calls and inlines the chain.

**Limitation:** the decorator stack is fixed at compile time. You cannot choose `Logging` vs. `NoLogging` at runtime with CRTP. For runtime selection, use the GoF runtime decorator — and accept the overhead.

---

## Command Pattern with Undo/Redo

The command pattern encapsulates an action as an object, enabling deferred execution, logging, and undo.

```cpp
struct Command {
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};
```

A history stack:

```cpp
class CommandHistory {
    std::vector<std::unique_ptr<Command>> history_;
    std::vector<std::unique_ptr<Command>> redo_stack_;
public:
    void execute(std::unique_ptr<Command> cmd) {
        cmd->execute();
        history_.push_back(std::move(cmd));
        redo_stack_.clear();   // new command invalidates redo stack
    }

    void undo() {
        if (history_.empty()) return;
        auto cmd = std::move(history_.back());
        history_.pop_back();
        cmd->undo();
        redo_stack_.push_back(std::move(cmd));
    }

    void redo() {
        if (redo_stack_.empty()) return;
        auto cmd = std::move(redo_stack_.back());
        redo_stack_.pop_back();
        cmd->execute();
        history_.push_back(std::move(cmd));
    }
};
```

**Lambda-based lightweight commands.** For simple reversible operations, a full class hierarchy is overkill. Pair a do-function and an undo-function:

```cpp
struct LambdaCommand : Command {
    std::function<void()> do_fn, undo_fn;
    void execute() override { do_fn(); }
    void undo()    override { undo_fn(); }
};

history.execute(std::make_unique<LambdaCommand>(
    LambdaCommand{
        [&]{ text.insert(pos, ch); },
        [&]{ text.erase(pos, 1);   }
    }
));
```

The closure captures the exact state needed for reversal. No subclass per operation. In text editors, CAD tools, and IDEs, this pattern handles hundreds of operations without specialised classes.

---

## ECS: Data-Oriented Design

**The OOP layout problem.**

```
OOP entity layout in memory:

Entity 0: [pos.x][pos.y][vel.dx][vel.dy][hp][name_ptr][ padding ]
Entity 1: [pos.x][pos.y][vel.dx][vel.dy][hp][name_ptr][ padding ]
Entity 2: [pos.x][pos.y][vel.dx][vel.dy][hp][name_ptr][ padding ]
           ↑
           Cache line fetched here includes hp and name_ptr — wasted
           when we only need pos and vel for physics update
```

For a physics update that needs only `Position` and `Velocity`, OOP reads every entity's full object — including `hp`, `name`, and padding that the physics system does not use. Each cache line is partially wasted.

**The ECS layout.**

```
ECS component arrays:

positions[] : [x0,y0][x1,y1][x2,y2][x3,y3][x4,y4][x5,y5] ...
               ↑ entire cache line is useful for physics
velocities[]: [dx0,dy0][dx1,dy1][dx2,dy2] ...

Physics system iterates:
  for entity in view<Position, Velocity>:
      positions[entity].x += velocities[entity].dx * dt
      positions[entity].y += velocities[entity].dy * dt
```

Both arrays fit in cache. The CPU prefetcher handles sequential access perfectly. No pointer chasing. At 10,000 entities, the difference is 10–100× faster iteration compared to OOP, depending on cache size and component size.

**ECS architecture diagram.**

```
┌──────────────────────────────────────────────────────────────┐
│                         World                                │
│                                                              │
│  entity_counter: 3                                           │
│                                                              │
│  storage<Position>:  { 0→{1,2}, 1→{3,4}, 2→{5,6} }         │
│  storage<Velocity>:  { 0→{0.1,0}, 1→{0,0.2}, 2→{0.1,0.1} } │
│  storage<Health>:    { 0→{100} }                             │
│                                                              │
│  view<Position,Velocity>()  → [0, 1, 2]                     │
│  view<Health>()             → [0]                            │
└──────────────────────────────────────────────────────────────┘
```

**Architectural properties:**

| Property | OOP Entity | ECS |
|----------|-----------|-----|
| Component access | `entity.pos` (direct) | `storage<Position>[id]` |
| Memory layout | Interleaved (bad for SIMD) | SoA (good for SIMD) |
| Add component at runtime | Not possible | O(1) map insert |
| Remove component | Rebuild class | O(1) map erase |
| System iteration | Pointer chasing | Sequential array scan |
| Cache utilisation | Poor (irrelevant fields loaded) | High (only needed fields) |

**Systems** in ECS are plain functions — not classes. They take a `World&` and iterate over a component view:

```cpp
void physics_system(World& world, float dt) {
    for (Entity e : world.view<Position, Velocity>()) {
        auto& pos = world.get_component<Position>(e);
        auto& vel = world.get_component<Velocity>(e);
        pos.x += vel.dx * dt;
        pos.y += vel.dy * dt;
    }
}
```

No inheritance. No virtual calls. No heap allocations during the update. The minimal ECS in `examples/03_ecs_world.cpp` uses `unordered_map<Entity, T>` per component type — correct semantics, not cache-optimal layout. A production ECS (EnTT, Flecs) uses packed arrays with sparse-set indexing for O(1) add/remove and cache-optimal iteration.
