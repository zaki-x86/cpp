#pragma once
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace foundation {

class ThreadPool {
    std::vector<std::thread>        workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex                      mutex_;
    std::condition_variable         cv_;
    bool                            stop_{false};

public:
    explicit ThreadPool(std::size_t n) {
        workers_.reserve(n);
        for (std::size_t i = 0; i < n; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock lock{mutex_};
                        cv_.wait(lock, [this]{ return stop_ || !tasks_.empty(); });
                        if (stop_ && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        { std::lock_guard lock{mutex_}; stop_ = true; }
        cv_.notify_all();
        for (auto& w : workers_) w.join();
    }

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using Ret = std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<Ret()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<Ret> fut = task->get_future();
        { std::lock_guard lock{mutex_}; tasks_.emplace([task]{ (*task)(); }); }
        cv_.notify_one();
        return fut;
    }
};

} // namespace foundation
