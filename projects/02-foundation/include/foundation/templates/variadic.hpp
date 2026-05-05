#pragma once
#include <tuple>
#include <utility>
#include <string>

namespace foundation {

// ---- Fold expressions: operate on all arguments at once ----

// sum: add any number of values via left fold
template<typename... Ts>
auto sum(Ts&&... args) {
    return (... + std::forward<Ts>(args));   // C++17 left fold
}

// product
template<typename... Ts>
auto product(Ts&&... args) {
    return (... * std::forward<Ts>(args));
}

// all_of: check predicate on all arguments
template<typename... Bools>
constexpr bool all_of(Bools... bs) {
    return (... && bs);
}

// any_of
template<typename... Bools>
constexpr bool any_of(Bools... bs) {
    return (... || bs);
}

// ---- Variadic function that applies a callable to each argument ----
template<typename F, typename... Ts>
void for_each_arg(F&& f, Ts&&... args) {
    (f(std::forward<Ts>(args)), ...);   // comma fold
}

// ---- Type-indexed get for std::tuple (convenience alias) ----
// Shows how std::get<I> works with tuple — pedagogical pointer.
template<std::size_t I, typename... Ts>
decltype(auto) tuple_get(std::tuple<Ts...>& t) {
    return std::get<I>(t);
}

// ---- Recursive variadic template: pre-C++17 style for comparison ----
// (Shows evolution from recursion to fold expressions)

// Base case
inline void print_all_recursive() {}

// Recursive case
template<typename Head, typename... Tail>
void print_all_recursive(Head&& h, Tail&&... tail) {
    (void)h;  // in demo context — real impl would print
    print_all_recursive(std::forward<Tail>(tail)...);
}

// C++17 equivalent using fold — single line:
template<typename... Ts>
void print_all_fold(Ts&&... args) {
    for_each_arg([](auto&& v){ (void)v; }, std::forward<Ts>(args)...);
}

} // namespace foundation
