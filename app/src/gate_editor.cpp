#include <gate_editor.hpp>

#include <imgui.h>

namespace netra::app {

static const char* gate_type_label(netra::graphics::GateType type) {
    using netra::graphics::GateType;
    switch (type) {
        case GateType::AND: return "AND";
        case GateType::NAND: return "NAND";
        case GateType::OR: return "OR";
        case GateType::NOR: return "NOR";
        case GateType::XOR: return "XOR";
        case GateType::XNOR: return "XNOR";
        case GateType::NOT: return "NOT";
        default: return "UNKNOWN";
    }
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

    if (ImGui::BeginMenu("Terminal")) {
        ImGui::MenuItem("Toggle terminal");
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void GateEditor::draw(netra::graphics::Window& window,
                     netra::graphics::Renderer& renderer,
                     std::vector<netra::graphics::Gate>& gates) {
    begin_top_menu_bar();

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float menu_h = ImGui::GetFrameHeight();
    m_palette_width = ImGui::GetWindowWidth();


    const ImVec2 canvas_pos(vp->Pos.x + m_palette_width, vp->Pos.y + menu_h);
    const ImVec2 canvas_size(vp->Size.x - m_palette_width, vp->Size.y - menu_h);


    /* Palette setup */
    ImGui::SetNextWindowPos(ImVec2(vp->Pos.x, vp->Pos.y + menu_h), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(m_palette_width, vp->Size.y - menu_h), ImGuiCond_Always);
    ImGui::SetNextWindowSizeConstraints(ImVec2(m_palette_width, vp->Size.y - menu_h), ImVec2(m_palette_width * 2, vp->Size.y - menu_h), nullptr, nullptr);
    ImGuiWindowFlags fixed_flags =
        ImGuiWindowFlags_NoMove |
        // ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar;

    if (ImGui::Begin("Palette", nullptr, fixed_flags)) {
        using netra::graphics::GateType;
        static const GateType types[] = {
            GateType::AND, GateType::NAND, GateType::OR, GateType::NOR,
            GateType::XOR, GateType::XNOR, GateType::NOT
        };

        for (auto t : types) {
            ImGui::Selectable(gate_type_label(t), false);

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (canvas_size.x > 0.0f && canvas_size.y > 0.0f) {
                    netra::graphics::Gate g{t, {canvas_size.x * 0.5f, canvas_size.y * 0.5f}};
                    if (t == GateType::XOR || t == GateType::XNOR) {
                        g.size = {120.0f, 80.0f};
                    }
                    g.position.x -= g.size.x * 0.5f;
                    g.position.y -= g.size.y * 0.5f;
                    gates.push_back(g);
                    m_selected_gate = static_cast<int>(gates.size()) - 1;
                    m_dragging_gate = -1;
                }
            }

            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                m_selected = t;
                ImGui::SetDragDropPayload("NETRA_GATE", &m_selected, sizeof(m_selected));
                ImGui::Text("%s", gate_type_label(t));
                ImGui::EndDragDropSource();
            }
        }

        ImGui::Separator();
        ImGui::TextUnformatted("Drag a gate into the canvas.");
        ImGui::TextUnformatted("Press 'd' to delete selected (canvas hover).");
    }
    ImGui::End();

    /* Canvas setup */
    ImGui::SetNextWindowPos(canvas_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(canvas_size, ImGuiCond_Always);
    ImGuiWindowFlags canvasFlags =
        fixed_flags |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoBackground;

    if (ImGui::Begin("Canvas", nullptr, canvasFlags)) {
        const ImVec2 canvas_size = ImGui::GetContentRegionAvail();
        if (canvas_size.x <= 0.0f || canvas_size.y <= 0.0f) {
            ImGui::TextUnformatted("Canvas has no space.");
            ImGui::End();
            return;
        }

        ImGui::InvisibleButton("canvas", canvas_size, ImGuiButtonFlags_MouseButtonLeft);
        m_canvas_hovered = ImGui::IsItemHovered();

        const ImVec2 origin = ImGui::GetItemRectMin();
        const ImVec2 mouse = ImGui::GetIO().MousePos;

        // Drop target: place gate at mouse
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NETRA_GATE")) {
                auto t = *static_cast<const netra::graphics::GateType*>(payload->Data);
                netra::graphics::Gate g{t, {mouse.x - origin.x, mouse.y - origin.y}};
                if (t == netra::graphics::GateType::XOR || t == netra::graphics::GateType::XNOR) {
                    g.size = {120.0f, 80.0f};
                }
                g.position.x -= g.size.x * 0.5f;
                g.position.y -= g.size.y * 0.5f;
                gates.push_back(g);
            }
            ImGui::EndDragDropTarget();
        }

        // Selection + dragging
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_canvas_hovered) {
            m_selected_gate = -1;
            for (int i = static_cast<int>(gates.size()) - 1; i >= 0; --i) {
                const auto& g = gates[static_cast<std::size_t>(i)];
                const float gx = origin.x + g.position.x;
                const float gy = origin.y + g.position.y;
                if (mouse.x >= gx && mouse.x <= gx + g.size.x && mouse.y >= gy && mouse.y <= gy + g.size.y) {
                    m_selected_gate = i;
                    m_dragging_gate = i;
                    m_drag_offset = ImVec2(mouse.x - gx, mouse.y - gy);
                    break;
                }
            }
        }

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_dragging_gate >= 0) {
            auto& g = gates[static_cast<std::size_t>(m_dragging_gate)];
            g.position.x = (mouse.x - origin.x) - m_drag_offset.x;
            g.position.y = (mouse.y - origin.y) - m_drag_offset.y;
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            m_dragging_gate = -1;
        }

        // Delete selected with 'd' when canvas is hovered
        if (m_canvas_hovered && m_selected_gate >= 0 && ImGui::IsKeyPressed(ImGuiKey_D, false)) {
            gates.erase(gates.begin() + m_selected_gate);
            m_selected_gate = -1;
            m_dragging_gate = -1;
        }

        // Shader based gates rendering.
        // ImGui uses top-left origin; OpenGL viewport/scissor uses bottom-left.
        // Use only ImGui viewport coordinates here to avoid framebuffer/window-size desync during resize.
        const int vx = static_cast<int>(origin.x - vp->Pos.x);
        const int vy = static_cast<int>(vp->Size.y - ((origin.y - vp->Pos.y) + canvas_size.y));
        const int vw = static_cast<int>(canvas_size.x);
        const int vh = static_cast<int>(canvas_size.y);
        renderer.render_region(gates, vx, vy, vw, vh);

        // Highlight selected gate (outline overlay)
        if (m_selected_gate >= 0 && m_selected_gate < static_cast<int>(gates.size())) {
            auto& g = gates[static_cast<std::size_t>(m_selected_gate)];
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 p0(origin.x + g.position.x, origin.y + g.position.y);
            ImVec2 p1(p0.x + g.size.x, p0.y + g.size.y);
            dl->AddRect(p0, p1, IM_COL32(255, 220, 0, 255), 6.0f, 0, 2.0f);
        }
    }
    ImGui::End();
}

} // namespace netra::app
