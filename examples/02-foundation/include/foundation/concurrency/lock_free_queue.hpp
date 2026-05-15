#pragma once
#include <array>
#include <atomic>
#include <cstddef>

namespace foundation {

// Lock-free Single-Producer Single-Consumer (SPSC) queue.
// Uses acquire/release atomics on head/tail — no mutex.
// Capacity = N-1 (one slot wasted to distinguish full from empty).
// N must be a power of 2 for the bitmask trick.
template<typename T, std::size_t N>
class SPSCQueue {
    static_assert((N & (N - 1)) == 0, "N must be a power of 2");

    std::array<T, N> buf_{};
    // Pad to separate cache lines — prevents false sharing between threads
    alignas(64) std::atomic<std::size_t> head_{0};  // written by consumer
    alignas(64) std::atomic<std::size_t> tail_{0};  // written by producer

public:
    // Called from producer thread only. Returns false if full.
    bool push(const T& val) {
        auto tail = tail_.load(std::memory_order_relaxed);
        auto next = (tail + 1) & (N - 1);
        if (next == head_.load(std::memory_order_acquire)) return false;  // full
        buf_[tail] = val;
        tail_.store(next, std::memory_order_release);
        return true;
    }

    // Called from consumer thread only. Returns false if empty.
    bool pop(T& out) {
        auto head = head_.load(std::memory_order_relaxed);
        if (head == tail_.load(std::memory_order_acquire)) return false;  // empty
        out = buf_[head];
        head_.store((head + 1) & (N - 1), std::memory_order_release);
        return true;
    }

    bool empty() const noexcept {
        return head_.load(std::memory_order_relaxed) ==
               tail_.load(std::memory_order_relaxed);
    }
};

} // namespace foundation
