#pragma once

#include <grid_coord.hpp>

#include "glm/vec2.hpp"
#include <cstdint>
#include <optional>

namespace netra::graphics {

using GridCoord = netra::GridCoord;

// Canvas-local grid.
// Origin is the top-left corner of the canvas (0,0).
// All grid coordinates are integer units; conversion to pixels is done via unit_px.
class Grid {
public:
    using unit_t = std::int32_t;

    explicit Grid(unit_t unit_px = 5) noexcept;

    [[nodiscard]] unit_t unit_px() const noexcept;
    void set_unit_px(unit_t unit_px) noexcept;

    // Exact conversion: no snapping/rounding.
    [[nodiscard]] glm::vec2 to_glm_vec2(GridCoord grid) const noexcept;

    // Returns the nearest pixel coordinate that lies exactly on the grid.
    [[nodiscard]] std::optional<glm::ivec2> pixels_to_grid_pixels(glm::vec2 px) const noexcept;

private:
    unit_t m_unit_px = 5;
};

} // namespace netra::graphics
