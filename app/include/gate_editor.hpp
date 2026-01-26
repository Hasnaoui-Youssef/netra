#pragma once

#include <core/world.hpp>
#include <core/entity.hpp>
#include <select_mode.hpp>
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
    select_mode::SelectModeHandler m_select_handler;

    Entity m_selected_entity{};
    Entity m_dragging_entity{};
    glm::vec2 m_drag_offset{0.0f, 0.0f};
    glm::vec2 m_drag_start_mouse{0.0f, 0.0f};
    glm::vec2 m_canvas_mouse_pos{0.0f, 0.0f};

    float m_palette_width = 220.f;
    bool m_canvas_hovered = false;

    // Create a primitive gate module with ports
    Entity create_gate(const std::string& type, GridCoord grid_pos);

    // Delete entity and its children (ports)
    void delete_entity(Entity entity);

    // Snap pixel position to nearest grid
    GridCoord snap_to_grid(glm::vec2 pixel_pos) const;


    // Get first port on given side
    Entity get_port_on_side(Entity module, PortSide side);

    // Wiring mode helpers
    void toggle_wiring_mode();
    void handle_wiring_click(GridCoord grid_pos);
    void handle_wiring_escape();
    Entity find_port_at(GridCoord grid_pos);
    Entity find_wire_point_at(GridCoord grid_pos);
    bool is_valid_wire_endpoint(Entity endpoint) const;
    bool are_ports_on_same_module(Entity port_a, Entity port_b) const;
    bool does_segment_intersect_module(GridCoord from, GridCoord to) const;
    GridCoord compute_orthogonal_corner(GridCoord from, GridCoord to) const;
    void commit_wire();
    void cancel_wire();
    void delete_wire(Entity wire);
};

} // namespace netra::app
