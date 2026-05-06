// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t08 02_sfinae_traits.cpp
// Demonstrates: SFINAE detection with void_t, C++20 concepts, constrained function templates

#include <cstdio>
#include <string>
#include <vector>
#include <type_traits>

// ─── SFINAE detection: has_size<T> ───────────────────────────────────────────
template<typename T, typename = void>
struct has_size : std::false_type {};

template<typename T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>>
    : std::true_type {};

template<typename T> inline constexpr bool has_size_v = has_size<T>::value;

// ─── SFINAE: print_size — overloaded via enable_if ───────────────────────────
template<typename T>
std::enable_if_t<has_size_v<T>> print_size(const T& x) {
    printf("size: %zu\n", x.size());
}

template<typename T>
std::enable_if_t<!has_size_v<T>> print_size(const T&) {
    printf("no size\n");
}

// ─── C++20: same thing with a concept ────────────────────────────────────────
template<typename T>
concept HasSize = requires(const T& x) {
    { x.size() } -> std::convertible_to<std::size_t>;
};

void print_size_v2(const HasSize auto& x) {
    printf("(concept) size: %zu\n", x.size());
}

void print_size_v2(const auto&) {
    printf("(concept) no size\n");
}

int main() {
    printf("--- SFINAE via enable_if ---\n");
    print_size(std::vector<int>{1, 2, 3});  // has .size()
    print_size(std::string("hello"));        // has .size()
    print_size(42);                          // no .size()
    print_size(3.14);                        // no .size()

    printf("\n--- Same via C++20 concept ---\n");
    print_size_v2(std::vector<int>{1, 2, 3});
    print_size_v2(std::string("hello"));
    print_size_v2(42);

    printf("\nhas_size<vector<int>> = %s\n", has_size_v<std::vector<int>> ? "true" : "false");
    printf("has_size<int>         = %s\n", has_size_v<int> ? "true" : "false");
    return 0;
}
