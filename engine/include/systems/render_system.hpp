#pragma once

#include <core/world.hpp>
#include <editor_state.hpp>
#include <graphics/grid.hpp>
#include <graphics/shader.hpp>

#include <glad.h>
#include <glm/vec2.hpp>
#include <string>
#include <unordered_map>

namespace netra {

// ECS-driven render system.
// Iterates world components and issues OpenGL draw calls.
class RenderSystem {
public:
    RenderSystem(World& world, graphics::Grid& grid, EditorState& editor);
    ~RenderSystem();

    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;
    RenderSystem(RenderSystem&&) = delete;
    RenderSystem& operator=(RenderSystem&&) = delete;

    void init(const std::string& shader_dir);
    void render(glm::vec2 viewport_size);
    void render_region(glm::vec2 viewport_size, int x, int y, int width, int height);

private:
    World& m_world;
    graphics::Grid& m_grid;
    EditorState& m_editor;

    // Quad geometry (for modules and ports)
    GLuint m_quad_vao = 0;
    GLuint m_quad_vbo = 0;

    // Line geometry (for wires)
    GLuint m_line_vao = 0;
    GLuint m_line_vbo = 0;

    // Shaders keyed by ShaderKey::key (e.g., "AND", "OR")
    std::unordered_map<std::string, graphics::Shader> m_shaders;

    // Simple shaders for ports and wires
    graphics::Shader m_port_shader;
    graphics::Shader m_wire_shader;

    void setup_quad();
    void setup_line();
    void load_gate_shaders(const std::string& shader_dir);

    void render_modules(const glm::mat4& view_proj, glm::vec2 viewport_size);
    void render_ports(const glm::mat4& view_proj, glm::vec2 viewport_size);
    void render_wires(const glm::mat4& view_proj, glm::vec2 viewport_size);

    std::string load_file(const std::string& path);
};

} // namespace netra
