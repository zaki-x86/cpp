#pragma once
#include <cmath>
#include <numbers>
#include <string>
#include <variant>

namespace foundation {

// Overloaded helper: combines multiple lambdas into one visitor callable.
// Idiomatic C++17 pattern for std::visit.
template<typename... Fs>
struct overloaded : Fs... { using Fs::operator()...; };

template<typename... Fs> overloaded(Fs...) -> overloaded<Fs...>;

// Variant shape types for demo
struct CircleShape    { double radius; };
struct RectangleShape { double width, height; };
struct TriangleShape  { double base, height; };

using Shape = std::variant<CircleShape, RectangleShape, TriangleShape>;

inline double shape_area(const Shape& s) {
    return std::visit(overloaded{
        [](const CircleShape&    c) { return std::numbers::pi * c.radius * c.radius; },
        [](const RectangleShape& r) { return r.width * r.height; },
        [](const TriangleShape&  t) { return 0.5 * t.base * t.height; }
    }, s);
}

inline std::string shape_name(const Shape& s) {
    return std::visit(overloaded{
        [](const CircleShape&)    { return std::string("Circle"); },
        [](const RectangleShape&) { return std::string("Rectangle"); },
        [](const TriangleShape&)  { return std::string("Triangle"); }
    }, s);
}

} // namespace foundation
