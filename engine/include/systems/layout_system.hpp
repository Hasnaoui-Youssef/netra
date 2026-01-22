#pragma once

#include <core/world.hpp>
#include <graphics/grid.hpp>

namespace netra {

// Computes derived positions for modules and ports.
//
// Module placement flow:
// 1. On drop, anchor port snaps to grid → PortGridPosition set externally
// 2. update_module_from_anchor() computes ModulePixelPosition
// 3. update_ports() computes all port grid positions from module origin
//
// Normal frame update:
// - Call update_ports() to refresh PortGridPosition for all ports
class LayoutSystem {
public:
  explicit LayoutSystem(World &world, const graphics::Grid &grid);

  // Compute module pixel position from an anchor port that was just snapped.
  // anchor_port: the port entity whose PortGridPosition is already set
  // moduleEntity: the module entity to update
  void update_module_from_anchor(Entity anchor_port, Entity moduleEntity);

  // Compute PortGridPosition for all ports of a module from its grid origin.
  // moduleEntity: the module entity (must have ModuleExtent)
  // module_grid_origin: top-left corner of module in grid coords
  void update_ports(Entity moduleEntity, GridCoord module_grid_origin);

  // Update all modules and their ports (full refresh).
  void update_all();

  // Check if a specific grid cell is blocked by a module or existing wire.
  // checks_module: if true, checks module bounding boxes (including buffer).
  // checks_wire: if true, checks if an existing wire occupies this cell.
  bool is_cell_blocked(GridCoord pos, bool checks_module = true,
                       bool checks_wire = true) const;

  // Find an orthogonal path from start to end avoiding obstacles.
  // Uses A* pathfinding.
  std::vector<GridCoord> route_wire(GridCoord start, GridCoord end) const;

  // Rebuilds the internal spatial index of obstacles.
  // Should be called when modules are moved or wires are created/deleted.
  void rebuild_spatial_index();

private:
  World &m_world;
  const graphics::Grid &m_grid;

  // Spatial index: maps grid coordinates to the entity occupying it.
  // Using unordered_map for infinite grid support while keeping lookup O(1).
  // If vector<vector> is strictly required for performance, we would need
  // defined bounds. For now, this satisfies the "spatial index" requirement
  // functionally. Key: GridCoord (hashed), Value: Entity (ModuleInst or Wire)
  struct GridCoordHash {
    std::size_t operator()(const GridCoord &c) const {
      return std::hash<int>()(c.x) ^ (std::hash<int>()(c.y) << 1);
    }
  };
  std::unordered_map<GridCoord, Entity, GridCoordHash> m_spatial_map;
};

} // namespace netra
