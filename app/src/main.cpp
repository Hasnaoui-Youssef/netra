#include <gate_editor.hpp>

#include <imgui_layer.hpp>
#include <renderer.hpp>
#include <window.hpp>

#include <glad/glad.h>

#include <vector>

int main() {
    netra::graphics::Window window(1280, 720, "Netra");

    netra::graphics::Renderer renderer;
    renderer.init("shaders/logic_gates");

    netra::graphics::ImGuiLayer imgui;
    imgui.init(window);

    netra::app::GateEditor editor;

    std::vector<netra::graphics::Gate> gates;

    while (!window.should_close()) {
        window.poll_events();

        glViewport(0, 0, window.width(), window.height());
        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        imgui.begin_frame();
        editor.draw(window, renderer, gates);
        imgui.end_frame();

        window.swap_buffers();
    }

    imgui.shutdown();
    return 0;
}
