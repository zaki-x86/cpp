// Chapter 07 — Example 3: C++20 Features
// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t22
//   tutorial/pillar-1-language/07-modern-cpp/examples/03_cpp20.cpp && /tmp/t22
//
// std::format not available on GCC 11.4 (requires GCC 13+). Using printf/cout instead.

#include <algorithm>
#include <array>
#include <chrono>
#include <concepts>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <ostream>
#include <ranges>
#include <span>
#include <string>
#include <thread>
#include <vector>

// ============================================================
// === Demo: Concepts ===
// ============================================================

// Concept: requires that std::ranges::sort can be called on T&
template<typename T>
concept Sortable = requires(T& c) {
    std::ranges::sort(c);
};

// Abbreviated function template using the concept
void sort_and_print(Sortable auto& c) {
    std::ranges::sort(c);
    printf("  sorted:");
    for (const auto& x : c) printf(" %d", x);
    printf("\n");
}

// Concept: requires operator<< to std::ostream
template<typename T>
concept Printable = requires(std::ostream& os, const T& v) {
    { os << v } -> std::same_as<std::ostream&>;
};

// Abbreviated function template — one liner
void print_value(const Printable auto& val) {
    std::cout << "  printable: " << val << "\n";
}

static void demo_concepts() {
    printf("=== Demo: Concepts ===\n");

    std::vector<int> v{5, 3, 1, 4, 2};
    sort_and_print(v);   // vector<int> satisfies Sortable

    std::array<int,5> a{9, 7, 6, 8, 10};
    sort_and_print(a);   // std::array<int,5> satisfies Sortable

    // std::list<int> does NOT satisfy Sortable — ranges::sort needs random access
    // Uncommenting below would give a CLEAR concept-violation error (not a template traceback):
    // std::list<int> lst{1,2,3};
    // sort_and_print(lst);  // concept constraint not satisfied

    print_value(42);
    print_value(std::string{"hello concepts"});
    printf("\n");
}

// ============================================================
// === Demo: std::span<int> ===
// ============================================================
static void print_span(std::span<int> s) {
    printf("  span[%zu]:", s.size());
    for (int x : s) printf(" %d", x);
    printf("\n");
}

static void double_span(std::span<int> s) {
    for (int& x : s) x *= 2;
}

static void demo_span() {
    printf("=== Demo: std::span<int> ===\n");

    int raw[]{1, 2, 3, 4, 5};
    std::vector<int> vec{10, 20, 30};
    std::array<int, 4> arr{100, 200, 300, 400};

    print_span(raw);   // raw array -> span
    print_span(vec);   // vector -> span
    print_span(arr);   // std::array -> span

    // Sub-span — first 3 elements of raw
    print_span(std::span<int>{raw, 3});

    // Mutating through span
    double_span(vec);
    printf("  vec doubled:");
    for (int x : vec) printf(" %d", x);
    printf("\n\n");
}

// ============================================================
// === Demo: std::jthread with std::stop_token ===
// ============================================================
static void demo_jthread() {
    printf("=== Demo: std::jthread with stop_token ===\n");

    int tick_count = 0;

    // jthread passes stop_token automatically as first parameter
    std::jthread worker([&tick_count](std::stop_token st) {
        while (!st.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            ++tick_count;
        }
    });

    // Main thread sleeps 350ms — worker should tick about 3 times
    std::this_thread::sleep_for(std::chrono::milliseconds(350));

    // jthread destructor: requests stop, then joins — automatic RAII
    // (worker goes out of scope at end of this function)
    printf("  tick_count after 350ms: %d (expected ~3)\n", tick_count);
    // jthread will join here when worker goes out of scope
    printf("  jthread auto-joined on scope exit\n\n");
}

// ============================================================
// === Demo: Ranges pipeline ===
// ============================================================
static void demo_ranges() {
    printf("=== Demo: Ranges pipeline (filter | transform | take) ===\n");

    // Build range 1..20 — avoid iota_view which has GCC 11 issues with some filters
    std::vector<int> nums;
    nums.reserve(20);
    for (int i = 1; i <= 20; ++i) nums.push_back(i);

    // Pipeline: even numbers -> squared -> first 5
    auto even   = [](int x){ return x % 2 == 0; };
    auto square = [](int x){ return x * x; };

    auto pipeline = nums
        | std::views::filter(even)
        | std::views::transform(square)
        | std::views::take(5);

    printf("  even squares (first 5): ");
    for (int x : pipeline) printf("%d ", x);
    printf("\n");

    // Demonstrate laziness: take(5) stops early
    // Source has 20 elements; filter produces {2,4,6,8,10,...}; we only need 5
    // → only processes source elements up to 10

    // Another pipeline: reverse + take
    auto last3 = nums | std::views::reverse | std::views::take(3);
    printf("  last 3 reversed: ");
    for (int x : last3) printf("%d ", x);
    printf("\n\n");
}

// ============================================================
// === Demo: Three-way comparison (operator<=>) ===
// ============================================================
struct Point {
    int x, y;
    // Generates ==, !=, <, >, <=, >= from this single declaration
    auto operator<=>(const Point&) const = default;
};

static void demo_spaceship() {
    printf("=== Demo: Three-way comparison (<=>) ===\n");

    std::vector<Point> pts{{3,1},{1,4},{1,2},{2,5}};

    // ranges::sort uses <=> via the defaulted operator
    std::ranges::sort(pts);

    printf("  sorted points (x then y):\n");
    for (const auto& p : pts) {
        printf("    (%d,%d)\n", p.x, p.y);
    }

    Point a{1,2}, b{1,4}, c{1,2};
    printf("  a < b: %s\n",  (a < b)  ? "true" : "false");
    printf("  a == c: %s\n", (a == c) ? "true" : "false");
    printf("  b > a: %s\n\n",(b > a)  ? "true" : "false");
}

// ============================================================
// === Demo: consteval ===
// ============================================================
consteval int ce_square(int n) { return n * n; }

// constexpr: MAY run at compile time or runtime
constexpr int cx_square(int n) { return n * n; }

static void demo_consteval() {
    printf("=== Demo: consteval vs constexpr ===\n");

    // consteval: MUST be a constant expression at call site
    constexpr int a = ce_square(7);   // compile-time: OK
    static_assert(a == 49, "ce_square(7) must be 49");

    // constexpr: may run at runtime
    int n = 8;
    int b = cx_square(n);   // runtime call — OK for constexpr, error for consteval
    // ce_square(n);        // ERROR if uncommented: n is not constexpr

    printf("  ce_square(7)  = %d  (compile-time only)\n", a);
    printf("  cx_square(%d) = %d  (runtime)\n", n, b);
    printf("  Both satisfy static_assert. consteval guarantees compile-time.\n\n");
}

// ============================================================
int main() {
    printf("=== Chapter 07: C++20 Features ===\n\n");

    demo_concepts();
    demo_span();
    demo_jthread();
    demo_ranges();
    demo_spaceship();
    demo_consteval();

    printf("All demos complete.\n");
    return 0;
}
