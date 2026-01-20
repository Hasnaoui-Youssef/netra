#include <graphics/grid.hpp>
#include <cmath>

namespace netra::graphics {

Grid::Grid(unit_t unit_px) noexcept : m_unit_px(unit_px) {}

Grid::unit_t Grid::unit_px() const noexcept { return m_unit_px; }

void Grid::set_unit_px(unit_t unit_px) noexcept { m_unit_px = unit_px; }

glm::vec2 Grid::to_glm_vec2(GridCoord grid) const noexcept {
    if (m_unit_px <= 0) {
        return {static_cast<float>(grid.x), static_cast<float>(grid.y)};
    }

    return {
        static_cast<float>(grid.x * m_unit_px),
        static_cast<float>(grid.y * m_unit_px),
    };
}


//We will be rounding each value
//This assumes both pixels and grid values start from the same origin (Top-Left corner)
std::optional<glm::ivec2> Grid::pixels_to_grid_pixels(glm::vec2 px) const noexcept {
    if (m_unit_px <= 0 || px.x < 0 || px.y < 0) return std::nullopt;

    const int rx = static_cast<int>(std::round(px.x / m_unit_px) * m_unit_px);
    const int ry = static_cast<int>(std::round(px.y / m_unit_px) * m_unit_px);

    return glm::ivec2{rx, ry};
}

} // namespace netra::graphics
