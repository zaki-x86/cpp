# Interview: Concurrency Q&A

---

## Q1: What is the difference between a data race and a race condition?

**Answer:**

A **data race** is a language-level construct: two threads access the same memory location concurrently, at least one access is a write, and there is no synchronization between them. The C++ standard says a program with a data race has **undefined behaviour** — not "wrong value," not "non-deterministic," but undefined. The compiler is permitted to assume data races do not occur and generate code accordingly.

A **race condition** is a logic-level correctness bug: the outcome of the program depends on the relative timing of threads. Race conditions can exist in programs with no data races. The classic example is check-then-act: checking `balance >= amount` and calling `withdraw(amount)` as two separately synchronized operations allows another thread to interleave between the check and the act.

You can have a data race without a race condition (unlikely — but undefined behaviour anyway) and a race condition without a data race (common — logic bug despite correct synchronization).

**Trap:** "I ran it a million times without seeing the wrong value, so there's no data race." The compiler can generate code that appears correct in testing but fails under a different optimization level, different CPU, or specific memory pressure.

**Follow-up:** "How do you detect each?" — TSan detects data races at runtime. Race conditions are detected through code review and invariant analysis — there is no automated tool for race conditions in general.

---

## Q2: What do memory_order_acquire and memory_order_release do, and are they just compiler barriers?

**Answer:**

`memory_order_acquire` on a load and `memory_order_release` on a store create a **synchronizes-with** relationship: if an acquire load reads the value written by a release store, then all memory operations that were sequenced-before the release store are guaranteed visible after the acquire load. This establishes a happens-before edge across threads.

They are **not** just compiler barriers. A compiler barrier (like `asm volatile("" ::: "memory")`) prevents the compiler from reordering operations around it, but the CPU can still reorder stores in its store buffer or load buffer. `acquire` and `release` emit the necessary **CPU fence instructions** on weakly-ordered architectures (ARM, POWER) to prevent hardware reordering.

On x86, acquire/release semantics are free at the hardware level because x86 uses TSO (Total Store Order), which already prohibits most reorderings. But the compiler barrier is still required to prevent compiler-level reordering. On ARM, full barrier instructions (dmb ish) are required.

**Trap:** "acquire/release are just compiler barriers and are free at runtime." They are free on x86, not on ARM.

**Follow-up:** "When would you use `acq_rel` instead?" — `acq_rel` is for read-modify-write operations (fetch_add, compare_exchange) that must both see all prior writes and make their result visible to subsequent acquire loads. Used in reference counting and lock implementations.

---

## Q3: What is a lock-free data structure, and what is the ABA problem?

**Answer:**

A **lock-free** data structure guarantees that at least one thread makes progress in a finite number of steps, even if other threads are delayed. No thread can prevent another from completing its operation indefinitely (unlike a mutex, where a sleeping thread holds the lock). Lock-free structures typically use `compare_exchange_weak` or `compare_exchange_strong` to atomically check and update a pointer or index.

The **ABA problem** occurs in CAS (compare-and-swap) based algorithms: a thread reads value A, another thread changes A to B and back to A, and the first thread's CAS succeeds even though the state has changed. The pointer value is the same but the object it pointed to has been deallocated and reallocated at the same address.

Example in a lock-free stack:
1. Thread 1 reads head = node_A.
2. Thread 2 pops node_A, pops node_B, pushes node_A (same pointer, different state).
3. Thread 1's CAS on head succeeds (still A), but node_A's next pointer is now stale.

Mitigations: tagged pointers (store a version counter in unused pointer bits), hazard pointers (prevent reclamation while any thread holds a reference), epoch-based reclamation.

**Trap:** "Lock-free means faster." Lock-free avoids OS scheduling overhead for lock acquisition, but lock-free data structures have their own costs: CAS retry loops under contention, memory reclamation overhead (hazard pointers or epoch-based schemes), and difficult correctness reasoning. A mutex-based structure with low contention often outperforms a lock-free equivalent. Measure first.

**Follow-up:** "What is the difference between lock-free and wait-free?" — Wait-free guarantees every thread makes progress in a finite number of steps regardless of other threads' behaviour (stronger than lock-free, which only guarantees system-wide progress). Wait-free algorithms are rare and complex; most "lock-free" structures in practice are only lock-free, not wait-free.

---

## Q4: What is the difference between std::thread and std::jthread, and is detaching a thread a safe alternative?

**Answer:**

`std::thread` joins or detaches. If it goes out of scope while joinable, `std::terminate` is called — this is the intentional (if surprising) design: the committee chose immediate termination over silent resource leaks. Callers must explicitly `join()` or `detach()` before the destructor.

`std::jthread` (C++20) automatically joins in its destructor — it is the RAII version of `std::thread`. It also provides a `std::stop_token` parameter for cooperative cancellation and a `request_stop()` method. In new C++20 code, `std::jthread` is the correct default.

**Is `detach()` a safe alternative?** No. A detached thread continues running after its `std::thread` object is destroyed. If the thread accesses any stack variables or objects that were destroyed when its scope exited, the result is undefined behaviour. The thread also has no way to signal its completion or propagate exceptions. `detach()` is appropriate only for truly fire-and-forget background daemons that access no local state and never need to signal completion — a very narrow use case. In most scenarios, detaching is a red flag.

**Trap:** "Detaching is a safe way to avoid the join requirement." It trades one problem (potential `std::terminate`) for a worse one (silent undefined behaviour from dangling references or use-after-free).

**Follow-up:** "How does cooperative cancellation with `stop_token` work?" — The thread receives a `std::stop_token` as its first argument and checks `st.stop_requested()` at appropriate points. The caller calls `t.request_stop()`. This is cooperative — the thread stops at its own convenience, making it safe (no resource leaks) unlike thread cancellation in POSIX.

---

## Q5: Explain the readers-writer problem and how std::shared_mutex solves it.

**Answer:**

The readers-writer problem: a data structure supports concurrent reads (multiple readers at once are safe) but requires exclusive access for writes (one writer, no concurrent readers). A regular `std::mutex` is overly conservative — it serializes reads even though concurrent reads are safe.

`std::shared_mutex` (C++17) provides two lock levels:
- **Shared lock** (`lock_shared()` / `std::shared_lock`): multiple threads can hold simultaneously.
- **Exclusive lock** (`lock()` / `std::unique_lock`): only one thread holds, and it excludes all shared holders.

Correct usage:
```cpp
std::shared_mutex rw;
// Read:  std::shared_lock<std::shared_mutex> lk(rw);
// Write: std::unique_lock<std::shared_mutex> lk(rw);
```

**Write starvation** is the main hazard: if readers continuously acquire the shared lock, a writer requesting the exclusive lock may wait indefinitely. The standard does not guarantee fairness.

**Trap:** "A regular mutex is simpler and probably fine." For read-heavy workloads with a high read-to-write ratio (say, 100:1 or more), `shared_mutex` can significantly improve throughput. For write-heavy or balanced workloads, the added complexity of managing two lock types is not worth it — a regular mutex is correct and often faster due to lower overhead.

**Follow-up:** "When would you use `shared_mutex` vs a regular mutex?" — `shared_mutex` when: (a) reads are much more frequent than writes, (b) read operations are non-trivial in duration (enough that concurrency matters), and (c) write starvation is not a concern. A configuration cache, DNS resolution cache, or routing table are canonical use cases.

---

## Q6: What is false sharing, and how do you fix it?

**Answer:**

False sharing occurs when two threads write to **different variables** that happen to occupy the same CPU cache line (typically 64 bytes on x86). Every write to one variable causes the entire cache line to be invalidated in the other thread's L1 cache (MESI Invalid state). The other thread must re-fetch the line from L2/L3 or main memory on its next access — even though its own variable has not changed.

The result is cache line ping-pong between cores, which can cause 10-20x slowdown in hot loops.

Diagnosis: performance profilers (perf, VTune) show high cache miss rates or high L1 invalidation rates on specific variables.

Fix: align each hot variable to its own cache line using `alignas`:

```cpp
alignas(std::hardware_destructive_interference_size) std::atomic<long> counter_a;
alignas(std::hardware_destructive_interference_size) std::atomic<long> counter_b;
```

`std::hardware_destructive_interference_size` is 64 on x86. GCC may warn about its use; suppress with `#pragma GCC diagnostic ignored "-Winterference-size"`, or use a compile-time constant of 64 directly.

**Trap:** "Padding wastes memory — not worth it." The memory cost is at most a few cache lines (64-128 bytes per variable). For a hot path accessed millions of times per second, the throughput gain easily justifies it. The standard even provides the constant for exactly this purpose.

**Follow-up:** "What is `hardware_constructive_interference_size`?" — The same value (64 on x86), but used for the opposite purpose: to ensure that related data you want to access together fits within a single cache line, maximizing cache locality.

---

## Q7: What is the difference between std::latch and std::barrier?

**Answer:**

Both `std::latch` and `std::barrier` are countdown synchronization primitives added in C++20.

**std::latch** is single-use. You set a count at construction; threads call `count_down()` to decrement; threads call `wait()` to block until the count reaches zero. Once zero, it cannot be reset. Use it for one-shot events: "wait for N initializations to complete," "wait for N threads to reach a checkpoint."

**std::barrier** is reusable. All participating threads call `arrive_and_wait()`, which blocks until all have arrived. Then all are released simultaneously, and the barrier resets automatically for the next phase. An optional completion function runs exactly once between phases (before threads are released). Use it for iterative parallel algorithms where threads must synchronize at the end of each phase.

```cpp
// Latch — one-shot:
std::latch ready{num_workers};
// Each worker: setup(); ready.count_down();
// Main: ready.wait(); run_main_loop();

// Barrier — reusable:
std::barrier phase_sync{num_workers, []{ write_output(); }};
// Each worker, each iteration: do_work(); phase_sync.arrive_and_wait();
```

**Trap:** "I'd just use a condition variable and a counter for both." You could, but `latch` and `barrier` are simpler to reason about (no spurious wakeup handling, no predicate logic), have clearer intent, and may be more efficiently implemented in the standard library. They are the right tool when the problem is "synchronize N threads at a point."

**Follow-up:** "What about `std::counting_semaphore`?" — A semaphore controls access to a resource pool. `acquire()` decrements; `release()` increments. It is more general (bidirectional control flow) and lower-level than latch or barrier. `std::binary_semaphore` (a semaphore with max count 1) is a useful lightweight signal between two threads.

---

## Q8: What are C++20 coroutines at the language level — what is promise_type, and how does co_await work?

**Answer:**

A C++20 coroutine is a function that contains at least one of `co_await`, `co_yield`, or `co_return`. The compiler transforms it into a state machine. The coroutine's local variables and the current suspension point are stored in a **coroutine frame** — a heap-allocated struct. Suspension and resumption are O(1) — no thread switch, no kernel involvement.

**`promise_type`** is a nested type within the coroutine's return type that controls the lifecycle:

| Member | What it controls |
|--------|-----------------|
| `get_return_object()` | Returns the handle/object given to the caller |
| `initial_suspend()` | `suspend_never` = eager start; `suspend_always` = lazy |
| `final_suspend() noexcept` | What happens when the coroutine ends |
| `return_value(T)` / `return_void()` | Handles `co_return` |
| `yield_value(T)` | Handles `co_yield` |
| `unhandled_exception()` | Exception handling |

**`co_await expr` transformation:**
1. Get an awaitable from `expr` (via `operator co_await` if needed).
2. Call `await_ready()` — if true, skip suspension.
3. Call `await_suspend(coroutine_handle)` — suspends the coroutine, returns control to the caller.
4. When resumed, call `await_resume()` — the result is the value of the `co_await` expression.

**Trap:** "C++ coroutines are like async/await in JavaScript or Python." The analogy is superficial. JavaScript's `async/await` has a built-in event loop and promise-based executor. Python's `asyncio` provides an event loop. C++20 coroutines are a **mechanism** with no built-in executor — you must either provide your own scheduler or use a library (Asio, libcoro, cppcoro). Calling `co_await some_coroutine()` does not automatically schedule it on a thread pool; you must wire up the resume logic yourself. This gives C++ coroutines maximum flexibility at the cost of a steeper learning curve.

**Follow-up:** "What is the coroutine frame and when is it allocated?" — The frame is heap-allocated via `operator new` when the coroutine is first called. The compiler may optimize away the allocation (HALO — Heap Allocation eLision Optimization) if the lifetime of the coroutine is statically bounded and the frame size is known — but this is not guaranteed. The frame is freed when the coroutine reaches `final_suspend` and the caller destroys the handle via `coroutine_handle::destroy()`.
