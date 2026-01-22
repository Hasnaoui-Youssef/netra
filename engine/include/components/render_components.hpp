#pragma once

#include "core/entity.hpp"
#include <grid_coord.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>
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

// Rendering association for an entity (e.g. module instance box, primitive gate
// symbol, etc.). This is a stable key that the graphics layer can map to an
// actual shader/program.
struct ShaderKey {
  std::string key;
};

// Which side the port is on (for visual orientation of pin/arrow).
struct PortVisual {
  PortSide side = PortSide::Left;
};

// Cached port position in canvas grid coordinates (derived from module position
// + offset).
struct PortGridPosition {
  GridCoord position{0, 0};
};

// A user-authored wire entity.
// Points are in canvas grid coordinates; endpoints can be implicit via ports.
// A wire always references a signal (multiple wires may share the same signal
// for fanout).
struct Wire {
  Entity signal{};               // The signal this wire carries
  Entity from_endpoint{};        // Port or wire point this wire starts from
  Entity to_endpoint{};          // Port or wire point this wire ends at
  std::vector<GridCoord> points; // Polyline points (excluding endpoints)
};

// Marker component for a wire junction point (where wires can connect).
// Attached to wire entities; the junction position is one of the wire's points.
struct WireJunction {
  std::size_t point_index = 0; // Index into Wire::points where junction exists
};

// Transient segment structure for crossing detection
struct HSegment {
  int y;
  int min_x;
  int max_x;
  Entity owner;
};

struct VSegment {
  int x;
  int min_y;
  int max_y;
  Entity owner;
};

struct WireSegments {
  std::unordered_map<int, std::vector<HSegment>> h_segments;
  std::unordered_map<int, std::vector<VSegment>> v_segments;

  void add_segments(const std::vector<GridCoord> &points, Entity owner);
};

} // namespace netra
