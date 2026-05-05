#pragma once
#include <memory>
#include <utility>

namespace foundation {

// AnyCallable<Ret(Args...)>: pedagogical std::function.
// Demonstrates type erasure: the concrete callable type is hidden
// behind a virtual Base interface — caller only knows the signature.
template<typename Sig> class AnyCallable;

template<typename Ret, typename... Args>
class AnyCallable<Ret(Args...)> {
    struct Base {
        virtual ~Base() = default;
        virtual Ret call(Args...) = 0;
        virtual std::unique_ptr<Base> clone() const = 0;
    };

    template<typename F>
    struct Wrapper final : Base {
        F fn;
        explicit Wrapper(F f) : fn{std::move(f)} {}
        Ret call(Args... args) override { return fn(args...); }
        std::unique_ptr<Base> clone() const override {
            return std::make_unique<Wrapper>(fn);
        }
    };

    std::unique_ptr<Base> impl_;

public:
    AnyCallable() = default;

    template<typename F>
    AnyCallable(F f) : impl_{std::make_unique<Wrapper<std::decay_t<F>>>(std::move(f))} {}

    AnyCallable(const AnyCallable& o) : impl_{o.impl_ ? o.impl_->clone() : nullptr} {}
    AnyCallable(AnyCallable&&) = default;
    AnyCallable& operator=(AnyCallable o) { std::swap(impl_, o.impl_); return *this; }

    Ret operator()(Args... args) const { return impl_->call(args...); }
    explicit operator bool() const noexcept { return impl_ != nullptr; }
};

} // namespace foundation
