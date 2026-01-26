#pragma once

#include <components/render_components.hpp>
#include "glm/vec2.hpp"
#include "systems/layout_system.hpp"
#include <graphics/grid.hpp>
#include <core/entity.hpp>
#include <core/world.hpp>
#include <optional>

namespace netra::app::select_mode {
struct DragInfo {
    Entity entity;
    glm::vec2 position_offset;
};

class SelectModeHandler {
public:
    SelectModeHandler(World& world, graphics::Grid& grid,LayoutSystem& layout_system, const glm::vec2& mouse_pos)
        : m_world(world), m_grid(grid),
        m_layout_system(layout_system),
        m_canvas_mouse_pos(mouse_pos)
    {}
    std::optional<Entity> handleMouseClick();
    void handleMouseDown();
    void handleMouseRelease();
    Entity getDragEntity() const { return this->info.entity; }

private:
    Entity get_module_anchor_port(Entity module);
    World& m_world;
    graphics::Grid& m_grid;
    LayoutSystem& m_layout_system;
    const glm::vec2& m_canvas_mouse_pos;
    DragInfo info;
};
}
