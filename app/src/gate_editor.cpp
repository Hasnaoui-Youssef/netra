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
    : m_grid(10)
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
        const glm::vec2 mouse_canvas(mouse.x - origin.x, mouse.y - origin.y);

        // Toggle wiring mode with 'w'
        if (m_canvas_hovered && ImGui::IsKeyPressed(ImGuiKey_W, false)) {
            toggle_wiring_mode();
        }

        // Escape handling (cancel wire in wiring mode)
        if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
            handle_wiring_escape();
        }

        // Drop target: create gate at mouse position (only in Select mode)
        if (m_editor_state.mode == EditorMode::Select && ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NETRA_GATE")) {
                std::string gate_type(static_cast<const char*>(payload->Data));
                GridCoord grid_pos = snap_to_grid(mouse_canvas);
                Entity new_gate = create_gate(gate_type, grid_pos);
                m_selected_entity = new_gate;
            }
            ImGui::EndDragDropTarget();
        }

        // Mode-specific input handling
        if (m_editor_state.mode == EditorMode::Wiring) {
            // Update mouse position for rubber band preview
            m_editor_state.wiring.mouse_grid_pos = snap_to_grid(mouse_canvas);

            // Calculate dynamic path if active
            if (m_editor_state.wiring.active) {
                GridCoord start_pos{};
                bool has_start = false;

                if (!m_editor_state.wiring.points.empty()) {
                    start_pos = m_editor_state.wiring.points.back();
                    has_start = true;
                } else if (m_editor_state.wiring.start_endpoint.valid()) {
                    if (auto* pos = m_world.get<PortGridPosition>(m_editor_state.wiring.start_endpoint)) {
                        start_pos = pos->position;
                        has_start = true;
                    }
                }

                if (has_start) {
                    // Avoid re-calculating if mouse hasn't moved (optimization)
                    // But we don't store last mouse pos easily here, A* is fast enough for now.

                    // Route wire
                    m_editor_state.wiring.current_path = m_layout_system.route_wire(start_pos, m_editor_state.wiring.mouse_grid_pos);
                }
            }

            // Wiring mode: click places points
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_canvas_hovered) {
                GridCoord grid_pos = snap_to_grid(mouse_canvas);
                handle_wiring_click(grid_pos);
            }
        } else {
            // Select mode: selection and drag
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
        } // End Select mode block

        // Dragging (only in Select mode)
        if (m_editor_state.mode == EditorMode::Select) {
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
        }

        // Delete with 'd' - handles both modules and wires
        if (m_canvas_hovered && m_selected_entity.valid() && ImGui::IsKeyPressed(ImGuiKey_D, false)) {
            if (m_world.has<Wire>(m_selected_entity)) {
                delete_wire(m_selected_entity);
            } else {
                delete_entity(m_selected_entity);
            }
        }

        // Render via RenderSystem
        // Convert ImGui coords to OpenGL (bottom-left origin)
        const int vx = static_cast<int>(origin.x - vp->Pos.x);
        const int vy = static_cast<int>(vp->Size.y - ((origin.y - vp->Pos.y) + content_size.y));
        const int vw = static_cast<int>(content_size.x);
        const int vh = static_cast<int>(content_size.y);

        m_render_system.render_region({static_cast<float>(vw), static_cast<float>(vh)}, vx, vy, vw, vh, m_dragging_entity);

        // Highlight selected module
        //if (m_selected_entity.valid()) {
        //    if (auto* pos = m_world.get<ModulePixelPosition>(m_selected_entity)) {
        //        if (auto* ext = m_world.get<ModuleExtent>(m_selected_entity)) {
        //            float w = static_cast<float>(ext->width * m_grid.unit_px());
        //            float h = static_cast<float>(ext->height * m_grid.unit_px());
        //            ImDrawList* dl = ImGui::GetWindowDrawList();
        //            ImVec2 p0(origin.x + pos->x, origin.y + pos->y);
        //            ImVec2 p1(p0.x + w, p0.y + h);
        //            dl->AddRect(p0, p1, IM_COL32(255, 220, 0, 255), 0.0f, 0, 2.0f);
        //        }
        //    }
        //}
    }
    ImGui::End();
}

// ... (toggle_wiring_mode)
void GateEditor::toggle_wiring_mode() {
    if (m_editor_state.mode == EditorMode::Wiring) {
        // Exit wiring mode, cancel any in-progress wire
        cancel_wire();
        m_editor_state.mode = EditorMode::Select;
    } else {
        // Enter wiring mode, clear selection
        m_selected_entity = Entity{};
        m_dragging_entity = Entity{};
        m_editor_state.mode = EditorMode::Wiring;
    }
}

void GateEditor::handle_wiring_click(GridCoord grid_pos) {
    auto& wiring = m_editor_state.wiring;

    // Helper to get the last point
    auto get_last_point = [&]() -> GridCoord {
        if (!wiring.points.empty()) {
            return wiring.points.back();
        }
        if (wiring.start_endpoint.valid()) {
            if (auto* pos = m_world.get<PortGridPosition>(wiring.start_endpoint)) {
                return pos->position;
            }
        }
        return grid_pos; // Should not happen if active
    };

    Entity port = find_port_at(grid_pos);
    Entity wire_point = find_wire_point_at(grid_pos);

    // Validation: Can we click here?
    // Must be a port OR a free cell (not blocked by module/wire).
    // Note: wire_point is valid too.
    bool is_blocked = m_layout_system.is_cell_blocked(grid_pos);
    bool is_valid_click = port.valid() || wire_point.valid() || !is_blocked;

    if (!is_valid_click) {
        return; // Clicked on module interior or obstacle
    }

    if (!wiring.active) {
        // Starting a new wire
        // STRICT: Must start from a valid input (Port or Junction).
        // User request: "wire creation should only start from a port" (or junction presumably)
        if (port.valid()) {
            wiring.active = true;
            wiring.start_endpoint = port;
            wiring.points.clear();
            wiring.current_path.clear();
        } else if (wire_point.valid()) {
            wiring.active = true;
            wiring.start_endpoint = wire_point;
            wiring.points.clear();
            wiring.current_path.clear();
        }
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

        // Check completion
        if (port.valid() || wire_point.valid()) {
            Entity endpoint = port.valid() ? port : wire_point;

            // Check if connecting to same module (invalid)
            if (port.valid() && wiring.start_endpoint.valid()) {
                if (are_ports_on_same_module(wiring.start_endpoint, port)) {
                    return;
                }
            }

            // Create wire
             // Create the wire entity
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
    Entity found = Entity{};
    m_world.view<Port, PortGridPosition>().each(
        [&](Entity e, Port&, PortGridPosition& pos) {
            if (pos.position == grid_pos) {
                found = e;
            }
        });
    return found;
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

