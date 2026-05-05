#pragma once
#include <vector>
#include <utility>

namespace mobile_robot {

// 2D occupancy grid: 0 = free, 100 = occupied
using Grid = std::vector<std::vector<int>>;
using Cell = std::pair<int, int>;  // {row, col}
using Path = std::vector<Cell>;

// Returns sequence of {row,col} cells from start to goal (inclusive).
// Returns empty path if goal is unreachable.
Path a_star(const Grid& grid, Cell start, Cell goal);

}  // namespace mobile_robot
