// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t12 02_variant_visitor.cpp
// Demonstrates: std::variant with exhaustive overloaded visitor

#include <cstdio>
#include <cmath>
#include <variant>
#include <vector>

struct Circle    { double radius; };
struct Rectangle { double width, height; };
struct Triangle  { double base, height; };

using Shape = std::variant<Circle, Rectangle, Triangle>;

// --- overloaded visitor pattern --------------------------------------------------
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// --- Exhaustive dispatch — compile error if any alternative is unhandled ---------
double area(const Shape& s) {
    return std::visit(overloaded{
        [](const Circle& c)    { return M_PI * c.radius * c.radius; },
        [](const Rectangle& r) { return r.width * r.height; },
        [](const Triangle& t)  { return 0.5 * t.base * t.height; }
    }, s);
}

int main() {
    std::vector<Shape> shapes = {
        Circle{5.0},
        Rectangle{4.0, 3.0},
        Triangle{6.0, 4.0},
        Circle{1.0},
    };

    printf("%-32s  %8s\n", "Shape", "Area");
    printf("%-32s  %8s\n", "-----", "----");

    for (const auto& s : shapes) {
        char buf[64];
        std::visit(overloaded{
            [&](const Circle& c)    { snprintf(buf, sizeof(buf), "Circle(r=%.1f)", c.radius); },
            [&](const Rectangle& r) { snprintf(buf, sizeof(buf), "Rectangle(%.1fx%.1f)", r.width, r.height); },
            [&](const Triangle& t)  { snprintf(buf, sizeof(buf), "Triangle(b=%.1f,h=%.1f)", t.base, t.height); }
        }, s);
        printf("%-32s  %8.2f\n", buf, area(s));
    }

    // If you add Hexagon to the variant and forget the visitor, you get:
    //   error: no matching function for call to 'overloaded<...>::operator()(Hexagon&)'
    // This is the exhaustiveness guarantee — not achievable with a switch/enum.

    printf("\n--- Variant index (discriminant) ---\n");
    for (const auto& s : shapes) {
        printf("index: %zu\n", s.index());
    }

    return 0;
}
