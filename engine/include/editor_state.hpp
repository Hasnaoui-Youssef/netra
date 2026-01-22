#pragma once

#include <graphics/camera2d.hpp>
#include <grid_coord.hpp>
#include <core/entity.hpp>

#include <vector>

namespace netra {

// Editor interaction modes.
enum class EditorMode : std::uint8_t {
    Select,  // Default: select/drag components
    Wiring,  // Placing wire points
};

// Transient state for a wire being constructed.
// Not persisted; cleared on mode exit or wire completion.
struct WiringState {
    std::vector<GridCoord> points;  // Points placed so far (in order)
    std::vector<GridCoord> current_path; // Dynamic path from last point to mouse (A* preview)
    Entity start_endpoint{};        // Port or wire junction where wiring began
    GridCoord mouse_grid_pos{};     // Current mouse position in grid coords (for preview)
    bool active = false;            // True if currently placing a wire
};

// Aggregates editor/view state that is not simulation data.
//
// Why this exists with only Camera2D:
// - Camera2D is a view concern, not a domain entity. It does not belong in ECS.
// - Future editor state (grid visibility, snap settings, edit mode, selection)
//   will also be view/editor concerns that don't fit the ECS model.
// - Grouping them here keeps RenderSystem's interface stable as we add features.
// - Avoids scattering unrelated singletons across the codebase.
//
// This is intentionally a plain struct, not a class with behavior.
// Systems read/write members directly.
struct EditorState {
    graphics::Camera2D camera;
    EditorMode mode = EditorMode::Select;
    WiringState wiring;
};

} // namespace netra
