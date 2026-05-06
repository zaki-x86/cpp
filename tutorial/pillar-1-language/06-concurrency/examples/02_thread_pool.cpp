// 02_thread_pool.cpp — Thread pool with jthread, packaged_task, and future
//
// Demonstrates: jthread RAII, condition_variable, packaged_task, future,
//               cooperative shutdown via stop flag.
//
// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t18 02_thread_pool.cpp
// Run:     /tmp/t18

#include <cassert>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

// ---------------------------------------------------------------------------
// ThreadPool
//
// N worker threads share one task queue protected by a mutex + CV.
// Shutdown: set stop_flag_ true, notify all workers, jthread destructors join.
// ---------------------------------------------------------------------------
class ThreadPool {
public:
    explicit ThreadPool(std::size_t n_threads) {
        workers_.reserve(n_threads);
        for (std::size_t i = 0; i < n_threads; ++i) {
            workers_.emplace_back([this](std::stop_token /*st*/) {
                worker_loop();
            });
        }
    }

    // Non-copyable, non-movable.
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mu_);
            stop_flag_ = true;
        }
        cv_.notify_all();
        // jthread destructors join all workers automatically.
    }

    // Submit a callable with no arguments. Returns a future for the result.
    template <typename F>
    auto submit(F&& f) -> std::future<std::invoke_result_t<F>> {
        using R = std::invoke_result_t<F>;

        // Package the callable so we can extract a future.
        auto task = std::make_shared<std::packaged_task<R()>>(std::forward<F>(f));
        std::future<R> fut = task->get_future();

        {
            std::unique_lock<std::mutex> lock(mu_);
            // Wrap the packaged_task in a type-erased std::function<void()>.
            tasks_.push([task]() { (*task)(); });
        }
        cv_.notify_one();

        return fut;
    }

private:
    void worker_loop() {
        for (;;) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(mu_);
                // Wait until there is a task OR we are stopping.
                cv_.wait(lock, [this] {
                    return stop_flag_ || !tasks_.empty();
                });

                // If stopping and no tasks remain, exit.
                if (stop_flag_ && tasks_.empty()) {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            // Execute outside the lock — never hold a lock while calling
            // user code (deadlock risk if the task itself submits more work).
            task();
        }
    }

    std::vector<std::jthread>            workers_;
    std::queue<std::function<void()>>    tasks_;
    std::mutex                           mu_;
    std::condition_variable              cv_;
    bool                                 stop_flag_{false};
};

// ---------------------------------------------------------------------------
// Demo: 4-thread pool, 10 tasks each returning index^2
// ---------------------------------------------------------------------------
int main() {
    ThreadPool pool{4};

    constexpr int N = 10;
    std::vector<std::future<int>> futures;
    futures.reserve(N);

    for (int i = 0; i < N; ++i) {
        futures.push_back(pool.submit([i]() -> int {
            return i * i;
        }));
    }

    std::cout << "Results (i^2 for i in 0..9):\n";
    for (int i = 0; i < N; ++i) {
        int result = futures[static_cast<std::size_t>(i)].get();
        std::cout << "  " << i << "^2 = " << result << "\n";
        assert(result == i * i);
    }

    std::cout << "All " << N << " tasks completed correctly.\n";
    return 0;
}
