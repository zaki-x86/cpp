# C++ Bible — Phase 1: Language Pillar

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Write the 7 language core chapters (01-memory through 07-modern-cpp) — the pure C++ language mastery pillar covering memory, OOP, templates, type system, design patterns, concurrency, and the C++11–23 standards tour.

**Architecture:** Each chapter follows the three-layer pyramid: core.md (2-page essentials, opinionated mentor voice), deep-dive.md (full reference-grade mechanics), interview.md (Q&A with traps and follow-ups), plus self-contained compilable examples. Lab sections link to projects/02-foundation.

**Tech Stack:** GCC 11.4.0, C++20, GoogleTest (for examples that use it), Mermaid diagrams in markdown.

---

## File Map

```
tutorial/pillar-1-language/
├── 01-memory/
│   ├── README.md
│   ├── core.md
│   ├── deep-dive.md
│   ├── interview.md
│   └── examples/
│       ├── 01_raii_scope_guard.cpp
│       ├── 02_smart_pointers.cpp
│       └── 03_arena_allocator.cpp
├── 02-oop/
│   ├── README.md
│   ├── core.md
│   ├── deep-dive.md
│   ├── interview.md
│   └── examples/
│       ├── 01_rule_of_five.cpp
│       ├── 02_crtp_mixin.cpp
│       └── 03_nvi_pattern.cpp
├── 03-templates/
│   ├── README.md
│   ├── core.md
│   ├── deep-dive.md
│   ├── interview.md
│   └── examples/
│       ├── 01_type_list.cpp
│       ├── 02_sfinae_traits.cpp
│       ├── 03_policy_sorter.cpp
│       └── 04_expression_templates.cpp
├── 04-type-system/
│   ├── README.md
│   ├── core.md
│   ├── deep-dive.md
│   ├── interview.md
│   └── examples/
│       ├── 01_strong_type.cpp
│       ├── 02_variant_visitor.cpp
│       └── 03_expected_chain.cpp
├── 05-design-patterns/
│   ├── README.md
│   ├── core.md
│   ├── deep-dive.md
│   ├── interview.md
│   └── examples/
│       ├── 01_type_erasure.cpp
│       ├── 02_self_reg_factory.cpp
│       └── 03_ecs_world.cpp
├── 06-concurrency/
│   ├── README.md
│   ├── core.md
│   ├── deep-dive.md
│   ├── interview.md
│   └── examples/
│       ├── 01_spsc_queue.cpp
│       ├── 02_thread_pool.cpp
│       └── 03_memory_order_demo.cpp
└── 07-modern-cpp/
    ├── README.md
    ├── core.md
    ├── deep-dive.md
    ├── interview.md
    └── examples/
        ├── 01_cpp11_cpp14.cpp
        ├── 02_cpp17.cpp
        └── 03_cpp20.cpp
```

---

## Constraints (read before writing any code)

- Compiler: GCC 11.4.0 (`g++ -std=c++20`). **No `std::expected`** (needs GCC 12+). **No `std::format`** (needs GCC 13+). Use `printf` / `cout` instead.
- C++20 features available: concepts, coroutines, ranges, `std::span`, `std::jthread`, `std::latch`, `std::barrier`, `std::counting_semaphore`, `std::stop_token`.
- TSan **cannot execute** in WSL2 — note this in any concurrency example comments. Build-only verification is acceptable.
- Verification command for all examples: `g++ -std=c++20 -Wall -Wextra -o /tmp/test tutorial/pillar-1-language/NN-topic/examples/NN_name.cpp && /tmp/test`
- Every example must be **self-contained** (no external headers beyond stdlib), **complete** (no ellipsis, no `// implement this`), and **verified compilable** before committing.

---

## Task 1: Chapter 01-memory

### Step 1.1 — Create directory structure
- [ ] `mkdir -p tutorial/pillar-1-language/01-memory/examples`

### Step 1.2 — Write `tutorial/pillar-1-language/01-memory/README.md`
- [ ] One-paragraph chapter overview
- [ ] Table of contents linking core.md, deep-dive.md, interview.md, examples/
- [ ] Prerequisites: none (first chapter)
- [ ] Time estimate: 2h core reading, 1h deep-dive, 30m interview prep

### Step 1.3 — Write `tutorial/pillar-1-language/01-memory/core.md`

Voice: opinionated mentor, first-person, direct. Target length: ~2 printed pages.

Write actual prose (not outlines) for each of these H2 sections:

- [ ] `## The Golden Rule` — "If you write `delete`, you're doing it wrong." Explain why manual memory management is a trap: every call site that can throw between `new` and `delete` leaks. Show a 3-line counter-example.
- [ ] `## Stack vs Heap` — When each is appropriate. Stack: automatic lifetime, bounded size, O(1) allocation (just decrement SP). Heap: unknown size at compile time, lifetime that outlives the function. Include a Mermaid diagram showing a stack frame with locals, return address, and saved registers.

  ```mermaid
  graph TD
      subgraph "Stack Frame (grows down)"
          A[return address] --> B[saved rbp]
          B --> C[local: int x]
          C --> D[local: Buffer buf — ptr+size]
      end
      D -->|heap ptr| E[(Heap: actual data)]
  ```

- [ ] `## RAII: The Most Important Idiom in C++` — Explain the pattern: acquire in constructor, release in destructor. Why it's exception-safe: destructors always run during stack unwinding. Show a 10-line `FileHandle` example inline. Name it the idiom that makes C++ tractable at scale.
- [ ] `## Smart Pointer Decision Tree` — `unique_ptr` as the default (zero overhead, single owner). `shared_ptr` when ownership is genuinely shared (reference counting costs). `weak_ptr` to observe without owning and break cycles. Include a Mermaid flowchart:

  ```mermaid
  flowchart TD
      A{Do you need shared ownership?} -->|No| B[unique_ptr — default choice]
      A -->|Yes| C{Will cycles form?}
      C -->|No| D[shared_ptr]
      C -->|Yes| E[shared_ptr + weak_ptr to break cycle]
  ```

- [ ] `## Custom Allocators in One Paragraph` — When the default allocator is your bottleneck (latency-sensitive hot paths, embedded systems, fragmentation prevention). Arena allocation trades flexibility for speed. Mention `std::pmr` as the stdlib answer.
- [ ] `## Production Rules` — Bulleted list: prefer stack, use `make_unique`/`make_shared`, never raw `new`/`delete` in application code, profile before reaching for custom allocators, use `std::pmr::monotonic_buffer_resource` for arena patterns.
- [ ] `## Lab` — Link to `projects/02-foundation/include/foundation/memory/` with a 2-sentence description of what is implemented there and how to run its tests.

### Step 1.4 — Write `tutorial/pillar-1-language/01-memory/deep-dive.md`

Voice: reference-grade, precise. Write full technical paragraphs (not outline bullets) for each H2:

- [ ] `## Stack Unwinding Through Destructors` — Describe the EH table (`.gcc_except_table`), how the runtime walks landing pads during a throw, and the guarantee that every local with a non-trivial destructor has its destructor called. Distinguish `noexcept` functions (no landing pad generated, direct `std::terminate` on throw).
- [ ] `## shared_ptr Reference Count Internals` — Control block layout: strong count, weak count, deleter, allocator, managed pointer. Diagram the two heap allocations of `shared_ptr(new T)` vs the single allocation of `make_shared<T>()`. Explain why `make_shared` prevents the managed object from being freed while weak_ptrs exist.

  ```
  make_shared<T>()  →  [ control_block | T_data ]   (one allocation)
  shared_ptr(new T) →  [ control_block ]  +  [ T_data ]  (two allocations)
  ```

- [ ] `## weak_ptr and the Cycle Problem` — Walk through a parent-child cycle with `shared_ptr` on both sides (memory leak). Show how replacing child→parent link with `weak_ptr` breaks the cycle. Explain `weak_ptr::lock()` and the race-free promotion to `shared_ptr`.
- [ ] `## Custom Deleters` — Signature `void(T*)`. Use cases: `fclose`, `munmap`, COM `Release()`. Show `unique_ptr<FILE, decltype(&fclose)>`. Note that deleters are part of `unique_ptr`'s type but stored in the control block for `shared_ptr` (type erasure via pointer to function / stored callable).
- [ ] `## Placement New and Aligned Storage` — Syntax: `new(ptr) T(args)`. Requires manual destructor call `p->~T()`. Use cases: pools, variant internals. `alignas(T)` and `std::aligned_storage` (deprecated C++23, use `alignas` array instead). Show a minimal pool implementation.
- [ ] `## std::pmr — Polymorphic Memory Resources` — `memory_resource` interface: `do_allocate`, `do_deallocate`, `do_is_equal`. Standard resources: `monotonic_buffer_resource`, `synchronized_pool_resource`, `unsynchronized_pool_resource`. `null_memory_resource` for testing. `pmr::vector<T>` carries an allocator without making it a template parameter.
- [ ] `## Arena Allocator Internals` — Bump pointer allocator: one `char[]` buffer, one `size_t` offset. `allocate(n, align)` rounds up offset to alignment, returns pointer, advances offset. `reset()` sets offset to zero — O(1), no individual frees. Fragmentation only at the end of the arena lifetime. Alignment math: `(offset + align - 1) & ~(align - 1)`.
- [ ] `## The C++ Memory Model` — Sequential consistency as the default. `memory_order_relaxed`, `_acquire`, `_release`, `_acq_rel`, `_seq_cst`. The happens-before relation: if A synchronizes-with B, and B happens-before C, then A happens-before C. Show a producer-consumer example with acquire/release annotations. Include a happens-before diagram.

  ```mermaid
  sequenceDiagram
      participant P as Producer Thread
      participant C as Consumer Thread
      P->>P: data = 42
      P->>P: flag.store(true, release)  [release fence]
      C->>C: while(!flag.load(acquire)) [acquire fence]
      C->>C: assert(data == 42)  ✓ guaranteed
  ```

- [ ] `## False Sharing and Cache Line Padding` — Cache lines are 64 bytes on x86. Two threads writing to variables in the same cache line cause the line to ping-pong between L1 caches (MESI protocol). Fix: `alignas(std::hardware_destructive_interference_size)` to pad hot counters to separate lines. Measure the difference (typical 5–10× throughput improvement in contended counters).

### Step 1.5 — Write `tutorial/pillar-1-language/01-memory/interview.md`

Use this exact format for all 8 Q&A pairs:

```
**Q: [question]**
**A:** [3-5 sentence answer]
**Trap:** [wrong answer most candidates give]
**Follow-up:** [next question]
```

- [ ] Q1: What is RAII and why does it matter for exception safety?
  - A: Define RAII — resource acquisition in constructor, release in destructor. Explain stack unwinding guarantees destructor calls. Mention that C++ exceptions make manual cleanup impossible to get right without it.
  - Trap: "It's just about deleting memory" — misses file handles, mutexes, network connections.
  - Follow-up: Can RAII fail? (yes: throwing from a destructor during stack unwinding calls `std::terminate`)

- [ ] Q2: When do you use `unique_ptr` vs `shared_ptr`?
  - A: `unique_ptr` is the default — zero overhead, unambiguous ownership. `shared_ptr` only when ownership is genuinely shared across independent lifetimes. Shared ownership is often a design smell.
  - Trap: Using `shared_ptr` everywhere "because it's safer."
  - Follow-up: What does `shared_ptr` cost vs `unique_ptr`? (atomic ref count on every copy/destruction)

- [ ] Q3: What is a memory leak and how do you find one in C++?
  - A: Memory is allocated but never freed, growing the process's RSS unboundedly. Detection: Valgrind (`--leak-check=full`), AddressSanitizer (`-fsanitize=address`), heap profilers (heaptrack, gperftools). Prevention: RAII and smart pointers.
  - Trap: "I just use Valgrind" — misses that ASan is much faster and catches more at runtime.
  - Follow-up: Can a `shared_ptr` leak memory? (yes — reference cycles)

- [ ] Q4: Explain the reference count in `shared_ptr` — what's in the control block?
  - A: Control block contains strong count, weak count, deleter (type-erased), allocator (type-erased), and a pointer to the managed object (or the object itself for `make_shared`). Strong count dropping to zero destroys the object; weak count dropping to zero frees the control block.
  - Trap: "It just has a counter." Missing the deleter, allocator, and two-count design.
  - Follow-up: Why does `make_shared` prevent the object memory from being freed while weak_ptrs exist?

- [ ] Q5: What is the difference between `delete` and `delete[]`?
  - A: `delete[]` calls the destructor for every element and uses the array deallocation path (which may need the element count stored by the runtime). Mismatching them is undefined behaviour. Smart pointers handle this: `unique_ptr<T[]>` uses `delete[]` automatically.
  - Trap: "They're the same for POD types." Still UB regardless.
  - Follow-up: How does `unique_ptr` know which to call? (`unique_ptr<T[]>` specialization uses `delete[]`)

- [ ] Q6: What is placement new and when would you use it?
  - A: Placement new constructs an object at a provided memory address without allocating. Syntax: `new(ptr) T(args)`. Requires manual destructor call. Used in memory pools, `std::variant` internals, and any situation where you want to decouple allocation from construction.
  - Trap: Forgetting to call the destructor explicitly (`ptr->~T()`).
  - Follow-up: What are the alignment requirements? (`ptr` must be aligned to `alignof(T)` or UB)

- [ ] Q7: What is `std::pmr` and when would you reach for it over a custom allocator?
  - A: `std::pmr` provides `memory_resource` as a virtual interface plus `pmr::` container aliases that accept a `memory_resource*` at runtime, decoupling the allocation strategy from the container's type. Use it when you want to swap allocators without changing container types (e.g., a `monotonic_buffer_resource` for a request-scoped arena). Roll a custom allocator only when you need compile-time policy and cannot afford virtual dispatch.
  - Trap: "I'd always write a custom allocator" — ignores the templating complexity and ABI cost.
  - Follow-up: What is `monotonic_buffer_resource` and what are its limitations? (no deallocation, buffer must outlive all containers)

- [ ] Q8: Explain `memory_order_acquire` and `memory_order_release`.
  - A: A store with `release` prevents all preceding memory operations from being reordered after it. A load with `acquire` prevents all subsequent memory operations from being reordered before it. Together they establish a synchronizes-with relationship: any thread that reads the released value with acquire is guaranteed to see all writes that happened before the release store.
  - Trap: "Acquire/release are just fences" — misses that the ordering is one-directional and only applies between the communicating threads.
  - Follow-up: What ordering would you use for a simple flag with no other shared data? (`relaxed` — no need for ordering if the flag is the only shared variable)

### Step 1.6 — Write `tutorial/pillar-1-language/01-memory/examples/01_raii_scope_guard.cpp`

- [ ] Implement `ScopeGuard<F>` template: constructor takes callable `F`, destructor calls it unless `dismiss()` was called. Non-copyable, non-movable.
- [ ] Helper function `make_scope_guard(F&&)` for CTAD.
- [ ] Demo 1: Normal scope exit — shows "Acquired resource" then "Released resource".
- [ ] Demo 2: `dismiss()` called — guard does not fire.
- [ ] Demo 3: Exception thrown inside scope — guard fires during unwinding, shows "Released resource (exception path)".
- [ ] All output goes to `stdout` via `printf` or `std::cout`.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t01 tutorial/pillar-1-language/01-memory/examples/01_raii_scope_guard.cpp && /tmp/t01`
- [ ] Expected output contains: "Acquired resource", "Doing work", "Released resource", "Guard dismissed", "Released resource (exception path)"

### Step 1.7 — Write `tutorial/pillar-1-language/01-memory/examples/02_smart_pointers.cpp`

- [ ] Demo 1: `unique_ptr` with custom deleter (`FILE*` via `fopen`/`fclose`). Prints "file closed" on destruction.
- [ ] Demo 2: `shared_ptr` cycle — Node A points to Node B, Node B points back to Node A, both via `shared_ptr`. Both go out of scope; demonstrate the leak by showing destructors are NOT called (i.e., print from destructor: if you see neither "Node A destroyed" nor "Node B destroyed", that's the leak).
- [ ] Demo 3: `weak_ptr` fix — same nodes but the back-pointer is `weak_ptr`. Both destructors ARE called.
- [ ] Demo 4: `make_shared` vs `shared_ptr(new T)` — show with a counter struct that tracks constructions. Explain in a comment why `make_shared` does one allocation.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t02 tutorial/pillar-1-language/01-memory/examples/02_smart_pointers.cpp && /tmp/t02`
- [ ] Expected: cycle demo prints nothing for destructors; weak_ptr demo prints "Node A destroyed" and "Node B destroyed".

### Step 1.8 — Write `tutorial/pillar-1-language/01-memory/examples/03_arena_allocator.cpp`

- [ ] Implement `ArenaAllocator` class: fixed `char buffer[4096]` member, `size_t offset = 0`.
- [ ] `allocate(size_t bytes, size_t align)` method: aligns offset up, checks capacity, returns `buffer + offset`, advances offset. Throws `std::bad_alloc` if out of space.
- [ ] `reset()` method: sets offset to zero.
- [ ] `bytes_used()` getter.
- [ ] Demo: allocate an `int`, a `double`, and a user-defined `struct Point { float x, y; }` from the arena. Print their addresses and verify they are all within the buffer address range.
- [ ] Demo: call `reset()` and allocate again. Show that the first allocation after reset returns the same address as the first allocation before reset (buffer reuse).
- [ ] Print alignment math: show that the `double` is 8-byte aligned and `Point` is 4-byte aligned.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t03 tutorial/pillar-1-language/01-memory/examples/03_arena_allocator.cpp && /tmp/t03`
- [ ] Expected: addresses printed, all within buffer range, reset reuse confirmed.

### Step 1.9 — Commit chapter 01-memory
- [ ] Run all three verification commands and confirm clean output.
- [ ] `git add tutorial/pillar-1-language/01-memory/`
- [ ] `git commit -m "docs(tutorial): write 01-memory chapter — RAII, smart pointers, allocators"`

---

## Task 2: Chapter 02-oop

### Step 2.1 — Create directory structure
- [ ] `mkdir -p tutorial/pillar-1-language/02-oop/examples`

### Step 2.2 — Write `tutorial/pillar-1-language/02-oop/README.md`
- [ ] Chapter overview: OOP in C++ — from raw classes to modern CRTP and NVI
- [ ] Table of contents
- [ ] Prerequisites: 01-memory (ownership affects OOP design)
- [ ] Time estimate

### Step 2.3 — Write `tutorial/pillar-1-language/02-oop/core.md`

- [ ] `## The Rule of Zero` — Prefer it. If your class members are all value types or smart pointers, the compiler-generated special members are correct and efficient. Rule of Zero means: define no destructors, no copy/move constructors, no copy/move assignment operators.
- [ ] `## The Rule of Five` — When you own a raw resource (raw pointer, file descriptor, mutex), you must define all five: destructor, copy constructor, copy assignment, move constructor, move assignment. The copy-and-swap idiom: implement copy assignment in terms of copy constructor + swap. Show the pattern inline.
- [ ] `## Move Semantics in Plain English` — A move is a "steal." The moved-from object is left in a valid but unspecified state. Moving a `vector` is O(1): copy the pointer, size, capacity, set the source's pointer to null. Copies are O(n). The compiler picks move when the source is an rvalue (temporary or `std::move`-cast).
- [ ] `## Virtual vs CRTP` — Runtime polymorphism via vtable: pay for indirection on every call, enables heterogeneous collections. CRTP: base calls `static_cast<Derived*>(this)->impl()` — zero overhead, but you cannot mix derived types in a single container. Decision tree: include Mermaid flowchart.

  ```mermaid
  flowchart TD
      A{Need heterogeneous collection?} -->|Yes| B[Virtual dispatch]
      A -->|No| C{Performance critical inner loop?}
      C -->|Yes| D[CRTP — compile-time polymorphism]
      C -->|No| E[Either works — prefer virtual for simplicity]
  ```

- [ ] `## Production Rules` — Always declare destructors `virtual` in base classes intended for polymorphism. Prefer `= default` and `= delete` explicitly. Use `final` to enable devirtualization. Mark single-argument constructors `explicit`. Never return by value from a virtual function if it could slice.
- [ ] `## Lab` — Link to `projects/02-foundation/include/foundation/oop/` and describe what is implemented there.

### Step 2.4 — Write `tutorial/pillar-1-language/02-oop/deep-dive.md`

- [ ] `## vtable Layout` — Every polymorphic class gets one vtable (static, shared across all instances). Each object gets a hidden `vptr` at offset 0 (on all major ABIs). vtable entries are function pointers. Multiple inheritance creates multiple vtable pointers. Show a memory diagram: `[vptr][member1][member2]` and `vtable: [~Base][virtualFn1][virtualFn2]`.
- [ ] `## Devirtualization by the Compiler` — When the dynamic type is known at the call site (local variable, `final` class, inlined allocation), the compiler may replace the indirect vtable call with a direct call or even inline it. Show how `final` annotation enables this.
- [ ] `## Copy-and-Swap Idiom` — Implement copy assignment as: take parameter by value (invokes copy constructor), swap with `*this`, return `*this`. The old data is destroyed when the parameter goes out of scope. Exception-safe because the copy either succeeds before the swap or throws without modifying `*this`.
- [ ] `## The Diamond Problem and Virtual Inheritance` — When `D` inherits from `B1` and `B2`, both of which inherit from `A`: without `virtual`, `D` has two `A` subobjects. With `virtual` inheritance, `A` appears exactly once. Virtual bases have a different layout (offset stored at runtime via `vbtable`). Cost: every access to a virtual base member goes through one indirection.
- [ ] `## Non-Virtual Interface Pattern` — Public non-virtual interface delegates to private virtual implementation. The base class controls pre/post invariants (logging, locking, validation). Derived classes customize behaviour via `do_*` private overrides. This makes base-class invariants enforceable regardless of derived-class behaviour.
- [ ] `## PIMPL for ABI Stability` — The "pointer to implementation" pattern: class body contains only a `unique_ptr<Impl>` forward declaration. Implementation moves to the `.cpp` file. Binary ABI is stable: adding private members to `Impl` does not change the header's class layout. Cost: one extra heap allocation per object, one extra indirection per call.
- [ ] `## Object Slicing` — Assigning a derived object to a base value type copies only the base subobject. Derived-specific members are silently discarded. The virtual dispatch of the copy is resolved at compile time. Prevention: accept polymorphic types by pointer or reference, never by value. The `= delete` of copy in abstract bases forces correct usage.
- [ ] `## Abstract Base Class vs Concept for Interface Design` — ABC (pure virtual) enables runtime polymorphism but requires heap allocation and pointer semantics. Concepts (C++20) enable compile-time polymorphism with value semantics. ABCs work with type-erased heterogeneous containers. Concepts work with templates. When you need both, combine a Concept for the template API and a type-erased wrapper (like `std::function`) for runtime storage.

### Step 2.5 — Write `tutorial/pillar-1-language/02-oop/interview.md`

- [ ] Q1: What is the Rule of Five and when do you need it?
  - A: When your class directly manages a resource (raw pointer, fd, socket), you must write destructor, copy constructor, copy assignment, move constructor, move assignment. The compiler cannot know the correct semantics. If you define any one of these, you should define all five.
  - Trap: "I just define a destructor and the compiler handles the rest." Incorrect — user-defined destructor suppresses move generation.
  - Follow-up: What is the Rule of Zero? (prefer designing classes so compiler-generated members are correct)

- [ ] Q2: What is the difference between copy and move semantics?
  - A: Copy duplicates the resource — `O(n)` for heap-owning types. Move transfers ownership — `O(1)`, leaves the source in a valid but empty state. The compiler selects move when the source is an rvalue. `std::move` is just a cast to rvalue reference, it does not move anything itself.
  - Trap: "std::move moves the object." It only casts; the move constructor/assignment does the actual work.
  - Follow-up: What happens if you `std::move` a `const` object? (falls back to copy — `const T&&` does not bind to `T&&`)

- [ ] Q3: How does virtual dispatch work internally?
  - A: Each polymorphic object stores a `vptr` (pointer to the class's vtable) at offset 0. A virtual call `obj->foo()` dereferences the vptr, indexes to the slot for `foo`, and calls through the function pointer. The vtable is a static array of function pointers, one per virtual function, populated at compile time.
  - Trap: "Virtual dispatch is slow." It is one indirect call — typically 1–3 ns. It is only expensive if it prevents inlining in a hot inner loop.
  - Follow-up: How can the compiler devirtualize a virtual call? (`final` keyword, local variables of known type, whole-program optimization)

- [ ] Q4: What is CRTP and what advantage does it have over virtual dispatch?
  - A: Curiously Recurring Template Pattern: `Base<Derived>` accesses `Derived` members via `static_cast<Derived*>(this)`. The function call is direct (not through a pointer), enabling inlining. Zero vtable overhead. Disadvantage: you cannot store `Base<Derived1>` and `Base<Derived2>` in the same container without type erasure.
  - Trap: "CRTP is always faster than virtual." Profiling required — devirtualization often makes the difference negligible.
  - Follow-up: What problem does CRTP solve for mixins? (add methods to Derived without inheritance of state — just add behaviour)

- [ ] Q5: What is the diamond problem and how does C++ solve it?
  - A: `D` inherits from `B1` and `B2`; both inherit from `A`. Without virtual inheritance `D` has two copies of `A`'s members. Calling `A`'s method is ambiguous. `virtual` inheritance ensures a single shared `A` subobject. Cost: virtual base offset stored in a vbtable, one extra indirection.
  - Trap: "Just use virtual inheritance everywhere." It has runtime cost and complex construction rules (most-derived class initializes the virtual base directly).
  - Follow-up: Who is responsible for constructing a virtual base? (the most-derived concrete class in the hierarchy)

- [ ] Q6: What is PIMPL and why would you use it?
  - A: Pointer to Implementation: the public header exposes only a `unique_ptr<Impl>` to a forward-declared class. All data and private methods live in the `.cpp` file. Benefit: adding private members does not change the public header, so downstream code does not need to recompile (ABI stability). Cost: one heap allocation per object, one pointer indirection per method call.
  - Trap: "PIMPL is just information hiding." The key benefit is ABI stability and compilation firewall, not just privacy.
  - Follow-up: How does PIMPL interact with the Rule of Five? (must define destructor in the `.cpp` where `Impl` is complete, or `unique_ptr` will fail to compile)

- [ ] Q7: What is object slicing and how do you prevent it?
  - A: Assigning a derived object to a base value copies only the base subobject — derived-specific members and overrides are lost. This happens silently with value semantics. Prevention: never accept polymorphic objects by value; always use pointer or reference. Deleting the copy constructor in abstract bases enforces this at compile time.
  - Trap: "Slicing only happens with assignment." Happens with pass-by-value function parameters too.
  - Follow-up: What does the compiler do with a virtual function call on a sliced object? (calls the base version — static dispatch, no vtable involved since it's a value, not a pointer)

- [ ] Q8: What is the Non-Virtual Interface pattern and why is it useful?
  - A: The public API is non-virtual; it delegates to a private (or protected) `do_*` virtual method. The base class wraps every customization point with pre- and post-conditions, logging, or locking. Derived classes cannot bypass these invariants — they can only customize the `do_*` method.
  - Trap: "You could just make the public method virtual." Then derived classes can bypass pre/post-conditions.
  - Follow-up: What is the Template Method design pattern and how does NVI relate to it? (NVI is the C++ implementation of Template Method — base class defines the algorithm skeleton, derived classes fill in steps)

### Step 2.6 — Write `tutorial/pillar-1-language/02-oop/examples/01_rule_of_five.cpp`

- [ ] Implement `Buffer` class: owns a `char*` heap allocation + `size_t size_`.
- [ ] Print "Buffer(n)" on construction, "Buffer copied" on copy, "Buffer moved" on move, "~Buffer" on destruction.
- [ ] Implement copy constructor (deep copy), copy assignment (copy-and-swap), move constructor (steal pointer, null source), move assignment (move-and-swap or direct steal).
- [ ] Demo: copy a Buffer — verify "Buffer copied" appears and both objects are independent.
- [ ] Demo: move a Buffer — verify "Buffer moved" appears and source is empty.
- [ ] Demo: self-assignment `b = b` — verify it does not crash or double-free.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t04 tutorial/pillar-1-language/02-oop/examples/01_rule_of_five.cpp && /tmp/t04`

### Step 2.7 — Write `tutorial/pillar-1-language/02-oop/examples/02_crtp_mixin.cpp`

- [ ] Implement `Printable<Derived>` CRTP mixin: `print()` calls `static_cast<Derived*>(this)->to_string()`.
- [ ] Implement `Comparable<Derived>` CRTP mixin: `operator<`, `operator>`, `operator==`, `operator!=` in terms of `static_cast<Derived*>(this)->value()`.
- [ ] Implement `Point` struct: `float x, y;` — inherits both mixins. Provides `to_string()` and `value()` (returns `x * x + y * y` for ordering).
- [ ] Template function `void describe(const T& p)` (not virtual, template parameter) that calls `p.print()`.
- [ ] Demo: create two `Point` objects, compare them, call `describe` on them.
- [ ] Verify no virtual functions exist (use `static_assert` or comment confirming zero vtable).
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t05 tutorial/pillar-1-language/02-oop/examples/02_crtp_mixin.cpp && /tmp/t05`

### Step 2.8 — Write `tutorial/pillar-1-language/02-oop/examples/03_nvi_pattern.cpp`

- [ ] Implement `Logger` abstract base class: public non-virtual `log(const std::string& msg)` that prepends a timestamp prefix (use `__TIME__` macro for simplicity) and calls private pure virtual `do_log(const std::string& formatted)`.
- [ ] Implement `ConsoleLogger` derived class: `do_log` writes to stdout.
- [ ] Implement `FileLogger` derived class: `do_log` writes to a `std::ostringstream` (simulated file), accessible via `contents()`.
- [ ] Demo: call `log("hello")` on both — show that the timestamp prefix appears in both outputs regardless of the derived class implementation.
- [ ] Show that calling `do_log` directly from outside the class fails (private) — comment explaining the protection.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t06 tutorial/pillar-1-language/02-oop/examples/03_nvi_pattern.cpp && /tmp/t06`

### Step 2.9 — Commit chapter 02-oop
- [ ] Run all three verification commands and confirm clean output.
- [ ] `git add tutorial/pillar-1-language/02-oop/`
- [ ] `git commit -m "docs(tutorial): write 02-oop chapter — Rule of Five, CRTP, NVI, PIMPL"`

---

## Task 3: Chapter 03-templates

### Step 3.1 — Create directory structure
- [ ] `mkdir -p tutorial/pillar-1-language/03-templates/examples`

### Step 3.2 — Write `tutorial/pillar-1-language/03-templates/README.md`
- [ ] Chapter overview: template metaprogramming from basics to expression templates
- [ ] Table of contents
- [ ] Prerequisites: 01-memory (allocator concepts), 02-oop (CRTP)
- [ ] Time estimate

### Step 3.3 — Write `tutorial/pillar-1-language/03-templates/core.md`

- [ ] `## Templates as Code Generation` — The compiler writes code for you. `sort<int>` and `sort<double>` are different functions — the compiler generates both from one template. This is zero-cost abstraction: all decisions made at compile time, resulting binary is as efficient as hand-written specializations.
- [ ] `## The Three Kinds of Templates` — Function templates (`template<typename T> T max(T,T)`), class templates (`template<typename T> class Stack`), variable templates (`template<typename T> constexpr T pi = 3.14159...`). Each has distinct instantiation rules.
- [ ] `## Concepts: SFINAE for Humans` — Before C++20, template constraints were expressed with `enable_if` and `void_t` — cryptic, error messages were unreadable. C++20 concepts express constraints directly: `template<Sortable T> void sort(T& c)`. Error messages name the failed concept. Write concepts first, use `requires` clauses for constraints.
- [ ] `## When to Template vs When to Use virtual` — Template when: behaviour varies by type known at compile time, performance is critical, you want value semantics. Use virtual when: you need a heterogeneous collection, the type is unknown until runtime, you want a stable binary interface. When in doubt, prefer virtual first, optimize to template if profiling justifies it.
- [ ] `## Production Rules` — Separate interface (header) from implementation only when explicit instantiation is used. Keep template definitions in `.h` or `.hpp`. Use `static_assert` with a message for early error. Prefer `if constexpr` over SFINAE for readability. Write a concept before writing a `requires` expression inline.
- [ ] `## Lab` — Link to `projects/02-foundation/include/foundation/templates/` and describe what is implemented there.

### Step 3.4 — Write `tutorial/pillar-1-language/03-templates/deep-dive.md`

- [ ] `## Two-Phase Name Lookup` — Phase 1 at template definition: non-dependent names are resolved. Phase 2 at instantiation: dependent names are resolved. A dependent name is one that depends on a template parameter. This is why `this->foo()` is needed in some base class contexts — `foo` is non-dependent without it and would fail phase 1 if not found.
- [ ] `## Dependent Names and the typename/template Disambiguators` — `T::iterator` is a dependent name. The compiler cannot know if it's a type or a value until T is known. Disambiguate with `typename T::iterator`. Similarly, `T::template foo<int>()` requires `template` to clarify that `foo` is a template. These are required by the standard and the compiler will reject code without them.
- [ ] `## Full and Partial Specialization` — Full specialization: provide a definition for a specific set of template arguments. Partial specialization: provide a definition for a subset of cases (e.g., `Stack<T*>` for any pointer type). Function templates cannot be partially specialized — use overloads or class template static methods instead.
- [ ] `## Variadic Templates: Recursive vs Fold Expressions` — Recursive: `head + sum(tail...)` requires a base case. Fold expressions (C++17): `(args + ...)` handles any arity in one expression. Left fold: `(... op pack)`, right fold: `(pack op ...)`, binary folds with init. Prefer fold expressions — they are clearer and generate less code.
- [ ] `## SFINAE: enable_if and void_t` — SFINAE: Substitution Failure Is Not An Error. When template argument substitution fails, the template is removed from the overload set — no hard error. `enable_if<condition, T>::type` is `T` when condition is true, otherwise substitution fails. `void_t<...>` detects ill-formed expressions. Both are superseded by C++20 concepts but appear in all pre-20 codebases.
- [ ] `## Tag Dispatch` — Overload resolution on a dummy type tag: `impl(T, std::true_type)` vs `impl(T, std::false_type)`. The tag carries type information but no runtime cost. Used to select algorithm variants based on iterator category (`std::random_access_iterator_tag`, etc.) without virtual dispatch.
- [ ] `## Type Lists` — A `TypeList<Ts...>` is a compile-time list of types. Operations: `Head<L>`, `Tail<L>`, `Size<L>`, `At<L, N>`, `Contains<L, T>`, `Append<L, T>`, `Filter<L, Pred>`. These are pure compile-time computations — no runtime overhead. Used in reflection systems, tuple implementations, and variadic visitor patterns.
- [ ] `## constexpr if vs SFINAE` — `if constexpr(condition)` discards the false branch at compile time without requiring the branch to be well-formed. Simpler than SFINAE for conditional code paths in a single function body. Cannot be used outside function templates — for conditional member functions or class structures, SFINAE or explicit specialization is still needed.
- [ ] `## Policy-Based Design` — Bundle a set of behaviours into a policy class and pass it as a template parameter. The class customizes its strategy at compile time. Example: `Sorter<AscendingPolicy>` vs `Sorter<DescendingPolicy>`. The policy class can contain types, constants, and static methods. Policies compose via multiple inheritance or template parameter packs.
- [ ] `## Expression Templates: Zero-Cost Arithmetic` — Instead of `Vec a = b + c + d` creating two temporaries, `operator+` returns a lightweight proxy `VecAdd<VecAdd<Vec,Vec>,Vec>`. `operator=` evaluates the expression lazily: `a[i] = b[i] + c[i] + d[i]` for each i. No temporaries. Used in Eigen, Blaze, xtensor. The technique exploits the deduction of the expression tree at compile time.
- [ ] `## Concept Subsumption Rules` — A concept A subsumes concept B if A's requirement set is a superset of B's. In overload resolution, a more constrained template is preferred. `Sortable` subsumes `Comparable` if `Sortable` requires `Comparable`. Subsumption only applies to atomic constraints that are identical expressions — not semantically equivalent ones.

### Step 3.5 — Write `tutorial/pillar-1-language/03-templates/interview.md`

- [ ] Q1: What is SFINAE and how does `enable_if` implement it?
  - A: SFINAE — when substituting template arguments causes a type expression to be invalid, the template is silently removed from the overload set rather than causing a hard error. `enable_if<B, T>::type` is defined as `T` when B is true, and has no `::type` member when B is false, causing substitution failure. This lets you write overloads that are conditionally available.
  - Trap: Thinking SFINAE applies to the function body. It only applies during overload resolution (template parameter substitution), not to the body.
  - Follow-up: Why is SFINAE on the return type different from SFINAE on a default template parameter? (stylistic difference — return type SFINAE can conflict with explicit specialization; both work in practice)

- [ ] Q2: What are C++20 concepts and how do they differ from SFINAE?
  - A: Concepts are named compile-time predicates expressed with `requires` expressions. They produce clear error messages that name the violated concept. Subsumption rules allow finer-grained overload selection. The same constraint expressed in SFINAE requires cryptic `enable_if` chains and produces pages of template error output. Concepts are the readable, maintainable replacement.
  - Trap: "Concepts are just syntactic sugar for SFINAE." Subsumption is a new capability — ordering of constrained overloads that SFINAE cannot express.
  - Follow-up: What is the difference between a `requires` clause and a `requires` expression? (clause constrains a template; expression tests validity and evaluates to bool)

- [ ] Q3: What is two-phase name lookup and why does it matter?
  - A: Phase 1 at template definition resolves non-dependent names. Phase 2 at instantiation resolves dependent names. A name that does not depend on template parameters must be visible at the point of template definition — adding it later in the translation unit will not help. Dependent names must be visible at the instantiation point.
  - Trap: "I can call a function defined after the template." Only if the call is dependent (through `T`). Non-dependent calls require prior declaration.
  - Follow-up: Why do you sometimes need `this->foo()` inside a template class method? (`foo` in a base class is dependent — `this->` makes it dependent so phase-2 lookup finds it)

- [ ] Q4: What is a variadic template and when would you use fold expressions vs recursion?
  - A: A variadic template accepts any number of type or non-type parameters (`template<typename... Ts>`). Recursive unpacking requires a base case and generates N template instantiations. Fold expressions (`(args + ...)`) handle any arity in a single expression with no recursion. Use folds for arithmetic and logical combinations; use recursion when you need per-element processing with state.
  - Trap: Forgetting the base case in recursive variadic templates — causes infinite recursion at compile time.
  - Follow-up: What is the difference between a left fold and a right fold? (`(... op pack)` is left-fold: `((a op b) op c)`; `(pack op ...)` is right-fold: `(a op (b op c))`)

- [ ] Q5: What is policy-based design?
  - A: A class is parameterized on policy classes that inject different behaviours at compile time via template parameters. The host class delegates specific operations to the policy. Multiple independent policies can be composed as multiple template parameters or through inheritance. Alexandrescu popularized this in "Modern C++ Design." It achieves compile-time strategy pattern with zero virtual overhead.
  - Trap: Confusing policy with the strategy pattern — they are the same concept, but policy is compile-time (template) and strategy is runtime (virtual).
  - Follow-up: What is the cost of policy-based design? (longer compile times, separate binary for each policy combination, harder to read template error messages)

- [ ] Q6: What are expression templates and why are they useful?
  - A: Expression templates encode arithmetic expressions as a compile-time tree of proxy types. `a + b` returns `Add<Vec, Vec>`, not `Vec`. Only when assigned to a `Vec` does evaluation happen, in a single loop with no temporaries. This eliminates all intermediate allocations in expressions like `a = b + c + d`. Used in every major numerical C++ library (Eigen, Blaze, xtensor, Armadillo).
  - Trap: "You can just use move semantics to avoid temporaries." Move eliminates copies but still creates each intermediate — expression templates eliminate the intermediates entirely.
  - Follow-up: What is the danger of expression templates? (storing references to temporaries — the expression tree may outlive the operands if not immediately evaluated)

- [ ] Q7: What is the difference between full and partial template specialization?
  - A: Full specialization provides an implementation for a specific, complete set of template arguments (`template<> class Stack<bool>`). Partial specialization provides an implementation for a subset of cases, leaving some parameters still templated (`template<typename T> class Stack<T*>`). Function templates cannot be partially specialized — only class and variable templates can.
  - Trap: "I can partially specialize function templates." This is not allowed by the standard — use overloading instead.
  - Follow-up: What is explicit instantiation and when would you use it? (forces the compiler to generate a template instance in a specific translation unit — reduces link time by avoiding redundant instantiations)

- [ ] Q8: What is a type list and what can you do with one?
  - A: A type list is a variadic template that carries a list of types as compile-time data: `TypeList<int, double, char>`. Operations: `Head` (first type), `Tail` (remaining types), `At<N>` (N-th type), `Size` (count), `Contains<T>`, `Append<T>`, `Filter<Pred>`. Used to implement tuples, variant internals, reflection systems, and compile-time dispatch tables.
  - Trap: Thinking type lists have runtime overhead. They are entirely a compile-time construct — zero bytes at runtime.
  - Follow-up: How would you implement `At<N, TypeList<Ts...>>`? (recursive template: base case `At<0, TypeList<T, Rest...>> = T`; recursive case strips head and decrements N)

### Step 3.6 — Write `tutorial/pillar-1-language/03-templates/examples/01_type_list.cpp`

- [ ] Implement `TypeList<Ts...>` primary template.
- [ ] `Head<L>` — first type.
- [ ] `Tail<L>` — `TypeList<rest...>`.
- [ ] `Size<L>` — `static constexpr size_t value`.
- [ ] `At<N, L>` — N-th type (recursive, with `static_assert` on out-of-bounds).
- [ ] `Contains<T, L>` — `static constexpr bool value`.
- [ ] Static assert tests at the bottom: verify with `int, double, char` list that `Size==3`, `Head==int`, `At<1>==double`, `Contains<double>==true`, `Contains<float>==false`.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t07 tutorial/pillar-1-language/03-templates/examples/01_type_list.cpp && /tmp/t07`

### Step 3.7 — Write `tutorial/pillar-1-language/03-templates/examples/02_sfinae_traits.cpp`

- [ ] Implement `has_size<T>` detection trait using `void_t` and SFINAE.
- [ ] Implement `is_container<T>` composing `has_size` with `has_begin` and `has_end`.
- [ ] `print_size` overloaded via `enable_if`: one overload for containers (prints `.size()`), one for non-containers (prints "no size").
- [ ] Show the same using C++20 `requires`: a concept `HasSize` and a constrained function template.
- [ ] Demo: `print_size(std::vector<int>{1,2,3})` → "size: 3". `print_size(42)` → "no size".
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t08 tutorial/pillar-1-language/03-templates/examples/02_sfinae_traits.cpp && /tmp/t08`

### Step 3.8 — Write `tutorial/pillar-1-language/03-templates/examples/03_policy_sorter.cpp`

- [ ] Implement `AscendingPolicy` with static `compare(a, b)` returning `a < b`.
- [ ] Implement `DescendingPolicy` with static `compare(a, b)` returning `a > b`.
- [ ] Implement `Sorter<Policy>` with `sort(first, last)` using `std::sort` with a lambda calling `Policy::compare`.
- [ ] Demo: sort `std::vector<int>{5,2,8,1,9,3}` ascending then descending. Print both results.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t09 tutorial/pillar-1-language/03-templates/examples/03_policy_sorter.cpp && /tmp/t09`

### Step 3.9 — Write `tutorial/pillar-1-language/03-templates/examples/04_expression_templates.cpp`

- [ ] Implement `Vec<N>` with `std::array<float, N>` storage and `operator[]`.
- [ ] Implement `VecAdd<L, R>` proxy: stores const references to L and R, `operator[](i)` returns `l[i] + r[i]`, `size()` returns `L::size`.
- [ ] `Vec<N>::operator+` returns `VecAdd<Vec<N>, Vec<N>>` (not Vec — no allocation).
- [ ] `VecAdd<L,R>::operator+` returns `VecAdd<VecAdd<L,R>, Vec<N>>`.
- [ ] `Vec<N>` has an assignment operator from any expression template type: evaluates lazily `(*this)[i] = expr[i]`.
- [ ] Allocation counter: a global `int g_alloc_count` incremented in `Vec` constructor. Show that `a = b + c + d` increments it only once (for `a`'s construction), not three times.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t10 tutorial/pillar-1-language/03-templates/examples/04_expression_templates.cpp && /tmp/t10`
- [ ] Expected: "allocations for a=b+c+d: 1" (plus the 4 from constructing a,b,c,d).

### Step 3.10 — Commit chapter 03-templates
- [ ] Run all four verification commands and confirm clean output.
- [ ] `git add tutorial/pillar-1-language/03-templates/`
- [ ] `git commit -m "docs(tutorial): write 03-templates chapter — TMP, SFINAE, concepts, expression templates"`

---

## Task 4: Chapter 04-type-system

### Step 4.1 — Create directory structure
- [ ] `mkdir -p tutorial/pillar-1-language/04-type-system/examples`

### Step 4.2 — Write `tutorial/pillar-1-language/04-type-system/README.md`
- [ ] Chapter overview: using the type system to prevent bugs at compile time
- [ ] Table of contents
- [ ] Prerequisites: 02-oop (constructors), 03-templates (template metaprogramming)
- [ ] Time estimate

### Step 4.3 — Write `tutorial/pillar-1-language/04-type-system/core.md`

- [ ] `## Make Illegal States Unrepresentable` — The core philosophy. If your type system allows a state that your program logic forbids, that is a design failure. Use `enum class` instead of `int` for states. Use strong types instead of raw `int`/`double`. Use `std::optional` instead of nullable pointers. The goal: a program that compiles is more likely to be correct.
- [ ] `## Strong Typedefs: Preventing int Aliasing Bugs` — `using Meters = double` is a type alias, not a new type — you can still pass a `Seconds` where `Meters` is expected. A strong typedef wraps the underlying type in a struct. Show a minimal `StrongType<T, Tag>`. Then show the bug it prevents: `move_robot(Meters{5.0}, Seconds{3.0})` vs `move_robot(5.0, 3.0)`.
- [ ] `## optional, variant, expected — The Three Nullables` — `optional<T>`: either a value or nothing. `variant<T1,T2,...>`: exactly one of several types. `expected<T,E>` (C++23 / manual): either a success value or an error. Choose: optional for "might not exist", variant for "one of these types", expected for "success or typed error". Never use raw `nullptr` as a sentinel — prefer optional.
- [ ] `## The Type System as a State Machine` — Model state transitions with types. A `Connection` can be `Disconnected`, `Connecting`, or `Connected` — use a `variant<Disconnected, Connecting, Connected>`. Only the `Connected` state has the `send()` method. Attempting to call `send()` on a `Disconnected` is a compile error. Include a Mermaid state diagram.

  ```mermaid
  stateDiagram-v2
      [*] --> Disconnected
      Disconnected --> Connecting : connect()
      Connecting --> Connected : on_handshake()
      Connecting --> Disconnected : on_timeout()
      Connected --> Disconnected : disconnect()
  ```

- [ ] `## Production Rules` — Use `enum class` over `enum`. Use `std::optional` over nullable pointers for optional values. Create strong types for domain concepts (Meters, UserId, Timestamp). Use `std::variant` for discriminated unions. Use `[[nodiscard]]` on functions that return error codes or optionals. Use `explicit` on single-argument constructors.
- [ ] `## Lab` — Link to `projects/02-foundation/include/foundation/types/` and describe what is implemented there.

### Step 4.4 — Write `tutorial/pillar-1-language/04-type-system/deep-dive.md`

- [ ] `## Phantom Types for Compile-Time State Machines` — A phantom type is a template parameter that is never instantiated in the type body — it exists only to carry information in the type tag. `Handle<Open>` and `Handle<Closed>` are different types; functions that require an open handle take only `Handle<Open>`. State transitions return a different phantom type. Zero runtime overhead.
- [ ] `## std::variant Internals — Aligned Storage and Discriminant` — `variant<Ts...>` stores a `discriminant` (which type is active) and an aligned storage large enough for the largest alternative. Accessing the wrong alternative is undefined behaviour (and detected by `-D_GLIBCXX_ASSERTIONS`). `std::visit` dispatches via a jump table (function pointer array indexed by discriminant) — O(1), no virtual dispatch.
- [ ] `## The overloaded<Fs...> Visitor Pattern` — `struct overloaded : Fs... { using Fs::operator()...; };` deduction guide: `template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;`. Enables passing lambdas directly to `std::visit`. This is the standard idiom — it appears in cppreference and every modern codebase.
- [ ] `## Monadic Operations: and_then, or_else, transform` — C++23 adds `optional::and_then(f)` (applies f if value present, returning `optional<U>`), `transform(f)` (applies f to value, wrapping result), `or_else(f)` (calls f if empty). Same operations on `expected`. These enable chaining without explicit `if(opt.has_value())` at every step. Implement them manually for GCC 11.
- [ ] `## User-Defined Literals` — `operator""_km(long double d)` creates a `Kilometers` strong type. `1.5_km` is syntactic sugar. Literal suffixes must start with `_` for user-defined (non-reserved). Raw literal operators, cooked literal operators, and literal operator templates. Standard library uses them: `1s` (duration), `"hello"s` (string), `0x1p-3f` (float hex literal — built-in).
- [ ] `## The Spaceship Operator and Generated Comparisons` — `auto operator<=>(const T&) const = default` generates `==`, `!=`, `<`, `>`, `<=`, `>=` based on lexicographic member comparison. The return type of `<=>` is a comparison category: `std::strong_ordering` (equality implies substitutability), `std::weak_ordering` (equality does not imply substitutability), `std::partial_ordering` (some values may be unordered, e.g., NaN). If you define `<=>`, the compiler generates `==` separately (C++20 rewrite rules for symmetry).
- [ ] `## explicit Constructors and Conversion Operators` — Without `explicit`, single-argument constructors enable implicit conversions: `void f(MyString s); f("hello")` — the `const char*` converts silently. With `explicit`, `f("hello")` is a compile error; `f(MyString("hello"))` is required. Same applies to conversion operators: `explicit operator bool()` prevents accidental arithmetic on objects. The `[[nodiscard]]` attribute on constructors is also useful.
- [ ] `## std::any Internals and Small Buffer Optimization` — `std::any` stores a type-erased value with runtime type tag. For small objects (typically <= 24 bytes), it uses an internal buffer to avoid heap allocation (SBO). Large objects go to the heap via a virtual-dispatch-based `_Storage` mechanism. `std::any_cast<T>` checks the type tag and throws `bad_any_cast` on mismatch. `std::any` is the "last resort" type — prefer `std::variant` when types are known.

### Step 4.5 — Write `tutorial/pillar-1-language/04-type-system/interview.md`

- [ ] Q1: What is a strong typedef and why use one over a plain type alias?
  - A: A type alias (`using Meters = double`) creates a new name but not a new type — the compiler allows mixing `Meters` and `Seconds`. A strong typedef wraps the underlying type in a struct, creating a truly distinct type that requires explicit construction. Prevents entire classes of unit confusion bugs at zero runtime cost.
  - Trap: "typedef/using is good enough." Not for distinct domain types — they share the same overload set.
  - Follow-up: How do you give a strong type arithmetic operators without repeating the implementation? (CRTP mixins for `Addable<Derived>`, `Scalable<Derived>`, etc.)

- [ ] Q2: When do you use `std::optional` vs a raw pointer?
  - A: `std::optional` for values that may not exist but are not heap-allocated independently. Raw pointer when the pointee has independent lifetime and you need pointer arithmetic or polymorphism. Sentinel null values in return types should almost always be `optional`. `optional<T>` on the stack costs `sizeof(T) + 1` byte (plus alignment padding).
  - Trap: "I use a pointer and check for nullptr." Misses that the caller may forget the check; `optional` makes it explicit.
  - Follow-up: Can `optional<T&>` hold a reference? (No — `optional` cannot hold references in the standard; use `std::reference_wrapper<T>` or a pointer)

- [ ] Q3: What is `std::variant` and how is it different from a C union?
  - A: `std::variant<Ts...>` is a type-safe discriminated union. It tracks which type is active (discriminant) and prevents reading the wrong alternative (throws `bad_variant_access`). C `union` requires manual tracking of the active member; accessing the wrong member is undefined behaviour. `variant` also runs the correct constructor/destructor, which `union` does not for non-trivial types.
  - Trap: "Variant is just a tagged union." It also integrates with `std::visit` for exhaustive dispatch — missing a type is a compile error.
  - Follow-up: What is `std::monostate` and when would you use it? (a default-constructible type to allow `variant` to be default-constructed when the first alternative is not default-constructible)

- [ ] Q4: What is the `overloaded` pattern for `std::visit`?
  - A: `overloaded<Fs...>` inherits from multiple callable types and exposes all their `operator()` overloads. Combined with a deduction guide, `std::visit(overloaded{[](int i){...}, [](double d){...}}, v)` dispatches to the correct lambda by argument type. It is the idiomatic way to write inline visitors without defining a named visitor struct.
  - Trap: Forgetting to handle all variant alternatives — causes a compile error with `overloaded`, which is exactly the desired behaviour.
  - Follow-up: What if you want a default case for unhandled types? (add a generic lambda `[](auto&&){}` as the last argument to `overloaded`)

- [ ] Q5: What is `std::expected` (C++23) and how does monadic error handling work?
  - A: `expected<T, E>` holds either a success value `T` or an error `E`, without exceptions. `and_then(f)` chains operations: if the current value is a success, applies `f`; if it's an error, propagates the error. `transform(f)` maps the success value. `or_else(f)` handles the error case. This creates a composable error propagation chain similar to Rust's `?` operator.
  - Trap: "This is just returning an error code." Monadic chaining eliminates the manual `if(result.has_error()) return result.error()` at each step.
  - Follow-up: When would you use `expected` over exceptions? (performance-critical code, no-exception environments, when errors are expected and frequent rather than exceptional)

- [ ] Q6: What is a phantom type and what problem does it solve?
  - A: A phantom type is a template parameter that appears only in the type tag, never in the data layout. `Handle<State>` uses `State` only in the type — no `State` member. `Handle<Open>` and `Handle<Closed>` are different types at compile time but identical at runtime. Functions that require a specific state (`void read(Handle<Open>)`) refuse to compile with the wrong state — turning runtime logic errors into compile errors.
  - Trap: "I'd just use a runtime bool flag." Requires a runtime check; phantom types make incorrect states impossible to construct.
  - Follow-up: How do you implement state transitions with phantom types? (return a different phantom instantiation from the transition function: `Handle<Closed> close(Handle<Open>)`)

- [ ] Q7: What are user-defined literals and how do you implement one?
  - A: Syntax: `operator""_suffix(...)`. Parameter types allowed: `unsigned long long`, `long double`, `const char*` (with optional `size_t`), `char`, `wchar_t`. `100_ms` creates a `Milliseconds` strong type; `"hello"_sv` creates a `string_view`. The suffix must start with `_` for user-defined literals (without `_` is reserved for the standard library).
  - Trap: "Literal operators can take any type." Only the four allowed parameter types — no `int`, no custom types.
  - Follow-up: What is a raw literal operator and when is it useful? (`operator""_bin(const char*)` receives the character sequence before any parsing — useful for binary or custom-base numeric literals)

- [ ] Q8: How does the spaceship operator (`<=>`) work?
  - A: `auto operator<=>(const T&) const = default` generates all six comparison operators. The return type `std::strong_ordering`, `std::weak_ordering`, or `std::partial_ordering` communicates the semantics. If you provide `<=>`, the compiler also generates `==` by rewriting expressions. Member-wise comparison is lexicographic. You can return `std::partial_ordering::unordered` for incomparable values (e.g., NaN).
  - Trap: Thinking `<=>` replaces `==` automatically. You still need to define `==` explicitly if you want custom equality, or use `= default` for both.
  - Follow-up: What is the difference between `strong_ordering` and `weak_ordering`? (`strong_ordering::equal` implies the values are substitutable; `weak_ordering::equivalent` does not — e.g., case-insensitive string comparison)

### Step 4.6 — Write `tutorial/pillar-1-language/04-type-system/examples/01_strong_type.cpp`

- [ ] Implement `StrongType<T, Tag>`: wraps `T value_`, provides explicit constructor and `value()` getter.
- [ ] CRTP mixin `Addable<Derived>`: `operator+` returning `Derived{lhs.value() + rhs.value()}`.
- [ ] CRTP mixin `Printable<Derived>`: `print()` calling `printf`.
- [ ] CRTP mixin `Comparable<Derived>`: `operator<`, `operator==`.
- [ ] Define `struct MetersTag{}; using Meters = StrongType<double, MetersTag, Addable, Printable, Comparable>;` (or equivalent — use variadic CRTP inheritance).
- [ ] Define `struct SecondsTag{}; using Seconds = StrongType<double, SecondsTag>;`.
- [ ] Demo: `Meters a{5.0} + Meters{3.0}` works. `Meters{5.0}.print()` works.
- [ ] Show that mixing types fails: add a `static_assert` or a `requires` clause, and a comment showing the error.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t11 tutorial/pillar-1-language/04-type-system/examples/01_strong_type.cpp && /tmp/t11`

### Step 4.7 — Write `tutorial/pillar-1-language/04-type-system/examples/02_variant_visitor.cpp`

- [ ] Define `struct Circle { double radius; };`, `struct Rectangle { double w, h; };`, `struct Triangle { double base, height; };`.
- [ ] `using Shape = std::variant<Circle, Rectangle, Triangle>;`.
- [ ] Implement `overloaded<Fs...>` pattern with deduction guide.
- [ ] `double area(const Shape& s)` using `std::visit(overloaded{...}, s)`.
- [ ] `void describe(const Shape& s)` using `std::visit(overloaded{...}, s)`.
- [ ] Demo: create a `std::vector<Shape>`, iterate and print area and description.
- [ ] Comment showing that adding a `Hexagon` to the variant causes a compile error (visitor is no longer exhaustive) — demonstrate by trying and showing the error message, or just show the comment.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t12 tutorial/pillar-1-language/04-type-system/examples/02_variant_visitor.cpp && /tmp/t12`

### Step 4.8 — Write `tutorial/pillar-1-language/04-type-system/examples/03_expected_chain.cpp`

- [ ] Implement `Expected<T, E>`: stores a `std::variant<T, E>`. Constructors for success and error. `has_value()`, `value()`, `error()`.
- [ ] `and_then(f)`: if success, applies `f(value())` returning `Expected<U, E>`; if error, returns `Expected<U, E>{error()}`.
- [ ] `transform(f)`: if success, returns `Expected<U, E>{f(value())}`.
- [ ] `or_else(f)`: if error, calls `f(error())`; if success, passes through.
- [ ] Functions: `parse_int(const std::string&)` returning `Expected<int, std::string>`, `validate_positive(int)` returning `Expected<int, std::string>`, `compute_sqrt(int)` returning `Expected<double, std::string>`.
- [ ] Demo success path: `parse_int("16").and_then(validate_positive).and_then(compute_sqrt)` → prints "sqrt(16) = 4".
- [ ] Demo error path: `parse_int("abc").and_then(validate_positive).and_then(compute_sqrt)` → prints "Error: not a number".
- [ ] Demo error path: `parse_int("-5").and_then(validate_positive)` → prints "Error: must be positive".
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t13 tutorial/pillar-1-language/04-type-system/examples/03_expected_chain.cpp && /tmp/t13`

### Step 4.9 — Commit chapter 04-type-system
- [ ] Run all three verification commands and confirm clean output.
- [ ] `git add tutorial/pillar-1-language/04-type-system/`
- [ ] `git commit -m "docs(tutorial): write 04-type-system chapter — strong types, variant, expected"`

---

## Task 5: Chapter 05-design-patterns

### Step 5.1 — Create directory structure
- [ ] `mkdir -p tutorial/pillar-1-language/05-design-patterns/examples`

### Step 5.2 — Write `tutorial/pillar-1-language/05-design-patterns/README.md`
- [ ] Chapter overview: which GoF patterns apply in C++, which are superseded, and which new C++-specific patterns matter
- [ ] Table of contents
- [ ] Prerequisites: 01-memory (RAII), 02-oop (virtual/CRTP), 03-templates (type erasure)
- [ ] Time estimate

### Step 5.3 — Write `tutorial/pillar-1-language/05-design-patterns/core.md`

- [ ] `## C++ Makes Some Patterns Unnecessary` — Garbage collection → RAII handles resource lifetime. Iterator pattern → built into range-for. Abstract factory → template policy achieves the same at compile time. The C++ standard library is itself a pattern library — learn it before inventing patterns.
- [ ] `## C++ Makes Some Patterns Better` — Type erasure outperforms vtable-based interfaces in many contexts (no heap allocation, better cache behaviour). CRTP decorator composes behaviours without runtime cost. Self-registering factory eliminates registration boilerplate. These are distinctly C++ idioms — use them.
- [ ] `## C++ Makes Some Patterns Dangerous` — Singleton: global mutable state, initialization order fiasco, untestable code. Use dependency injection instead. Global observer: lifetime issues when subjects outlive observers. Mutable global: data races in multi-threaded code. These are not banned, but require deliberate design.
- [ ] `## The Five Patterns You Actually Use` — In order of frequency in production C++ codebases: (1) RAII (every resource), (2) Observer (event systems, UI, messaging), (3) Factory (plugin systems, testing), (4) Type Erasure (heterogeneous containers, callbacks), (5) ECS (game engines, simulation, any system needing cache-friendly bulk updates). Learn these deeply.
- [ ] `## Production Rules` — Avoid singletons — use dependency injection. Prefer type erasure over virtual when the types are known statically. Register factories at startup via static initializers, not via if-else chains. Use ECS when you iterate over 10,000+ entities of similar type. Prefer `std::function` over raw function pointers (type-erased, captures state).
- [ ] `## Lab` — Link to `projects/02-foundation/include/foundation/patterns/` and describe what is implemented there.

### Step 5.4 — Write `tutorial/pillar-1-language/05-design-patterns/deep-dive.md`

- [ ] `## Type Erasure: The Concept + Model Pattern` — Three components: a `Concept` class (pure virtual interface), a `Model<T>` template (wraps any T conforming to the interface via virtual inheritance of Concept), and the outer class that owns a `unique_ptr<Concept>`. The outer class forwards calls to the model. No constraint on T at the definition site — only at construction. This is how `std::function`, `std::any`, and ranges views are implemented.
- [ ] `## Small Buffer Optimization for Type-Erased Objects` — Instead of `unique_ptr<Concept>`, use a `std::aligned_storage` buffer. If `sizeof(Model<T>) <= buffer_size` and alignment is satisfied, construct in-place; otherwise heap-allocate. Store a flag to know which path was taken. `std::function` does this — small lambdas (no capture or small capture) avoid heap allocation. Implement with `std::launder` after placement new.
- [ ] `## Self-Registering Factory with Static Initializers` — A global factory map `unordered_map<string, CreateFn>`. Each product type defines a static object whose constructor calls `factory.register("name", create_fn)`. This constructor runs before `main()` via static initialization. The macro `REGISTER_PRODUCT(Name, Type)` automates this. Risk: static initialization order fiasco — use a local static (Meyers singleton) for the factory map.
- [ ] `## Thread-Safe Observer` — The naive observer stores `vector<Observer*>` — unsafe to modify during notification and unsafe from multiple threads. Thread-safe design: store observers in a mutex-protected container, use a snapshot for notification (copy the list under lock, notify outside the lock), handle dead observers with `weak_ptr<Observer>`. C++20 alternative: `std::atomic<shared_ptr<vector<Observer*>>>` for lock-free read path.
- [ ] `## CRTP Decorator for Mixin Composition` — `template<typename Base> class Logging : public Base { ... }`. Chain decorators: `Logging<Caching<Network>>`. Each decorator adds behaviour by wrapping the Base's interface. All resolved at compile time — zero virtual overhead. Order of decorators matters (outermost is called first). This is the Decorator pattern without the runtime cost.
- [ ] `## Command Pattern with Undo/Redo` — Each command is an object with `execute()` and `undo()`. A history stack (`vector<unique_ptr<Command>>`) supports multi-level undo. `execute()` pushes to history; `undo()` pops and calls `undo()`. Redo requires a separate redo stack. In C++, lambdas serve as lightweight commands; pairing `do_fn` and `undo_fn` covers most cases without class hierarchy.
- [ ] `## ECS: Data-Oriented Design` — Traditional OOP: each entity is an object, members are scattered in memory. ECS: components stored in contiguous arrays indexed by entity ID. Iterating position+velocity components accesses two arrays sequentially — cache-friendly. Adding/removing components does not invalidate other component arrays. Systems are pure functions over component views. Result: 10–100× faster iteration for large entity counts due to cache effects.

### Step 5.5 — Write `tutorial/pillar-1-language/05-design-patterns/interview.md`

- [ ] Q1: What is type erasure and how does it differ from virtual dispatch?
  - A: Type erasure stores an object of an arbitrary type behind a uniform interface without requiring the caller to know the concrete type. Virtual dispatch requires the concrete type to inherit from a known base. Type erasure imposes no inheritance constraint on the stored type — any type that satisfies the required operations qualifies. `std::function` and `std::any` are standard-library type erasure.
  - Trap: "Type erasure uses virtual functions internally." The Concept+Model pattern does use virtual internally, but the external API has no inheritance requirement. True type erasure hides all virtual machinery.
  - Follow-up: What are the performance characteristics of type erasure vs virtual? (same indirect call cost; type erasure can apply SBO to avoid heap allocation; virtual requires heap-allocated objects for polymorphism)

- [ ] Q2: What is a self-registering factory and how does it work?
  - A: Each product type registers itself with a global factory map during static initialization (before `main()`). A macro generates a static variable whose constructor calls `factory.register(name, create_fn)`. The factory creates objects by name without an if-else chain. Adding a new product type requires only adding the `REGISTER` macro — no changes to the factory.
  - Trap: "Static initialization order is safe." It is not — across translation units, initialization order is unspecified. Fix: use a local static (Meyers singleton) for the factory map to ensure it is initialized before any registrar accesses it.
  - Follow-up: What is the static initialization order fiasco? (global objects across TUs may initialize in any order — a registrar constructing before the factory map is initialized corrupts the map)

- [ ] Q3: What is the singleton pattern and why is it considered harmful in C++?
  - A: Singleton ensures only one instance of a class exists. Harmful because: (1) it is global mutable state — untestable (cannot inject a mock), (2) initialization order fiasco across TUs, (3) hidden dependencies (callers do not declare their dependency), (4) lifetime issues in multi-threaded teardown. Alternative: pass the shared object as a constructor parameter (dependency injection).
  - Trap: "Thread-safe double-checked locking fixes it." Even with `std::call_once`, the underlying issues of untestability and hidden coupling remain.
  - Follow-up: When is a singleton actually acceptable? (stateless utilities, logger (with care), OS-level resources with genuine system-wide uniqueness)

- [ ] Q4: What is the observer pattern and how do you make it thread-safe?
  - A: Subject maintains a list of observer callbacks; notifies them on state change. Thread-safe: protect the observer list with a mutex. During notification, copy the list under the lock, then notify outside the lock (avoids deadlock if the observer modifies the list). Use `weak_ptr` for observers to handle dead-observer cleanup automatically.
  - Trap: "I'll lock the mutex during notification." This causes deadlock if any observer tries to add/remove an observer during notification.
  - Follow-up: How would you make the observer list lock-free on the read path? (`atomic<shared_ptr<vector<Observer*>>>` — readers load the shared_ptr atomically; writers replace it with a new copy)

- [ ] Q5: What is Entity-Component-System and why is it cache-friendly?
  - A: Each entity is an ID. Components are data structs (Position, Velocity, Health). Systems iterate over entities that have a specific set of components. Components are stored in contiguous arrays per type. Iterating all positions accesses a single contiguous array — every cache line used. Traditional OOP mixes all component data in each entity object — iterating positions loads the full entity into cache even though only position is needed.
  - Trap: "ECS is just a database pattern." The defining property is cache-friendly data layout and separation of data from logic, not just the entity-component abstraction.
  - Follow-up: What is the cost of ECS? (complexity of component storage, non-obvious ownership, harder to reason about single-entity behaviour)

- [ ] Q6: What is small buffer optimization and where is it used in the standard library?
  - A: SBO stores small objects in a fixed internal buffer instead of heap-allocating. If the object fits (typically <= 16–32 bytes), no allocation occurs. `std::function` uses SBO for small lambdas. `std::string` uses SBO for short strings (SSO — Small String Optimization). `std::any` uses SBO for small values. The implementation stores a flag or uses the pointer value to distinguish inline vs heap storage.
  - Trap: "std::function always heap-allocates." Not for small lambdas with small captures — most implementations use SBO to avoid this.
  - Follow-up: What is the threshold size for std::string SSO on libstdc++? (15 characters including null terminator — strings up to 15 chars are stored inline)

- [ ] Q7: Implement the CRTP Decorator pattern.
  - A: `template<typename Base> class Logging : public Base { void operation() { log("before"); Base::operation(); log("after"); } }`. Chain: `Logging<Caching<RealWorker>> obj;`. Each decorator wraps the next by inheriting from it and overriding the operation. All calls resolve at compile time via static dispatch. The key is that each decorator is itself a template parameterized on the next decorator.
  - Trap: "This is the same as virtual decorator." The CRTP version has zero virtual overhead — all dispatch is direct function calls resolved at compile time.
  - Follow-up: What is the limitation of CRTP decorator vs virtual decorator? (CRTP: the chain is fixed at compile time; cannot change decorators at runtime. Virtual: can change at runtime, but pays the virtual call cost and requires heap allocation)

- [ ] Q8: What is the difference between compile-time and runtime polymorphism — when do you choose each?
  - A: Compile-time (templates, CRTP): the type is known at the call site, zero overhead, enables inlining. Runtime (virtual): the type is unknown at the call site, one indirect call, enables heterogeneous collections. Choose compile-time when all types are known statically and performance is critical. Choose runtime when you need a heterogeneous container or a stable binary interface (plugins, shared libraries).
  - Trap: "Virtual is always slower." At 1–3ns per call, it is rarely the bottleneck. Measure before optimizing.
  - Follow-up: How do you get both? (type erasure: store runtime-polymorphic objects behind a type-erased interface, while the inner model uses static dispatch)

### Step 5.6 — Write `tutorial/pillar-1-language/05-design-patterns/examples/01_type_erasure.cpp`

- [ ] Implement `AnyCallable` — type-erased callable (pedagogical `std::function<void()>`).
- [ ] Inner `Concept` class: pure virtual `call()` and `clone() -> Concept*`.
- [ ] Inner `Model<F>` class template: stores `F f_`, implements `call()` and `clone()`.
- [ ] `AnyCallable` outer class: `unique_ptr<Concept> self_`. Constructor takes any callable. `operator()()` delegates to `self_->call()`. Copy constructor uses `self_->clone()`.
- [ ] Demo: store a lambda, a free function pointer, and a struct with `operator()()` in three `AnyCallable` objects. Call each. Show that copy works correctly.
- [ ] Target: ~60 lines excluding comments.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t14 tutorial/pillar-1-language/05-design-patterns/examples/01_type_erasure.cpp && /tmp/t14`

### Step 5.7 — Write `tutorial/pillar-1-language/05-design-patterns/examples/02_self_reg_factory.cpp`

- [ ] Global factory: `std::unordered_map<std::string, std::function<std::unique_ptr<Shape>()>>& factory()` — Meyers singleton returning a local static.
- [ ] `Shape` abstract base class with `virtual void draw() = 0` and virtual destructor.
- [ ] `Circle` and `Rectangle` derived classes with `draw()` printing their name.
- [ ] `REGISTER_SHAPE(name, type)` macro: defines a local static object whose constructor calls `factory()[name] = []{ return std::make_unique<type>(); }`.
- [ ] Apply `REGISTER_SHAPE` for both Circle and Rectangle in the same `.cpp` file.
- [ ] `main()`: create shapes by calling `factory()["circle"]()` and `factory()["rectangle"]()`. Call `draw()` on each.
- [ ] Show that no if-else chain exists in the factory lookup — just a map lookup.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t15 tutorial/pillar-1-language/05-design-patterns/examples/02_self_reg_factory.cpp && /tmp/t15`

### Step 5.8 — Write `tutorial/pillar-1-language/05-design-patterns/examples/03_ecs_world.cpp`

- [ ] Entity type: `using Entity = uint32_t`.
- [ ] `World` class: `create_entity()` returns next entity ID.
- [ ] `add_component<T>(Entity, T)`: stores in `std::unordered_map<Entity, T>` (one map per component type, using a type-indexed map or separate templated storage).
- [ ] `get_component<T>(Entity) -> T&`: returns reference to stored component.
- [ ] `view<Cs...>()`: returns a range of entity IDs that have all of `Cs...`. Implement as returning a `std::vector<Entity>` of matching entities.
- [ ] Component types: `struct Position { float x, y; };`, `struct Velocity { float dx, dy; };`.
- [ ] Demo game loop: create 3 entities. Add Position and Velocity to all. Add a `Health` component to entity 0 only. Run 3 update ticks: for each entity in `view<Position, Velocity>()`, advance position by velocity. Print positions after each tick.
- [ ] Show contiguous access: print addresses of Position components and note they are sequential (within the map's node — or if using a vector-backed storage, they are contiguous).
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t16 tutorial/pillar-1-language/05-design-patterns/examples/03_ecs_world.cpp && /tmp/t16`

### Step 5.9 — Commit chapter 05-design-patterns
- [ ] Run all three verification commands and confirm clean output.
- [ ] `git add tutorial/pillar-1-language/05-design-patterns/`
- [ ] `git commit -m "docs(tutorial): write 05-design-patterns chapter — type erasure, factory, ECS"`

---

## Task 6: Chapter 06-concurrency

### Step 6.1 — Create directory structure
- [ ] `mkdir -p tutorial/pillar-1-language/06-concurrency/examples`

### Step 6.2 — Write `tutorial/pillar-1-language/06-concurrency/README.md`
- [ ] Chapter overview: writing correct multi-threaded C++ from the memory model up
- [ ] Table of contents
- [ ] Prerequisites: 01-memory (memory model), 04-type-system (atomic types)
- [ ] WSL2 note: TSan cannot execute (kernel VM constraint) — all examples are designed to run without TSan
- [ ] Time estimate

### Step 6.3 — Write `tutorial/pillar-1-language/06-concurrency/core.md`

- [ ] `## The Three Problems` — Data races: two threads access the same memory concurrently, at least one writes, no synchronization — undefined behaviour, not just "wrong value." Race conditions: the outcome depends on timing (even with synchronization). The memory model: on modern CPUs, writes are not immediately visible to other threads — you need explicit ordering.
- [ ] `## The Three Tools` — Mutex (`std::mutex` + `std::unique_lock`): protects shared state, blocking, prevents data races. Atomic (`std::atomic<T>`): lock-free operations on small types, non-blocking, with memory ordering. Condition variable: wait for a condition to become true, avoids spinning. Each has its use case — do not reach for atomics when a mutex is clearer.
- [ ] `## Prefer jthread` — `std::jthread` (C++20) joins on destruction (RAII). `std::thread` terminates the process if it goes out of scope while joinable. `jthread` also supports cooperative cancellation via `std::stop_token`. There is no reason to use `std::thread` in new C++20 code.
- [ ] `## The Rule: Protect All Shared Mutable State` — If two or more threads can access the same data and at least one access is a write, you need synchronization. No exceptions. "I know the write finishes first" is not synchronization — it is a data race. Use ASan + TSan (in Docker/native Linux) to detect violations — the runtime cost is acceptable in test builds.
- [ ] `## Mermaid: Producer-Consumer with condition_variable`

  ```mermaid
  sequenceDiagram
      participant P as Producer
      participant Q as Queue + Mutex + CV
      participant C as Consumer
      P->>Q: lock(mutex)
      P->>Q: push(item)
      P->>Q: unlock(mutex)
      P->>Q: cv.notify_one()
      C->>Q: lock(mutex)
      C->>Q: cv.wait(lock, !empty)
      Q-->>C: item
      C->>Q: pop(item)
      C->>Q: unlock(mutex)
  ```

- [ ] `## Production Rules` — Default to `unique_lock` + `mutex`, not raw atomics. Use `lock_guard` for simple scopes. Never hold a lock while calling user code (deadlock risk). Prefer `jthread` over `thread`. Use `stop_token` for cancellation. Measure with benchmarks before going lock-free — lock-free code is hard to reason about and often not faster in practice.
- [ ] `## Lab` — Link to `projects/02-foundation/include/foundation/concurrency/` and describe what is implemented there.

### Step 6.4 — Write `tutorial/pillar-1-language/06-concurrency/deep-dive.md`

- [ ] `## The C++ Memory Model: happens-before and synchronizes-with` — The memory model defines when one thread's writes are guaranteed to be visible to another thread's reads. Happens-before: if A happens-before B, then A's effects are visible to B. Synchronizes-with: a store with release synchronizes-with a load with acquire that reads the stored value. Sequenced-before: within a single thread, operations execute in program order. The transitive closure of these relations defines the full happens-before graph.
- [ ] `## The Six Memory Orders with Examples` — `relaxed`: no ordering, only atomicity (counter increments). `acquire`: load — no subsequent reads/writes move before this load. `release`: store — no preceding reads/writes move after this store. `acq_rel`: read-modify-write — combines acquire and release. `consume`: data dependency ordering (deprecated in practice, treat as acquire). `seq_cst`: total order across all seq_cst operations — default, most expensive. Show a 3-line code example for each.
- [ ] `## Lock-Free SPSC Queue Design` — Single-producer single-consumer ring buffer. Two atomics: `head` (producer writes, consumer reads) and `tail` (consumer writes, producer reads). Producer: read `tail` with `acquire`, compute next head, check not full, write slot, store new head with `release`. Consumer: read `head` with `acquire`, read slot, store new tail with `release`. No mutex needed because only one thread writes each atomic. `false_sharing` between head and tail — pad them to separate cache lines.
- [ ] `## Work-Stealing Thread Pool` — N worker threads each with a local deque. A submitted task goes to the submitting thread's deque. Each worker pops from the back of its own deque (LIFO — good cache locality). When a worker's deque is empty, it steals from the front of another worker's deque (FIFO — oldest tasks first). Stealing is the source of contention — use a lock or a lock-free deque (Chase-Lev algorithm) for the steal operation.
- [ ] `## std::shared_mutex — Readers-Writer Lock` — Multiple threads can hold the shared (read) lock simultaneously. Only one thread can hold the exclusive (write) lock. `lock_shared()` / `unlock_shared()` for readers. `lock()` / `unlock()` for writers. Prefer `std::shared_lock<shared_mutex>` for readers and `std::unique_lock<shared_mutex>` for writers. Write starvation is possible — be aware in write-heavy workloads.
- [ ] `## C++20 Synchronization Primitives: semaphore, latch, barrier` — `counting_semaphore<N>`: counting sync primitive; `binary_semaphore` is `counting_semaphore<1>`. `std::latch`: single-use countdown to zero; threads wait at zero. `std::barrier`: reusable latch that resets after each phase; optional completion function runs when the count reaches zero before reset. Use latch for one-shot "wait for N events"; use barrier for iterative phases.
- [ ] `## C++20 Coroutines: promise_type and the co_await Transformation` — A coroutine is a function that can be suspended and resumed. The compiler transforms `co_await expr` into: suspend the coroutine, store the current state (locals, instruction pointer) in the coroutine frame (heap-allocated), return control to the caller. When resumed, re-enter at the suspension point. `promise_type` controls: how the coroutine starts (`initial_suspend`), how it ends (`final_suspend`), and how values are yielded (`yield_value`) or returned (`return_value`).
- [ ] `## False Sharing and hardware_destructive_interference_size` — Two threads write to different variables that happen to share a cache line — every write by thread A invalidates thread B's copy of the line (MESI Invalid). The line bounces between L1 caches. Fix: `alignas(std::hardware_destructive_interference_size) int counter_a;` and the same for `counter_b`. `hardware_destructive_interference_size` is 64 on x86 (the L1 cache line size). Also: `hardware_constructive_interference_size` for data you want in the same line.
- [ ] `## The Broken Double-Checked Locking Pattern (and the Fix)` — Naive DCLP: check pointer without lock, lock, check again, construct if null, unlock, use. Broken because the pointer write and the construction write may be reordered — another thread sees a non-null but unconstructed object. Fix: use `std::atomic<T*>` for the pointer with `acquire` load and `release` store. Or better: use `std::call_once` or a local static (Meyers singleton) — both are correctly ordered by the standard.

### Step 6.5 — Write `tutorial/pillar-1-language/06-concurrency/interview.md`

- [ ] Q1: What is a data race and how is it different from a race condition?
  - A: A data race is a specific undefined behaviour: two threads access the same memory location concurrently, at least one access is a write, and there is no synchronization. Data race = UB, regardless of whether you observe the wrong value. A race condition is a higher-level logic bug where the outcome depends on timing — it can exist even with proper synchronization if the synchronization logic is incorrect.
  - Trap: "It's a data race if I get the wrong value." UB means you might get the right value, crash, or corrupt memory — the absence of visible symptoms does not mean the race does not exist.
  - Follow-up: How do you detect data races? (ThreadSanitizer at runtime — note: cannot run in WSL2, requires native Linux or Docker)

- [ ] Q2: Explain `memory_order_acquire` and `memory_order_release` — what do they guarantee?
  - A: A store with `release` prevents all preceding reads and writes from being reordered after the store. A load with `acquire` prevents all subsequent reads and writes from being reordered before the load. Together, if thread A stores value V with release, and thread B loads V with acquire, then all of A's writes before the store are visible to B after the load. This is the fundamental message-passing guarantee.
  - Trap: "Acquire/release are just compiler barriers." They are also CPU barriers — they prevent hardware reordering, not just compiler reordering.
  - Follow-up: What does `memory_order_seq_cst` add over acquire/release? (a total global order on all seq_cst operations — all threads observe them in the same sequence)

- [ ] Q3: What is a lock-free data structure and what makes one correct?
  - A: Lock-free: at least one thread makes progress even if others are suspended. Correctness requires: linearizability (each operation appears to take effect atomically at some point between its call and return), no ABA problem (a value changing from A to B to A may be misidentified as unchanged — use tagged pointers or hazard pointers), and proper memory ordering on every atomic operation.
  - Trap: "Lock-free means fast." Lock-free is often slower than a mutex for low-contention workloads due to CAS retry loops and memory ordering overhead. Profile first.
  - Follow-up: What is the ABA problem? (thread reads A, another changes A→B→A, first thread CAS succeeds even though the state changed — fix with version tag or hazard pointers)

- [ ] Q4: What is the difference between `std::thread` and `std::jthread`?
  - A: `std::thread` must be explicitly joined or detached before destruction — failure causes `std::terminate`. `std::jthread` automatically joins on destruction (RAII). `std::jthread` also provides a `std::stop_token` for cooperative cancellation — the thread can check `stop_requested()` and exit cleanly. Use `jthread` in all new C++20 code; `thread` has no advantage.
  - Trap: "std::thread::detach is a safe alternative." Detached threads have no lifetime tracking — they can access destroyed objects after the main thread exits.
  - Follow-up: How does `stop_token` cooperative cancellation work? (`jthread` provides `stop_source`; calling `request_stop()` sets the stop flag; the thread checks `stop_token.stop_requested()` and exits cleanly)

- [ ] Q5: What is the readers-writer problem and how does `std::shared_mutex` solve it?
  - A: Multiple readers can safely access shared data concurrently; a writer needs exclusive access. `shared_mutex` allows multiple simultaneous `lock_shared()` holders, but `lock()` (exclusive) waits for all readers to release and blocks new readers. This maximises read throughput for read-heavy workloads. `shared_lock<shared_mutex>` for readers, `unique_lock<shared_mutex>` for writers.
  - Trap: "Use a regular mutex — it's simpler." For a heavily read cache or config object, a `shared_mutex` can be 10× faster under read-heavy load.
  - Follow-up: What is writer starvation and how do you prevent it? (readers continuously arrive, the writer never gets the lock; fix with a fairness policy or `std::experimental::barrier`)

- [ ] Q6: What is false sharing and how do you fix it?
  - A: False sharing occurs when two threads write to different variables that lie within the same 64-byte cache line. Each write by one thread invalidates the other thread's L1 cache copy of the line, causing it to reload the entire line. This can reduce throughput by 10× on contended counters. Fix: align each hot variable to a cache line boundary with `alignas(std::hardware_destructive_interference_size)` — ensures each variable occupies its own cache line.
  - Trap: "Padding wastes memory." True — only apply to hot, frequently-written variables. Read-only data benefits from packing into the same cache line (`hardware_constructive_interference_size`).
  - Follow-up: How do you measure false sharing? (performance counters: `perf stat -e cache-misses,LLC-load-misses` or Intel VTune's memory access analysis)

- [ ] Q7: What is `std::latch` and how is it different from `std::barrier`?
  - A: `std::latch` is a single-use countdown: initialized with N, threads call `count_down()`, blocked threads call `wait()` until the count reaches zero. It cannot be reset. `std::barrier` is reusable: each phase, all N participating threads call `arrive_and_wait()`, which blocks until all arrive, then all are released together. `barrier` runs an optional completion function at each phase. Use latch for "wait for N events to happen once"; use barrier for iterative multi-phase algorithms.
  - Trap: "I'd use condition_variable." Latch is simpler and more efficient for this specific pattern — no spurious wakeup, no predicate, no lock needed.
  - Follow-up: What is the completion function in `std::barrier`? (a callable invoked once per phase when the count reaches zero, before threads are released — runs on the last arriving thread)

- [ ] Q8: Explain how C++20 coroutines work at the level of `promise_type` and `co_await`.
  - A: The compiler transforms a coroutine into a state machine. On first call, it allocates a coroutine frame on the heap (contains locals, return address, promise). `promise_type` must define `get_return_object()` (produces the coroutine handle returned to caller), `initial_suspend()` (whether to start immediately or suspend), `final_suspend()` (whether to suspend before frame destruction), and `return_value()`/`return_void()`. `co_await expr` evaluates `expr` to an awaitable; if `await_ready()` returns false, the coroutine suspends via `await_suspend(handle)`.
  - Trap: "Coroutines are like async/await in other languages." The mechanics are similar but C++ coroutines are lower-level — no built-in executor or scheduler; the caller of `resume()` provides the scheduling.
  - Follow-up: What is coroutine frame elision and when does the compiler apply it? (heap allocation elision when the compiler can prove the coroutine lifetime is bounded by the caller — rare in practice without explicit `noinline` control)

### Step 6.6 — Write `tutorial/pillar-1-language/06-concurrency/examples/01_spsc_queue.cpp`

- [ ] Implement `SPSCQueue<T, N>`: ring buffer of N slots (`std::array<T, N>`), `std::atomic<size_t> head_` (producer-owned) and `tail_` (consumer-owned), each padded to `std::hardware_destructive_interference_size` to prevent false sharing.
- [ ] `push(T)`: read tail with `acquire`, compute next head, return false if full, write slot, store head with `release`.
- [ ] `pop(T&)`: read head with `acquire`, return false if empty, read slot, store tail with `release`.
- [ ] Demo: producer `std::jthread` pushes integers 0..999999 (1M items). Consumer `std::jthread` pops and sums them. After both finish, assert sum equals expected value (0+1+...+999999 = 499999500000). Print "PASS" or "FAIL".
- [ ] Comment: "TSan cannot execute in WSL2 — use Docker or native Linux for race detection."
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -O2 -o /tmp/t17 tutorial/pillar-1-language/06-concurrency/examples/01_spsc_queue.cpp && /tmp/t17`

### Step 6.7 — Write `tutorial/pillar-1-language/06-concurrency/examples/02_thread_pool.cpp`

- [ ] Implement `ThreadPool` with N worker threads (`std::vector<std::jthread>`), a `std::queue<std::function<void()>> tasks_`, a `std::mutex`, and a `std::condition_variable`.
- [ ] Constructor: start N threads, each running a loop: lock mutex, wait on cv for task or stop, pop task, unlock, execute task.
- [ ] `submit<F>(F&& f) -> std::future<std::invoke_result_t<F>>`: wraps f in a `std::packaged_task`, gets future, pushes packaged task to queue, notifies cv, returns future.
- [ ] Destructor: set stop flag, notify all, join all threads (via `jthread` RAII or explicit join).
- [ ] Demo: create a pool with 4 threads. Submit 10 tasks each sleeping 10ms and returning their index squared. Collect all 10 futures and print results.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t18 tutorial/pillar-1-language/06-concurrency/examples/02_thread_pool.cpp && /tmp/t18`

### Step 6.8 — Write `tutorial/pillar-1-language/06-concurrency/examples/03_memory_order_demo.cpp`

- [ ] Comment header: "TSan cannot execute in WSL2 (kernel VM constraint). Run under Docker or native Linux for race detection."
- [ ] Demo 1 — Correct message passing: `std::atomic<int> data{0}`, `std::atomic<bool> ready{false}`. Producer writes `data.store(42, relaxed)` then `ready.store(true, release)`. Consumer spins on `ready.load(acquire)` then reads `data.load(relaxed)` and asserts == 42. Run 100 iterations to show it always works.
- [ ] Demo 2 — Timing demo (not a race demo since actual broken relaxed ordering is hard to demonstrate without TSan): Show two threads incrementing a shared `std::atomic<long>` counter — one using `seq_cst`, one using `relaxed`. Measure time for 10M increments each. Print "seq_cst: Xms, relaxed: Yms" to show the ordering overhead.
- [ ] Demo 3 — `std::memory_order_relaxed` counter: four threads each increment an atomic counter 1M times with `relaxed`. After joining, print total and assert == 4M. Prove atomicity (no lost updates) even with relaxed ordering — the operation is still atomic, just unordered.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -O2 -o /tmp/t19 tutorial/pillar-1-language/06-concurrency/examples/03_memory_order_demo.cpp && /tmp/t19`

### Step 6.9 — Commit chapter 06-concurrency
- [ ] Run all three verification commands and confirm clean output.
- [ ] `git add tutorial/pillar-1-language/06-concurrency/`
- [ ] `git commit -m "docs(tutorial): write 06-concurrency chapter — memory model, lock-free, coroutines"`

---

## Task 7: Chapter 07-modern-cpp

### Step 7.1 — Create directory structure
- [ ] `mkdir -p tutorial/pillar-1-language/07-modern-cpp/examples`

### Step 7.2 — Write `tutorial/pillar-1-language/07-modern-cpp/README.md`
- [ ] Chapter overview: the evolution of C++ from C++11 through C++23 — what changed and why it matters
- [ ] Table of contents
- [ ] Prerequisites: all previous chapters (this is the capstone standards tour)
- [ ] Time estimate

### Step 7.3 — Write `tutorial/pillar-1-language/07-modern-cpp/core.md`

Write actual prose tables (not stub text). For each standard, list the 5 most impactful features with a one-sentence "why it matters" for each.

- [ ] `## C++11 — The Modern C++ Foundation`

  | Feature | Why It Matters |
  |---|---|
  | Move semantics + rvalue references | Eliminates copies of expensive resources — `vector` of `unique_ptr` becomes practical |
  | `auto` type deduction | Reduces boilerplate and prevents type errors in generic code |
  | Lambda expressions | First-class functions — algorithms become usable without separate functor classes |
  | `nullptr` | Replaces `NULL` (which is `0`) — eliminates ambiguity between pointer and integer overloads |
  | `std::thread` + `std::mutex` | First standardized portable threading — no more platform-specific API calls |

- [ ] `## C++14 — Polishing C++11`

  | Feature | Why It Matters |
  |---|---|
  | Generic lambdas (`auto` params) | Lambdas become templates — one lambda works for any type |
  | `make_unique<T>()` | The missing C++11 companion to `make_shared` — now the preferred way to create `unique_ptr` |
  | `std::exchange` | Atomic get-and-set for move operations — simplifies move constructors |
  | `constexpr` relaxation | `constexpr` functions can now contain loops and local variables |
  | Return type deduction | `auto` return type deduced from `return` statements — useful in generic code |

- [ ] `## C++17 — Mainstream Features`

  | Feature | Why It Matters |
  |---|---|
  | Structured bindings | `auto [key, val] = *it` — range-for over maps becomes readable |
  | `if constexpr` | Replaces SFINAE for conditional code paths — readable, maintainable |
  | `std::optional<T>` | Explicit nullable values — no more sentinel values or output parameters |
  | `std::variant<Ts...>` | Type-safe discriminated union — replaces tagged unions |
  | `std::string_view` | Non-owning string reference — zero-copy string passing |

- [ ] `## C++20 — The Biggest Revision Since C++11`

  | Feature | Why It Matters |
  |---|---|
  | Concepts | Named constraints on templates — readable error messages and subsumption-based overloading |
  | Ranges + Views | Lazy composable algorithms — `views::filter | views::transform` replaces loops |
  | Coroutines | Language-level suspend/resume — enables async I/O and generators without callbacks |
  | `std::jthread` | RAII threads with cooperative cancellation — no more forgetting to join |
  | Modules | Replaces `#include` — faster build times and no macro leakage across translation units |

- [ ] `## C++23 — Incremental Quality of Life`

  | Feature | Why It Matters |
  |---|---|
  | `std::expected<T,E>` | Monadic error handling without exceptions — composable, typed error values |
  | `std::format` | Type-safe printf replacement — finally a readable format string API (GCC 13+) |
  | `std::flat_map` | Sorted vector-backed map — cache-friendly for read-heavy maps |
  | Deducing `this` (explicit object parameter) | Enables CRTP-style patterns without CRTP — simpler recursive lambdas |
  | `std::mdspan` | Multi-dimensional array view — zero-cost abstraction over contiguous memory |

### Step 7.4 — Write `tutorial/pillar-1-language/07-modern-cpp/deep-dive.md`

Write one full H2 section per standard with detailed coverage of every major feature:

- [ ] `## C++11 — Complete Feature Reference`
  - Move semantics, rvalue references, `std::move`, `std::forward`
  - Perfect forwarding and universal references
  - Lambda expressions: capture list (by value, by reference, by move), mutable lambdas, generic lambdas (actually C++14 — note this)
  - `auto` and trailing return types
  - `decltype` and `decltype(auto)`
  - Range-based for
  - `nullptr`
  - Strongly-typed enums (`enum class`)
  - `constexpr` functions (C++11 restrictions)
  - Initializer lists and uniform initialization
  - Variadic templates
  - `static_assert`
  - `noexcept` specifier and `noexcept` operator
  - `std::thread`, `std::mutex`, `std::condition_variable`, `std::atomic`
  - Smart pointers: `unique_ptr`, `shared_ptr`, `weak_ptr`
  - `std::function`, `std::bind`
  - Type traits (`<type_traits>`)
  - `std::tuple`, `std::array`
  - Hash maps: `unordered_map`, `unordered_set`

- [ ] `## C++14 — Polishing the Foundation`
  - Generic lambdas (`[](auto x)`)
  - Relaxed `constexpr` (loops, local variables)
  - `make_unique<T>()`
  - Return type deduction (`auto`)
  - `decltype(auto)` for perfect forwarding wrappers
  - Variable templates (`template<typename T> constexpr T pi`)
  - Binary literals (`0b1010`) and digit separators (`1'000'000`)
  - `std::exchange`
  - `std::integer_sequence`

- [ ] `## C++17 — Production Ready`
  - Structured bindings (`auto [a, b] = pair`)
  - `if constexpr`
  - `std::optional<T>`
  - `std::variant<Ts...>` and `std::visit`
  - `std::any`
  - `std::string_view`
  - Guaranteed copy elision (NRVO/RVO mandatory)
  - Fold expressions
  - Class Template Argument Deduction (CTAD)
  - `if` and `switch` with initializer
  - Parallel algorithms (`std::sort(std::execution::par, ...)`)
  - `std::filesystem`
  - `std::byte`
  - `[[nodiscard]]`, `[[maybe_unused]]`, `[[fallthrough]]`

- [ ] `## C++20 — The Biggest Revision`
  - Concepts and `requires` expressions/clauses
  - Ranges library: `views::filter`, `views::transform`, `views::take`, `ranges::sort`
  - Coroutines (`co_await`, `co_yield`, `co_return`)
  - `std::jthread` and `std::stop_token`
  - Modules (`import`, `export module`)
  - Three-way comparison operator (`<=>`)
  - `consteval` and `constinit`
  - `std::span<T>`
  - `std::latch`, `std::barrier`, `counting_semaphore`
  - `std::format` (GCC 13+ for libfmt, not available on GCC 11 — note this)
  - Designated initializers (`Point p = {.x=1, .y=2}`)
  - `std::bit_cast<T>`
  - Abbreviated function templates (`void f(auto x)`)
  - Lambda improvements: default-constructible, `[=, this]` explicit capture

- [ ] `## C++23 — Incremental Improvements`
  - `std::expected<T, E>` with monadic operations
  - `std::format` (stable, GCC 13+)
  - `std::flat_map` and `std::flat_set`
  - Deducing `this` (explicit object parameter)
  - `std::mdspan<T, Extents>`
  - `std::print` and `std::println`
  - `std::generator<T>` coroutine type
  - `std::stacktrace`
  - Ranges additions: `views::zip`, `views::enumerate`, `views::chunk`
  - `if consteval`
  - `[[assume(expr)]]` attribute

### Step 7.5 — Write `tutorial/pillar-1-language/07-modern-cpp/interview.md`

- [ ] Q1: What was the most important addition in C++11?
  - A: Move semantics and rvalue references. They transformed C++ from a language where passing by value meant expensive copies to one where resources are transferred in O(1). Every modern C++ standard library benefit — `unique_ptr`, move-only types, efficient containers — rests on this foundation. Without move semantics, modern C++ as we know it would not exist.
  - Trap: "Lambda expressions." Lambdas are important but syntactic sugar over functors. Move semantics changed the fundamental cost model of the language.
  - Follow-up: What is the difference between `std::move` and `std::forward`? (`move` always casts to rvalue; `forward` conditionally casts based on the deduced type — used in perfect forwarding to preserve lvalue/rvalue category)

- [ ] Q2: What is a lambda and how does capture work?
  - A: A lambda is syntactic sugar for a closure object (anonymous struct with `operator()`). The capture list determines which local variables from the enclosing scope are stored in the closure: `[=]` captures all by value (copies), `[&]` by reference, `[x]` captures only x by value, `[&x]` by reference, `[x = std::move(x)]` by move (C++14). Capturing by reference is dangerous if the lambda outlives the local scope.
  - Trap: "Capture by value is always safe." The copied value may be out of date relative to later modifications — capturing by reference gives the current value but risks dangling references.
  - Follow-up: What is a generic lambda and when was it introduced? (C++14: `[](auto x)` — the `operator()` becomes a template)

- [ ] Q3: What are structured bindings (C++17) and where are they useful?
  - A: `auto [a, b, c] = expr` binds names to the elements of a tuple-like or aggregate. For `std::pair`: `auto [key, val] = *map.find("x")`. For structs: destructures public data members. For arrays: binds each element. Most useful for: range-for over maps, multi-return functions returning structs/pairs, `std::tie` replacement.
  - Trap: "Structured bindings create copies." By default yes — use `auto& [a, b]` for references or `const auto& [a, b]` for const references.
  - Follow-up: Can you use structured bindings with a user-defined type? (yes, by specializing `std::tuple_size`, `std::tuple_element`, and providing `get<N>()`)

- [ ] Q4: What is `if constexpr` and how is it different from a regular `if`?
  - A: `if constexpr(cond)` discards the false branch at compile time — the discarded branch does not need to be well-formed. A regular `if` with a constant expression still compiles both branches. `if constexpr` enables generic functions that behave differently based on type properties without SFINAE. Example: `if constexpr(std::is_integral_v<T>) { /* integer path */ } else { /* float path */ }`.
  - Trap: "`if constexpr` evaluates at compile time like `constexpr if` in other languages." The condition must be a constant expression, but the non-discarded branch still executes at runtime.
  - Follow-up: What is the limitation of `if constexpr` compared to full SFINAE? (cannot be used to conditionally enable/disable member functions or class members — only works inside function bodies)

- [ ] Q5: What are C++20 ranges and how do views differ from algorithms?
  - A: Ranges are an abstraction over a sequence defined by a begin/end pair. Range algorithms (`ranges::sort`, `ranges::find`) take ranges directly instead of iterator pairs. Views are lazy, non-owning transformations: `views::filter(vec, pred)` returns a view that applies `pred` on iteration — no copy, no allocation. Views compose with `|`: `vec | views::filter(f) | views::transform(g)`. Algorithms are eager (execute immediately); views are lazy (execute on access).
  - Trap: "Views copy the data." Views are non-owning adaptors — they reference the original range and apply transformations on demand.
  - Follow-up: What is a range adaptor vs a range algorithm? (adaptor: takes a range and returns a view — lazy, composable via `|`. Algorithm: takes a range and returns a result — eager)

- [ ] Q6: What is CTAD (Class Template Argument Deduction)?
  - A: Before C++17, you had to write `std::pair<int, double> p(1, 2.0)`. With CTAD, `std::pair p(1, 2.0)` deduces `pair<int, double>`. The compiler uses deduction guides (explicit or implicit from constructors) to deduce template arguments. Eliminates factory functions like `make_pair` in most cases. Also works with custom classes that provide deduction guides.
  - Trap: "CTAD always works like function template deduction." CTAD uses class-level deduction guides, not just constructor signatures — sometimes explicit guides are needed for non-obvious deductions.
  - Follow-up: What is a user-defined deduction guide? (`template<typename T> MyClass(T) -> MyClass<std::decay_t<T>>;` — guides deduction when the constructor signature alone is ambiguous)

- [ ] Q7: What are C++20 modules and how do they differ from headers?
  - A: Modules are compiled once and their interface is exported as a binary module interface unit — no repeated parsing per translation unit. Headers are textually included in every TU that uses them — parsed from scratch each time. Modules do not leak macros across the interface. Import order does not matter. Build times improve significantly for large codebases (only changed modules recompile). The `export module foo;` declaration defines a named module; `import foo;` imports it.
  - Trap: "I can just rename my header to .cppm and it becomes a module." Headers use `#include` which is textual inclusion; modules require `export module` declarations and a module-aware build system.
  - Follow-up: What is a module partition and what problem does it solve? (splits a large module across multiple files: `module foo:bar;` — each partition is compiled separately, then the primary module interface re-exports them)

- [ ] Q8: What C++23 features should you know for a 2026 interview?
  - A: `std::expected<T,E>` with `and_then`/`transform`/`or_else` — the monadic error handling story. Deducing `this` (explicit object parameter) — simplifies CRTP and enables recursive lambdas. `std::flat_map` — sorted, cache-friendly map backed by a vector. `std::mdspan` — multi-dimensional array view, critical for numerical work. `std::print`/`std::println` — ergonomic output. `std::generator<T>` — the standard coroutine generator type.
  - Trap: "`std::format` is C++23." `std::format` is C++20; it is just not available on GCC 11 (requires GCC 13). Know the distinction between standard version and compiler support.
  - Follow-up: Which C++23 feature has the most impact on template-heavy code? (deducing `this` — enables simpler CRTP-like patterns without the template parameter, and fixes the "recursive lambda" limitation)

### Step 7.6 — Write `tutorial/pillar-1-language/07-modern-cpp/examples/01_cpp11_cpp14.cpp`

Complete, self-contained demo. Each section starts with a `printf("=== Demo: <name> ===\n")`.

- [ ] `auto` type deduction: `auto x = 42; auto y = 3.14;` with static_assert on type.
- [ ] Range-for: iterate a `std::vector<int>` and accumulate.
- [ ] Lambda capture: by value (`[x]`), by reference (`[&x]`), by move (`[y = std::move(v)]`).
- [ ] Rvalue references: show `Widget&&` parameter in a move constructor.
- [ ] `nullptr`: show `sizeof(nullptr)` and comparison `p == nullptr`.
- [ ] `constexpr` function: `constexpr int factorial(int n)` called at compile time.
- [ ] Initializer list: `std::vector<int>{1,2,3,4,5}`.
- [ ] Variadic template (fold-free): `sum(Ts... args)` using recursive template.
- [ ] `make_unique` (C++14): create and use `unique_ptr<int>`.
- [ ] Generic lambda (C++14): `auto add = [](auto a, auto b){ return a+b; }` called with int and double.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t20 tutorial/pillar-1-language/07-modern-cpp/examples/01_cpp11_cpp14.cpp && /tmp/t20`

### Step 7.7 — Write `tutorial/pillar-1-language/07-modern-cpp/examples/02_cpp17.cpp`

- [ ] Structured bindings with `std::pair`, with an aggregate struct, and with `std::map` range-for.
- [ ] `if constexpr` in a generic `describe_type<T>()` that prints "integer", "floating point", or "other".
- [ ] `std::optional`: chain of three optional transformations (`find_user`, `get_email`, `get_domain`), showing short-circuit on empty.
- [ ] `std::variant` + `overloaded` visitor: `Shape` variant with `Circle`, `Rectangle`. Compute area and perimeter with `std::visit`.
- [ ] `std::string_view`: show zero-copy substring and compare without allocating.
- [ ] Fold expressions: `sum_all(Ts... args)` using `(args + ...)`. Show with 5 arguments.
- [ ] CTAD: `std::pair p{42, "hello"s}` — deduce `pair<int, string>`. `std::vector v{1,2,3}` — deduce `vector<int>`.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t21 tutorial/pillar-1-language/07-modern-cpp/examples/02_cpp17.cpp && /tmp/t21`

### Step 7.8 — Write `tutorial/pillar-1-language/07-modern-cpp/examples/03_cpp20.cpp`

Note at top: `// std::format not available on GCC 11.4 (requires GCC 13+). Using printf/cout instead.`

- [ ] Concept `Sortable<T>`: requires `std::ranges::sort(std::declval<T&>())` is valid. Apply to a function `void sort_and_print(Sortable auto& c)`.
- [ ] Concept `Printable<T>`: requires `operator<<` exists. Apply with abbreviated function template.
- [ ] `std::span<int>`: write `void print_span(std::span<int> s)`. Call with raw array, with vector, with array.
- [ ] `std::jthread` with `std::stop_token`: thread loops printing "working" every 100ms until stop is requested. Main sleeps 350ms then lets jthread go out of scope (auto-requests stop and joins).
- [ ] Ranges pipeline: create `std::vector<int>{1..20}`, apply `views::filter([](int x){ return x % 2 == 0; })`, then `views::transform([](int x){ return x * x; })`, then print the first 5 results via `views::take(5)`.
- [ ] Three-way comparison: implement `struct Point { int x, y; auto operator<=>(const Point&) const = default; }`. Sort a vector of points with `std::ranges::sort` (uses `<=>` transitively).
- [ ] `consteval` function: `consteval int square(int n) { return n*n; }`. Show it can only be called at compile time (use in `static_assert`). Note the difference from `constexpr`.
- [ ] Verify: `g++ -std=c++20 -Wall -Wextra -o /tmp/t22 tutorial/pillar-1-language/07-modern-cpp/examples/03_cpp20.cpp && /tmp/t22`

### Step 7.9 — Commit chapter 07-modern-cpp
- [ ] Run all three verification commands and confirm clean output.
- [ ] `git add tutorial/pillar-1-language/07-modern-cpp/`
- [ ] `git commit -m "docs(tutorial): write 07-modern-cpp chapter — C++11 through C++23 standards tour"`

---

## Phase Complete

When all 7 tasks are done, verify the following:

### Deliverable Checklist
- [ ] `tutorial/pillar-1-language/` directory exists with 7 subdirectories (01-memory through 07-modern-cpp)
- [ ] Each chapter contains: `README.md`, `core.md`, `deep-dive.md`, `interview.md`, and `examples/` directory
- [ ] Total example files: 22 (3+3+4+3+3+3+3)
- [ ] All 22 examples compile clean under `g++ -std=c++20 -Wall -Wextra`
- [ ] All 22 examples run and produce expected output
- [ ] Each `interview.md` contains exactly 8 Q&A pairs in the defined format
- [ ] Each `core.md` contains all specified H2 sections with actual prose (not stubs)
- [ ] Each `deep-dive.md` contains all specified H2 sections with reference-grade content
- [ ] 7 git commits, one per chapter, with the commit messages specified above
- [ ] No example uses `std::format` (GCC 13+ only) or `std::expected` (GCC 12+ only) without a manual implementation
- [ ] All Mermaid diagrams are syntactically valid (render correctly in GitHub markdown preview)

### What Was Produced
- 7 chapters covering the complete C++ language pillar
- 22 self-contained, verified, compilable code examples
- 56 interview Q&A pairs with traps and follow-ups
- Reference documentation for C++11 through C++23
- Labs linking to `projects/02-foundation` for hands-on practice

### Transition to Phase 2
Phase 2 will cover the Systems Pillar: OS, networking, database internals, and performance engineering. Chapters 08–14 will follow the same three-layer pyramid format established here.
