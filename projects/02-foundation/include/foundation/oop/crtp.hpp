#pragma once
#include <numbers>
#include <cmath>
#include <string>
#include <vector>

namespace foundation {

// ---- CRTP: Static Polymorphism ----
// area() and perimeter() resolved at compile time — no vtable.
template<typename Derived>
class Shape {
public:
    double area()      const { return static_cast<const Derived*>(this)->area_impl(); }
    double perimeter() const { return static_cast<const Derived*>(this)->perimeter_impl(); }
};

class Circle : public Shape<Circle> {
    double r_;
public:
    explicit Circle(double r) : r_{r} {}
    double area_impl()      const { return std::numbers::pi * r_ * r_; }
    double perimeter_impl() const { return 2.0 * std::numbers::pi * r_; }
};

class Square : public Shape<Square> {
    double s_;
public:
    explicit Square(double s) : s_{s} {}
    double area_impl()      const { return s_ * s_; }
    double perimeter_impl() const { return 4.0 * s_; }
};

// ---- CRTP: Mixin Composition ----
// Compose independent behaviors without virtual overhead.

template<typename Derived>
class Loggable {
    mutable std::vector<std::string> log_;
public:
    void log(std::string msg) const { log_.push_back(std::move(msg)); }
    int  log_count()          const { return static_cast<int>(log_.size()); }
    const std::vector<std::string>& logs() const { return log_; }
};

template<typename Derived>
class Sortable {
public:
    void sort_data() {
        auto* self = static_cast<Derived*>(this);
        self->log("before sort");
        // actual sort work omitted for demo clarity
        self->log("after sort");
    }
};

// Multiple CRTP mixins — no virtual calls anywhere
class LoggableSorter
    : public Loggable<LoggableSorter>
    , public Sortable<LoggableSorter>
{};

} // namespace foundation
