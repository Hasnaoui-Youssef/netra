#pragma once

#include "shader.hpp"
#include "window.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <array>

namespace netra::graphics {

enum class GateType {
    AND,
    NAND,
    OR,
    NOR,
    XOR,
    XNOR,
    NOT,
    COUNT
};

struct Gate {
    GateType type;
    glm::vec2 position;
    glm::vec2 size{100.0f, 80.0f};
    bool dragging = false;
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init(const std::string& shader_dir);
    void render(const std::vector<Gate>& gates, const Window& window);

private:
    std::array<Shader, static_cast<size_t>(GateType::COUNT)> m_shaders;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;

    void setup_quad();
    std::string load_file(const std::string& path);
};

} // namespace netra::graphics
