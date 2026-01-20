#pragma once

#include <gates.hpp>
#include "shader.hpp"
#include "window.hpp"
#include <vector>
#include <array>

namespace netra::graphics {

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init(const std::string& shader_dir);
    void render(const std::vector<Gate>& gates, const Window& window);
    void render_region(const std::vector<Gate>& gates, int x, int y, int width, int height);

private:
    std::array<Shader, static_cast<size_t>(GateType::COUNT)> m_shaders;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;

    void setup_quad();
    std::string load_file(const std::string& path);
};

} // namespace netra::graphics
