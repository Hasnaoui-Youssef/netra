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

// Canvas transform expressed in integer grid coordinates.
// Interpretation of units is owned by the active Grid.
struct CanvasTransform {
    GridCoord position{0, 0}; // top-left
    GridCoord size{0, 0};     // width/height in the same units as position
};

// Rendering association for an entity (e.g. module instance box, primitive gate symbol, etc.).
// This is a stable key that the graphics layer can map to an actual shader/program.
struct ShaderKey {
    std::string key;
};

struct PortVisual {
    PortSide side = PortSide::Left;
};

// Cached port position in canvas grid coordinates.
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
