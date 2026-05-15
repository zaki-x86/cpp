#include <foundation/concurrency/lock_free_queue.hpp>
#include <foundation/concurrency/thread_pool.hpp>
#include <foundation/concurrency/coroutine_task.hpp>
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <future>
#include <chrono>
#include <numeric>

// A coroutine that chains two operations
foundation::Task<double> async_compute(double x) {
    co_return x * x + 1.0;
}

int main() {
    std::cout << "=== Concurrency Demo ===\n\n";

    // --- SPSCQueue ---
    std::cout << "-- Lock-Free SPSC Queue --\n";
    {
        foundation::SPSCQueue<int, 256> q;
        constexpr int N = 100;
        std::atomic<int> sum{0};

        auto t0 = std::chrono::steady_clock::now();

        std::thread producer([&]{
            for (int i = 0; i < N; ++i)
                while (!q.push(i)) {}
        });

        std::thread consumer([&]{
            int received = 0;
            while (received < N) {
                int v;
                if (q.pop(v)) { sum += v; ++received; }
            }
        });

        producer.join();
        consumer.join();

        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - t0).count();

        int expected = N * (N - 1) / 2;
        std::cout << "  transferred " << N << " items, sum=" << sum
                  << " (expected=" << expected << "), " << ms << " us\n";
    }

    // --- ThreadPool ---
    std::cout << "\n-- Thread Pool --\n";
    {
        foundation::ThreadPool pool(4);
        constexpr int N = 20;
        std::vector<std::future<int>> futures;
        futures.reserve(N);
        for (int i = 0; i < N; ++i)
            futures.push_back(pool.submit([i]{ return i * i; }));

        int total = 0;
        for (auto& f : futures) total += f.get();
        std::cout << "  sum of squares 0..19 = " << total << "\n";

        // Exception propagation
        auto bad = pool.submit([]() -> int {
            throw std::runtime_error("intentional");
            return 0;
        });
        try {
            bad.get();
        } catch (const std::exception& e) {
            std::cout << "  caught exception from pool task: " << e.what() << "\n";
        }
    }

    // --- Coroutines ---
    std::cout << "\n-- C++20 Coroutines (Task<T>) --\n";
    {
        auto t1 = foundation::async_square(7);
        std::cout << "  async_square(7) = " << t1.result() << "\n";

        auto t2 = async_compute(4.0);
        std::cout << "  async_compute(4.0) = " << t2.result() << "\n";

        // Eager: side effect happens on Task construction
        int flag = 0;
        auto make = [&]() -> foundation::Task<int> {
            flag = 42;
            co_return flag;
        };
        auto t3 = make();
        std::cout << "  eager execution, flag set before result(): " << flag << "\n";
        std::cout << "  result = " << t3.result() << "\n";
    }

    std::cout << "\nDone.\n";
}
