#pragma once

#include <types.hpp>
#include "core/entity.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace netra {

// Module definition - a "type" or "class" of module (e.g., "AND gate", "Adder")
struct ModuleDef {
    std::string name;
    bool is_primitive = false;
    Entity internal_root = Entity{};
};

// Module instance - a placed instance of a module definition
struct ModuleInst {
    std::string instance_name;
    Entity definition;
};

// Port on a module
struct Port {
    std::string name;
    PortDirection direction = PortDirection::In;
    std::uint32_t width = 1;
    Entity owner;
    Entity connected_signal;
};

// Signal/wire connecting ports
struct Signal {
    std::string name;
    std::uint32_t width = 1;
    Entity scope;
    std::vector<Entity> connected_ports;
};

// Parent/child hierarchy
struct Hierarchy {
    Entity parent;
    std::vector<Entity> children;
};

// Transform for rendering
struct Transform {
    float x = 0.0f;
    float y = 0.0f;
    float width = 100.0f;
    float height = 80.0f;
};

// Tag for identifying port entities on a specific side
struct PortLayout {
    std::size_t index = 0;
    std::size_t total = 1;
};

// Value storage for simulation
class BitValue {
public:
    BitValue();
    explicit BitValue(std::uint32_t width);
    
    void set_bit(std::uint32_t idx, bool val);
    bool get_bit(std::uint32_t idx) const;
    
    std::uint32_t width() const;
    void resize(std::uint32_t new_width);
    void clear();

private:
    std::vector<std::uint8_t> m_bits;
    std::uint32_t m_width = 0;
};

} // namespace netra
