#pragma once

#include "core/entity.hpp"
#include <grid_coord.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace netra {

enum class PortSide : std::uint8_t {
    Left,
    Right,
    Top,
    Bottom,
};

// Module extent in grid units (ensures both edges align to grid).
struct ModuleExtent {
    std::int32_t width = 1;
    std::int32_t height = 1;
};

// Module render position in pixels (derived from port grid positions).
struct ModulePixelPosition {
    float x = 0.0f;
    float y = 0.0f;
};

// Port offset from module top-left corner in grid units.
// Authored when creating module definition.
struct PortOffset {
    std::int32_t x = 0;
    std::int32_t y = 0;
};

// Rendering association for an entity (e.g. module instance box, primitive gate symbol, etc.).
// This is a stable key that the graphics layer can map to an actual shader/program.
struct ShaderKey {
    std::string key;
};

// Which side the port is on (for visual orientation of pin/arrow).
struct PortVisual {
    PortSide side = PortSide::Left;
};

// Cached port position in canvas grid coordinates (derived from module position + offset).
struct PortGridPosition {
    GridCoord position{0, 0};
};

// A user-authored wire entity.
// Points are in canvas grid coordinates; endpoints can be implicit via ports.
struct Wire {
    Entity from_port{};
    Entity to_port{};
    std::vector<GridCoord> points; // optional polyline points (may include endpoints)
};

} // namespace netra
