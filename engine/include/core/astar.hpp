#pragma once

#include <grid_coord.hpp>
#include <vector>
#include <functional>

namespace netra {

// Function signature for checking if a cell is blocked.
// Returns true if the cell at (x, y) is blocked.
using ObstacleCheck = std::function<bool(GridCoord)>;

// Finds an orthogonal path from start to end on a grid.
// Uses A* with Manhattan distance and turn penalties.
// 
// start: Starting grid coordinate.
// end: Target grid coordinate.
// is_blocked: Callback to check if a specific coordinate is blocked (obstacle).
//             Note: The 'end' coordinate is NOT checked against obstacles to allow connecting to ports on modules.
//
// Returns: A vector of grid coordinates representing the path (including start and end).
//          Returns an empty vector if no path is found.
std::vector<GridCoord> find_orthogonal_path(GridCoord start, GridCoord end, ObstacleCheck is_blocked);

} // namespace netra
