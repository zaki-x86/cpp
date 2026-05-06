// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t13 03_expected_chain.cpp
// Demonstrates: manual Expected<T,E> with monadic and_then/transform/or_else chaining
// (std::expected is C++23 -- not available in GCC 11 without -std=c++23)

#include <cstdio>
#include <cmath>
#include <string>
#include <variant>

// --- Expected<T, E> --------------------------------------------------------------
template<typename T, typename E>
class Expected {
    std::variant<T, E> data_;
    bool has_value_;

    Expected() = default;
public:
    static Expected success(T v) {
        Expected r;
        r.data_      = std::move(v);
        r.has_value_ = true;
        return r;
    }
    static Expected error(E e) {
        Expected r;
        r.data_      = std::move(e);
        r.has_value_ = false;
        return r;
    }

    bool    has_value() const { return has_value_; }
    const T& value()   const { return std::get<T>(data_); }
    const E& err()     const { return std::get<E>(data_); }

    // and_then: chain operations that return Expected<U, E>
    template<typename F>
    auto and_then(F&& f) const {
        using U = decltype(f(value()));
        if (has_value_) return f(value());
        else            return U::error(err());
    }

    // transform: map success value, wrapping result in Expected<U, E>
    template<typename F>
    auto transform(F&& f) const {
        using U = std::decay_t<decltype(f(value()))>;
        if (has_value_) return Expected<U, E>::success(f(value()));
        else            return Expected<U, E>::error(err());
    }

    // or_else: handle error, pass success through unchanged
    template<typename F>
    Expected or_else(F&& f) const {
        if (!has_value_) return f(err());
        return *this;
    }
};

using Result       = Expected<int,    std::string>;
using DoubleResult = Expected<double, std::string>;

// --- Pipeline functions ----------------------------------------------------------
Result parse_int(const std::string& s) {
    try {
        return Result::success(std::stoi(s));
    } catch (...) {
        return Result::error("not a number: '" + s + "'");
    }
}

Result validate_positive(int n) {
    if (n > 0) return Result::success(n);
    return Result::error("must be positive, got " + std::to_string(n));
}

DoubleResult compute_sqrt(int n) {
    return DoubleResult::success(std::sqrt(static_cast<double>(n)));
}

int main() {
    printf("--- Monadic chaining with Expected<T,E> ---\n\n");

    // Success path
    auto r1 = parse_int("16")
        .and_then(validate_positive)
        .and_then(compute_sqrt);
    if (r1.has_value())
        printf("sqrt(16) = %.1f\n", r1.value());
    else
        printf("Error: %s\n", r1.err().c_str());

    // Error: not a number (error propagates — validate_positive and compute_sqrt never called)
    auto r2 = parse_int("abc")
        .and_then(validate_positive)
        .and_then(compute_sqrt);
    if (r2.has_value())
        printf("result = %.1f\n", r2.value());
    else
        printf("Error: %s\n", r2.err().c_str());

    // Error: not positive (parse succeeds, validate fails, compute_sqrt skipped)
    auto r3 = parse_int("-5")
        .and_then(validate_positive);
    if (r3.has_value())
        printf("result = %d\n", r3.value());
    else
        printf("Error: %s\n", r3.err().c_str());

    // transform: square the result without wrapping in Expected
    printf("\n--- transform ---\n");
    auto r4 = parse_int("7")
        .and_then(validate_positive)
        .transform([](int n) { return n * n; });
    if (r4.has_value())
        printf("7^2 = %d\n", r4.value());

    // or_else: provide a fallback when an error occurs
    printf("\n--- or_else fallback ---\n");
    auto r5 = parse_int("bad")
        .or_else([](const std::string& e) -> Result {
            printf("Recovering from: %s\n", e.c_str());
            return Result::success(0);  // default value on failure
        });
    printf("Fallback value: %d\n", r5.value());

    return 0;
}
