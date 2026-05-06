// 03_memory_order_demo.cpp — Memory ordering demonstrations
//
// TSan cannot execute in WSL2 (kernel VM constraint).
// Run under Docker or native Linux for race detection.
// Example: docker run --rm -v $(pwd):/w -w /w gcc:11
//          g++ -std=c++20 -fsanitize=thread -O1 -o /tmp/mo 03_memory_order_demo.cpp
//          /tmp/mo
//
// Three demos:
//   1. Correct acquire/release message passing
//   2. seq_cst vs relaxed timing (single-thread microbenchmark)
//   3. Relaxed counter correctness (4 threads, 4M increments)

#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

// ============================================================================
// Demo 1 — Acquire/release message passing
//
// Producer writes data, then signals ready with a release store.
// Consumer spins on an acquire load of ready before reading data.
// The acquire/release pair establishes a happens-before edge:
// data = 42 is guaranteed visible when the consumer reads it.
// ============================================================================
static void demo_message_passing() {
    std::cout << "=== Demo 1: acquire/release message passing ===\n";

    std::atomic<int>  data{0};
    std::atomic<bool> ready{false};

    std::jthread producer([&]() {
        data.store(42, std::memory_order_relaxed);   // write data
        ready.store(true, std::memory_order_release); // publish: release
    });

    std::jthread consumer([&]() {
        // Acquire load: once we see ready==true, data==42 is guaranteed.
        while (!ready.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        int val = data.load(std::memory_order_relaxed);
        assert(val == 42 && "acquire/release guarantees data visibility");
        std::cout << "  Consumer read data = " << val << "  (expected 42) PASS\n";
    });

    producer.join();
    consumer.join();
}

// ============================================================================
// Demo 2 — seq_cst vs relaxed timing (single-thread microbenchmark)
//
// Incrementing a single atomic 10M times, first with seq_cst (default),
// then with relaxed.
// On x86, seq_cst and relaxed fetch_add both emit 'lock addq' — the LOCK
// prefix is required for atomicity regardless of ordering constraint.
// The cost difference here is small on x86's TSO model.
// On ARM/RISC-V, the gap is larger: relaxed emits no barrier while seq_cst
// emits a full 'dmb ish' fence.
// ============================================================================
static void demo_ordering_cost() {
    std::cout << "\n=== Demo 2: seq_cst vs relaxed ordering cost ===\n";

    constexpr long ITERS = 10'000'000L;

    std::atomic<long> counter{0};

    // seq_cst
    auto t0 = std::chrono::steady_clock::now();
    for (long i = 0; i < ITERS; ++i) {
        counter.fetch_add(1, std::memory_order_seq_cst);
    }
    auto t1 = std::chrono::steady_clock::now();
    long ms_seq = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    counter.store(0, std::memory_order_relaxed);

    // relaxed
    auto t2 = std::chrono::steady_clock::now();
    for (long i = 0; i < ITERS; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
    auto t3 = std::chrono::steady_clock::now();
    long ms_rel = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();

    std::cout << "  seq_cst: " << ms_seq << "ms\n";
    std::cout << "  relaxed: " << ms_rel << "ms\n";
    std::cout << "  (On x86 the difference is small; on ARM it is larger)\n";
}

// ============================================================================
// Demo 3 — Relaxed counter correctness
//
// Four threads each increment a shared atomic counter 1M times with
// memory_order_relaxed. Relaxed provides only atomicity — no ordering
// constraints relative to other memory operations. But atomicity is enough
// to guarantee no lost updates on a single variable.
//
// After all threads finish, counter == 4M exactly.
// ============================================================================
static void demo_relaxed_counter() {
    std::cout << "\n=== Demo 3: relaxed counter correctness ===\n";

    constexpr int NUM_THREADS = 4;
    constexpr int ITERS_PER_THREAD = 1'000'000;
    constexpr long EXPECTED = static_cast<long>(NUM_THREADS) * ITERS_PER_THREAD;

    std::atomic<long> counter{0};

    std::vector<std::jthread> threads;
    threads.reserve(NUM_THREADS);

    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&counter]() {
            for (int i = 0; i < ITERS_PER_THREAD; ++i) {
                // Relaxed: no ordering, only atomicity.
                // Safe here because we only care about the final count,
                // not any associated data dependency.
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // jthread destructors join all threads.
    threads.clear();

    long total = counter.load(std::memory_order_seq_cst);
    assert(total == EXPECTED && "no lost updates with atomic fetch_add");

    std::cout << "  " << NUM_THREADS << " threads x " << ITERS_PER_THREAD
              << " increments = " << total
              << "  expected=" << EXPECTED
              << "  PASS\n";
}

int main() {
    demo_message_passing();
    demo_ordering_cost();
    demo_relaxed_counter();

    std::cout << "\nAll demos passed.\n";
    return 0;
}
