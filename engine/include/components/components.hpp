#pragma once

#include <types.hpp>
#include "core/entity.hpp"
#include <boost/dynamic_bitset.hpp>
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
// Ideally, we would have a single writer/multi-reader regulation per update cycle.
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

// Tag for identifying port entities on a specific side
struct PortLayout {
    std::size_t index = 0;
    std::size_t total = 1;
};

// Value storage for simulation
// How is this meant to work:
// A module defines the transformation of values from its input ports to output ports
// Signals propagate the value between the connected ports.
// Ports are the "Points" at which data has meaning and we can observe the value.
class BitValue {
public:
    BitValue();
    explicit BitValue(std::uint32_t width);
    void set_bit(std::uint32_t idx, bool val);

    void set_bits(std::uint32_t start_idx, std::uint32_t end_idx, const boost::dynamic_bitset<>& val);
    void set_bits(std::uint32_t start_idx, const boost::dynamic_bitset<>& val);

    bool get_bit(std::uint32_t idx) const;
    BitValue range(std::uint32_t start_idx, std::uint32_t end_idx);
    BitValue range(std::uint32_t start_idx);

    std::uint32_t width() const;
    void resize(std::uint32_t new_width);
    void clear();

    boost::dynamic_bitset<>::reference operator[](std::uint32_t idx);
    bool operator[](std::uint32_t idx) const;

  private:
    boost::dynamic_bitset<> m_bits;
};

} // namespace netra
