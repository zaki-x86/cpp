// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t11 01_strong_type.cpp
// Demonstrates: strong typedef pattern — distinct types from the same underlying type

#include <cstdio>
#include <compare>

// --- CRTP mixins for operator injection -------------------------------------------
template<typename Derived>
struct Addable {
    Derived operator+(const Derived& o) const {
        return Derived{static_cast<const Derived*>(this)->value() + o.value()};
    }
};

template<typename Derived>
struct Scalable {
    Derived operator*(double factor) const {
        return Derived{static_cast<const Derived*>(this)->value() * factor};
    }
};

template<typename Derived>
struct Printable {
    void print(const char* label = "") const {
        printf("%s%.4g\n", label, static_cast<const Derived*>(this)->value());
    }
};

template<typename Derived>
struct Comparable {
    bool operator< (const Derived& o) const { return val() <  o.val(); }
    bool operator==(const Derived& o) const { return val() == o.val(); }
    bool operator> (const Derived& o) const { return val() >  o.val(); }
private:
    double val() const { return static_cast<const Derived*>(this)->value(); }
};

// --- StrongType base --------------------------------------------------------------
template<typename T, typename Tag>
struct StrongTypeBase {
    explicit StrongTypeBase(T v) : value_(v) {}
    T value() const { return value_; }
private:
    T value_;
};

// --- Domain types -----------------------------------------------------------------
struct MetersTag  {};
struct SecondsTag {};

struct Meters
    : StrongTypeBase<double, MetersTag>
    , Addable<Meters>
    , Scalable<Meters>
    , Printable<Meters>
    , Comparable<Meters>
{
    explicit Meters(double v) : StrongTypeBase(v) {}
};

struct Seconds
    : StrongTypeBase<double, SecondsTag>
    , Printable<Seconds>
{
    explicit Seconds(double v) : StrongTypeBase(v) {}
};

// UDLs
Meters  operator""_m (long double v) { return Meters{static_cast<double>(v)}; }
Seconds operator""_s (long double v) { return Seconds{static_cast<double>(v)}; }

// Function requiring explicit types — cannot mix Meters and Seconds
void move_robot(Meters distance, Seconds time) {
    double speed = distance.value() / time.value();
    printf("Moving %.2f m in %.2f s (%.2f m/s)\n",
           distance.value(), time.value(), speed);
}

int main() {
    printf("--- Strong types demo ---\n\n");

    Meters a{5.0};
    Meters b{3.0};
    Meters c = a + b;
    printf("a + b = "); c.print();

    Meters d = a * 2.0;
    printf("a * 2 = "); d.print();

    printf("a < b: %s\n", (a < b) ? "true" : "false");
    printf("a > b: %s\n\n", (a > b) ? "true" : "false");

    // UDL syntax
    auto dist = 10.0_m;
    auto time = 4.0_s;
    move_robot(dist, time);

    // move_robot(time, dist);  // would not compile: cannot convert Seconds to Meters
    // move_robot(5.0, 3.0);    // would not compile: no implicit conversion

    printf("\n--- Compile-time protection (uncomment to see errors) ---\n");
    printf("Passing Seconds where Meters expected: compile error\n");
    printf("Passing raw double: compile error\n");
    printf("sizeof(Meters) = %zu (same as double -- zero overhead)\n", sizeof(Meters));

    return 0;
}
