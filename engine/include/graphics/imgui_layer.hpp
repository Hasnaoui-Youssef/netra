#pragma once

#include "window.hpp"

namespace netra::graphics {

class ImGuiLayer {
public:
    ImGuiLayer() = default;
    ~ImGuiLayer() = default;

    void init(Window& window);
    void begin_frame();
    void end_frame();
    void shutdown();
};

} // namespace netra::graphics
