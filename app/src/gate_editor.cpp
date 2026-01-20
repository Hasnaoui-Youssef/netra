#include <gate_editor.hpp>
#include <components/components.hpp>
#include <components/render_components.hpp>

#include <imgui.h>
#include <glm/glm.hpp>

namespace netra::app {

// Gate definitions: name, extent (grid units), port offsets
struct GateTemplate {
    const char* name;
    std::int32_t width;
    std::int32_t height;
    struct PortDef {
        const char* name;
        PortDirection dir;
        PortSide side;
        std::int32_t offset_x;
        std::int32_t offset_y;
    };
    std::vector<PortDef> ports;
};

static const GateTemplate k_gate_templates[] = {
    // Ports at edges: left x=0, right x=width, inputs at 1/4 and 3/4 height, output at center
    {"AND",  20, 16, {{"A", PortDirection::In, PortSide::Left, 0, 4},
                      {"B", PortDirection::In, PortSide::Left, 0, 12},
                      {"Y", PortDirection::Out, PortSide::Right, 20, 8}}},
    {"NAND", 20, 16, {{"A", PortDirection::In, PortSide::Left, 0, 4},
                      {"B", PortDirection::In, PortSide::Left, 0, 12},
                      {"Y", PortDirection::Out, PortSide::Right, 20, 8}}},
    {"OR",   20, 16, {{"A", PortDirection::In, PortSide::Left, 0, 4},
                      {"B", PortDirection::In, PortSide::Left, 0, 12},
                      {"Y", PortDirection::Out, PortSide::Right, 20, 8}}},
    {"NOR",  20, 16, {{"A", PortDirection::In, PortSide::Left, 0, 4},
                      {"B", PortDirection::In, PortSide::Left, 0, 12},
                      {"Y", PortDirection::Out, PortSide::Right, 20, 8}}},
    {"XOR",  20, 16, {{"A", PortDirection::In, PortSide::Left, 0, 4},
                      {"B", PortDirection::In, PortSide::Left, 0, 12},
                      {"Y", PortDirection::Out, PortSide::Right, 20, 8}}},
    {"XNOR", 20, 16, {{"A", PortDirection::In, PortSide::Left, 0, 4},
                      {"B", PortDirection::In, PortSide::Left, 0, 12},
                      {"Y", PortDirection::Out, PortSide::Right, 20, 8}}},
    {"NOT",  20, 16, {{"A", PortDirection::In, PortSide::Left, 0, 8},
                      {"Y", PortDirection::Out, PortSide::Right, 20, 8}}},
};

static const GateTemplate* find_template(const std::string& name) {
    for (const auto& t : k_gate_templates) {
        if (t.name == name) return &t;
    }
    return nullptr;
}

static void begin_top_menu_bar() {
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        ImGui::MenuItem("New");
        ImGui::MenuItem("Open...");
        ImGui::MenuItem("Save");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Run")) {
        ImGui::MenuItem("Step");
        ImGui::MenuItem("Run");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Debug")) {
        ImGui::MenuItem("Toggle debug");
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

GateEditor::GateEditor()
    : m_grid(5)
    , m_layout_system(m_world, m_grid)
    , m_render_system(m_world, m_grid, m_editor_state) {}

void GateEditor::init(const std::string& shader_dir) {
    m_render_system.init(shader_dir);
}

Entity GateEditor::create_gate(const std::string& type, GridCoord grid_pos) {
    const GateTemplate* tmpl = find_template(type);
    if (!tmpl) return Entity{};

    // Create module definition entity (reusable, but for simplicity create one per instance)
    Entity def_entity = m_world.create();
    m_world.emplace<ModuleDef>(def_entity, type, true);

    // Create module instance entity
    Entity inst_entity = m_world.create();
    m_world.emplace<ModuleInst>(inst_entity, type + "_inst", def_entity);
    m_world.emplace<ModuleExtent>(inst_entity, tmpl->width, tmpl->height);
    m_world.emplace<ShaderKey>(inst_entity, type);

    // Compute pixel position from grid
    float px = static_cast<float>(grid_pos.x * m_grid.unit_px());
    float py = static_cast<float>(grid_pos.y * m_grid.unit_px());
    m_world.emplace<ModulePixelPosition>(inst_entity, px, py);

    // Create ports
    Hierarchy hier{Entity{}, {}};
    for (const auto& port_def : tmpl->ports) {
        Entity port_entity = m_world.create();
        m_world.emplace<Port>(port_entity, port_def.name, port_def.dir, 1u, inst_entity, Entity{});
        m_world.emplace<PortOffset>(port_entity, port_def.offset_x, port_def.offset_y);
        m_world.emplace<PortVisual>(port_entity, port_def.side);
        m_world.emplace<PortGridPosition>(port_entity, GridCoord{
            grid_pos.x + port_def.offset_x,
            grid_pos.y + port_def.offset_y
        });
        hier.children.push_back(port_entity);
    }
    m_world.emplace<Hierarchy>(inst_entity, Entity{}, std::move(hier.children));

    return inst_entity;
}

void GateEditor::delete_entity(Entity entity) {
    if (!m_world.alive(entity)) return;

    // Delete children (ports)
    if (auto* hier = m_world.get<Hierarchy>(entity)) {
        for (Entity child : hier->children) {
            m_world.destroy(child);
        }
    }

    m_world.destroy(entity);

    if (m_selected_entity == entity) m_selected_entity = Entity{};
    if (m_dragging_entity == entity) m_dragging_entity = Entity{};
}

GridCoord GateEditor::snap_to_grid(glm::vec2 pixel_pos) const {
    int unit = m_grid.unit_px();
    return GridCoord{
        static_cast<std::int32_t>(std::round(pixel_pos.x / static_cast<float>(unit))),
        static_cast<std::int32_t>(std::round(pixel_pos.y / static_cast<float>(unit)))
    };
}

PortSide GateEditor::get_anchor_side(glm::vec2 drag_delta) const {
    if (std::abs(drag_delta.x) > std::abs(drag_delta.y)) {
        return drag_delta.x > 0 ? PortSide::Left : PortSide::Right;
    } else {
        return drag_delta.y > 0 ? PortSide::Top : PortSide::Bottom;
    }
}

Entity GateEditor::get_port_on_side(Entity module, PortSide side) {
    auto* hier = m_world.get<Hierarchy>(module);
    if (!hier) return Entity{};

    for (Entity port : hier->children) {
        if (auto* vis = m_world.get<PortVisual>(port)) {
            if (vis->side == side) return port;
        }
    }
    // Fallback: return first port
    return hier->children.empty() ? Entity{} : hier->children[0];
}

void GateEditor::draw(graphics::Window& window) {
    begin_top_menu_bar();

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float menu_h = ImGui::GetFrameHeight();

    const ImVec2 canvas_pos(vp->Pos.x + m_palette_width, vp->Pos.y + menu_h);
    const ImVec2 canvas_size(vp->Size.x - m_palette_width, vp->Size.y - menu_h);

    // Palette
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, vp->Pos.y + menu_h), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(m_palette_width, vp->Size.y - menu_h), ImGuiCond_Always);
    ImGuiWindowFlags fixed_flags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar;

    if (ImGui::Begin("Palette", nullptr, fixed_flags)) {
        for (const auto& tmpl : k_gate_templates) {
            ImGui::Selectable(tmpl.name, false);

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                ImGui::SetDragDropPayload("NETRA_GATE", tmpl.name, strlen(tmpl.name) + 1);
                ImGui::Text("%s", tmpl.name);
                ImGui::EndDragDropSource();
            }
        }

        ImGui::Separator();
        ImGui::TextUnformatted("Drag a gate onto the canvas.");
        ImGui::TextUnformatted("Press 'd' to delete selected.");
    }
    ImGui::End();

    // Canvas
    ImGui::SetNextWindowPos(canvas_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(canvas_size, ImGuiCond_Always);
    ImGuiWindowFlags canvasFlags =
        fixed_flags |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoBackground;

    if (ImGui::Begin("Canvas", nullptr, canvasFlags)) {
        const ImVec2 content_size = ImGui::GetContentRegionAvail();
        if (content_size.x <= 0.0f || content_size.y <= 0.0f) {
            ImGui::End();
            return;
        }

        ImGui::InvisibleButton("canvas", content_size, ImGuiButtonFlags_MouseButtonLeft);
        m_canvas_hovered = ImGui::IsItemHovered();

        const ImVec2 origin = ImGui::GetItemRectMin();
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const glm::vec2 mouse_canvas(mouse.x - origin.x, mouse.y - origin.y);

        // Drop target: create gate at mouse position
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NETRA_GATE")) {
                std::string gate_type(static_cast<const char*>(payload->Data));
                GridCoord grid_pos = snap_to_grid(mouse_canvas);
                Entity new_gate = create_gate(gate_type, grid_pos);
                m_selected_entity = new_gate;
            }
            ImGui::EndDragDropTarget();
        }

        // Selection and drag start
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_canvas_hovered) {
            m_selected_entity = Entity{};
            m_dragging_entity = Entity{};

            // Hit test modules (reverse order for top-most first)
            std::vector<std::pair<Entity, ModulePixelPosition*>> modules;
            m_world.view<ModuleInst, ModulePixelPosition, ModuleExtent>().each(
                [&](Entity e, ModuleInst&, ModulePixelPosition& pos, ModuleExtent& ext) {
                    modules.push_back({e, &pos});
                }
            );

            for (auto it = modules.rbegin(); it != modules.rend(); ++it) {
                auto [e, pos] = *it;
                auto* ext = m_world.get<ModuleExtent>(e);
                float w = static_cast<float>(ext->width * m_grid.unit_px());
                float h = static_cast<float>(ext->height * m_grid.unit_px());

                if (mouse_canvas.x >= pos->x && mouse_canvas.x <= pos->x + w &&
                    mouse_canvas.y >= pos->y && mouse_canvas.y <= pos->y + h) {
                    m_selected_entity = e;
                    m_dragging_entity = e;
                    m_drag_offset = glm::vec2(mouse_canvas.x - pos->x, mouse_canvas.y - pos->y);
                    m_drag_start_mouse = mouse_canvas;
                    break;
                }
            }
        }

        // Dragging
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_dragging_entity.valid()) {
            if (auto* pos = m_world.get<ModulePixelPosition>(m_dragging_entity)) {
                pos->x = mouse_canvas.x - m_drag_offset.x;
                pos->y = mouse_canvas.y - m_drag_offset.y;
            }
        }

        // Drop: snap to grid
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && m_dragging_entity.valid()) {
            if (auto* pos = m_world.get<ModulePixelPosition>(m_dragging_entity)) {
                glm::vec2 drag_delta = mouse_canvas - m_drag_start_mouse;
                PortSide anchor_side = get_anchor_side(drag_delta);
                Entity anchor_port = get_port_on_side(m_dragging_entity, anchor_side);

                if (anchor_port.valid()) {
                    auto* port_offset = m_world.get<PortOffset>(anchor_port);
                    if (port_offset) {
                        // Compute where the anchor port currently is in pixels
                        glm::vec2 port_pixel{
                            pos->x + static_cast<float>(port_offset->x * m_grid.unit_px()),
                            pos->y + static_cast<float>(port_offset->y * m_grid.unit_px())
                        };

                        // Snap that port to grid
                        GridCoord snapped_port_grid = snap_to_grid(port_pixel);

                        // Update port grid position
                        if (auto* port_grid = m_world.get<PortGridPosition>(anchor_port)) {
                            port_grid->position = snapped_port_grid;
                        } else {
                            m_world.emplace<PortGridPosition>(anchor_port, snapped_port_grid);
                        }

                        // Use layout system to compute module position from anchor
                        m_layout_system.update_module_from_anchor(anchor_port, m_dragging_entity);
                    }
                }
            }
            m_dragging_entity = Entity{};
        }

        // Delete with 'd'
        if (m_canvas_hovered && m_selected_entity.valid() && ImGui::IsKeyPressed(ImGuiKey_D, false)) {
            delete_entity(m_selected_entity);
        }

        // Render via RenderSystem
        // Convert ImGui coords to OpenGL (bottom-left origin)
        const int vx = static_cast<int>(origin.x - vp->Pos.x);
        const int vy = static_cast<int>(vp->Size.y - ((origin.y - vp->Pos.y) + content_size.y));
        const int vw = static_cast<int>(content_size.x);
        const int vh = static_cast<int>(content_size.y);

        m_render_system.render_region({static_cast<float>(vw), static_cast<float>(vh)}, vx, vy, vw, vh, m_dragging_entity);

        // Highlight selected module
        if (m_selected_entity.valid()) {
            if (auto* pos = m_world.get<ModulePixelPosition>(m_selected_entity)) {
                if (auto* ext = m_world.get<ModuleExtent>(m_selected_entity)) {
                    float w = static_cast<float>(ext->width * m_grid.unit_px());
                    float h = static_cast<float>(ext->height * m_grid.unit_px());
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    ImVec2 p0(origin.x + pos->x, origin.y + pos->y);
                    ImVec2 p1(p0.x + w, p0.y + h);
                    dl->AddRect(p0, p1, IM_COL32(255, 220, 0, 255), 0.0f, 0, 2.0f);
                }
            }
        }
    }
    ImGui::End();
}

} // namespace netra::app
