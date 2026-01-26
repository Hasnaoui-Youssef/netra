#pragma once

#include "core/entity.hpp"
#include <components/render_components.hpp>
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
  RenderSystem(World &world, graphics::Grid &grid, EditorState &editor);
  ~RenderSystem();

  RenderSystem(const RenderSystem &) = delete;
  RenderSystem &operator=(const RenderSystem &) = delete;
  RenderSystem(RenderSystem &&) = delete;
  RenderSystem &operator=(RenderSystem &&) = delete;

  void init(const std::string &shader_dir);
  void render(glm::vec2 viewport_size, Entity dragging_module = Entity{});
  void render_region(glm::vec2 viewport_size, int x, int y, int width,
                     int height, Entity dragging_module = Entity{});

private:
  World &m_world;
  graphics::Grid &m_grid;
  EditorState &m_editor;

  // Gate quad: [-1,1] with UVs for SDF shaders
  GLuint m_gate_vao = 0;
  GLuint m_gate_vbo = 0;

  // Simple quad: [0,1] for solid color primitives
  GLuint m_quad_vao = 0;
  GLuint m_quad_vbo = 0;

  // Line geometry (for wires)
  GLuint m_line_vao = 0;
  GLuint m_line_vbo = 0;

  // Shaders keyed by ShaderKey::key (e.g., "AND", "OR")
  std::unordered_map<std::string, graphics::Shader> m_shaders;

  //Wire triangle vertices
  std::vector<float> triangle_vertices;
  //Wire thickness and half thickness in pixels
  constexpr static float thickness = 3.0f;
  constexpr static float half_th = thickness * 0.5f;


  // Simple shaders for ports and wires
  graphics::Shader m_port_shader;
  graphics::Shader m_wire_shader;

  void setup_gate_quad();
  void setup_quad();
  void setup_wire_mesh();
  void load_gate_shaders(const std::string &shader_dir);

  void render_modules(const glm::mat4 &view_proj, glm::vec2 viewport_size);
  void render_ports(const glm::mat4 &view_proj, glm::vec2 viewport_size,
                    Entity dragging_module);
  void render_wires(const glm::mat4 &view_proj, glm::vec2 viewport_size);
  void collect_committed_segments(WireSegments &segments, Entity e,
                                  const Wire &wire);
  void collect_preview_segments(WireSegments &segments, Entity e,
                                const Wire &wire);

  void add_orthogonal_wire_vertices(glm::vec2 p1, glm::vec2 p2);

  std::string load_file(const std::string &path);
};

} // namespace netra
