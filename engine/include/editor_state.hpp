#pragma once

#include <graphics/camera2d.hpp>

namespace netra {

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
};

} // namespace netra
