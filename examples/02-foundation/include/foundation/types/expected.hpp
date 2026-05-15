#pragma once
// foundation::Expected<T,E>: polyfill for std::expected (C++23).
// Same monadic interface (.and_then, .or_else, .value_or, .error()).
// Use std::expected directly when GCC 12+ / Clang 14+ is available.

#include <stdexcept>
#include <utility>
#include <variant>
#include <functional>

namespace foundation {

template<typename E>
struct Unexpected {
    E error;
    explicit Unexpected(E e) : error{std::move(e)} {}
};

template<typename E>
Unexpected<E> make_unexpected(E e) { return Unexpected<E>{std::move(e)}; }

template<typename T, typename E>
class Expected {
    std::variant<T, E> data_;
    bool has_value_;

public:
    // Construct from value
    Expected(T val) : data_{std::move(val)}, has_value_{true} {}
    // Construct from error
    Expected(Unexpected<E> err) : data_{std::move(err.error)}, has_value_{false} {}

    bool has_value() const noexcept { return has_value_; }
    explicit operator bool() const noexcept { return has_value_; }

    T& value() & {
        if (!has_value_) throw std::runtime_error("Expected: no value");
        return std::get<T>(data_);
    }
    const T& value() const& {
        if (!has_value_) throw std::runtime_error("Expected: no value");
        return std::get<T>(data_);
    }
    T value_or(T def) const { return has_value_ ? std::get<T>(data_) : def; }

    const E& error() const {
        if (has_value_) throw std::runtime_error("Expected: no error");
        return std::get<E>(data_);
    }

    // Monadic: transform value if present
    template<typename F>
    auto and_then(F&& f) const -> decltype(f(std::declval<T>())) {
        using Ret = decltype(f(std::declval<T>()));
        if (has_value_) return f(std::get<T>(data_));
        return Ret{Unexpected<E>{std::get<E>(data_)}};
    }

    // Monadic: handle error if present
    template<typename F>
    auto or_else(F&& f) const -> decltype(f(std::declval<E>())) {
        using Ret = decltype(f(std::declval<E>()));
        if (!has_value_) return f(std::get<E>(data_));
        return Ret{std::get<T>(data_)};
    }
};

// Convenience: error-returning functions
enum class MathError { DivisionByZero, NegativeSqrt, ParseError };

inline Expected<double, MathError> safe_divide(double a, double b) {
    if (b == 0.0) return make_unexpected(MathError::DivisionByZero);
    return a / b;
}

inline Expected<double, MathError> safe_sqrt(double x) {
    if (x < 0.0) return make_unexpected(MathError::NegativeSqrt);
    return std::sqrt(x);
}

inline Expected<double, MathError> parse_double(const std::string& s) {
    try {
        std::size_t pos{};
        double v = std::stod(s, &pos);
        if (pos != s.size()) return make_unexpected(MathError::ParseError);
        return v;
    } catch (...) {
        return make_unexpected(MathError::ParseError);
    }
}

// Monadic chain: parse → sqrt
inline Expected<double, MathError> parse_and_sqrt(const std::string& s) {
    return parse_double(s).and_then(safe_sqrt);
}

} // namespace foundation
