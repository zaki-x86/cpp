// 01_spsc_queue.cpp — Lock-free single-producer single-consumer ring buffer
//
// Demonstrates: acquire/release atomics, cache-line padding, jthread.
//
// TSan cannot execute in WSL2 (kernel VM constraint).
// Use Docker or native Linux for race detection.
// Example: docker run --rm -v $(pwd):/w -w /w gcc:11
//          g++ -std=c++20 -fsanitize=thread -O1 -o /tmp/spsc 01_spsc_queue.cpp
//          /tmp/spsc

#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <new>
#include <thread>

// Use a compile-time constant instead of hardware_destructive_interference_size
// to avoid -Winterference-size diagnostic on GCC 12+.
static constexpr std::size_t CACHE_LINE = 64;

// ---------------------------------------------------------------------------
// SPSCQueue<T, N>
//
// Ring buffer with capacity N-1 (one slot is always unused to distinguish
// full from empty without a separate counter).
//
// Invariants:
//   - head_ is written only by the producer.
//   - tail_ is written only by the consumer.
//   - head_ == tail_          => empty
//   - (head_ + 1) % N == tail_ => full
// ---------------------------------------------------------------------------
template <typename T, std::size_t N>
class SPSCQueue {
    static_assert(N >= 2, "Queue capacity must be at least 2");

public:
    SPSCQueue() : head_(0), tail_(0) {}

    // Non-copyable, non-movable.
    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;

    // Push one item. Returns false if the queue is full.
    // Called from the producer thread only.
    bool push(const T& value) {
        const std::size_t h = head_.load(std::memory_order_relaxed);
        const std::size_t next_h = (h + 1) % N;

        // Read tail with acquire so we see the consumer's latest pop.
        if (next_h == tail_.load(std::memory_order_acquire)) {
            return false;  // full
        }

        slots_[h] = value;

        // Release: make the slot write visible to the consumer before
        // updating head_.
        head_.store(next_h, std::memory_order_release);
        return true;
    }

    // Pop one item into 'out'. Returns false if the queue is empty.
    // Called from the consumer thread only.
    bool pop(T& out) {
        // Read head with acquire so we see the producer's slot write.
        const std::size_t h = head_.load(std::memory_order_acquire);
        const std::size_t t = tail_.load(std::memory_order_relaxed);

        if (h == t) {
            return false;  // empty
        }

        out = slots_[t];

        const std::size_t next_t = (t + 1) % N;

        // Release: make the slot consumption visible to the producer before
        // updating tail_.
        tail_.store(next_t, std::memory_order_release);
        return true;
    }

private:
    std::array<T, N> slots_{};

    // Each atomic on its own cache line to prevent false sharing.
    // Without padding, both atomics share a 64-byte line. Every release store
    // by the producer invalidates the consumer's cached copy of that line,
    // causing cache-line ping-pong between cores.
    alignas(CACHE_LINE) std::atomic<std::size_t> head_;
    alignas(CACHE_LINE) std::atomic<std::size_t> tail_;
};

// ---------------------------------------------------------------------------
// Demo: 1M item throughput test
// ---------------------------------------------------------------------------
int main() {
    constexpr int    TOTAL    = 1'000'000;
    constexpr long long EXPECTED = static_cast<long long>(TOTAL - 1) *
                                   static_cast<long long>(TOTAL) / 2;
    // Expected sum = 0 + 1 + ... + 999999 = 499999500000

    // Queue capacity: 1024 slots (ring buffer holds 1023 items at once).
    SPSCQueue<int, 1024> q;

    // Shared result — written by consumer only, read by main after join.
    long long sum = 0;

    // Producer: push integers 0 .. TOTAL-1.
    std::jthread producer([&q]() {
        for (int i = 0; i < TOTAL; ++i) {
            while (!q.push(i)) {
                std::this_thread::yield();  // spin if full
            }
        }
    });

    // Consumer: pop and accumulate.
    std::jthread consumer([&q, &sum]() {
        int received = 0;
        int val = 0;
        while (received < TOTAL) {
            if (q.pop(val)) {
                sum += val;
                ++received;
            } else {
                std::this_thread::yield();  // spin if empty
            }
        }
    });

    // jthread joins on destruction — both threads complete here.
    producer.join();
    consumer.join();

    if (sum == EXPECTED) {
        std::cout << "PASS  sum=" << sum << "  expected=" << EXPECTED << "\n";
        return 0;
    } else {
        std::cout << "FAIL  sum=" << sum << "  expected=" << EXPECTED << "\n";
        return 1;
    }
}
