#include <systems/render_system.hpp>
#include <components/components.hpp>
#include <components/render_components.hpp>
#include <graphics/camera2d.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace netra {

RenderSystem::RenderSystem(World& world, graphics::Grid& grid, EditorState& editor)
    : m_world(world)
    , m_grid(grid)
    , m_editor(editor) {}

RenderSystem::~RenderSystem() {
    if (m_gate_vao) glDeleteVertexArrays(1, &m_gate_vao);
    if (m_gate_vbo) glDeleteBuffers(1, &m_gate_vbo);
    if (m_quad_vao) glDeleteVertexArrays(1, &m_quad_vao);
    if (m_quad_vbo) glDeleteBuffers(1, &m_quad_vbo);
    if (m_line_vao) glDeleteVertexArrays(1, &m_line_vao);
    if (m_line_vbo) glDeleteBuffers(1, &m_line_vbo);
}

void RenderSystem::init(const std::string& shader_dir) {
    setup_gate_quad();
    setup_quad();
    setup_line();
    load_gate_shaders(shader_dir);

    // Simple solid color shaders for ports and wires
    const char* solid_vert = R"(
        #version 430 core
        layout (location = 0) in vec2 aPos;
        uniform mat4 u_view_proj;
        uniform vec2 u_position;
        uniform vec2 u_size;
        void main() {
            vec2 world_pos = aPos * u_size + u_position;
            gl_Position = u_view_proj * vec4(world_pos, 0.0, 1.0);
        }
    )";

    const char* solid_frag = R"(
        #version 430 core
        out vec4 FragColor;
        uniform vec4 u_color;
        void main() {
            FragColor = u_color;
        }
    )";

    m_port_shader = graphics::Shader(solid_vert, solid_frag);
    m_wire_shader = graphics::Shader(solid_vert, solid_frag);
}

void RenderSystem::setup_gate_quad() {
    // Fullscreen quad [-1,1] with UVs for SDF gate shaders
    float vertices[] = {
        // pos        // uv
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &m_gate_vao);
    glGenBuffers(1, &m_gate_vbo);

    glBindVertexArray(m_gate_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_gate_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void RenderSystem::setup_quad() {
    // Unit quad [0,1] x [0,1] for solid color primitives
    float vertices[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    };

    glGenVertexArrays(1, &m_quad_vao);
    glGenBuffers(1, &m_quad_vbo);

    glBindVertexArray(m_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void RenderSystem::setup_line() {
    glGenVertexArrays(1, &m_line_vao);
    glGenBuffers(1, &m_line_vbo);

    glBindVertexArray(m_line_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_line_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void RenderSystem::load_gate_shaders(const std::string& shader_dir) {
    std::string vert_src = load_file(shader_dir + "/gates.vert");

    const std::pair<const char*, const char*> gate_shaders[] = {
        {"AND",  "/and.frag"},
        {"NAND", "/nand.frag"},
        {"OR",   "/or.frag"},
        {"NOR",  "/nor.frag"},
        {"XOR",  "/xor.frag"},
        {"XNOR", "/xnor.frag"},
        {"NOT",  "/not.frag"},
    };

    for (const auto& [key, frag_file] : gate_shaders) {
        std::string frag_src = load_file(shader_dir + frag_file);
        m_shaders[key] = graphics::Shader(vert_src, frag_src);
    }
}

void RenderSystem::render(glm::vec2 viewport_size, Entity dragging_module) {
    render_region(viewport_size, 0, 0,
                  static_cast<int>(viewport_size.x),
                  static_cast<int>(viewport_size.y),
                  dragging_module);
}

void RenderSystem::render_region(glm::vec2 viewport_size, int x, int y, int width, int height, Entity dragging_module) {
    if (width <= 0 || height <= 0) return;

    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);

    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 view_proj = m_editor.camera.view_projection(viewport_size);

    render_modules(view_proj, viewport_size);
    render_wires(view_proj, viewport_size);
    render_ports(view_proj, viewport_size, dragging_module);

    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
}

void RenderSystem::render_modules(const glm::mat4& view_proj, glm::vec2 viewport_size) {
    glBindVertexArray(m_gate_vao);

    m_world.view<ModuleInst, ModulePixelPosition, ModuleExtent, ShaderKey>().each(
        [&](Entity, ModuleInst&, ModulePixelPosition& pos, ModuleExtent& extent, ShaderKey& shader_key) {
            auto it = m_shaders.find(shader_key.key);
            if (it == m_shaders.end()) return;

            auto& shader = it->second;
            shader.use();

            // Convert extent from grid units to pixels
            float width_px = static_cast<float>(extent.width * m_grid.unit_px());
            float height_px = static_cast<float>(extent.height * m_grid.unit_px());

            // Gate shaders expect u_position (top-left in NDC) and u_size (in NDC)
            glm::vec2 ndc_pos = m_editor.camera.to_ndc({pos.x, pos.y}, viewport_size);
            glm::vec2 ndc_size{
                (width_px * m_editor.camera.zoom / viewport_size.x) * 2.0f,
                (height_px * m_editor.camera.zoom / viewport_size.y) * 2.0f
            };

            shader.set_vec2("u_position", ndc_pos);
            shader.set_vec2("u_size", ndc_size);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    );

    glBindVertexArray(0);
}

void RenderSystem::render_ports(const glm::mat4& view_proj, glm::vec2 viewport_size, Entity dragging_module) {
    glBindVertexArray(m_quad_vao);
    m_port_shader.use();
    m_port_shader.set_mat4("u_view_proj", view_proj);
    m_port_shader.set_vec4("u_color", {0.0f, 0.0f, 0.0f, 1.0f});

    float port_size = static_cast<float>(m_grid.unit_px()) * 0.6f;

    m_world.view<Port, PortGridPosition>().each(
        [&](Entity, Port& port, PortGridPosition& grid_pos) {
            // Skip ports of dragging module
            if (dragging_module.valid() && port.owner == dragging_module) {
                return;
            }

            glm::vec2 pixel_pos = m_grid.to_glm_vec2(grid_pos.position);

            m_port_shader.set_vec2("u_position", {pixel_pos.x - port_size * 0.5f,
                                                   pixel_pos.y - port_size * 0.5f});
            m_port_shader.set_vec2("u_size", {port_size, port_size});

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    );

    glBindVertexArray(0);
}

void RenderSystem::render_wires(const glm::mat4& view_proj, glm::vec2 viewport_size) {
    m_wire_shader.use();
    m_wire_shader.set_mat4("u_view_proj", view_proj);
    m_wire_shader.set_vec4("u_color", {0.2f, 0.8f, 0.2f, 1.0f});
    m_wire_shader.set_vec2("u_position", {0.0f, 0.0f});
    m_wire_shader.set_vec2("u_size", {1.0f, 1.0f});

    glBindVertexArray(m_line_vao);

    // Render committed wires
    m_world.view<Wire>().each(
        [&](Entity, Wire& wire) {
            std::vector<glm::vec2> points;

            // Start endpoint (port or junction)
            if (wire.from_endpoint.valid()) {
                if (auto* pos = m_world.get<PortGridPosition>(wire.from_endpoint)) {
                    points.push_back(m_grid.to_glm_vec2(pos->position));
                }
            }

            for (const auto& grid_pt : wire.points) {
                points.push_back(m_grid.to_glm_vec2(grid_pt));
            }

            // End endpoint (port or junction)
            if (wire.to_endpoint.valid()) {
                if (auto* pos = m_world.get<PortGridPosition>(wire.to_endpoint)) {
                    points.push_back(m_grid.to_glm_vec2(pos->position));
                }
            }

            if (points.size() < 2) return;

            glBindBuffer(GL_ARRAY_BUFFER, m_line_vbo);
            glBufferData(GL_ARRAY_BUFFER,
                         static_cast<GLsizeiptr>(points.size() * sizeof(glm::vec2)),
                         points.data(),
                         GL_DYNAMIC_DRAW);

            glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(points.size()));
        }
    );

    // Render in-progress wire (preview)
    if (m_editor.wiring.active) {
        m_wire_shader.set_vec4("u_color", {0.5f, 0.8f, 1.0f, 0.8f}); // Distinct preview color

        std::vector<glm::vec2> preview_points;

        // Start from endpoint if available
        if (m_editor.wiring.start_endpoint.valid()) {
            if (auto* pos = m_world.get<PortGridPosition>(m_editor.wiring.start_endpoint)) {
                preview_points.push_back(m_grid.to_glm_vec2(pos->position));
            }
        }

        for (const auto& grid_pt : m_editor.wiring.points) {
            preview_points.push_back(m_grid.to_glm_vec2(grid_pt));
        }

        // Add orthogonal rubber band to mouse position
        GridCoord mouse_pos = m_editor.wiring.mouse_grid_pos;
        if (!preview_points.empty()) {
            // Get last point in grid coords
            GridCoord last_grid{};
            if (!m_editor.wiring.points.empty()) {
                last_grid = m_editor.wiring.points.back();
            } else if (m_editor.wiring.start_endpoint.valid()) {
                if (auto* pos = m_world.get<PortGridPosition>(m_editor.wiring.start_endpoint)) {
                    last_grid = pos->position;
                }
            }

            // Add orthogonal corner if needed (horizontal first, then vertical)
            if (last_grid.x != mouse_pos.x && last_grid.y != mouse_pos.y) {
                GridCoord corner{mouse_pos.x, last_grid.y};
                preview_points.push_back(m_grid.to_glm_vec2(corner));
            }
        }
        preview_points.push_back(m_grid.to_glm_vec2(mouse_pos));

        if (preview_points.size() >= 2) {
            glBindBuffer(GL_ARRAY_BUFFER, m_line_vbo);
            glBufferData(GL_ARRAY_BUFFER,
                         static_cast<GLsizeiptr>(preview_points.size() * sizeof(glm::vec2)),
                         preview_points.data(),
                         GL_DYNAMIC_DRAW);

            glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(preview_points.size()));
        }
    }

    glBindVertexArray(0);
}

std::string RenderSystem::load_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

} // namespace netra
