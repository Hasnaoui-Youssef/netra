#pragma once

#include "glm/vec2.hpp"
#include <cstdint>
#include <optional>

namespace netra::graphics {

struct GridCoord {
    std::int32_t x = 0;
    std::int32_t y = 0;
};

// Canvas-local grid.
// Origin is the top-left corner of the canvas (0,0), expressed in pixels.
// All grid coordinates are integer units; conversion to pixels is done via unit_px.
class Grid {
public:
    using unit_t = std::int32_t;

    explicit Grid(unit_t unit_px = 5) noexcept;

    [[nodiscard]] unit_t unit_px() const noexcept;
    void set_unit_px(unit_t unit_px) noexcept;

    // Exact conversion: no snapping/rounding.
    [[nodiscard]] glm::vec2 to_glm_vec2(GridCoord grid) const noexcept;

    // Returns a grid coordinate only if the pixel coordinate lies exactly on the grid.
    [[nodiscard]] std::optional<GridCoord> pixels_to_grid(glm::vec2 px) const noexcept;

private:
    unit_t m_unit_px = 5;
};

} // namespace netra::graphics
