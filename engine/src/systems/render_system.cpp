#include <array>
#include <components/components.hpp>
#include <components/render_components.hpp>
#include <graphics/camera2d.hpp>
#include <systems/render_system.hpp>

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <stdexcept>

namespace netra {

RenderSystem::RenderSystem(World &world, graphics::Grid &grid,
                           EditorState &editor)
    : m_world(world), m_grid(grid), m_editor(editor) {}

RenderSystem::~RenderSystem() {
  if (m_gate_vao)
    glDeleteVertexArrays(1, &m_gate_vao);
  if (m_gate_vbo)
    glDeleteBuffers(1, &m_gate_vbo);
  if (m_quad_vao)
    glDeleteVertexArrays(1, &m_quad_vao);
  if (m_quad_vbo)
    glDeleteBuffers(1, &m_quad_vbo);
  if (m_line_vao)
    glDeleteVertexArrays(1, &m_line_vao);
  if (m_line_vbo)
    glDeleteBuffers(1, &m_line_vbo);
}

void RenderSystem::init(const std::string &shader_dir) {
  // ... (init)
  setup_gate_quad();
  setup_quad();
  setup_wire_mesh();
  load_gate_shaders(shader_dir);

  // Simple solid color shaders for ports and wires
  const char *solid_vert = R"(
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

  const char *solid_frag = R"(
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
  std::array<float, 24> vertices = {
      // pos        // uv
      -1.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,
      0.0f,  0.0f, 1.0f, -1.0f, 1.0f,  0.0f,

      -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  -1.0f,
      1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  1.0f,
  };

  glGenVertexArrays(1, &m_gate_vao);
  glGenBuffers(1, &m_gate_vbo);

  glBindVertexArray(m_gate_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_gate_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        reinterpret_cast<void *>(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

void RenderSystem::setup_quad() {
  // Unit quad [0,1] x [0,1] for solid color primitives
  std::array<float, 12> vertices = {
      0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,

      0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
  };

  glGenVertexArrays(1, &m_quad_vao);
  glGenBuffers(1, &m_quad_vbo);

  glBindVertexArray(m_quad_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

void RenderSystem::load_gate_shaders(const std::string &shader_dir) {
  std::string vert_src = load_file(shader_dir + "/gates.vert");

  const auto gate_shaders =
      std::to_array<std::pair<const char *, const char *>>({
          {"AND", "/and.frag"},
          {"NAND", "/nand.frag"},
          {"OR", "/or.frag"},
          {"NOR", "/nor.frag"},
          {"XOR", "/xor.frag"},
          {"XNOR", "/xnor.frag"},
          {"NOT", "/not.frag"},
      });

  for (const auto &[key, frag_file] : gate_shaders) {
    std::string frag_src = load_file(shader_dir + frag_file);
    m_shaders[key] = graphics::Shader(vert_src, frag_src);
  }
}

void RenderSystem::render(glm::vec2 viewport_size, Entity dragging_module) {
  render_region(viewport_size, 0, 0, static_cast<int>(viewport_size.x),
                static_cast<int>(viewport_size.y), dragging_module);
}

void RenderSystem::render_region(glm::vec2 viewport_size, int x, int y,
                                 int width, int height,
                                 Entity dragging_module) {
  if (width <= 0 || height <= 0)
    return;

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

void RenderSystem::render_modules([[maybe_unused]] const glm::mat4 &view_proj,
                                  glm::vec2 viewport_size) {
  glBindVertexArray(m_gate_vao);

  m_world.view<ModuleInst, ModulePixelPosition, ModuleExtent, ShaderKey>().each(
      [this, &viewport_size](
          Entity, ModuleInst &, const ModulePixelPosition &pos,
          const ModuleExtent &extent, const ShaderKey &shader_key) {
        auto it = m_shaders.find(shader_key.key);
        if (it == m_shaders.end())
          return;

        const auto &shader = it->second;
        shader.use();

        // Convert extent from grid units to pixels
        auto width_px = static_cast<float>(extent.width * m_grid.unit_px());
        auto height_px = static_cast<float>(extent.height * m_grid.unit_px());

        // Gate shaders expect u_position (top-left in NDC) and u_size (in NDC)
        glm::vec2 ndc_pos =
            m_editor.camera.to_ndc({pos.x, pos.y}, viewport_size);
        glm::vec2 ndc_size{
            (width_px * m_editor.camera.zoom / viewport_size.x) * 2.0f,
            (height_px * m_editor.camera.zoom / viewport_size.y) * 2.0f};

        shader.set_vec2("u_position", ndc_pos);
        shader.set_vec2("u_size", ndc_size);

        glDrawArrays(GL_TRIANGLES, 0, 6);
      });

  glBindVertexArray(0);
}

void RenderSystem::render_ports(const glm::mat4 &view_proj,
                                [[maybe_unused]] glm::vec2 viewport_size,
                                Entity dragging_module) {
  glBindVertexArray(m_quad_vao);
  m_port_shader.use();
  m_port_shader.set_mat4("u_view_proj", view_proj);
  m_port_shader.set_vec4("u_color", {0.0f, 0.0f, 0.0f, 1.0f});

  auto port_size = static_cast<float>(m_grid.unit_px()) * 0.6f;

  m_world.view<Port, PortGridPosition>().each(
      [this, &dragging_module, &port_size](Entity, const Port &port,
                                           const PortGridPosition &grid_pos) {
        // Skip ports of dragging module
        if (dragging_module.valid() && port.owner == dragging_module) {
          return;
        }

        glm::vec2 pixel_pos = m_grid.to_glm_vec2(grid_pos.position);

        m_port_shader.set_vec2("u_position", {pixel_pos.x - port_size * 0.5f,
                                              pixel_pos.y - port_size * 0.5f});
        m_port_shader.set_vec2("u_size", {port_size, port_size});

        glDrawArrays(GL_TRIANGLES, 0, 6);
      });

  glBindVertexArray(0);
}
void RenderSystem::setup_wire_mesh() {
  glGenVertexArrays(1, &m_line_vao); // Reusing m_line names for wire mesh
  glGenBuffers(1, &m_line_vbo);

  glBindVertexArray(m_line_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_line_vbo);
  // Dynamic draw, size will change per frame
  glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

  // 2 floats per vertex (pos)
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

// ...

void RenderSystem::render_wires(const glm::mat4 &view_proj,
                                [[maybe_unused]] glm::vec2 viewport_size) {
  m_wire_shader.use();
  m_wire_shader.set_mat4("u_view_proj", view_proj);
  m_wire_shader.set_vec4("u_color", {0.2f, 0.8f, 0.2f, 1.0f});
  // Identity transform, vertices are in world space
  m_wire_shader.set_vec2("u_position", {0.0f, 0.0f});
  m_wire_shader.set_vec2("u_size", {1.0f, 1.0f});

  glBindVertexArray(m_line_vao);

  // Simple buckets: y -> list of HSegments at that y
  // x -> list of VSegments at that x
  // Given the grid nature, this is Map<int, vector>.
  WireSegments segments;
  // 1. Collect all segments (Committed + Preview)

  // Committed
  m_world.view<Wire>().each([this, &segments](Entity e, Wire &wire) {
    collect_committed_segments(segments, e, wire);
  });

  // Preview only if start_point is valid
  if (m_editor.wiring.active && m_editor.wiring.start_endpoint.valid()) {
    // Reconstruct preview points same way as render logic
    std::vector<GridCoord> preview_pts;
    if (auto const *pos =
            m_world.get<PortGridPosition>(m_editor.wiring.start_endpoint)) {
      preview_pts.push_back(pos->position);
      // GateEditor will populate it via A* in
      // handle_wiring_click (wait, handle_wiring uses it for commit). The
      // preview path needs to be calculated by GateEditor frame-by-frame or
      // stored in EditorState. Assumption: GateEditor will update wiring.points
      // with the A* path dynamically.
      preview_pts.insert(preview_pts.end(), m_editor.wiring.points.begin(),
                         m_editor.wiring.points.end());
      // Note: we treat preview as a "wire" with null owner entity for crossing
      // checks (owner check handles valid only)
      segments.add_segments(preview_pts, Entity{});
    }
  }

  // 2. Generate Mesh
  std::vector<float> vertices;
  const float thickness = 3.0f; // Pixels
  const float half_th = thickness * 0.5f;
  const auto unit_px = static_cast<float>(m_grid.unit_px());

  auto add_rect = [&](glm::vec2 p1, glm::vec2 p2) {
    glm::vec2 dir = glm::normalize(p2 - p1);
    glm::vec2 perp = {-dir.y, dir.x};
    glm::vec2 offset = perp * half_th;

    // 4 corners
    glm::vec2 c1 = p1 + offset;
    glm::vec2 c2 = p1 - offset;
    glm::vec2 c3 = p2 - offset;
    glm::vec2 c4 = p2 + offset;

    // 2 triangles
    vertices.insert(vertices.end(), {c1.x, c1.y, c2.x, c2.y, c3.x, c3.y});
    vertices.insert(vertices.end(), {c1.x, c1.y, c3.x, c3.y, c4.x, c4.y});
  };

  auto add_arc = [&](glm::vec2 center) {
    // Simple semi-circle arc "jumping" over the horizontal wire
    // Center is the crossing point
    const int segs = 8;
    float radius = unit_px * 0.5f; // half grid unit radius
    // Vertical wire means blocking horizontal wire.
    // We are drawing the vertical wire. We want to jump over.
    // The jump is along Y axis (along wire) and bumps in Z? No 2D.
    // Bumps in X? "Arc like visual... similar to electrical schematics".
    // Usually, vertical line breaks and curves around.
    // Let's assume vertical line goes Down.
    // Arc goes to the Right (convex).

    // We break the straight line.
    // Actually, easiest is to just draw a sprite or generate geometry.
    // Geometry: Half circle to the right.
    glm::vec2 start{center.x, center.y - radius};
    glm::vec2 end{center.x, center.y + radius};

    // Approximate arc
    float angle_step = std::numbers::pi_v<float> / segs;
    glm::vec2 prev = start;
    for (int i = 1; i <= segs; ++i) {
      float angle = (-std::numbers::pi_v<float> / 2.0f) +
                    (static_cast<float>(i) * angle_step);
      glm::vec2 curr{center.x + cos(angle) * radius * 0.5f,
                     center.y + sin(angle) * radius};
      add_rect(prev, curr);
      prev = curr;
    }
  };

  auto process_points = [&](const std::vector<GridCoord> &pts, Entity owner) {
    if (pts.size() < 2)
      return;
    for (size_t i = 0; i < pts.size() - 1; ++i) {
      GridCoord p1 = pts[i];
      GridCoord p2 = pts[i + 1];

      // Draw segment p1 -> p2
      // Check for crossings if Vertical
      bool is_ver = (p1.x == p2.x);

      if (is_ver) {
        int x = p1.x;
        int y_min = std::min(p1.y, p2.y);
        int y_max = std::max(p1.y, p2.y);

        // Collect crossings
        std::vector<int> crossings;
        for (int y = y_min + 1; y < y_max; ++y) {
          if (segments.h_segments.contains(y)) {
            for (const auto &seg : segments.h_segments[y]) {
              // Check overlap
              if (x > seg.min_x && x < seg.max_x) {
                // Determine if we should arc
                // Rule: "Start point cannot be inside module... two wires
                // cross... arc" Owner check: if same owner, maybe don't arc?
                // Usually distinct wires.
                if (!owner.valid() || !seg.owner.valid() ||
                    !(owner == seg.owner)) {
                  crossings.push_back(y);
                }
              }
            }
          }
        }
        std::sort(crossings.begin(), crossings.end());

        // Draw segments split by crossings
        glm::vec2 curr_start = m_grid.to_glm_vec2(p1);
        glm::vec2 final_end = m_grid.to_glm_vec2(p2);

        // Ensure we traverse in direction of p1->p2
        bool forward = (p2.y > p1.y);

        if (!crossings.empty()) {
          glm::vec2 pos = curr_start;
          // Iterate crossings in order of propagation
          if (!forward)
            std::reverse(crossings.begin(), crossings.end());

          for (int y_cross : crossings) {
            glm::vec2 cross_pt = m_grid.to_glm_vec2({x, y_cross});
            float radius = unit_px * 0.5f;

            // Stop before crossing
            glm::vec2 stop_pt = cross_pt;
            stop_pt.y += (forward ? -radius : radius);

            add_rect(pos, stop_pt);

            // Draw arc
            add_arc(cross_pt);

            // Restart after crossing
            pos = cross_pt;
            pos.y += (forward ? radius : -radius);
          }
          // Final segment
          add_rect(pos, final_end);
        } else {
          add_rect(curr_start, final_end);
        }
      } else {
        // Horizontal: just draw
        add_rect(m_grid.to_glm_vec2(p1), m_grid.to_glm_vec2(p2));
      }
    }
  };

  // Generate for committed
  m_world.view<Wire>().each([&](Entity e, Wire &wire) {
    std::vector<GridCoord> full_points;
    if (wire.from_endpoint.valid()) {
      if (auto *pos = m_world.get<PortGridPosition>(wire.from_endpoint))
        full_points.push_back(pos->position);
    }
    full_points.insert(full_points.end(), wire.points.begin(),
                       wire.points.end());
    if (wire.to_endpoint.valid()) {
      if (auto *pos = m_world.get<PortGridPosition>(wire.to_endpoint))
        full_points.push_back(pos->position);
    }
    process_points(full_points, e);
  });

  // Generate for Preview (Different color handled by separate draw call? No,
  // logic is mixed now.) We'll draw preview as SAME color for now, or we can
  // flush buffer and change color.

  if (!vertices.empty()) {
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size()) / 2);
  }

  if (m_editor.wiring.active) {
    vertices.clear();
    m_wire_shader.set_vec4("u_color",
                           {0.5f, 0.8f, 1.0f, 0.8f}); // Preview color

    std::vector<GridCoord> preview_pts;
    if (m_editor.wiring.start_endpoint.valid()) {
      if (auto const *pos =
              m_world.get<PortGridPosition>(m_editor.wiring.start_endpoint)) {
        preview_pts.push_back(pos->position);
      }
    }
    preview_pts.insert(preview_pts.end(), m_editor.wiring.points.begin(),
                       m_editor.wiring.points.end());
    preview_pts.insert(preview_pts.end(), m_editor.wiring.current_path.begin(),
                       m_editor.wiring.current_path.end());

    process_points(preview_pts, Entity{});

    if (!vertices.empty()) {
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                   vertices.data(), GL_DYNAMIC_DRAW);
      glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size() / 2));
    }
  }

  glBindVertexArray(0);
}

void RenderSystem::collect_committed_segments(WireSegments &segments, Entity e,
                                              const Wire &wire) {
  std::vector<GridCoord> full_points;
  if (wire.from_endpoint.valid()) {
    if (auto const *pos = m_world.get<PortGridPosition>(wire.from_endpoint))
      full_points.push_back(pos->position);
  }
  full_points.insert(full_points.end(), wire.points.begin(), wire.points.end());
  if (wire.to_endpoint.valid()) {
    if (auto const *pos = m_world.get<PortGridPosition>(wire.to_endpoint))
      full_points.push_back(pos->position);
  }
  segments.add_segments(full_points, e);
}

std::string RenderSystem::load_file(const std::string &path) {
  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Failed to open file: " + path);
  }
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

} // namespace netra
