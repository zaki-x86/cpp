#include <gtest/gtest.h>
#include <foundation/templates/type_traits.hpp>
#include <foundation/templates/variadic.hpp>
#include <foundation/templates/concepts.hpp>
#include <foundation/templates/policy.hpp>
#include <vector>
#include <list>

// --- Type traits / TypeList ---

TEST(TypeTraits, TypeListSize) {
    using L = foundation::TypeList<int, double, float>;
    EXPECT_EQ(L::size, 3u);
}

TEST(TypeTraits, TypeAtFirst) {
    using L = foundation::TypeList<int, double, float>;
    static_assert(std::is_same_v<foundation::type_at_t<L, 0>, int>);
}

TEST(TypeTraits, TypeAtLast) {
    using L = foundation::TypeList<int, double, float>;
    static_assert(std::is_same_v<foundation::type_at_t<L, 2>, float>);
}

TEST(TypeTraits, TypeListContains) {
    using L = foundation::TypeList<int, double, float>;
    static_assert(foundation::type_list_contains<L, double>::value);
    static_assert(!foundation::type_list_contains<L, char>::value);
}

TEST(TypeTraits, HasSizeTrueForVector) {
    static_assert(foundation::has_size<std::vector<int>>::value);
}

TEST(TypeTraits, HasSizeFalseForInt) {
    static_assert(!foundation::has_size<int>::value);
}

TEST(TypeTraits, RemoveCvref) {
    static_assert(std::is_same_v<foundation::remove_cvref_t<const int&>, int>);
    static_assert(std::is_same_v<foundation::remove_cvref_t<volatile double&&>, double>);
}

TEST(TypeTraits, PowCt) {
    static_assert(foundation::pow_ct<2, 10>::value == 1024);
    static_assert(foundation::pow_ct<3, 3>::value == 27);
}

// --- Variadic / fold expressions ---

TEST(Variadic, SumFold) {
    EXPECT_EQ(foundation::sum(1, 2, 3, 4), 10);
    EXPECT_DOUBLE_EQ(foundation::sum(1.5, 2.5), 4.0);
}

TEST(Variadic, ProductFold) {
    EXPECT_EQ(foundation::product(2, 3, 4), 24);
}

TEST(Variadic, AllOfAndAnyOf) {
    EXPECT_TRUE(foundation::all_of(true, true, true));
    EXPECT_FALSE(foundation::all_of(true, false, true));
    EXPECT_TRUE(foundation::any_of(false, false, true));
    EXPECT_FALSE(foundation::any_of(false, false, false));
}

TEST(Variadic, ForEachArgCallsOnEach) {
    int count = 0;
    foundation::for_each_arg([&](auto){ ++count; }, 1, 2.0, 'x', "str");
    EXPECT_EQ(count, 4);
}

// --- Concepts ---

TEST(Concepts, AddInts) {
    EXPECT_EQ(foundation::add(3, 4), 7);
}

TEST(Concepts, ClampValue) {
    EXPECT_EQ(foundation::clamp(15, 0, 10), 10);
    EXPECT_EQ(foundation::clamp(-5, 0, 10), 0);
    EXPECT_EQ(foundation::clamp(5, 0, 10), 5);
}

TEST(Concepts, ConstevalFactorial) {
    static_assert(foundation::factorial(5) == 120);
    static_assert(foundation::factorial(0) == 1);
}

// --- Policy-based sort ---

TEST(PolicySort, StdSortAscending) {
    foundation::Sorter<foundation::StdSortPolicy> s;
    std::vector<int> v{3, 1, 4, 1, 5};
    s.sort(v.begin(), v.end());
    EXPECT_TRUE(std::is_sorted(v.begin(), v.end()));
}

TEST(PolicySort, ReverseSortDescending) {
    foundation::Sorter<foundation::ReverseSortPolicy> s;
    std::vector<int> v{3, 1, 4, 1, 5};
    s.sort(v.begin(), v.end());
    EXPECT_TRUE(std::is_sorted(v.begin(), v.end(), std::greater<int>{}));
}

// --- Expression templates ---

TEST(ExprTemplates, Vec3AddNoTemporary) {
    foundation::Vec3 a{1, 2, 3};
    foundation::Vec3 b{4, 5, 6};
    auto expr = a + b;
    foundation::Vec3 c = foundation::eval(expr);
    EXPECT_DOUBLE_EQ(c.x, 5.0);
    EXPECT_DOUBLE_EQ(c.y, 7.0);
    EXPECT_DOUBLE_EQ(c.z, 9.0);
}
