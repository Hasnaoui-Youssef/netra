#include <core/astar.hpp>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <limits>

namespace netra {

namespace {

struct Node {
    GridCoord pos;
    GridCoord parent_pos; // For path reconstruction
    int g_cost;           // Cost from start
    int h_cost;           // Heuristic to end
    int direction;        // Direction entered from: 0=none, 1=up, 2=down, 3=left, 4=right

    int f_cost() const { return g_cost + h_cost; }

    bool operator>(const Node& other) const {
        return f_cost() > other.f_cost();
    }
};

// Simple hash for GridCoord to use in unordered_map
struct GridCoordHash {
    std::size_t operator()(const GridCoord& c) const {
        return std::hash<int>()(c.x) ^ (std::hash<int>()(c.y) << 1);
    }
};

int manhattan_distance(GridCoord a, GridCoord b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

} // namespace

std::vector<GridCoord> find_orthogonal_path(GridCoord start, GridCoord end, ObstacleCheck is_blocked) {
    if (start.x == end.x && start.y == end.y) {
        return {start};
    }
    if(is_blocked(end)) return {};

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open_set;
    std::unordered_map<GridCoord, int, GridCoordHash> best_g_cost;
    std::unordered_map<GridCoord, GridCoord, GridCoordHash> came_from;

    open_set.push({start, start, 0, manhattan_distance(start, end), 0});
    best_g_cost[start] = 0;

    const int MOVE_COST = 1;
    const int TURN_PENALTY = 50;

    // Directions: 0=up, 1=down, 2=left, 3=right
    const GridCoord dirs[] = {{0, 1}, {0, -1}, {-1, 0}, {1, 0}};
    const int dir_ids[] = {1, 2, 3, 4}; // IDs matching dirs array

    while (!open_set.empty()) {
        Node current = open_set.top();
        open_set.pop();

        if (current.pos.x == end.x && current.pos.y == end.y) {
            // Reconstruct path
            std::vector<GridCoord> path;
            GridCoord curr = end;
            while (curr.x != start.x || curr.y != start.y) {
                path.push_back(curr);
                curr = came_from[curr];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (int i = 0; i < 4; ++i) {
            GridCoord neighbor_pos{current.pos.x + dirs[i].x, current.pos.y + dirs[i].y};

            bool is_target = (neighbor_pos.x == end.x && neighbor_pos.y == end.y);
            if (!is_target && is_blocked(neighbor_pos)) {
                continue;
            }

            int new_turn_penalty = 0;
            if (current.direction != 0 && current.direction != dir_ids[i]) {
                new_turn_penalty = TURN_PENALTY;
            }

            int new_g_cost = current.g_cost + MOVE_COST + new_turn_penalty;

            if (best_g_cost.find(neighbor_pos) == best_g_cost.end() || new_g_cost < best_g_cost[neighbor_pos]) {
                best_g_cost[neighbor_pos] = new_g_cost;
                came_from[neighbor_pos] = current.pos;
                int h = manhattan_distance(neighbor_pos, end) * MOVE_COST; // Scale H to match G scale
                open_set.push({neighbor_pos, current.pos, new_g_cost, h, dir_ids[i]});
            }
        }
    }

    return {}; // No path found
}

} // namespace netra
