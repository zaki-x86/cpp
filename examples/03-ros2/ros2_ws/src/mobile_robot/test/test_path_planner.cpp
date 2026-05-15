#include <gtest/gtest.h>
#include "mobile_robot/a_star.hpp"

using namespace mobile_robot;

// 5x5 free grid
static Grid make_free_grid(int rows, int cols) {
    return Grid(rows, std::vector<int>(cols, 0));
}

TEST(AStar, ReachableGoalReturnsNonEmptyPath) {
    auto grid = make_free_grid(5, 5);
    auto path = a_star(grid, {0, 0}, {4, 4});
    EXPECT_FALSE(path.empty());
    EXPECT_EQ(path.front(), (Cell{0, 0}));
    EXPECT_EQ(path.back(),  (Cell{4, 4}));
}

TEST(AStar, BlockedGoalReturnsEmptyPath) {
    auto grid = make_free_grid(5, 5);
    // Surround {2,2} with walls
    grid[1][2] = 100; grid[3][2] = 100;
    grid[2][1] = 100; grid[2][3] = 100;
    auto path = a_star(grid, {0, 0}, {2, 2});
    EXPECT_TRUE(path.empty());
}

TEST(AStar, StraightLineHorizontal) {
    auto grid = make_free_grid(1, 5);
    auto path = a_star(grid, {0, 0}, {0, 4});
    EXPECT_EQ(path.size(), 5u);
}

TEST(AStar, StartEqualsGoal) {
    auto grid = make_free_grid(3, 3);
    auto path = a_star(grid, {1, 1}, {1, 1});
    ASSERT_EQ(path.size(), 1u);
    EXPECT_EQ(path.front(), (Cell{1, 1}));
}

TEST(AStar, PathAvoidWall) {
    // Wall across the middle: row 2, cols 0-3
    auto grid = make_free_grid(5, 5);
    for (int c = 0; c < 4; ++c) grid[2][c] = 100;
    // Must go around via col 4
    auto path = a_star(grid, {0, 0}, {4, 0});
    EXPECT_FALSE(path.empty());
    // Verify path never enters an occupied cell
    for (auto& [r, c] : path) {
        EXPECT_EQ(grid[r][c], 0) << "Path entered occupied cell at " << r << "," << c;
    }
}
