#pragma once
#include <algorithm>

namespace foundation {

// ---- Policy-based design (Alexandrescu "Modern C++ Design") ----
// SortPolicy is a template parameter — plugged in at compile time.
// Zero virtual overhead. Swap policies without changing Sorter.

struct StdSortPolicy {
    template<typename It>
    static void apply(It first, It last) { std::sort(first, last); }
};

struct StableSortPolicy {
    template<typename It>
    static void apply(It first, It last) { std::stable_sort(first, last); }
};

struct ReverseSortPolicy {
    template<typename It>
    static void apply(It first, It last) {
        std::sort(first, last, std::greater<>{});
    }
};

template<typename SortPolicy = StdSortPolicy>
class Sorter {
public:
    template<typename It>
    void sort(It first, It last) { SortPolicy::apply(first, last); }
};

// ---- Expression Templates ----
// Avoids intermediate temporaries in arithmetic expressions.
// e.g. a + b + c traverses the data once, not three times.

template<typename E>
struct VecExpr {
    double operator[](std::size_t i) const {
        return static_cast<const E&>(*this)[i];
    }
};

struct Vec3 : VecExpr<Vec3> {
    double x, y, z;
    Vec3(double x_, double y_, double z_) : x{x_}, y{y_}, z{z_} {}
    double operator[](std::size_t i) const {
        return i == 0 ? x : i == 1 ? y : z;
    }
};

template<typename L, typename R>
struct VecAdd : VecExpr<VecAdd<L,R>> {
    const L& lhs; const R& rhs;
    VecAdd(const L& l, const R& r) : lhs{l}, rhs{r} {}
    double operator[](std::size_t i) const { return lhs[i] + rhs[i]; }
};

template<typename L, typename R>
VecAdd<L,R> operator+(const VecExpr<L>& l, const VecExpr<R>& r) {
    return {static_cast<const L&>(l), static_cast<const R&>(r)};
}

// Materialize the lazy expression into a Vec3 — single traversal
template<typename E>
Vec3 eval(const VecExpr<E>& e) { return {e[0], e[1], e[2]}; }

} // namespace foundation
