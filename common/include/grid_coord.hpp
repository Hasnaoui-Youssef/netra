#pragma once

#include <cstdint>

namespace netra {

// Canvas-local integer grid coordinates (grid units, not pixels).
// Conversion to pixels is owned by the rendering layer (see graphics::Grid).
struct GridCoord {
    std::int32_t x = 0;
    std::int32_t y = 0;
};

} // namespace netra
