#pragma once

#include <graphics/window.hpp>

#include <imgui.h>

namespace netra::app {

// Placeholder editor - old renderer-based code removed.
// Will be rewritten to use ECS + RenderSystem.
class GateEditor {
public:
    GateEditor() = default;

    void draw(netra::graphics::Window& window);

private:
    float m_palette_width = 220.f;
};

} // namespace netra::app
