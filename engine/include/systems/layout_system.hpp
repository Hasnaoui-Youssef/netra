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
    explicit LayoutSystem(World& world, const graphics::Grid& grid);

    // Compute module pixel position from an anchor port that was just snapped.
    // anchor_port: the port entity whose PortGridPosition is already set
    // module: the module entity to update
    void update_module_from_anchor(Entity anchor_port, Entity module);

    // Compute PortGridPosition for all ports of a module from its grid origin.
    // module: the module entity (must have ModuleExtent)
    // module_grid_origin: top-left corner of module in grid coords
    void update_ports(Entity module, GridCoord module_grid_origin);

    // Update all modules and their ports (full refresh).
    void update_all();

private:
    World& m_world;
    const graphics::Grid& m_grid;
};

} // namespace netra
