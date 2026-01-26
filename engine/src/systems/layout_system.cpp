#include <components/components.hpp>
#include <components/render_components.hpp>
#include <systems/layout_system.hpp>

#include <core/astar.hpp>
#include <unordered_set>

namespace netra {

LayoutSystem::LayoutSystem(World &world, const graphics::Grid &grid)
    : m_world(world), m_grid(grid) {
  // Initial build
  rebuild_spatial_index();
}

// ... (existing update_module_from_anchor, update_ports - keep them)

void LayoutSystem::update_module_from_anchor(Entity anchor_port,
                                             Entity moduleEntity) {
  auto const *port_grid_pos = m_world.get<PortGridPosition>(anchor_port);
  auto const *port_offset = m_world.get<PortOffset>(anchor_port);

  if (!port_grid_pos || !port_offset) {
    return;
  }

  // Module grid origin = anchor port grid position - port offset
  GridCoord module_grid_origin{port_grid_pos->position.x - port_offset->x,
                               port_grid_pos->position.y - port_offset->y};

  // Convert to pixels
  glm::vec2 pixel_pos = m_grid.to_glm_vec2(module_grid_origin);

  // Update or create ModulePixelPosition
  if (m_world.has<ModulePixelPosition>(moduleEntity)) {
    auto *pos = m_world.get<ModulePixelPosition>(moduleEntity);
    pos->x = pixel_pos.x;
    pos->y = pixel_pos.y;
  } else {
    m_world.emplace<ModulePixelPosition>(moduleEntity, pixel_pos.x,
                                         pixel_pos.y);
  }

  // Update all ports for this module
  update_ports(moduleEntity, module_grid_origin);

  // Refresh spatial index as module moved
  // Optimization: could just update this module, but rebuild is safer for now
  rebuild_spatial_index();
}

void LayoutSystem::update_ports(Entity moduleEntity,
                                GridCoord module_grid_origin) {
  // Iterate all ports and update those belonging to this module
  m_world.view<Port, PortOffset>().each(
      [this, &moduleEntity, &module_grid_origin](
          Entity port_entity, const Port &port, const PortOffset &offset) {
        if (!(port.owner == moduleEntity)) {
          return;
        }

        GridCoord port_grid_pos{module_grid_origin.x + offset.x,
                                module_grid_origin.y + offset.y};

        if (m_world.has<PortGridPosition>(port_entity)) {
          auto *pos = m_world.get<PortGridPosition>(port_entity);
          pos->position = port_grid_pos;
        } else {
          m_world.emplace<PortGridPosition>(port_entity, port_grid_pos);
        }
      });
}

void LayoutSystem::update_all() {
  // For each module with pixel position, derive grid origin and update ports
  m_world.view<ModuleInst, ModulePixelPosition, ModuleExtent>().each(
      [this](Entity moduleEntity, const ModuleInst &,
             const ModulePixelPosition &pixel_pos, const ModuleExtent &) {
        // Reverse: pixel position to grid origin
        // grid_origin = pixel_pos / unit_px
        auto unit_px = m_grid.unit_px();
        GridCoord module_grid_origin{
            static_cast<std::int32_t>(pixel_pos.x /
                                      static_cast<float>(unit_px)),
            static_cast<std::int32_t>(pixel_pos.y /
                                      static_cast<float>(unit_px))};

        update_ports(moduleEntity, module_grid_origin);
      });

  rebuild_spatial_index();
}

bool LayoutSystem::is_cell_blocked(GridCoord pos, bool checks_module,
                                   bool checks_wire) const {
  auto it = m_spatial_map.find(pos);
  if (it == m_spatial_map.end()) {
    return false;
  }

  Entity occupier = it->second;
  if (!occupier.valid())
    return false;

  if (checks_module && m_world.has<ModuleInst>(occupier)) {
    return true;
  }

  if (checks_wire && m_world.has<Wire>(occupier)) {
    return true;
  }

  return false;
}

std::vector<GridCoord> LayoutSystem::route_wire(GridCoord start,
                                                GridCoord end) const {
  // Ensure spatial index is up to date (usually it is, unless we just added
  // something without update) For now we assume update_event or similar keeps
  // it fresh, or we rely on update_all() called in loop.

  // Define obstacle callback
  auto obstacle_cb = [this](GridCoord pos) {
    // Block if cell is occupied by generic obstacle
    // But allow end position (it might be a port, which is technically
    // "occupied" by module, but AStar excludes end check by default? No, AStar
    // implementation calls is_blocked(neighbor). Our AStar implementation
    // checks is_blocked for neighbors unless it is the target. So we just
    // return true if it's a module/wire.
    return is_cell_blocked(pos, true, true);
  };

  return find_orthogonal_path(start, end, obstacle_cb);
}

void LayoutSystem::rebuild_spatial_index() {
  m_spatial_map.clear();
  auto &world = const_cast<World &>(
      m_world); // View iteration needs mutable? No, usually not, but maybe.

  // 1. Add Modules (with padding) and their Ports (as explict connectable
  // spots? No, actually ports are on the module) Wait, if I pad the module, I
  // block the ports! Ports are usually at the edge. If I block module + 1 unit
  // buffer, I block ports. Solution: Block module interior + border, BUT do NOT
  // block the specific coordinates where ports are located.

  // First, collect all port locations so we don't block them with padding
  std::unordered_set<GridCoord, GridCoordHash> port_locations;
  world.view<Port, PortGridPosition>().each(
      [&port_locations](Entity, Port &, const PortGridPosition &pos) {
        port_locations.insert(pos.position);
      });

  world.view<ModuleInst, ModulePixelPosition, ModuleExtent>().each(
      [this, &port_locations](Entity e, const ModuleInst &,
                              const ModulePixelPosition &pixel_pos,
                              const ModuleExtent &ext) {
        auto unit_px = m_grid.unit_px();
        GridCoord origin{static_cast<std::int32_t>(pixel_pos.x /
                                                   static_cast<float>(unit_px)),
                         static_cast<std::int32_t>(
                             pixel_pos.y / static_cast<float>(unit_px))};

        // Module Area
        for (int y = 0; y < ext.height; ++y) {
          for (int x = 0; x < ext.width; ++x) {
            m_spatial_map[{origin.x + x, origin.y + y}] = e;
          }
        }

        // Padding Area (1 unit around) - Optional based on user request
        // "shouldn't go alongside the border" This implies a buffer. But we
        // must NOT block ports.
        for (int y = -1; y <= ext.height; ++y) {
          for (int x = -1; x <= ext.width; ++x) {
            // Skip if inside module (already handled)
            if (x >= 0 && x < ext.width && y >= 0 && y < ext.height)
              continue;

            GridCoord pad_pos{origin.x + x, origin.y + y};

            // If this is a port location, DO NOT block it
            if (port_locations.contains(pad_pos))
              continue;

            // Mark as blocked by this module
            m_spatial_map[pad_pos] = e;
          }
        }
      });

  // 2. Add Existing Wires
  world.view<Wire>().each([this](Entity e, const Wire &wire) {
    for (const auto &pt : wire.points) {
      m_spatial_map[pt] = e;
    }
  });
}


} // namespace netra
