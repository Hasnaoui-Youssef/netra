#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

namespace netra::graphics {

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool should_close() const;
    void poll_events() const;
    void swap_buffers() const;

    int width() const { return m_width; }
    int height() const { return m_height; }
    GLFWwindow* handle() const { return m_window; }

    glm::vec2 get_cursor_pos() const;
    bool is_mouse_button_pressed(int button) const;

private:
    GLFWwindow* m_window = nullptr;
    int m_width;
    int m_height;

    static void framebuffer_callback(GLFWwindow* window, int width, int height);
};

} // namespace netra::graphics
