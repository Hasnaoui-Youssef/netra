#include <graphics/camera2d.hpp>
#include <graphics/grid.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace netra::graphics {

glm::vec2 Camera2D::to_ndc(glm::vec2 pixel_pos, glm::vec2 viewport_size) const noexcept {
    // Apply pan and zoom, then convert to NDC [-1, 1]
    glm::vec2 transformed = (pixel_pos + pan) * zoom;

    // NDC: x [-1, 1] left to right, y [-1, 1] bottom to top
    // Screen: origin top-left, y increases downward
    glm::vec2 ndc{
        (transformed.x / viewport_size.x) * 2.0f - 1.0f,
        1.0f - (transformed.y / viewport_size.y) * 2.0f
    };

    return ndc;
}

glm::vec2 Camera2D::to_ndc(GridCoord grid, const Grid& grid_ref, glm::vec2 viewport_size) const noexcept {
    glm::vec2 pixel_pos = grid_ref.to_glm_vec2(grid);
    return to_ndc(pixel_pos, viewport_size);
}

glm::mat4 Camera2D::view_projection(glm::vec2 viewport_size) const noexcept {
    // Orthographic projection with pan and zoom
    // Screen coords: origin top-left, y down
    // NDC: origin center, y up

    float scaled_width = viewport_size.x / zoom;
    float scaled_height = viewport_size.y / zoom;

    float left = -pan.x;
    float right = left + scaled_width;
    float top = -pan.y;
    float bottom = top + scaled_height;

    // Flip y for top-left origin convention
    return glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
}

} // namespace netra::graphics
