# Deep Dive: Concurrency Internals

## The C++ Memory Model: happens-before and synchronizes-with

The memory model is defined in terms of three relations:

**Sequenced-before**: within a single thread, each statement is sequenced-before the next. If A is sequenced-before B, A's effects are visible to B — this is just program order within a thread.

**Synchronizes-with**: a store with `memory_order_release` **synchronizes-with** a load with `memory_order_acquire` that reads the value written by that store (or a value written later in the same thread). This creates a cross-thread guarantee: everything that happened before the release store is visible after the acquire load.

**Happens-before**: the transitive closure of sequenced-before and synchronizes-with. If A happens-before B, then all of A's side effects are guaranteed visible when B executes.

The critical insight: if there is no happens-before edge between a write and a read, there is no guarantee that the read sees the write. This is not a theoretical concern — it reflects what real CPUs do with store buffers and weakly-ordered memory systems.

Example — correct message passing:
```
Thread 1:                    Thread 2:
data = 42;     // write      while (!ready.load(acquire)) {}  // acquire load
ready.store(true, release);  // release store
                             use(data);   // sees 42 — guaranteed
```

The `release` on the store and the `acquire` on the load create a synchronizes-with edge. The store of `data = 42` is sequenced-before the release store, so it happens-before the acquire load, which is sequenced-before `use(data)`. The full chain is established.

Without the acquire/release, `use(data)` might see 0 — not because of a compiler bug, but because the hardware is permitted to reorder the stores, and the abstract machine explicitly allows this.

---

## The Six Memory Orders with Examples

### memory_order_relaxed

Only guarantees atomicity of the operation itself. No ordering relative to any other memory operation. Used for counters and statistics where you need the count to be consistent but do not need to see associated data.

```cpp
std::atomic<int> hit_count{0};
// In any thread:
hit_count.fetch_add(1, std::memory_order_relaxed);
```

### memory_order_acquire

Load only. No load or store in the current thread can be reordered before this load. If this load reads a value written by a release store, all writes before that release store are visible after this acquire load.

```cpp
std::atomic<bool> ready{false};
// Consumer:
while (!ready.load(std::memory_order_acquire)) { /* spin */ }
// All writes done before ready.store(release) are now visible
```

### memory_order_release

Store only. No load or store in the current thread can be reordered after this store. Creates a synchronizes-with edge with any subsequent acquire load that reads this value.

```cpp
std::atomic<bool> ready{false};
// Producer:
data = 42;  // sequenced before the release store
ready.store(true, std::memory_order_release);
```

### memory_order_acq_rel

Read-modify-write operations only (fetch_add, compare_exchange, etc.). Combines acquire and release semantics: acts as an acquire for the read part and a release for the write part. Used in lock implementations and reference counting.

```cpp
std::atomic<int> ref_count{1};
// In destructor-equivalent:
if (ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
    // acquire: see all decrements from other threads
    // release: ensure our stores are visible before we destroy
    delete this;
}
```

### memory_order_consume

Like acquire but only enforces data-dependency ordering: only operations that depend on the loaded value are ordered. Intended for pointer following (load a pointer, then access through it). In practice, no major compiler implements it differently from acquire (the standard acknowledges this), so treat it as acquire and move on.

```cpp
// Treat as acquire in practice:
Node* p = head.load(std::memory_order_consume);
int val = p->value;  // dependency on p — formally ordered by consume
```

### memory_order_seq_cst

The default. Provides a single total order across all `seq_cst` operations in the program. Every thread agrees on the order of all `seq_cst` operations. Strongest guarantee, most expensive — requires a full memory fence on x86 (MFENCE or LOCK XCHG) and heavier fencing on ARM.

```cpp
std::atomic<bool> x{false}, y{false};
std::atomic<int> z{0};
// With seq_cst: if thread 1 stores x=true and thread 2 stores y=true,
// at least one of them sees the other's store — guaranteed single total order
x.store(true, std::memory_order_seq_cst);
```

---

## Lock-Free SPSC Queue Design

A single-producer single-consumer (SPSC) queue is the simplest correct lock-free data structure: it has exactly one writer to each atomic and one reader.

**Structure:**
```
[ slot0 | slot1 | slot2 | ... | slotN-1 ]  (ring buffer, capacity N)
  ^                                          head_ — producer's write position
                  ^                          tail_ — consumer's read position
```

- `head_`: owned by the producer. Producer reads it to know where to write next. Consumer reads it to know if the queue is non-empty.
- `tail_`: owned by the consumer. Consumer reads it to know where to read next. Producer reads it to know if the queue is full.

**Push (producer only):**
1. Load `tail_` with `acquire` — see the consumer's latest pop.
2. Compute `next_head = (head_ + 1) % N`.
3. If `next_head == tail_`, queue is full — return false.
4. Write the value into `slots_[head_]`.
5. Store `next_head` into `head_` with `release` — make the written slot visible to the consumer.

**Pop (consumer only):**
1. Load `head_` with `acquire` — see the producer's latest push.
2. If `head_ == tail_`, queue is empty — return false.
3. Read the value from `slots_[tail_]`.
4. Compute `next_tail = (tail_ + 1) % N`.
5. Store `next_tail` into `tail_` with `release` — make the consumed slot visible to the producer.

**Why no mutex?** The producer never writes `tail_` and the consumer never writes `head_`. There is no contention on a single atomic. The acquire/release pairs establish the happens-before edge: the consumer's acquire load of `head_` synchronizes-with the producer's release store of `head_`, guaranteeing the slot contents written before the store are visible after the load.

**False sharing** between `head_` and `tail_`: even though no two threads write the same atomic, if `head_` and `tail_` share a cache line, every store to one invalidates the other thread's cached copy of the other. Fix: pad each atomic to its own cache line using `alignas(std::hardware_destructive_interference_size)`.

```
Cache line 0: [ head_ (8 bytes) | padding (56 bytes) ]
Cache line 1: [ tail_ (8 bytes) | padding (56 bytes) ]
```

---

## Work-Stealing Thread Pool

A simple work-queue thread pool (one shared queue, N workers) has a contention bottleneck: all N workers compete to pop from the same queue under a single mutex. For coarse-grained tasks this is fine; for fine-grained tasks it becomes a bottleneck.

**Work-stealing** distributes the queue across workers:

- Each worker thread has its own **deque** (double-ended queue).
- A new task submitted by a worker goes to the back of that worker's deque.
- Each worker pops from the **back** of its own deque (LIFO — the newest task, likely hot in cache).
- When a worker's deque is empty, it **steals** from the **front** of another worker's deque (FIFO — the oldest task, least likely to be re-stolen).

**Why LIFO local, FIFO steal?**
- Local LIFO exploits cache locality: the task pushed last was probably just created and has warm data.
- Steal FIFO: the oldest tasks are the largest (parent tasks in a recursive decomposition) — stealing the oldest work balances load better.

**Contention is at the steal operation only.** A worker's deque sees contention only when another thread tries to steal from the front simultaneously. The Chase-Lev lock-free deque algorithm uses a size-1 atomic gap between `top` (steal end) and `bottom` (owner end) to make the common case (owner push/pop) lock-free, with only rare contention at the steal end.

For most applications, a lock-protected deque per worker (with a short lock window) achieves 95% of the theoretical optimum with far simpler code.

---

## std::shared_mutex — Readers-Writer Lock

`std::shared_mutex` (C++17) allows multiple concurrent readers or one exclusive writer:

| Operation | Function | Lock type |
|-----------|----------|-----------|
| Reader acquire | `lock_shared()` | `std::shared_lock<shared_mutex>` |
| Reader release | `unlock_shared()` | (released by `shared_lock` destructor) |
| Writer acquire | `lock()` | `std::unique_lock<shared_mutex>` |
| Writer release | `unlock()` | (released by `unique_lock` destructor) |

```cpp
std::shared_mutex rw;
std::map<int, std::string> data;

// Read path (many threads):
std::shared_lock<std::shared_mutex> read_lock(rw);
auto it = data.find(key);

// Write path (exclusive):
std::unique_lock<std::shared_mutex> write_lock(rw);
data[key] = value;
```

**Write starvation** is possible in write-heavy workloads: if readers continuously hold the shared lock, a writer waiting for exclusive access may starve. The standard does not mandate fairness. In write-heavy workloads, a regular mutex is often better. `shared_mutex` is beneficial for read-heavy workloads with occasional writes.

---

## C++20 Synchronization Primitives: semaphore, latch, barrier

### counting_semaphore and binary_semaphore

A semaphore maintains a count. `acquire()` decrements and blocks if count is zero. `release(n)` increments by n and wakes waiting threads.

```cpp
std::counting_semaphore<4> pool{4};  // max 4 concurrent users
pool.acquire();   // blocks if all 4 slots taken
do_work();
pool.release();   // returns a slot
```

`std::binary_semaphore` is `std::counting_semaphore<1>` — useful as a lightweight signal between threads (similar to a condition variable but without requiring a mutex).

### std::latch

A `std::latch` is a single-use countdown counter. Threads call `count_down()` to decrement; threads call `wait()` to block until the count reaches zero. Once zero, it stays zero — a latch cannot be reset.

```cpp
std::latch ready{3};  // count of 3
// In each of 3 threads:
initialize_subsystem();
ready.count_down();  // signal done
// In main thread:
ready.wait();  // blocks until all 3 have counted down
```

### std::barrier

A `std::barrier` is a reusable latch. All participating threads call `arrive_and_wait()`, which blocks until all have arrived. Then all are released simultaneously, and the barrier resets for the next phase. An optional completion function runs (on an unspecified thread) when the last thread arrives, before any are released.

```cpp
std::barrier sync{4, []{ swap_buffers(); }};  // 4 threads, completion fn
// In each thread, once per phase:
do_phase_work();
sync.arrive_and_wait();  // all 4 block here; completion fn runs; all released
do_next_phase();
sync.arrive_and_wait();  // reusable — works again
```

Use `latch` for one-shot "wait for N events to occur." Use `barrier` for iterative phases where you need all workers to complete each phase before starting the next.

---

## C++20 Coroutines: promise_type and the co_await Transformation

A coroutine is a function that can suspend its execution and be resumed later. The compiler transforms the coroutine body into a state machine stored in a **coroutine frame** (heap-allocated).

**The transformation of `co_await expr`:**
1. Evaluate `expr` to get an **awaitable**.
2. Call `awaitable.await_ready()`. If true, continue without suspending.
3. If not ready, call `awaitable.await_suspend(handle)` passing the current coroutine's handle. The coroutine suspends — control returns to the caller or resumer.
4. When resumed, call `awaitable.await_resume()` to get the result value.

**promise_type controls the coroutine lifecycle:**

```cpp
struct Task {
    struct promise_type {
        Task get_return_object() { return Task{...}; }
        std::suspend_never  initial_suspend() { return {}; }  // eager start
        std::suspend_always final_suspend() noexcept { return {}; }  // pause at end
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
};
```

- `initial_suspend()`: returning `suspend_never` makes the coroutine start eagerly. Returning `suspend_always` makes it lazy (start only when explicitly resumed).
- `final_suspend()`: returning `suspend_always` keeps the frame alive after the coroutine completes, allowing the caller to extract a result. Must be `noexcept`.
- `yield_value(T)`: called for `co_yield expr` — stores the value and suspends.
- `return_value(T)` or `return_void()`: called for `co_return`.

**C++20 coroutines have no built-in executor.** Unlike Python's asyncio or JavaScript's event loop, there is no scheduler that drives coroutines automatically. You must explicitly resume them (call `handle.resume()`), or use a library (libcoro, cppcoro, Asio) that provides an executor. Raw C++20 coroutines are a mechanism, not a framework.

---

## False Sharing and hardware_destructive_interference_size

Two threads write to different variables. The variables happen to share a 64-byte cache line. Every time thread A writes its variable, it invalidates thread B's cache line (MESI protocol: line transitions to Invalid state). Thread B must re-fetch the line from L3 or main memory before its next write — even though thread B's variable has not logically changed.

This is **false sharing**: the hardware cannot distinguish "thread A wrote byte 0" from "thread A wrote the whole line."

**Benchmark impact:** 10-20x slowdown is common in tight loops.

**Fix:** align each frequently-written variable to its own cache line:

```cpp
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winterference-size"
struct Counters {
    alignas(std::hardware_destructive_interference_size) std::atomic<long> a{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<long> b{0};
};
#pragma GCC diagnostic pop
```

`std::hardware_destructive_interference_size` is 64 on x86 (the L1 cache line size). GCC 12+ may warn about its use with `-Wall -Wextra` because the value is ABI-dependent — the diagnostic pragma suppresses this. Alternatively, use `static constexpr size_t CACHE_LINE = 64;` as a portable constant for x86.

The complementary constant `std::hardware_constructive_interference_size` (also 64 on x86) is for data you **want** on the same cache line — e.g., a small struct accessed together that fits in 64 bytes.

---

## The Broken Double-Checked Locking Pattern (and the Fix)

Double-checked locking is a classic pattern for lazy initialization:

```cpp
// BROKEN — do not use
Widget* g_widget = nullptr;
std::mutex mu;

Widget* get_widget() {
    if (g_widget == nullptr) {       // first check — no lock
        std::lock_guard lk(mu);
        if (g_widget == nullptr) {   // second check — with lock
            g_widget = new Widget(); // construction + pointer write
        }
    }
    return g_widget;
}
```

This is broken because the pointer write and the construction of the Widget object can be **reordered by the CPU or compiler**. Another thread reading `g_widget != nullptr` in the first check may see the pointer before the object's fields have been written. The result is a non-null pointer to an unconstructed object — undefined behaviour.

**Fix 1: std::atomic with acquire/release**

```cpp
std::atomic<Widget*> g_widget{nullptr};
std::mutex mu;

Widget* get_widget() {
    Widget* p = g_widget.load(std::memory_order_acquire);
    if (p == nullptr) {
        std::lock_guard lk(mu);
        p = g_widget.load(std::memory_order_relaxed);
        if (p == nullptr) {
            p = new Widget();
            g_widget.store(p, std::memory_order_release);
        }
    }
    return p;
}
```

The release store ensures the object is fully constructed before the pointer becomes visible. The acquire load ensures the reader sees the construction.

**Fix 2: Meyers singleton (function-local static)**

```cpp
Widget& get_widget() {
    static Widget instance;  // initialized once, on first call — guaranteed by C++11
    return instance;
}
```

C++11 guarantees that function-local statics are initialized exactly once, with correct synchronization, even if multiple threads call the function simultaneously. This is the simplest correct implementation. The generated code uses a hidden flag and a mutex (or an equivalent atomic protocol).

**Fix 3: std::call_once**

```cpp
std::once_flag flag;
Widget* g_widget = nullptr;

Widget* get_widget() {
    std::call_once(flag, []{ g_widget = new Widget(); });
    return g_widget;
}
```

`std::call_once` guarantees the callable runs exactly once across all threads, with full synchronization. Subsequent calls return immediately. Prefer the Meyers singleton unless you specifically need dynamic allocation or pointer semantics.
