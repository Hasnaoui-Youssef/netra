#include "graphics/wire.hpp"

namespace netra::graphics::wire {
    void CrossDetectionBucket::add_segment(const Segment& seg) {
        if(seg.is_horizontal()) h_segments[seg.start.y].push_back(seg);
        else v_segments[seg.start.x].push_back(seg);
    }
    void CrossDetectionBucket::clear() {
        h_segments.clear();
        v_segments.clear();
    }
    std::vector<int> CrossDetectionBucket::find(const Segment& seg) const {
        if(!seg.is_vertical()) return {};
        return {};
    }
}
