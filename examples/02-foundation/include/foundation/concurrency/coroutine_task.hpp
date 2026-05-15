#pragma once
#include <coroutine>
#include <exception>
#include <utility>

namespace foundation {

// Minimal eager Task<T>: a C++20 coroutine that runs synchronously to completion
// unless co_awaited. Demonstrates the coroutine machinery:
// promise_type, coroutine_handle, initial_suspend, final_suspend.
template<typename T>
class Task {
public:
    struct promise_type {
        T value_{};
        std::exception_ptr exception_{};

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never  initial_suspend() noexcept { return {}; }  // eager: run immediately
        std::suspend_always final_suspend()   noexcept { return {}; }  // keep alive for result

        void return_value(T v) noexcept { value_ = std::move(v); }
        void unhandled_exception() { exception_ = std::current_exception(); }
    };

    explicit Task(std::coroutine_handle<promise_type> h) : handle_{h} {}
    ~Task() { if (handle_) handle_.destroy(); }

    Task(const Task&) = delete;
    Task(Task&& o) noexcept : handle_{std::exchange(o.handle_, {})} {}

    T result() {
        if (handle_.promise().exception_)
            std::rethrow_exception(handle_.promise().exception_);
        return handle_.promise().value_;
    }

    bool done() const noexcept { return !handle_ || handle_.done(); }

private:
    std::coroutine_handle<promise_type> handle_;
};

// Example coroutine using Task<T>
inline Task<int> async_square(int x) {
    co_return x * x;
}

} // namespace foundation
