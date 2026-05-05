#include "mobile_robot/a_star.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <queue>
#include <set>

namespace mobile_robot {

namespace {

double heuristic(Cell a, Cell b) {
    return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

bool in_bounds(const Grid& g, Cell c) {
    return c.first >= 0 && c.first < (int)g.size() &&
           c.second >= 0 && c.second < (int)g[0].size();
}

bool passable(const Grid& g, Cell c) {
    return g[c.first][c.second] == 0;
}

const std::vector<Cell> DIRS = {{-1,0},{1,0},{0,-1},{0,1}};

}  // namespace

Path a_star(const Grid& grid, Cell start, Cell goal) {
    if (!in_bounds(grid, start) || !in_bounds(grid, goal)) return {};
    if (!passable(grid, goal)) return {};
    if (start == goal) return {start};

    // {f_score, cell}
    using PQEntry = std::pair<double, Cell>;
    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<>> open;

    std::map<Cell, Cell>   came_from;
    std::map<Cell, double> g_score;
    std::set<Cell>         closed;

    g_score[start] = 0.0;
    open.push({heuristic(start, goal), start});

    while (!open.empty()) {
        auto [f, current] = open.top(); open.pop();
        if (current == goal) {
            // Reconstruct path
            Path path;
            Cell c = goal;
            while (c != start) {
                path.push_back(c);
                c = came_from[c];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }
        if (closed.count(current)) continue;
        closed.insert(current);

        for (auto& dir : DIRS) {
            Cell next = {current.first + dir.first, current.second + dir.second};
            if (!in_bounds(grid, next) || !passable(grid, next) || closed.count(next))
                continue;
            double tentative_g = g_score[current] + 1.0;
            if (!g_score.count(next) || tentative_g < g_score[next]) {
                g_score[next]    = tentative_g;
                came_from[next]  = current;
                open.push({tentative_g + heuristic(next, goal), next});
            }
        }
    }
    return {};  // unreachable
}

}  // namespace mobile_robot
