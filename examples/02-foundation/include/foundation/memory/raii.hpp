#pragma once
#include <utility>

namespace foundation {

// Generic RAII scope guard — runs a callable on destruction.
// Movable, non-copyable. Call dismiss() to cancel the cleanup.
template<typename F>
class ScopeGuard {
    F fn_;
    bool active_{true};
public:
    explicit ScopeGuard(F&& f) : fn_{std::forward<F>(f)} {}
    ~ScopeGuard() { if (active_) fn_(); }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&& other) noexcept
        : fn_{std::move(other.fn_)}, active_{other.active_} {
        other.active_ = false;
    }

    void dismiss() noexcept { active_ = false; }
};

template<typename F>
[[nodiscard]] auto make_scope_guard(F&& f) {
    return ScopeGuard<std::decay_t<F>>{std::forward<F>(f)};
}

// FOUNDATION_DEFER: defer-style cleanup (like Go's defer / Rust's drop)
#define FOUNDATION_DEFER(expr) \
    auto FOUNDATION_DEFER_##__LINE__ = ::foundation::make_scope_guard([&]{ expr; })

} // namespace foundation
