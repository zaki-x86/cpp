// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t07 01_type_list.cpp
// Demonstrates: compile-time TypeList operations — Head, Tail, Size, At, Contains

#include <cstddef>
#include <cstdio>
#include <type_traits>

// ─── TypeList primary template ───────────────────────────────────────────────
template<typename... Ts>
struct TypeList {};

// ─── Head: first element ─────────────────────────────────────────────────────
template<typename L> struct Head;
template<typename T, typename... Ts>
struct Head<TypeList<T, Ts...>> { using type = T; };
template<typename L> using Head_t = typename Head<L>::type;

// ─── Tail: rest after first ──────────────────────────────────────────────────
template<typename L> struct Tail;
template<typename T, typename... Ts>
struct Tail<TypeList<T, Ts...>> { using type = TypeList<Ts...>; };
template<typename L> using Tail_t = typename Tail<L>::type;

// ─── Size: element count ─────────────────────────────────────────────────────
template<typename L> struct Size;
template<typename... Ts>
struct Size<TypeList<Ts...>> { static constexpr std::size_t value = sizeof...(Ts); };
template<typename L>
inline constexpr std::size_t Size_v = Size<L>::value;

// ─── At: N-th element ────────────────────────────────────────────────────────
template<std::size_t N, typename L> struct At;
template<std::size_t N, typename T, typename... Ts>
struct At<N, TypeList<T, Ts...>> : At<N - 1, TypeList<Ts...>> {};
template<typename T, typename... Ts>
struct At<0, TypeList<T, Ts...>> { using type = T; };
template<std::size_t N, typename L> using At_t = typename At<N, L>::type;

// ─── Contains: membership test ───────────────────────────────────────────────
template<typename T, typename L> struct Contains;
template<typename T>
struct Contains<T, TypeList<>> : std::false_type {};
template<typename T, typename Head, typename... Ts>
struct Contains<T, TypeList<Head, Ts...>>
    : std::conditional_t<std::is_same_v<T, Head>,
                         std::true_type,
                         Contains<T, TypeList<Ts...>>> {};
template<typename T, typename L>
inline constexpr bool Contains_v = Contains<T, L>::value;

// ─── Static-assert tests ─────────────────────────────────────────────────────
using List = TypeList<int, double, char>;

static_assert(Size_v<List> == 3,           "Size failed");
static_assert(std::is_same_v<Head_t<List>, int>,    "Head failed");
static_assert(std::is_same_v<At_t<1, List>, double>,"At<1> failed");
static_assert(std::is_same_v<At_t<2, List>, char>,  "At<2> failed");
static_assert(Contains_v<double, List>,    "Contains<double> failed");
static_assert(!Contains_v<float, List>,   "Contains<float> should be false");
static_assert(std::is_same_v<Tail_t<List>, TypeList<double, char>>, "Tail failed");

int main() {
    printf("TypeList<int, double, char>:\n");
    printf("  Size    = %zu  (expected 3)\n",   Size_v<List>);
    printf("  Head    = int  (static_assert passed)\n");
    printf("  At<1>   = double (static_assert passed)\n");
    printf("  Contains<double> = true  (static_assert passed)\n");
    printf("  Contains<float>  = false (static_assert passed)\n");
    printf("All static_assert checks passed at compile time.\n");
    return 0;
}
