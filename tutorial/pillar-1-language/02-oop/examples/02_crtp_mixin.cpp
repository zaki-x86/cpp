// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t05 02_crtp_mixin.cpp
// Demonstrates: CRTP mixins — zero vtable, compile-time polymorphism

#include <cstdio>
#include <string>

// Mixin 1: adds print() to any Derived that provides to_string()
template<typename Derived>
struct Printable {
    void print() const {
        printf("%s\n", static_cast<const Derived*>(this)->to_string().c_str());
    }
};

// Mixin 2: adds all comparison operators to any Derived that provides value()
template<typename Derived>
struct Comparable {
    bool operator< (const Derived& o) const { return val() <  o.val(); }
    bool operator> (const Derived& o) const { return val() >  o.val(); }
    bool operator==(const Derived& o) const { return val() == o.val(); }
    bool operator!=(const Derived& o) const { return val() != o.val(); }
private:
    float val() const { return static_cast<const Derived*>(this)->value(); }
};

// Concrete type using both mixins — no virtual functions anywhere
struct Point : Printable<Point>, Comparable<Point> {
    float x, y;
    Point(float x, float y) : x(x), y(y) {}

    std::string to_string() const {
        char buf[64];
        snprintf(buf, sizeof(buf), "Point(%.1f, %.1f)", x, y);
        return buf;
    }

    float value() const { return x * x + y * y; }  // squared distance from origin
};

// Template function — works with any T that has .print()
// No virtual, no pointer, no vtable
template<typename T>
void describe(const T& p) {
    p.print();
}

int main() {
    Point p1{1.0f, 2.0f};
    Point p2{3.0f, 0.0f};

    printf("--- CRTP mixin demo ---\n");
    printf("p1: "); describe(p1);
    printf("p2: "); describe(p2);

    printf("\np1.value() = %.1f (squared dist)\n", p1.value());
    printf("p2.value() = %.1f (squared dist)\n", p2.value());

    printf("\nComparisons:\n");
    printf("p1 < p2:  %s (%.1f < %.1f)\n", (p1 < p2) ? "true" : "false", p1.value(), p2.value());
    printf("p1 > p2:  %s\n", (p1 > p2) ? "true" : "false");
    printf("p1 == p2: %s\n", (p1 == p2) ? "true" : "false");

    // Confirm: no virtual functions (Point has no vptr)
    // sizeof(Point) == 2 * sizeof(float) == 8, not 8+8 (no vptr)
    printf("\nsizeof(Point) = %zu (no vptr overhead — just 2 floats)\n", sizeof(Point));
    static_assert(sizeof(Point) == 2 * sizeof(float),
                  "Point must have no vtable pointer");

    return 0;
}
