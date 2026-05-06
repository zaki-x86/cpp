#pragma once
#include <concepts>
#include <numeric>
#include <span>
#include <stdexcept>

namespace demo {

// Greatest common divisor — Euclidean algorithm
template <std::integral T>
constexpr T gcd(T a, T b) noexcept {
    while (b != T{0}) {
        a = std::exchange(b, a % b);
    }
    return a;
}

// Least common multiple — derived from GCD
template <std::integral T>
constexpr T lcm(T a, T b) {
    if (a == T{0} || b == T{0}) return T{0};
    return (a / gcd(a, b)) * b;
}

// Dot product of two equal-length spans
template <std::floating_point T>
constexpr T dot(std::span<const T> a, std::span<const T> b) {
    if (a.size() != b.size())
        throw std::invalid_argument("dot: spans must have equal size");
    T result{};
    for (std::size_t i = 0; i < a.size(); ++i)
        result += a[i] * b[i];
    return result;
}

// Clamp value to [lo, hi]
template <typename T>
constexpr T clamp(T value, T lo, T hi) noexcept {
    return value < lo ? lo : (value > hi ? hi : value);
}

} // namespace demo
