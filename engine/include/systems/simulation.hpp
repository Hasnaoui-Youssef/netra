#pragma once

#include "core/world.hpp"
#include "components/components.hpp"
#include <functional>
#include <string>
#include <vector>

namespace netra {

using BehaviorFunc = std::function<void(const std::vector<BitValue>&, std::vector<BitValue>&)>;

class Simulation {
public:
    explicit Simulation(World& world);
    
    void register_primitive(const std::string& name, BehaviorFunc func);
    void step();
    void run(std::size_t cycles);

private:
    World& m_world;
    std::unordered_map<std::string, BehaviorFunc> m_primitives;
};

namespace primitives {
    void register_basic_gates(Simulation& sim);
}

} // namespace netra
