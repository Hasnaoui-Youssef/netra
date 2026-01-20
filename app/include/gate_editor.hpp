#pragma once

#include <gates.hpp>
#include <renderer.hpp>
#include <window.hpp>

#include <imgui.h>
#include <vector>

namespace netra::app {

class GateEditor {
public:
    GateEditor() = default;

    void draw(netra::graphics::Window& window,
              netra::graphics::Renderer& renderer,
              std::vector<netra::graphics::Gate>& gates);

private:
    netra::GateType m_selected = netra::GateType::AND;

    int m_selected_gate = -1;
    int m_dragging_gate = -1;
    float m_palette_width = 220.f;
    ImVec2 m_drag_offset{0.0f, 0.0f};
    bool m_canvas_hovered = false;
};

} // namespace netra::app
