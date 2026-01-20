#include <gate_editor.hpp>

#include <graphics/imgui_layer.hpp>
#include <graphics/window.hpp>

#include <glad/glad.h>

int main() {
    netra::graphics::Window window(1280, 720, "Netra");

    netra::graphics::ImGuiLayer imgui;
    imgui.init(window);

    netra::app::GateEditor editor;
    editor.init("shaders/logic_gates");

    while (!window.should_close()) {
        window.poll_events();

        glViewport(0, 0, window.width(), window.height());
        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        imgui.begin_frame();
        editor.draw(window);
        imgui.end_frame();

        window.swap_buffers();
    }

    imgui.shutdown();
    return 0;
}
