# Chapter 06: Concurrency — Writing Correct Multi-Threaded C++

Multi-threaded code is easy to write and hard to write correctly. The C++ memory model tells you exactly what guarantees you have — and the rules are stricter than most developers expect. A data race is undefined behaviour: not "probably fine," not "wrong value sometimes" — undefined behaviour that the compiler is allowed to exploit. This chapter builds correct intuition from the memory model up: what the hardware actually does, what C++ promises, and how to write code you can reason about.

**Prerequisites:**
- Chapter 01-memory (memory model, acquire/release, happens-before — essential foundation)
- Chapter 04-type-system (atomic types, `std::atomic<T>` — you will use these directly)

**WSL2 note:** ThreadSanitizer (TSan) cannot execute in WSL2 due to a kernel virtual-memory mapping constraint. All examples in this chapter are designed to run and demonstrate correct behaviour without TSan. To use TSan for race detection, run under Docker or native Linux.

**Time estimate:** Core = 45 minutes · Deep Dive = 3 hours · Interview = 30 minutes

**Table of Contents:**

| File | What It Covers |
|------|----------------|
| `core.md` | The three concurrency problems. The three tools. `jthread`. The rule: protect all shared mutable state. Production rules. Lab link. |
| `deep-dive.md` | Memory model happens-before/synchronizes-with. All six memory orders with examples. Lock-free SPSC queue. Work-stealing thread pool. `shared_mutex`. C++20 latch/barrier/semaphore. Coroutine internals. False sharing. Broken DCLP and the fix. |
| `interview.md` | 8 Q&A with traps — data race vs race condition, acquire/release, ABA problem, `jthread`, readers-writer, false sharing, latch vs barrier, coroutines. |
| `examples/01_spsc_queue.cpp` | Lock-free single-producer single-consumer ring buffer with cache-line padding. 1M item throughput demo. |
| `examples/02_thread_pool.cpp` | Thread pool with `jthread`, `packaged_task`, `future`, `condition_variable`. Graceful shutdown via stop flag. |
| `examples/03_memory_order_demo.cpp` | Acquire/release message passing. `seq_cst` vs `relaxed` timing. Relaxed counter correctness. |

**Reading paths:**

- "My tests are flaky — sometimes they fail, sometimes they pass" → `core.md` (Data races, Race conditions) + `interview.md` Q1
- "I need to pass data between a producer thread and a consumer thread" → `core.md` (condition_variable) + `examples/01_spsc_queue.cpp` + `deep-dive.md` (SPSC design)
- "I've heard lock-free is faster — should I use it everywhere?" → `core.md` (Three Tools) + `deep-dive.md` (SPSC queue) + `interview.md` Q3
- "What is memory_order_acquire and when do I use it?" → `deep-dive.md` (Six Memory Orders) + `examples/03_memory_order_demo.cpp`
- "What are C++20 coroutines actually doing?" → `deep-dive.md` (Coroutines) + `interview.md` Q8
- "The interviewer asked about false sharing" → `deep-dive.md` (False Sharing) + `interview.md` Q6

**Atlas Lab:** `projects/02-foundation/include/foundation/concurrency/` implements `lock_free_queue.hpp` (SPSC ring buffer), `thread_pool.hpp` (work queue pool), and `coroutine_task.hpp` (eager coroutine task). Run: `ctest --preset debug -R test_concurrency` from `projects/02-foundation/`.
