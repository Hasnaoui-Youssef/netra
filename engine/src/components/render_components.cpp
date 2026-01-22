#include "components/render_components.hpp"

namespace netra {
// Intentionally empty: render-facing components are plain data.
void WireSegments::add_segments(const std::vector<GridCoord> &points,
                                Entity owner) {
  if (points.size() < 2)
    return;

  for (size_t i = 0; i < points.size() - 1; ++i) {
    const GridCoord &p1 = points[i];
    const GridCoord &p2 = points[i + 1];

    if (p1.y == p2.y) {
      // Horizontal segment
      int min_x = std::min(p1.x, p2.x);
      int max_x = std::max(p1.x, p2.x);
      h_segments[p1.y].push_back({p1.y, min_x, max_x, owner});
    } else if (p1.x == p2.x) {
      // Vertical segment
      int min_y = std::min(p1.y, p2.y);
      int max_y = std::max(p1.y, p2.y);
      v_segments[p1.x].push_back({p1.x, min_y, max_y, owner});
    } else {
      // L-shaped segment: treat as two segments
      int x1 = p1.x;
      int y1 = p1.y;
      int x2 = p2.x;
      int y2 = p2.y;

      // Horizontal part
      h_segments[y1].push_back({y1, std::min(x1, x2), std::max(x1, x2), owner});
      // Vertical part
      v_segments[x2].push_back({x2, std::min(y1, y2), std::max(y1, y2), owner});
    }
  }
}
} // namespace netra
