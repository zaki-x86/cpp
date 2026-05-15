#include <foundation/templates/type_traits.hpp>
#include <foundation/templates/variadic.hpp>
#include <foundation/templates/concepts.hpp>
#include <foundation/templates/policy.hpp>
#include <iostream>
#include <vector>
#include <typeinfo>

int main() {
    std::cout << "=== Templates Demo ===\n\n";

    // --- TypeList ---
    std::cout << "-- TypeList ---\n";
    {
        using L = foundation::TypeList<int, double, float, char>;
        std::cout << "  size: " << L::size << "\n";
        std::cout << "  contains double: " << foundation::type_list_contains<L, double>::value << "\n";
        std::cout << "  contains bool:   " << foundation::type_list_contains<L, bool>::value << "\n";
        constexpr auto pow10 = foundation::pow_ct<2, 10>::value;
        std::cout << "  2^10 at compile time: " << pow10 << "\n";
    }

    // --- Variadic / fold ---
    std::cout << "\n-- Variadic / Fold Expressions --\n";
    {
        std::cout << "  sum(1..5): " << foundation::sum(1, 2, 3, 4, 5) << "\n";
        std::cout << "  product(2,3,4): " << foundation::product(2, 3, 4) << "\n";
        std::cout << "  all_of(T,T,F): " << foundation::all_of(true, true, false) << "\n";
        std::cout << "  any_of(F,F,T): " << foundation::any_of(false, false, true) << "\n";

        std::cout << "  for_each_arg types: ";
        foundation::for_each_arg([](auto v){
            std::cout << typeid(v).name() << " ";
        }, 42, 3.14, 'x');
        std::cout << "\n";
    }

    // --- Concepts ---
    std::cout << "\n-- Concepts (C++20) --\n";
    {
        std::cout << "  add(3, 4) = " << foundation::add(3, 4) << "\n";
        std::cout << "  add(1.5, 2.5) = " << foundation::add(1.5, 2.5) << "\n";
        std::cout << "  clamp(15, 0, 10) = " << foundation::clamp(15, 0, 10) << "\n";
        std::cout << "  describe int: " << foundation::describe_type<int>() << "\n";
        std::cout << "  describe vector<int>: " << foundation::describe_type<std::vector<int>>() << "\n";

        constexpr auto f5 = foundation::factorial(5);
        std::cout << "  consteval 5! = " << f5 << "\n";
    }

    // --- Policy-based sort ---
    std::cout << "\n-- Policy-Based Sort --\n";
    {
        std::vector<int> v{5, 2, 8, 1, 9, 3};

        foundation::Sorter<foundation::StdSortPolicy> asc;
        auto va = v;
        asc.sort(va.begin(), va.end());
        std::cout << "  std sort: ";
        for (int x : va) std::cout << x << " ";
        std::cout << "\n";

        foundation::Sorter<foundation::ReverseSortPolicy> desc;
        auto vd = v;
        desc.sort(vd.begin(), vd.end());
        std::cout << "  reverse:  ";
        for (int x : vd) std::cout << x << " ";
        std::cout << "\n";

        foundation::Sorter<foundation::StableSortPolicy> stbl;
        auto vs = v;
        stbl.sort(vs.begin(), vs.end());
        std::cout << "  stable:   ";
        for (int x : vs) std::cout << x << " ";
        std::cout << "\n";
    }

    // --- Expression templates ---
    std::cout << "\n-- Expression Templates (zero temporaries) --\n";
    {
        foundation::Vec3 a{1.0, 2.0, 3.0};
        foundation::Vec3 b{4.0, 5.0, 6.0};
        auto expr = a + b;  // VecAdd<Vec3,Vec3> — no Vec3 copy yet
        foundation::Vec3 c = foundation::eval(expr);
        std::cout << "  (1,2,3)+(4,5,6) = ("
                  << c.x << "," << c.y << "," << c.z << ")\n";
    }

    std::cout << "\nDone.\n";
}
