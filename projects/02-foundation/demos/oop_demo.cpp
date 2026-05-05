#include <foundation/oop/rules.hpp>
#include <foundation/oop/crtp.hpp>
#include <foundation/oop/virtual_design.hpp>
#include <iostream>
#include <vector>
#include <cstddef>

int main() {
    std::cout << "=== OOP Demo ===\n\n";

    // --- Rule of 5 ---
    std::cout << "-- Rule of 5 (Buffer) --\n";
    {
        foundation::Buffer a(8);
        a.data()[0] = std::byte{42};

        foundation::Buffer b = a;          // copy
        b.data()[0] = std::byte{99};
        std::cout << "  after copy, a[0]=" << std::to_integer<int>(a.data()[0])
                  << "  b[0]=" << std::to_integer<int>(b.data()[0]) << "\n";

        foundation::Buffer c = std::move(a);
        std::cout << "  after move, a.size()=" << a.size()
                  << "  c.size()=" << c.size() << "\n";
    }

    // --- CRTP static polymorphism ---
    std::cout << "\n-- CRTP (static polymorphism) --\n";
    {
        foundation::Circle circle(5.0);
        foundation::Square square(4.0);
        std::cout << "  Circle area=" << circle.area()
                  << "  perimeter=" << circle.perimeter() << "\n";
        std::cout << "  Square area=" << square.area()
                  << "  perimeter=" << square.perimeter() << "\n";
    }

    // --- CRTP mixins ---
    std::cout << "\n-- CRTP Mixins (Loggable + Sortable) --\n";
    {
        foundation::LoggableSorter ls;
        ls.log("preparing");
        ls.sort_data();  // internally calls log("before sort") + log("after sort")
        std::cout << "  log entries: " << ls.log_count() << "\n";
        for (const auto& msg : ls.logs())
            std::cout << "    [log] " << msg << "\n";
    }

    // --- NVI ---
    std::cout << "\n-- NVI (Non-Virtual Interface) --\n";
    {
        foundation::LoggingReader reader;
        for (int i = 0; i < 3; ++i) reader.read();
        std::cout << "  pre_count="  << reader.pre_count()
                  << "  impl_count=" << reader.impl_count()
                  << "  post_count=" << reader.post_count() << "\n";
    }

    // --- Virtual inheritance (diamond) ---
    std::cout << "\n-- Virtual Inheritance (diamond) --\n";
    {
        foundation::DiamondDerived d;
        d.set_value(42);
        std::cout << "  DiamondDerived::get_value() = " << d.get_value() << "\n";
    }

    std::cout << "\nDone.\n";
}
