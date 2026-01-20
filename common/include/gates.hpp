#pragma once

#include <glm/glm.hpp>

namespace netra {

enum class GateType : std::uint8_t {
    AND,
    NAND,
    OR,
    NOR,
    XOR,
    XNOR,
    NOT,
    INVALID,
    COUNT
};

namespace graphics {
    struct Gate {
        GateType type;
        glm::vec2 position;
        glm::vec2 size{100.0f, 80.0f};
        bool dragging = false;
    };
};// namespace netra::graphics


}// namespace netra
