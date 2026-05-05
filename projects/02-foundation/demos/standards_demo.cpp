#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <optional>
#include <variant>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <functional>
#include <ranges>
#include <span>
#include <concepts>
#include <numbers>
#include <bit>
#include <compare>
#include <coroutine>

// ---- C++11 ----
namespace cpp11 {
    // Range-based for, auto, lambda, move semantics
    void demo() {
        std::cout << "-- C++11 --\n";

        // Uniform initializer list
        std::vector<int> v{1, 2, 3, 4, 5};

        // Lambda with capture
        int multiplier = 3;
        std::for_each(v.begin(), v.end(), [multiplier](int x){
            std::cout << "  " << x << "*" << multiplier << "=" << x*multiplier << "\n";
        });

        // move semantics
        std::vector<int> a{10, 20, 30};
        std::vector<int> b = std::move(a);
        std::cout << "  after move: a.size()=" << a.size() << " b.size()=" << b.size() << "\n";

        // unique_ptr
        auto p = std::make_unique<int>(42);
        std::cout << "  unique_ptr: " << *p << "\n";

        // nullptr
        int* np = nullptr;
        std::cout << "  nullptr == (int*)0: " << (np == nullptr) << "\n";

        // constexpr
        constexpr int sq = 7 * 7;
        std::cout << "  constexpr 7^2=" << sq << "\n";

        // static_assert
        static_assert(sizeof(int) == 4, "int must be 4 bytes");
    }
}

// ---- C++14 ----
namespace cpp14 {
    // Generic lambdas, make_unique, variable templates
    template<typename T>
    constexpr T pi = T(3.14159265358979);

    auto add = [](auto a, auto b){ return a + b; };  // generic lambda

    void demo() {
        std::cout << "-- C++14 --\n";
        std::cout << "  generic lambda: 3+4=" << add(3, 4) << " 1.5+2.5=" << add(1.5, 2.5) << "\n";
        std::cout << "  pi<float>=" << pi<float> << " pi<double>=" << pi<double> << "\n";
        // Return type deduction
        auto fn = [](int x) { return x * 2; };
        std::cout << "  auto return deduction: " << fn(5) << "\n";
    }
}

// ---- C++17 ----
namespace cpp17 {
    void demo() {
        std::cout << "-- C++17 --\n";

        // if constexpr
        auto show_type = [](auto v) {
            if constexpr (std::is_integral_v<decltype(v)>)
                std::cout << "  integral: " << v << "\n";
            else
                std::cout << "  floating: " << v << "\n";
        };
        show_type(42);
        show_type(3.14);

        // structured bindings
        std::tuple<int, double, std::string> t{1, 2.5, "hello"};
        auto [i, d, s] = t;
        std::cout << "  structured binding: " << i << " " << d << " " << s << "\n";

        // std::optional
        std::optional<int> opt = 99;
        std::cout << "  optional has_value: " << opt.has_value() << " val=" << *opt << "\n";
        opt.reset();
        std::cout << "  optional after reset: " << opt.value_or(-1) << "\n";

        // std::variant
        std::variant<int, double, std::string> var = "C++17";
        std::cout << "  variant string: " << std::get<std::string>(var) << "\n";
        var = 42;
        std::cout << "  variant int: " << std::get<int>(var) << "\n";

        // string_view (zero-copy)
        std::string_view sv = "hello world";
        std::cout << "  string_view substr: " << sv.substr(6) << "\n";

        // fold expression (in variadic — shown in templates demo)
        // Guaranteed copy elision (NRVO), inline variables (in headers)
    }
}

// ---- C++20 ----
namespace cpp20 {
    // Concepts
    template<std::integral T>
    T double_it(T v) { return v * 2; }

    // Ranges
    void demo() {
        std::cout << "-- C++20 --\n";

        // Concepts
        std::cout << "  double_it(5)=" << double_it(5) << "\n";

        // Ranges pipeline
        auto v = std::vector{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        auto result = v
            | std::views::filter([](int x){ return x % 2 == 0; })
            | std::views::transform([](int x){ return x * x; });
        std::cout << "  even squares: ";
        for (int x : result) std::cout << x << " ";
        std::cout << "\n";

        // std::span (non-owning view)
        std::span<int> sp{v.data(), 5};
        std::cout << "  span[0..4]: ";
        for (int x : sp) std::cout << x << " ";
        std::cout << "\n";

        // std::numbers
        std::cout << "  std::numbers::pi=" << std::numbers::pi << "\n";
        std::cout << "  std::numbers::e=" << std::numbers::e << "\n";

        // std::bit_popcount
        std::cout << "  popcount(255)=" << std::popcount(255u) << "\n";

        // 3-way comparison (spaceship)
        auto cmp = 3 <=> 5;
        std::cout << "  3<=>5 is less: " << (cmp < 0) << "\n";

        // Designated initializers
        struct Point { int x; int y; int z = 0; };
        Point p{.x = 1, .y = 2};
        std::cout << "  designated init: " << p.x << "," << p.y << "," << p.z << "\n";

        // constinit (static storage, compile-time init, runtime mutable)
        static constinit int counter = 0;
        counter += 1;
        std::cout << "  constinit counter: " << counter << "\n";
    }
}

int main() {
    std::cout << "=== C++ Standards Progression Demo ===\n\n";
    cpp11::demo();
    std::cout << "\n";
    cpp14::demo();
    std::cout << "\n";
    cpp17::demo();
    std::cout << "\n";
    cpp20::demo();
    std::cout << "\nDone.\n";
}
