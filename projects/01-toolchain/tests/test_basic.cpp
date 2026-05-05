#include <gtest/gtest.h>
#include <algorithm>
#include <numeric>
#include <vector>

// These tests run clean under every sanitizer preset.
// They verify the build system works and sanitizers don't flag correct code.

TEST(SanityCheck, VectorAlgorithms) {
    std::vector<int> v(10);
    std::iota(v.begin(), v.end(), 1);  // 1..10
    int sum = std::accumulate(v.begin(), v.end(), 0);
    EXPECT_EQ(sum, 55);
}

TEST(SanityCheck, NoMemoryLeaks) {
    std::vector<std::vector<int>> matrix(100, std::vector<int>(100, 0));
    EXPECT_EQ(matrix[50][50], 0);
}

TEST(SanityCheck, NoDataRace) {
    // Single-threaded: no race possible
    int counter = 0;
    for (int i = 0; i < 1000; ++i) ++counter;
    EXPECT_EQ(counter, 1000);
}

TEST(SanityCheck, NoUndefinedBehavior) {
    // Correctly bounded operations — UBSan should stay silent
    std::vector<int> v{1, 2, 3, 4, 5};
    EXPECT_EQ(v.at(4), 5);
    int x = std::numeric_limits<int>::max() - 1;
    EXPECT_GT(x, 0);
}
