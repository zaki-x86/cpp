#include <gtest/gtest.h>
#include <foundation/oop/rules.hpp>
#include <foundation/oop/crtp.hpp>
#include <foundation/oop/virtual_design.hpp>
#include <cstddef>

// --- Rule of 5 (Buffer) ---

TEST(RuleOfFive, DefaultConstructEmpty) {
    foundation::Buffer b;
    EXPECT_EQ(b.size(), 0u);
}

TEST(RuleOfFive, ConstructWithSize) {
    foundation::Buffer b(16);
    EXPECT_EQ(b.size(), 16u);
}

TEST(RuleOfFive, CopyConstructorDeepCopies) {
    foundation::Buffer a(8);
    a.data()[0] = std::byte{42};
    foundation::Buffer b = a;
    b.data()[0] = std::byte{99};
    EXPECT_EQ(a.data()[0], std::byte{42});  // a unchanged
}

TEST(RuleOfFive, MoveConstructorTransfersOwnership) {
    foundation::Buffer a(8);
    a.data()[0] = std::byte{7};
    foundation::Buffer b = std::move(a);
    EXPECT_EQ(b.size(), 8u);
    EXPECT_EQ(b.data()[0], std::byte{7});
    EXPECT_EQ(a.size(), 0u);  // moved-from is empty
}

TEST(RuleOfFive, SelfAssignmentSafe) {
    foundation::Buffer b(4);
    b.data()[0] = std::byte{55};
    b = b;
    EXPECT_EQ(b.size(), 4u);
    EXPECT_EQ(b.data()[0], std::byte{55});
}

// --- CRTP static polymorphism ---

TEST(CRTP, CircleArea) {
    foundation::Circle c(3.0);
    EXPECT_NEAR(c.area(), 28.274, 0.001);
}

TEST(CRTP, SquareArea) {
    foundation::Square s(4.0);
    EXPECT_NEAR(s.area(), 16.0, 0.001);
}

TEST(CRTP, PerimeterCalc) {
    foundation::Circle c(1.0);
    EXPECT_NEAR(c.perimeter(), 6.2831, 0.001);
}

// --- NVI (Non-Virtual Interface) ---

TEST(NVI, LoggingReaderCountsNVI) {
    foundation::LoggingReader reader;
    for (int i = 0; i < 3; ++i) reader.read();
    EXPECT_EQ(reader.impl_count(), 3);
    EXPECT_EQ(reader.pre_count(),  3);
    EXPECT_EQ(reader.post_count(), 3);
}

// --- Virtual inheritance (diamond) ---

TEST(VirtualInheritance, DiamondResolvesBase) {
    foundation::DiamondDerived d;
    // Single shared Base subobject — set_value/get_value unambiguous
    d.set_value(42);
    EXPECT_EQ(d.get_value(), 42);
}
