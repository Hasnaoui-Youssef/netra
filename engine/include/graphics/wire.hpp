#pragma once

#include "core/entity.hpp"
#include "core/world.hpp"
#include "graphics/grid.hpp"
#include <cstdlib>
#include <unordered_map>
#include <vector>
namespace netra::graphics::wire {
struct Segment  {
    graphics::GridCoord start;
    graphics::GridCoord end;
    Entity owner;

    bool is_vertical() const { return std::abs(start.x) - std::abs(end.x) == 0;}
    bool is_horizontal() const { return std::abs(start.y) - std::abs(end.y) == 0;}
};

class CrossDetectionBucket {
public:
    void add_segment(const Segment& seg);
    std::vector<int> find(const Segment& seg) const;
    void clear();
private:
    std::unordered_map<int, std::vector<Segment>> h_segments;
    std::unordered_map<int, std::vector<Segment>> v_segments;
};
class GeometryBuilder {
public:
    void add_straight_segment(glm::vec2 start, glm::vec2 end);
    void add_arc_segment(glm::vec2 center, float radius, bool forward);
    const std::vector<float>& get_vertices() { return m_vertices; }
    void clear() { m_vertices.clear() ; }

private:
    constexpr static float thickness = 3.0f;
    constexpr static float half_th = thickness / 2;
    std::vector<float> m_vertices;
    void add_rect(glm::vec2 p1, glm::vec2 p2);
};

class SegmentCollector {
    SegmentCollector(World& world, graphics::Grid& grid)
        :m_world(world), m_grid(grid) {}
private:
    World& m_world;
    graphics::Grid& m_grid;
};
}
