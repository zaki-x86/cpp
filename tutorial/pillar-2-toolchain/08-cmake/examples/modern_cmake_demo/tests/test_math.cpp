#include "demo/math.hpp"
#include <gtest/gtest.h>
#include <array>

TEST(GCD, KnownValues) {
    EXPECT_EQ(demo::gcd(48, 18), 6);
    EXPECT_EQ(demo::gcd(0, 5),   5);
    EXPECT_EQ(demo::gcd(7, 7),   7);
    EXPECT_EQ(demo::gcd(13, 4),  1);
}

TEST(LCM, KnownValues) {
    EXPECT_EQ(demo::lcm(4, 6),  12);
    EXPECT_EQ(demo::lcm(0, 5),   0);
    EXPECT_EQ(demo::lcm(7, 7),   7);
}

TEST(Dot, ThreeDimensional) {
    std::array<double, 3> a{1.0, 2.0, 3.0};
    std::array<double, 3> b{4.0, 5.0, 6.0};
    EXPECT_DOUBLE_EQ(demo::dot<double>(a, b), 32.0);
}

TEST(Dot, MismatchThrows) {
    std::array<double, 2> a{1.0, 2.0};
    std::array<double, 3> b{1.0, 2.0, 3.0};
    EXPECT_THROW(demo::dot<double>(a, b), std::invalid_argument);
}

TEST(Clamp, BelowLo) { EXPECT_EQ(demo::clamp(-5, 0, 10), 0);  }
TEST(Clamp, InRange) { EXPECT_EQ(demo::clamp(5,  0, 10), 5);  }
TEST(Clamp, AboveHi) { EXPECT_EQ(demo::clamp(15, 0, 10), 10); }
