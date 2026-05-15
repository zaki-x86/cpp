#include <foundation/types/strong_type.hpp>
#include <foundation/types/variant_patterns.hpp>
#include <foundation/types/expected.hpp>
#include <iostream>

int main() {
    std::cout << "=== Types Demo ===\n\n";

    // --- StrongType ---
    std::cout << "-- StrongType / Phantom Types --\n";
    {
        using namespace foundation::literals;
        auto dist = 5.0_km;
        auto time = 3.0_s;
        std::cout << "  distance: " << dist.value() << " km\n";
        std::cout << "  time:     " << time.value() << " s\n";

        // Ordering via spaceship
        auto d1 = 2.0_km, d2 = 8.0_km;
        std::cout << "  2km < 8km: " << (d1 < d2) << "\n";
        std::cout << "  2km == 2km: " << (d1 == d1) << "\n";
    }

    // --- std::variant + overloaded ---
    std::cout << "\n-- std::variant + overloaded visitor --\n";
    {
        std::vector<foundation::Shape> shapes = {
            foundation::CircleShape{5.0},
            foundation::RectangleShape{3.0, 4.0},
            foundation::TriangleShape{6.0, 4.0},
        };
        for (const auto& s : shapes) {
            std::cout << "  " << foundation::shape_name(s)
                      << " area = " << foundation::shape_area(s) << "\n";
        }
    }

    // --- Expected<T,E> ---
    std::cout << "\n-- Expected<T,E> (monadic error handling) --\n";
    {
        auto show = [](const char* expr, auto result) {
            if (result.has_value())
                std::cout << "  " << expr << " = " << result.value() << "\n";
            else
                std::cout << "  " << expr << " => ERROR\n";
        };

        show("10 / 2",  foundation::safe_divide(10.0, 2.0));
        show("1 / 0",   foundation::safe_divide(1.0, 0.0));
        show("sqrt(9)", foundation::safe_sqrt(9.0));
        show("sqrt(-1)",foundation::safe_sqrt(-1.0));

        // Monadic chain: parse "25.0" -> sqrt -> result
        auto r = foundation::parse_and_sqrt("25.0");
        show("parse_and_sqrt('25.0')", r);

        auto r2 = foundation::parse_and_sqrt("bad");
        show("parse_and_sqrt('bad')", r2);

        // value_or fallback
        auto v = foundation::safe_divide(1.0, 0.0).value_or(-999.0);
        std::cout << "  value_or(-999) on error = " << v << "\n";

        // or_else
        auto recovered = foundation::safe_divide(1.0, 0.0)
            .or_else([](foundation::MathError){ return foundation::Expected<double, foundation::MathError>{0.0}; });
        std::cout << "  or_else recovery = " << recovered.value() << "\n";
    }

    std::cout << "\nDone.\n";
}
