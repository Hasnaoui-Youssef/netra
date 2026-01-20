#pragma once

#include <grid_coord.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace netra::graphics {

class Grid;

// 2D orthographic camera for canvas viewing.
// Handles pan and zoom; converts pixel coordinates to NDC.
struct Camera2D {
    glm::vec2 pan{0.0f, 0.0f};  // offset in pixels (positive = content moves right/down)
    float zoom = 1.0f;          // multiplier (1.0 = 100%, >1 = zoom in)

    // Convert pixel position to NDC given viewport size.
    [[nodiscard]] glm::vec2 to_ndc(glm::vec2 pixel_pos, glm::vec2 viewport_size) const noexcept;

    // Convenience: grid coord → NDC (composes with Grid).
    [[nodiscard]] glm::vec2 to_ndc(GridCoord grid, const Grid& grid_ref, glm::vec2 viewport_size) const noexcept;

    // View-projection matrix suitable for shader uniforms.
    [[nodiscard]] glm::mat4 view_projection(glm::vec2 viewport_size) const noexcept;
};

} // namespace netra::graphics
