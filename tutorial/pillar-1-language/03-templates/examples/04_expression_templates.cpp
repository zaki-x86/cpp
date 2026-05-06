// Compile: g++ -std=c++20 -Wall -Wextra -o /tmp/t10 04_expression_templates.cpp
// Demonstrates: expression templates — zero-allocation chained vector arithmetic

#include <array>
#include <cstddef>
#include <cstdio>

int g_alloc_count = 0;  // counts Vec<N> constructions (each allocates logically)

// ─── Vec<N>: fixed-size float vector ─────────────────────────────────────────
template<std::size_t N>
struct Vec {
    std::array<float, N> data{};

    Vec() { ++g_alloc_count; }

    explicit Vec(std::initializer_list<float> il) {
        ++g_alloc_count;
        std::size_t i = 0;
        for (float v : il) { if (i < N) data[i++] = v; }
    }

    float  operator[](std::size_t i) const { return data[i]; }
    float& operator[](std::size_t i)       { return data[i]; }
    static constexpr std::size_t size() { return N; }

    // Assign from any expression template (lazy evaluation)
    template<typename Expr>
    Vec& operator=(const Expr& expr) {
        for (std::size_t i = 0; i < N; ++i)
            data[i] = expr[i];
        return *this;
    }
};

// ─── VecAdd<L,R>: lazy addition proxy ────────────────────────────────────────
template<typename L, typename R>
struct VecAdd {
    const L& l;
    const R& r;
    VecAdd(const L& l, const R& r) : l(l), r(r) {}
    float operator[](std::size_t i) const { return l[i] + r[i]; }
    static constexpr std::size_t size() { return L::size(); }
};

// operator+ returns the proxy, NOT a Vec (no allocation)
template<std::size_t N>
VecAdd<Vec<N>, Vec<N>> operator+(const Vec<N>& l, const Vec<N>& r) {
    return {l, r};
}

template<typename L, typename R, std::size_t N>
VecAdd<VecAdd<L,R>, Vec<N>> operator+(const VecAdd<L,R>& l, const Vec<N>& r) {
    return {l, r};
}

int main() {
    g_alloc_count = 0;

    Vec<4> b{1.f, 2.f, 3.f, 4.f};
    Vec<4> c{10.f, 20.f, 30.f, 40.f};
    Vec<4> d{100.f, 200.f, 300.f, 400.f};
    Vec<4> a;  // destination

    int allocs_before = g_alloc_count;
    a = b + c + d;  // expression template: no intermediate Vec allocated
    int allocs_during = g_alloc_count - allocs_before;

    printf("b = [1, 2, 3, 4]\n");
    printf("c = [10, 20, 30, 40]\n");
    printf("d = [100, 200, 300, 400]\n");
    printf("a = b + c + d = [%.0f, %.0f, %.0f, %.0f]\n",
           a[0], a[1], a[2], a[3]);
    printf("\nVec constructions for a=b+c+d: %d (expected 0 -- no temporaries)\n",
           allocs_during);
    printf("Total Vec constructions (a,b,c,d): %d\n", allocs_before);

    // Without expression templates, naive += would allocate 2 temporaries:
    //   tmp1 = b + c   (1 alloc)
    //   tmp2 = tmp1 + d (1 alloc)
    //   a = tmp2        (copy/move)
    // Expression templates: 0 temporaries, 1 loop pass

    return 0;
}
