#include <systems/layout_system.hpp>
#include <components/components.hpp>
#include <components/render_components.hpp>

namespace netra {

LayoutSystem::LayoutSystem(World& world, const graphics::Grid& grid)
    : m_world(world)
    , m_grid(grid) {}

void LayoutSystem::update_module_from_anchor(Entity anchor_port, Entity module) {
    auto* port_grid_pos = m_world.get<PortGridPosition>(anchor_port);
    auto* port_offset = m_world.get<PortOffset>(anchor_port);

    if (!port_grid_pos || !port_offset) {
        return;
    }

    // Module grid origin = anchor port grid position - port offset
    GridCoord module_grid_origin{
        port_grid_pos->position.x - port_offset->x,
        port_grid_pos->position.y - port_offset->y
    };

    // Convert to pixels
    glm::vec2 pixel_pos = m_grid.to_glm_vec2(module_grid_origin);

    // Update or create ModulePixelPosition
    if (m_world.has<ModulePixelPosition>(module)) {
        auto* pos = m_world.get<ModulePixelPosition>(module);
        pos->x = pixel_pos.x;
        pos->y = pixel_pos.y;
    } else {
        m_world.emplace<ModulePixelPosition>(module, pixel_pos.x, pixel_pos.y);
    }

    // Update all ports for this module
    update_ports(module, module_grid_origin);
}

void LayoutSystem::update_ports(Entity module, GridCoord module_grid_origin) {
    // Iterate all ports and update those belonging to this module
    m_world.view<Port, PortOffset>().each(
        [&](Entity port_entity, Port& port, PortOffset& offset) {
            if (port.owner != module) {
                return;
            }

            GridCoord port_grid_pos{
                module_grid_origin.x + offset.x,
                module_grid_origin.y + offset.y
            };

            if (m_world.has<PortGridPosition>(port_entity)) {
                auto* pos = m_world.get<PortGridPosition>(port_entity);
                pos->position = port_grid_pos;
            } else {
                m_world.emplace<PortGridPosition>(port_entity, port_grid_pos);
            }
        }
    );
}

void LayoutSystem::update_all() {
    // For each module with pixel position, derive grid origin and update ports
    m_world.view<ModuleInst, ModulePixelPosition, ModuleExtent>().each(
        [&](Entity module, ModuleInst&, ModulePixelPosition& pixel_pos, ModuleExtent&) {
            // Reverse: pixel position to grid origin
            // grid_origin = pixel_pos / unit_px
            auto unit_px = m_grid.unit_px();
            GridCoord module_grid_origin{
                static_cast<std::int32_t>(pixel_pos.x / static_cast<float>(unit_px)),
                static_cast<std::int32_t>(pixel_pos.y / static_cast<float>(unit_px))
            };

            update_ports(module, module_grid_origin);
        }
    );
}

} // namespace netra
