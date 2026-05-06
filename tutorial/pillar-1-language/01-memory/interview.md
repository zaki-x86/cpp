# Memory — Interview Q&A

> Eight Q&A pairs. Full answers, common traps, senior follow-ups.

---

**Q1: What is RAII and why does it matter for exception safety?**

**A:** RAII (Resource Acquisition Is Initialization) is the pattern of acquiring a resource in a constructor and releasing it in the destructor. It matters for exception safety because C++ guarantees that destructors run during stack unwinding — so if an exception is thrown, every fully-constructed local object has its destructor called. This means a resource wrapped in RAII cannot leak, regardless of how the scope exits. Without RAII, you need explicit cleanup on every exit path (normal return, every possible exception), which is impossible to maintain correctly at scale.

**Trap:** "RAII is just about deleting memory." RAII applies to any resource: file handles, mutex locks, network connections, GPU allocations, COM reference counts. Memory is one example, not the definition.

**Follow-up:** Can RAII fail? Yes — if a destructor throws during stack unwinding, `std::terminate()` is called. The language cannot handle two simultaneous exceptions. This is why destructors must never throw. Mark all destructors `noexcept`.

---

**Q2: When do you use `unique_ptr` vs `shared_ptr`?**

**A:** `unique_ptr` is the default choice — zero overhead, unambiguous single ownership, destructor runs when the `unique_ptr` is destroyed. `shared_ptr` is for when ownership is genuinely shared across independent lifetimes: when multiple objects each need to keep the managed object alive and you cannot express a single owner. Note that `shared_ptr` costs two atomic operations (increment on copy, decrement on destruction) and a heap-allocated control block. Shared ownership is often a design smell — if you find yourself using `shared_ptr` everywhere, ask whether your ownership model is clear.

**Trap:** Using `shared_ptr` everywhere "because it is safer." It is not safer — it is more expensive. A `shared_ptr` cycle leaks just as surely as a raw pointer without `delete`.

**Follow-up:** What does `shared_ptr` cost vs `unique_ptr`? Every copy of a `shared_ptr` is an atomic increment. Every destruction is an atomic decrement. On x86 with false sharing, two threads destroying the last shared_ptr to the same object can cause cache line contention. `unique_ptr` has zero overhead — it is a wrapper around a pointer with an empty destructor function.

---

**Q3: What is a memory leak and how do you find one in C++?**

**A:** A memory leak is allocated memory that is never freed, causing the process's RSS to grow unboundedly over time. Detection tools: AddressSanitizer (`-fsanitize=address`) catches leaks at program exit and is faster than Valgrind; Valgrind with `--leak-check=full` gives more detail but runs 10–50× slower; heaptrack records every allocation and can show you a flamegraph of allocating call stacks. Prevention: RAII and smart pointers make leaks structurally impossible in well-written code — if you use `make_unique` everywhere, you cannot forget to free.

**Trap:** "I just use Valgrind." Valgrind misses leaks in objects that are still referenced at exit (reachable memory). ASan's leak detection (`-fsanitize=leak`) is more comprehensive and 10× faster.

**Follow-up:** Can a `shared_ptr` leak memory? Yes. A reference cycle — A holds a `shared_ptr` to B, B holds a `shared_ptr` to A — means neither reference count ever reaches zero, and neither object is ever destroyed. Break cycles with `weak_ptr`.

---

**Q4: Explain the reference count in `shared_ptr` — what is in the control block?**

**A:** The control block contains: a strong count (number of `shared_ptr` copies, atomic long), a weak count (number of `weak_ptr` copies plus one, atomic long), a type-erased deleter (a callable that destroys the managed object), a type-erased allocator (for the control block itself), and either a pointer to the managed object or the object inline (for `make_shared`). When the strong count reaches zero, the deleter is called and the managed object is destroyed. When the weak count reaches zero (the last `weak_ptr` and the last `shared_ptr` are both gone), the control block is freed.

**Trap:** "It just has a counter." Missing the deleter, allocator, and two separate counts. The two-count design is what allows `weak_ptr` to extend the control block lifetime beyond the object lifetime.

**Follow-up:** Why does `make_shared` prevent the object memory from being freed while weak_ptrs exist? Because `make_shared` stores the object inline in the control block — one allocation for both. When the strong count hits zero, the destructor runs, but the memory cannot be freed until the weak count also hits zero. With `shared_ptr(new T)`, the object is a separate allocation that can be freed independently when strong count hits zero.

---

**Q5: What is the difference between `delete` and `delete[]`?**

**A:** `delete` calls the destructor for a single object and then frees the memory. `delete[]` calls the destructor for every element of an array and then frees the memory using the array deallocation path, which may need the element count stored by the runtime alongside the allocation. Mismatching them — using `delete` on memory allocated with `new[]`, or vice versa — is undefined behavior. In practice, it often corrupts the allocator's internal bookkeeping. Smart pointers handle this automatically: `unique_ptr<T[]>` specialization uses `delete[]`.

**Trap:** "They are the same for POD types because PODs have no destructor." Still undefined behavior. The mismatched deallocation path may use the wrong size, corrupting the heap.

**Follow-up:** How does `unique_ptr` know which to call? The `unique_ptr<T[]>` partial specialization uses `delete[]` in its destructor. The non-array `unique_ptr<T>` uses `delete`. The distinction is in the template specialization, not at runtime.

---

**Q6: What is placement new and when would you use it?**

**A:** Placement new constructs an object at a provided memory address without allocating. Syntax: `new(ptr) T(args)`. The memory must be pre-allocated and properly aligned (at least `alignof(T)`). You must call the destructor explicitly: `ptr->~T()`. Uses: memory pools (pre-allocate a block, construct objects in it on demand), `std::variant` internals (the standard library uses placement new to store alternative types in a `union`-like storage), and any situation where you need to separate allocation from construction.

**Trap:** Forgetting to call the destructor explicitly. If `T` has a non-trivial destructor (member variables that need cleanup), not calling it causes resource leaks. For trivially destructible types (plain structs of ints), omitting the destructor call is harmless but still bad practice.

**Follow-up:** What are the alignment requirements? `ptr` must be aligned to at least `alignof(T)`. If it is not, behavior is undefined. Use `alignas(T)` on the buffer. `std::aligned_storage` (deprecated C++23) or just `alignas(T) char buf[sizeof(T)]` works.

---

**Q7: What is `std::pmr` and when would you reach for it?**

**A:** `std::pmr` (polymorphic memory resources, C++17) provides a `memory_resource` base class with a virtual allocate/deallocate interface. `pmr::` container aliases (`pmr::vector<T>`, `pmr::string`, etc.) take a `memory_resource*` at construction time, allowing you to change allocation strategy at runtime without changing the container's type. The primary use case is request-scoped arenas: construct a `monotonic_buffer_resource` backed by a stack-allocated buffer at the start of a request, allocate all request-scoped data from it, and when the request completes, the buffer goes out of scope and everything is freed in O(1). This eliminates thousands of `malloc`/`free` calls per request.

**Trap:** "I would always write a custom allocator." Custom allocators (the pre-C++17 `std::allocator` API) are template parameters that infect the container's type, making different-allocator containers incompatible. `pmr` solves this: `pmr::vector<int>` with arena allocator and `pmr::vector<int>` with pool allocator are the same type.

**Follow-up:** What is `monotonic_buffer_resource` and what are its limitations? It is a bump-pointer arena: allocations advance a pointer, there is no individual deallocation. Call `release()` to free everything. Limitations: no per-object deallocation (so you cannot return individual objects to the pool), the backing buffer must outlive all containers using the resource, and it can exhaust the buffer (falls back to upstream resource, which may be `new`).

---

**Q8: Explain `memory_order_acquire` and `memory_order_release`.**

**A:** A store with `memory_order_release` prevents all preceding memory operations (reads and writes) from being reordered after the store. A load with `memory_order_acquire` prevents all subsequent memory operations from being reordered before the load. Together, they establish a **synchronizes-with** relationship: any thread that observes the released value via an acquire load is guaranteed to see all writes the releasing thread made before the release store. This is the fundamental building block of lock-free programming — it lets you pass data between threads with minimal hardware fencing.

**Trap:** "Acquire/release are just memory fences." They are directional and thread-specific. A `release` store only synchronizes with the specific `acquire` load that reads the stored value. It does not create a global ordering visible to all threads — that requires `seq_cst`.

**Follow-up:** What ordering would you use for a counter that is only read and incremented, with no associated data? `memory_order_relaxed`. If you only need atomicity (no other shared data depends on the counter's value being visible in a specific order), relaxed is correct and is the fastest option — no fence instruction generated on x86 (where TSO already provides acquire/release for free, but relaxed avoids `lock` prefix on increment on some architectures).
