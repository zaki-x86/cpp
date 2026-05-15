#include <gtest/gtest.h>
#include <foundation/types/strong_type.hpp>
#include <foundation/types/variant_patterns.hpp>
#include <foundation/types/expected.hpp>

// --- StrongType ---

TEST(StrongType, KilometersNotImplicitDouble) {
    using Km = foundation::StrongType<double, struct KmTag>;
    Km k{5.0};
    EXPECT_DOUBLE_EQ(k.value(), 5.0);
}

TEST(StrongType, UDLKilometers) {
    using namespace foundation::literals;
    auto d = 3.0_km;
    EXPECT_DOUBLE_EQ(d.value(), 3.0);
}

TEST(StrongType, UDLSeconds) {
    using namespace foundation::literals;
    auto t = 10.0_s;
    EXPECT_DOUBLE_EQ(t.value(), 10.0);
}

TEST(StrongType, SpaceshipLess) {
    using Km = foundation::StrongType<double, struct KmTag>;
    Km a{1.0}, b{2.0};
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

TEST(StrongType, SpaceshipEqual) {
    using Km = foundation::StrongType<double, struct KmTag>;
    Km a{3.0}, b{3.0};
    EXPECT_EQ(a, b);
}

// --- Variant / overloaded ---

TEST(Variant, CircleArea) {
    foundation::Shape s = foundation::CircleShape{5.0};
    EXPECT_NEAR(foundation::shape_area(s), 78.539, 0.001);
}

TEST(Variant, RectArea) {
    foundation::Shape s = foundation::RectangleShape{3.0, 4.0};
    EXPECT_DOUBLE_EQ(foundation::shape_area(s), 12.0);
}

TEST(Variant, TriangleArea) {
    foundation::Shape s = foundation::TriangleShape{6.0, 4.0};
    EXPECT_DOUBLE_EQ(foundation::shape_area(s), 12.0);
}

TEST(Variant, ShapeName) {
    EXPECT_EQ(foundation::shape_name(foundation::Shape{foundation::CircleShape{1.0}}), "Circle");
    EXPECT_EQ(foundation::shape_name(foundation::Shape{foundation::RectangleShape{1.0,1.0}}), "Rectangle");
}

// --- Expected<T,E> ---

TEST(Expected, SafeDivideOk) {
    auto r = foundation::safe_divide(10.0, 2.0);
    ASSERT_TRUE(r.has_value());
    EXPECT_DOUBLE_EQ(r.value(), 5.0);
}

TEST(Expected, SafeDivideDivByZero) {
    auto r = foundation::safe_divide(1.0, 0.0);
    EXPECT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), foundation::MathError::DivisionByZero);
}

TEST(Expected, SafeSqrtOk) {
    auto r = foundation::safe_sqrt(9.0);
    ASSERT_TRUE(r.has_value());
    EXPECT_DOUBLE_EQ(r.value(), 3.0);
}

TEST(Expected, SafeSqrtNegative) {
    auto r = foundation::safe_sqrt(-1.0);
    EXPECT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), foundation::MathError::NegativeSqrt);
}

TEST(Expected, ValueOr) {
    auto r = foundation::safe_divide(1.0, 0.0);
    EXPECT_DOUBLE_EQ(r.value_or(-1.0), -1.0);
}

TEST(Expected, AndThenChain) {
    // safe_divide(16,2) -> 8  -> safe_sqrt(8) has_value
    auto r = foundation::safe_divide(16.0, 2.0)
                 .and_then([](double v){ return foundation::safe_sqrt(v); });
    ASSERT_TRUE(r.has_value());
    EXPECT_NEAR(r.value(), 2.828, 0.001);
}

TEST(Expected, AndThenPropagatesError) {
    auto r = foundation::safe_divide(1.0, 0.0)
                 .and_then([](double v){ return foundation::safe_sqrt(v); });
    EXPECT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), foundation::MathError::DivisionByZero);
}

TEST(Expected, ParseAndSqrtOk) {
    auto r = foundation::parse_and_sqrt("16.0");
    ASSERT_TRUE(r.has_value());
    EXPECT_DOUBLE_EQ(r.value(), 4.0);
}

TEST(Expected, ParseAndSqrtBadInput) {
    auto r = foundation::parse_and_sqrt("abc");
    EXPECT_FALSE(r.has_value());
}
