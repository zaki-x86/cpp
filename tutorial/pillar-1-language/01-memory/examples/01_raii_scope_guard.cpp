// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t01 01_raii_scope_guard.cpp
// Demonstrates: RAII ScopeGuard — acquire on entry, release on exit (even via exception)

#include <cstdio>
#include <functional>
#include <stdexcept>
#include <utility>

template<typename F>
class ScopeGuard {
    F fn_;
    bool active_;
public:
    explicit ScopeGuard(F&& fn) : fn_(std::move(fn)), active_(true) {}
    ~ScopeGuard() { if (active_) fn_(); }

    void dismiss() noexcept { active_ = false; }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&&) = delete;
    ScopeGuard& operator=(ScopeGuard&&) = delete;
};

template<typename F>
ScopeGuard<F> make_scope_guard(F&& fn) {
    return ScopeGuard<F>(std::forward<F>(fn));
}

// --- Demo 1: Normal scope exit ---
void demo_normal() {
    printf("\n--- Demo 1: Normal scope exit ---\n");
    printf("Acquired resource\n");
    auto guard = make_scope_guard([]{ printf("Released resource\n"); });
    printf("Doing work\n");
}  // guard fires here

// --- Demo 2: dismiss() called ---
void demo_dismissed() {
    printf("\n--- Demo 2: Guard dismissed ---\n");
    printf("Acquired resource\n");
    auto guard = make_scope_guard([]{ printf("Released resource (should NOT print)\n"); });
    guard.dismiss();
    printf("Guard dismissed — resource will be managed elsewhere\n");
}

// --- Demo 3: Exception path ---
void demo_exception() {
    printf("\n--- Demo 3: Exception path ---\n");
    printf("Acquired resource\n");
    auto guard = make_scope_guard([]{ printf("Released resource (exception path)\n"); });
    try {
        throw std::runtime_error("simulated failure");
    } catch (const std::exception& e) {
        printf("Caught: %s\n", e.what());
        // guard fires during catch block's scope exit (after catch finishes)
    }
}

int main() {
    demo_normal();
    demo_dismissed();
    demo_exception();
    return 0;
}
