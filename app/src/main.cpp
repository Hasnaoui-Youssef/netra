#include <renderer.hpp>
#include <window.hpp>
#include <vector>
#include <cmath>

using namespace netra::graphics;

int main() {
    Window window(1024, 768, "Netra - Logic Gates");
    Renderer renderer;
    renderer.init("shaders/logic_gates");

    std::vector<Gate> gates = {
        {GateType::AND,  {50.0f,  50.0f}},
        {GateType::NAND, {200.0f, 50.0f}},
        {GateType::OR,   {350.0f, 50.0f}},
        {GateType::NOR,  {500.0f, 50.0f}},
        {GateType::XOR,  {50.0f,  200.0f}},
        {GateType::XNOR, {200.0f, 200.0f}},
        {GateType::NOT,  {350.0f, 200.0f}},
    };

    Gate* dragged_gate = nullptr;
    glm::vec2 drag_offset{0.0f};

    while (!window.should_close()) {
        window.poll_events();

        glm::vec2 mouse = window.get_cursor_pos();
        bool mouse_down = window.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);

        if (mouse_down) {
            if (!dragged_gate) {
                for (auto& gate : gates) {
                    if (mouse.x >= gate.position.x && 
                        mouse.x <= gate.position.x + gate.size.x &&
                        mouse.y >= gate.position.y && 
                        mouse.y <= gate.position.y + gate.size.y) {
                        dragged_gate = &gate;
                        dragged_gate->dragging = true;
                        drag_offset = mouse - gate.position;
                        break;
                    }
                }
            } else {
                glm::vec2 new_pos = mouse - drag_offset;
                new_pos.x = std::max(0.0f, std::min(new_pos.x, 
                    static_cast<float>(window.width()) - dragged_gate->size.x));
                new_pos.y = std::max(0.0f, std::min(new_pos.y, 
                    static_cast<float>(window.height()) - dragged_gate->size.y));
                dragged_gate->position = new_pos;
            }
        } else {
            if (dragged_gate) {
                dragged_gate->dragging = false;
                dragged_gate = nullptr;
            }
        }

        renderer.render(gates, window);
        window.swap_buffers();
    }

    return 0;
}
