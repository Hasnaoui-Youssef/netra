#include "renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace netra::graphics {

Renderer::Renderer() = default;

Renderer::~Renderer() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}

void Renderer::init(const std::string& shader_dir) {
    setup_quad();

    std::string vert_src = load_file(shader_dir + "/gates.vert");
    
    const char* frag_files[] = {
        "/and.frag", "/nand.frag", "/or.frag", "/nor.frag",
        "/xor.frag", "/xnor.frag", "/not.frag"
    };

    for (size_t i = 0; i < static_cast<size_t>(GateType::COUNT); ++i) {
        std::string frag_src = load_file(shader_dir + frag_files[i]);
        m_shaders[i] = Shader(vert_src, frag_src);
    }
}

void Renderer::render(const std::vector<Gate>& gates, const Window& window) {
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(m_vao);

    for (const auto& gate : gates) {
        auto& shader = m_shaders[static_cast<size_t>(gate.type)];
        shader.use();

        float x = (gate.position.x / window.width()) * 2.0f - 1.0f;
        float y = 1.0f - (gate.position.y / window.height()) * 2.0f;
        float w = (gate.size.x / window.width()) * 2.0f;
        float h = (gate.size.y / window.height()) * 2.0f;

        shader.set_vec2("u_position", {x, y});
        shader.set_vec2("u_size", {w, h});

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);
}

void Renderer::setup_quad() {
    float vertices[] = {
        // pos        // uv
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

std::string Renderer::load_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

} // namespace netra::graphics
