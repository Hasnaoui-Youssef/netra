#include <gate_editor.hpp>
#include <components/components.hpp>
#include <components/render_components.hpp>

#include <imgui.h>
#include <glm/glm.hpp>
#include <optional>

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
    {"XNOR", 20, 16, {{"A", PortDirection::In, PortSide::Left, -1, 4},
                      {"B", PortDirection::In, PortSide::Left, -1, 12},
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
    : m_grid(10)
    , m_layout_system(m_world, m_grid)
    , m_render_system(m_world, m_grid, m_editor_state)
    , m_select_handler(m_world, m_grid,m_layout_system, m_canvas_mouse_pos)
    {}

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
    float unit = static_cast<float>(m_grid.unit_px());
    return GridCoord{
        static_cast<std::int32_t>(std::round(pixel_pos.x / unit)),
        static_cast<std::int32_t>(std::round(pixel_pos.y / unit))
    };
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
        ImGui::TextUnformatted("Press 'w' to toggle wiring mode.");
        ImGui::TextUnformatted("Press 'd' to delete selected.");

        // Show current mode
        ImGui::Separator();
        if (m_editor_state.mode == EditorMode::Wiring) {
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "MODE: WIRING");
            ImGui::TextUnformatted("Click to place wire points.");
            ImGui::TextUnformatted("ESC to cancel.");
        } else {
            ImGui::TextUnformatted("MODE: SELECT");
        }
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
        m_canvas_mouse_pos.x = mouse.x - origin.x;
        m_canvas_mouse_pos.y = mouse.y - origin.y;

        if (m_canvas_hovered && ImGui::IsKeyPressed(ImGuiKey_W, false)) {
            toggle_wiring_mode();
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
            handle_wiring_escape();
        }

        switch (m_editor_state.mode) {
            case EditorMode::Select: {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_canvas_hovered) {
                    if(auto selected_entity = m_select_handler.handleMouseClick()) {
                        m_selected_entity = selected_entity.value();
                    }else {
                        m_selected_entity = Entity{};
                    }
                }
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    m_select_handler.handleMouseDown();
                }
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                    m_select_handler.handleMouseRelease();
                }
                break;
            }
            case EditorMode::Wiring: {
                m_editor_state.wiring.mouse_grid_pos = snap_to_grid(m_canvas_mouse_pos);

                if (m_editor_state.wiring.active) {
                    GridCoord start_pos{};
                    if (!m_editor_state.wiring.points.empty()) {
                        start_pos = m_editor_state.wiring.points.back();
                        m_editor_state.wiring.current_path = m_layout_system.route_wire(start_pos, m_editor_state.wiring.mouse_grid_pos);
                    }
                }

                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_canvas_hovered) {
                    //We should commit the currently previewed wire segment
                    GridCoord grid_pos = m_editor_state.wiring.mouse_grid_pos;
                    handle_wiring_click(grid_pos);
                }
                break;
            }
            default : {}
        }



        // Drop target: create gate at mouse position (only in Select mode)
        if (m_editor_state.mode == EditorMode::Select && ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NETRA_GATE")) {
                std::string gate_type(static_cast<const char*>(payload->Data));
                GridCoord grid_pos = snap_to_grid(m_canvas_mouse_pos);
                Entity new_gate = create_gate(gate_type, grid_pos);
                m_selected_entity = new_gate;
            }
            ImGui::EndDragDropTarget();
        }

        // Delete with 'd' - handles both modules and wires
        if (m_canvas_hovered && m_selected_entity.valid() && ImGui::IsKeyPressed(ImGuiKey_D, false)) {
            if (m_world.has<Wire>(m_selected_entity)) {
                delete_wire(m_selected_entity);
            } else {
                delete_entity(m_selected_entity);
            }
        }
        const int vx = static_cast<int>(origin.x - vp->Pos.x);
        const int vy = static_cast<int>(vp->Size.y - ((origin.y - vp->Pos.y) + content_size.y));
        const int vw = static_cast<int>(content_size.x);
        const int vh = static_cast<int>(content_size.y);

        m_render_system.render_region({static_cast<float>(vw), static_cast<float>(vh)}, vx, vy, vw, vh, m_select_handler.getDragEntity());
    }
    ImGui::End();
}

// ... (toggle_wiring_mode)
void GateEditor::toggle_wiring_mode() {
    if (m_editor_state.mode == EditorMode::Wiring) {
        cancel_wire();
        m_editor_state.mode = m_editor_state.last_mode;
    } else {
        m_selected_entity = Entity{};
        m_editor_state.last_mode = m_editor_state.mode;
        m_editor_state.mode = EditorMode::Wiring;
    }
}

void GateEditor::handle_wiring_click(GridCoord grid_pos) {
    auto& wiring = m_editor_state.wiring;

    Entity port = find_port_at(grid_pos);
    Entity wire_point = find_wire_point_at(grid_pos);

    // Validation: Can we click here?
    // Must be a port OR a free cell (not blocked by module/wire).
    // Note: wire_point is valid too.
    bool is_blocked = m_layout_system.is_cell_blocked(grid_pos);
    bool is_valid_click = (port.valid() ^ wire_point.valid()) || !is_blocked;

    if (!is_valid_click) {
        return; // Clicked on module interior or obstacle
    }

    if (!wiring.active) {
        if (port.valid()) {
            wiring.start_endpoint = port;
        } else if (wire_point.valid()) {
            wiring.start_endpoint = wire_point;
        }else {
            return;
        }
        wiring.active = true;
        wiring.points.clear();
        wiring.points.push_back(grid_pos);
        wiring.current_path.clear();
        // Else: Ignore click on empty space
    } else {
        // Continuing/Finishing wire
        // Commit the current preview path
        if (!wiring.current_path.empty()) {
            // Append current path to points
            // Note: current_path includes start (last point) and end (mouse).
            // We shouldn't duplicate the start point if it's already in points?
            // A* returns [start, ... end].
            // wiring.points usually ends with 'start'.

            // If points is empty, start is start_endpoint pos.
            // If points not empty, points.back() is start.

            size_t start_idx = 0;
            // If we have points, or start endpoint, the first point of current_path is redundant?
            // Yes, A* path[0] == start.
            if (!wiring.current_path.empty()) {
                start_idx = 1;
            }

            for (size_t i = start_idx; i < wiring.current_path.size(); ++i) {
                wiring.points.push_back(wiring.current_path[i]);
            }
        }

        wiring.current_path.clear();

        if (port.valid() || wire_point.valid()) {
            Entity endpoint = port.valid() ? port : wire_point;

            if (port.valid() && wiring.start_endpoint.valid()) {
                if (are_ports_on_same_module(wiring.start_endpoint, port)) {
                    cancel_wire();
                    return;
                }
            }

            Entity wire_entity = m_world.create();

            // Create or find signal for this wire
            Entity signal_entity = m_world.create();
            m_world.emplace<Signal>(signal_entity, "wire_signal", 1u, Entity{}, std::vector<Entity>{});

            Wire wire_comp{};
            wire_comp.signal = signal_entity;
            wire_comp.from_endpoint = wiring.start_endpoint;
            wire_comp.to_endpoint = endpoint;
            wire_comp.points = wiring.points;

            m_world.emplace<Wire>(wire_entity, std::move(wire_comp));

            // Update signal's connected ports
            auto* sig = m_world.get<Signal>(signal_entity);
            if (sig) {
                if (wiring.start_endpoint.valid() && m_world.has<Port>(wiring.start_endpoint)) {
                    sig->connected_ports.push_back(wiring.start_endpoint);
                    if (auto* p = m_world.get<Port>(wiring.start_endpoint)) {
                        p->connected_signal = signal_entity;
                    }
                }
                if (endpoint.valid() && m_world.has<Port>(endpoint)) {
                    sig->connected_ports.push_back(endpoint);
                    if (auto* p = m_world.get<Port>(endpoint)) {
                        p->connected_signal = signal_entity;
                    }
                }
            }

            // Explicitly update spatial index with new wire so we can't route through it immediately
            m_layout_system.rebuild_spatial_index();

            cancel_wire();
        }
        // Else: Clicked on empty space. Point added (via path commit). Continue wiring.
    }
}

void GateEditor::handle_wiring_escape() {
    if (m_editor_state.mode == EditorMode::Wiring) {
        if (m_editor_state.wiring.active) {
            cancel_wire();
        } else {
            m_editor_state.mode = EditorMode::Select;
        }
    }
}



Entity GateEditor::find_port_at(GridCoord grid_pos) {
    std::optional<Entity> found = m_world.view<Port, PortGridPosition>().find_first(
            [&grid_pos](Port&, PortGridPosition& pos){ return pos.position == grid_pos; }
            );
    return found.has_value() ? found.value() : Entity{};
}

Entity GateEditor::find_wire_point_at(GridCoord grid_pos) {
    // Check endpoints and points of existing wires
    // If we hit a wire point, we return the wire entity (as endpoint reference)?
    // Or a special entity representing that point?
    // For now, let's say we can only connect to endpoints or explicitly split wires.
    // The simplified requirement: "starts from valid ports".
    // But "can also connect to existing wire junctions" implies we might need to hit a wire.
    // If we hit a wire, we might need to split it?
    // For now, let's just return Entity{} unless it matches an endpoint.
    // Actually, finding a wire point usually implies "splitting" the wire into two.
    // That's complex. Let's stick to Ports for now as per "strict validation".
    // But handle_wiring_click calls it. Let's implement basics.

    Entity found = Entity{};
    m_world.view<Wire>().each([&](Entity e, Wire& wire) {
        for (const auto& pt : wire.points) {
            if (pt == grid_pos) {
                found = e; // Return the wire entity
                return;
            }
        }
    });
    return found;
}

bool GateEditor::is_valid_wire_endpoint(Entity endpoint) const {
    return m_world.alive(endpoint) && (
        m_world.has<Port>(endpoint) || m_world.has<Wire>(endpoint)
    );
}

void GateEditor::commit_wire() {
    // Logic moved to handle_wiring_click, this might be unused or for cleanup
    // We already committed in handle_wiring_click.
    // Let's leave it empty or remove usage if possible.
    // Re-implementation just in case:
     if (m_editor_state.wiring.points.empty()) return;
}

void GateEditor::cancel_wire() {
    m_editor_state.wiring.active = false;
    m_editor_state.wiring.points.clear();
    m_editor_state.wiring.current_path.clear();
    m_editor_state.wiring.start_endpoint = Entity{};
}


void GateEditor::delete_wire(Entity wire) {
    if (!m_world.has<Wire>(wire)) return;

    // Disconnect signal from ports
    if (auto* w = m_world.get<Wire>(wire)) {
        Entity sig_entity = w->signal;
        if (m_world.alive(sig_entity)) {
             m_world.destroy(sig_entity);
             // Note: ports connected to this signal need update?
             // Since signal is destroyed, port->connected_signal becomes invalid or should be cleared.
             // We should iterate ports? Or rely on weak ref?
             // System cleanup would be better, but here:
             m_world.view<Port>().each([&](Entity, Port& p) {
                 if (p.connected_signal == sig_entity) {
                     p.connected_signal = Entity{};
                 }
             });
        }
    }
    m_world.destroy(wire);

    // Refresh spatial index
    m_layout_system.rebuild_spatial_index();

    if (m_selected_entity == wire) m_selected_entity = Entity{};
}

bool GateEditor::are_ports_on_same_module(Entity port_a, Entity port_b) const {
    auto* pa = m_world.get<Port>(port_a);
    auto* pb = m_world.get<Port>(port_b);
    if (pa && pb) {
        return pa->owner == pb->owner;
    }
    return false;
}

} // namespace netra::app

