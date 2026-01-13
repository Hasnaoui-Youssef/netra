#include "systems/simulation.hpp"

namespace netra {

Simulation::Simulation(World& world) : m_world(world) {}

void Simulation::register_primitive(const std::string& name, BehaviorFunc func) {
    m_primitives[name] = std::move(func);
}

void Simulation::step() {
    m_world.view<ModuleInst>().each([this](Entity entity, ModuleInst& inst) {
        auto* def = m_world.get<ModuleDef>(inst.definition);
        if (!def || !def->is_primitive) return;
        
        auto it = m_primitives.find(def->name);
        if (it == m_primitives.end()) return;
        
        std::vector<BitValue> inputs;
        std::vector<BitValue> outputs;
        std::vector<Entity> output_ports;
        
        m_world.view<Port>().each([&](Entity port_entity, Port& port) {
            if (port.owner != entity) return;
            
            if (port.direction == PortDirection::In) {
                if (auto* val = m_world.get<BitValue>(port.connected_signal)) {
                    inputs.push_back(*val);
                } else {
                    inputs.push_back(BitValue(port.width));
                }
            } else if (port.direction == PortDirection::Out) {
                outputs.push_back(BitValue(port.width));
                output_ports.push_back(port_entity);
            }
        });
        
        it->second(inputs, outputs);
        
        for (std::size_t i = 0; i < output_ports.size() && i < outputs.size(); ++i) {
            if (auto* port = m_world.get<Port>(output_ports[i])) {
                if (port->connected_signal.valid()) {
                    m_world.emplace<BitValue>(port->connected_signal, outputs[i]);
                }
            }
        }
    });
}

void Simulation::run(std::size_t cycles) {
    for (std::size_t i = 0; i < cycles; ++i) {
        step();
    }
}

namespace primitives {

void register_basic_gates(Simulation& sim) {
    sim.register_primitive("AND", [](const std::vector<BitValue>& in, std::vector<BitValue>& out) {
        if (in.size() < 2 || out.empty()) return;
        out[0] = BitValue(1);
        out[0].set_bit(0, in[0].get_bit(0) && in[1].get_bit(0));
    });
    
    sim.register_primitive("OR", [](const std::vector<BitValue>& in, std::vector<BitValue>& out) {
        if (in.size() < 2 || out.empty()) return;
        out[0] = BitValue(1);
        out[0].set_bit(0, in[0].get_bit(0) || in[1].get_bit(0));
    });
    
    sim.register_primitive("NOT", [](const std::vector<BitValue>& in, std::vector<BitValue>& out) {
        if (in.empty() || out.empty()) return;
        out[0] = BitValue(1);
        out[0].set_bit(0, !in[0].get_bit(0));
    });
    
    sim.register_primitive("NAND", [](const std::vector<BitValue>& in, std::vector<BitValue>& out) {
        if (in.size() < 2 || out.empty()) return;
        out[0] = BitValue(1);
        out[0].set_bit(0, !(in[0].get_bit(0) && in[1].get_bit(0)));
    });
    
    sim.register_primitive("NOR", [](const std::vector<BitValue>& in, std::vector<BitValue>& out) {
        if (in.size() < 2 || out.empty()) return;
        out[0] = BitValue(1);
        out[0].set_bit(0, !(in[0].get_bit(0) || in[1].get_bit(0)));
    });
    
    sim.register_primitive("XOR", [](const std::vector<BitValue>& in, std::vector<BitValue>& out) {
        if (in.size() < 2 || out.empty()) return;
        out[0] = BitValue(1);
        out[0].set_bit(0, in[0].get_bit(0) != in[1].get_bit(0));
    });
    
    sim.register_primitive("XNOR", [](const std::vector<BitValue>& in, std::vector<BitValue>& out) {
        if (in.size() < 2 || out.empty()) return;
        out[0] = BitValue(1);
        out[0].set_bit(0, in[0].get_bit(0) == in[1].get_bit(0));
    });
}

} // namespace primitives

} // namespace netra
