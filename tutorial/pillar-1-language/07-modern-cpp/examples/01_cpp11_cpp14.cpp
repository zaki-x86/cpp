// Chapter 07 — Example 1: C++11 and C++14 Core Features
// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t20
//   tutorial/pillar-1-language/07-modern-cpp/examples/01_cpp11_cpp14.cpp && /tmp/t20

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// ============================================================
// === Demo: auto type deduction ===
// ============================================================
static void demo_auto() {
    printf("=== Demo: auto type deduction ===\n");

    auto i   = 42;          // int
    auto d   = 3.14;        // double
    auto s   = std::string{"hello"};  // std::string
    auto v   = std::vector<int>{1, 2, 3};  // std::vector<int>

    static_assert(std::is_same_v<decltype(i), int>,         "i must be int");
    static_assert(std::is_same_v<decltype(d), double>,      "d must be double");
    static_assert(std::is_same_v<decltype(s), std::string>, "s must be string");
    static_assert(std::is_same_v<decltype(v), std::vector<int>>, "v must be vector<int>");

    printf("  i=%d, d=%.2f, s=%s, v.size=%zu\n",
           i, d, s.c_str(), v.size());
    printf("  All static_assert checks passed.\n\n");
}

// ============================================================
// === Demo: range-for over vector ===
// ============================================================
static void demo_range_for() {
    printf("=== Demo: range-for over std::vector<int> ===\n");

    std::vector<int> v{10, 20, 30, 40, 50};

    printf("  values:");
    for (const auto& x : v) printf(" %d", x);
    printf("\n");

    // Modify via reference
    for (auto& x : v) x *= 2;
    printf("  doubled:");
    for (const auto& x : v) printf(" %d", x);
    printf("\n\n");
}

// ============================================================
// === Demo: lambda capture ===
// ============================================================
static void demo_lambdas() {
    printf("=== Demo: lambda capture ===\n");

    int x = 10;

    // Capture by value: x is copied at lambda creation
    auto by_val = [x](int y) { return x + y; };
    x = 99;  // change x AFTER capture
    printf("  by_val(5) = %d  (expected 15 — captured x=10 before change)\n",
           by_val(5));

    // Capture by reference: refers to x at time of call
    auto by_ref = [&x](int y) { return x + y; };
    printf("  by_ref(5) = %d  (expected 104 — x is now 99)\n",
           by_ref(5));

    // Capture by move (C++14 init-capture)
    std::vector<int> big{1, 2, 3, 4, 5};
    auto by_move = [captured = std::move(big)]() { return captured.size(); };
    printf("  by_move() size = %zu  (big.size after move = %zu)\n",
           by_move(), big.size());

    // Mutable lambda: modify captured-by-value copy
    auto counter = [n = 0]() mutable -> int { return ++n; };
    // Use separate calls to avoid undefined evaluation order in printf args
    int c1 = counter(), c2 = counter(), c3 = counter();
    printf("  counter: %d %d %d\n\n", c1, c2, c3);
}

// ============================================================
// === Demo: rvalue references and move constructor ===
// ============================================================
struct Widget {
    std::string name;
    int*        data;
    std::size_t size;

    explicit Widget(std::string n, std::size_t sz)
        : name(std::move(n)), data(new int[sz]), size(sz) {
        for (std::size_t i = 0; i < sz; ++i) data[i] = static_cast<int>(i);
    }

    // Move constructor — steal resources
    Widget(Widget&& other) noexcept
        : name(std::move(other.name)),
          data(other.data),
          size(other.size) {
        other.data = nullptr;
        other.size = 0;
    }

    // Move assignment
    Widget& operator=(Widget&& other) noexcept {
        if (this != &other) {
            delete[] data;
            name = std::move(other.name);
            data = other.data;  size = other.size;
            other.data = nullptr; other.size = 0;
        }
        return *this;
    }

    ~Widget() { delete[] data; }

    // No copy (to make the demo clear)
    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
};

static void demo_move() {
    printf("=== Demo: rvalue references and move constructor ===\n");

    Widget w1{"alpha", 4};
    printf("  w1 before move: name=%s, data[2]=%d, size=%zu\n",
           w1.name.c_str(), w1.data[2], w1.size);

    Widget w2 = std::move(w1);  // move constructor called
    printf("  w2 after move:  name=%s, data[2]=%d, size=%zu\n",
           w2.name.c_str(), w2.data[2], w2.size);
    printf("  w1 after move:  name=%s, data=%s, size=%zu\n\n",
           w1.name.c_str(), w1.data ? "non-null" : "nullptr", w1.size);
}

// ============================================================
// === Demo: nullptr ===
// ============================================================
static void demo_nullptr() {
    printf("=== Demo: nullptr ===\n");

    int* p = nullptr;
    printf("  sizeof(nullptr) = %zu\n", sizeof(nullptr));
    printf("  p == nullptr: %s\n", (p == nullptr) ? "true" : "false");
    printf("  p == 0: %s  (nullptr compares equal to 0 numerically)\n\n",
           (p == 0) ? "true" : "false");
}

// ============================================================
// === Demo: constexpr function ===
// ============================================================
constexpr int factorial(int n) {
    // C++14 relaxed constexpr — loops allowed
    int result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

static void demo_constexpr() {
    printf("=== Demo: constexpr function ===\n");

    constexpr int f5 = factorial(5);  // evaluated at compile time
    static_assert(f5 == 120, "factorial(5) must be 120");

    printf("  factorial(5)  = %d  (compile-time)\n", f5);
    printf("  factorial(10) = %d  (runtime call)\n\n", factorial(10));
}

// ============================================================
// === Demo: initializer list ===
// ============================================================
static void demo_initializer_list() {
    printf("=== Demo: initializer list ===\n");

    std::vector<int> v{1, 2, 3, 4, 5};  // uses initializer_list<int> constructor
    printf("  vector:");
    for (int x : v) printf(" %d", x);
    printf("\n");

    // Brace-init prevents narrowing conversions
    // int bad{3.14};  // error: narrowing
    int fine{42};
    printf("  fine = %d\n\n", fine);
}

// ============================================================
// === Demo: variadic template (recursive, C++11 style) ===
// ============================================================
template<typename T>
T sum_args(T t) { return t; }

template<typename T, typename... Rest>
T sum_args(T t, Rest... rest) {
    return t + static_cast<T>(sum_args(rest...));
}

static void demo_variadic() {
    printf("=== Demo: variadic template (recursive) ===\n");
    printf("  sum(1,2,3,4,5)    = %d\n",    sum_args(1, 2, 3, 4, 5));
    printf("  sum(1.5,2.5,3.0)  = %.1f\n\n", sum_args(1.5, 2.5, 3.0));
}

// ============================================================
// === Demo: make_unique (C++14) ===
// ============================================================
static void demo_make_unique() {
    printf("=== Demo: make_unique (C++14) ===\n");

    auto p = std::make_unique<Widget>("beta", 3);
    printf("  unique_ptr<Widget>: name=%s, size=%zu\n",
           p->name.c_str(), p->size);
    // p goes out of scope here — Widget::~Widget() called automatically
    printf("  (unique_ptr automatically deleted Widget on scope exit)\n\n");
}

// ============================================================
// === Demo: generic lambda (C++14) ===
// ============================================================
static void demo_generic_lambda() {
    printf("=== Demo: generic lambda (C++14) ===\n");

    // auto parameters make the lambda a template
    auto add = [](auto a, auto b) { return a + b; };

    printf("  add(1, 2)          = %d\n",     add(1, 2));
    printf("  add(1.5, 2.5)      = %.1f\n",   add(1.5, 2.5));
    printf("  add(string+string) = %s\n\n",
           add(std::string{"hello "}, std::string{"world"}).c_str());

    // Generic sort comparator
    std::vector<int> v{5, 3, 1, 4, 2};
    std::sort(v.begin(), v.end(), [](auto a, auto b){ return a < b; });
    printf("  sorted:");
    for (int x : v) printf(" %d", x);
    printf("\n\n");
}

// ============================================================
int main() {
    printf("=== Chapter 07: C++11 and C++14 Core Features ===\n\n");

    demo_auto();
    demo_range_for();
    demo_lambdas();
    demo_move();
    demo_nullptr();
    demo_constexpr();
    demo_initializer_list();
    demo_variadic();
    demo_make_unique();
    demo_generic_lambda();

    printf("All demos complete.\n");
    return 0;
}
