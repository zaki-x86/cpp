#pragma once
#include <concepts>
#include <ostream>
#include <string>
#include <type_traits>

namespace foundation {

// ---- Concept definitions ----

// Printable: can be written to an ostream
template<typename T>
concept Printable = requires(T v, std::ostream& os) {
    { os << v } -> std::same_as<std::ostream&>;
};

// Numeric: arithmetic, non-pointer
template<typename T>
concept Numeric = std::is_arithmetic_v<T> && !std::is_pointer_v<T>;

// Addable: supports operator+
template<typename T>
concept Addable = requires(T a, T b) { { a + b } -> std::convertible_to<T>; };

// Container: has begin/end returning iterators
template<typename C>
concept Container = requires(C c) {
    { c.begin() } -> std::input_or_output_iterator;
    { c.end()   } -> std::input_or_output_iterator;
};

// ---- Functions constrained by concepts ----

// Abbreviated template syntax (C++20)
auto add(Numeric auto a, Numeric auto b) { return a + b; }

template<typename T> requires Numeric<T>
T clamp(T val, T lo, T hi) {
    return val < lo ? lo : val > hi ? hi : val;
}

// ---- if constexpr: compile-time branching ----
template<typename T>
std::string describe_type() {
    if constexpr (std::is_integral_v<T>)         return "integral";
    else if constexpr (std::is_floating_point_v<T>) return "float";
    else if constexpr (std::is_pointer_v<T>)     return "pointer";
    else                                          return "other";
}

// ---- consteval: must be evaluated at compile time ----
consteval int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

static_assert(factorial(5) == 120);  // verified at compile time

} // namespace foundation
