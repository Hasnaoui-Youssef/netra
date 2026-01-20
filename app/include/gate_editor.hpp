#pragma once

#include <core/world.hpp>
#include <core/entity.hpp>
#include <components/render_components.hpp>
#include <editor_state.hpp>
#include <graphics/grid.hpp>
#include <graphics/window.hpp>
#include <systems/layout_system.hpp>
#include <systems/render_system.hpp>

#include <imgui.h>

namespace netra::app {

// ECS-based gate editor.
// Owns World and systems; renders via RenderSystem.
class GateEditor {
public:
    GateEditor();

    void init(const std::string& shader_dir);
    void draw(graphics::Window& window);

    // Currently dragging entity (ports hidden during drag)
    [[nodiscard]] Entity dragging_entity() const { return m_dragging_entity; }

private:
    World m_world;
    graphics::Grid m_grid;
    EditorState m_editor_state;
    LayoutSystem m_layout_system;
    RenderSystem m_render_system;

    Entity m_selected_entity{};
    Entity m_dragging_entity{};
    glm::vec2 m_drag_offset{0.0f, 0.0f};
    glm::vec2 m_drag_start_mouse{0.0f, 0.0f};

    float m_palette_width = 220.f;
    bool m_canvas_hovered = false;

    // Create a primitive gate module with ports
    Entity create_gate(const std::string& type, GridCoord grid_pos);

    // Delete entity and its children (ports)
    void delete_entity(Entity entity);

    // Snap pixel position to nearest grid
    GridCoord snap_to_grid(glm::vec2 pixel_pos) const;

    // Determine anchor side from drag direction
    PortSide get_anchor_side(glm::vec2 drag_delta) const;

    // Get first port on given side
    Entity get_port_on_side(Entity module, PortSide side);
};

} // namespace netra::app
