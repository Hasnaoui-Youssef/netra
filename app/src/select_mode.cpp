#include <components/components.hpp>
#include <components/render_components.hpp>
#include <optional>
#include <select_mode.hpp>


namespace netra::app::select_mode {
std::optional<Entity> SelectModeHandler::handleMouseClick(){
    std::optional<Entity> selected_entity = m_world.view<ModulePixelPosition, ModuleExtent>()
        .find_first(
            [this](Entity e, const ModulePixelPosition& pos, const ModuleExtent& ext) {
                float w = static_cast<float>(ext.width * m_grid.unit_px());
                float h = static_cast<float>(ext.height * m_grid.unit_px());
                return (m_canvas_mouse_pos.x >= pos.x && m_canvas_mouse_pos.x <= pos.x + w &&
                        m_canvas_mouse_pos.y >= pos.y && m_canvas_mouse_pos.y <= pos.y + h);
            }
    );
    if(selected_entity.has_value()) {
        info = DragInfo{.entity = selected_entity.value()};
        if(auto const* pos = m_world.get<ModulePixelPosition>(selected_entity.value())) {
            info.position_offset = glm::vec2(m_canvas_mouse_pos.x - pos->x, m_canvas_mouse_pos.y - pos->y);
        }else {
            info = DragInfo{.entity = Entity{} };
            return std::nullopt;
        }
    }else {
        info = DragInfo{.entity = Entity{} };
    }
    return selected_entity;
}
void SelectModeHandler::handleMouseDown() {
    if (!info.entity.valid()) return;
    if (auto *pos = m_world.get<ModulePixelPosition>(info.entity)) {
        pos->x = m_canvas_mouse_pos.x - info.position_offset.x;
        pos->y = m_canvas_mouse_pos.y - info.position_offset.y;
    }
}
void SelectModeHandler::handleMouseRelease(){
    if (!info.entity.valid()) return;
    Entity anchor_port = get_module_anchor_port(info.entity);
    auto const* pos = m_world.get<ModulePixelPosition>(info.entity);
    if(!pos) return;
    if(!anchor_port.valid()) return;
    auto const* port_offset = m_world.get<PortOffset>(anchor_port);
    if(!port_offset) return;
    GridCoord snapped_port_pos = GridCoord {
        static_cast<std::int32_t>(pos->x / static_cast<float>(m_grid.unit_px()) + port_offset->x),
        static_cast<std::int32_t>(pos->y / static_cast<float>(m_grid.unit_px()) + port_offset->y)
    };
    if(auto* port_grid = m_world.get<PortGridPosition>(anchor_port)) {
        port_grid->position = snapped_port_pos;
    }else{
        m_world.emplace<PortGridPosition>(anchor_port, snapped_port_pos);
    }
    m_layout_system.update_module_from_anchor(anchor_port, info.entity);
    info = DragInfo{};
}

Entity SelectModeHandler::get_module_anchor_port(Entity module) {
    auto* hier = m_world.get<Hierarchy>(module);
    if(!hier) return Entity{};
    return hier->children.empty() ? Entity{} : hier->children[0];
}
}
