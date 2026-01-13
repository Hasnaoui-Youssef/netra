#include "test_framework.hpp"
#include <core/world.hpp>
#include <components/components.hpp>
#include <systems/simulation.hpp>

using namespace netra;

namespace {

// Helper to create a gate with ports and signals
struct TestGate {
    Entity def;
    Entity inst;
    std::vector<Entity> input_ports;
    std::vector<Entity> input_signals;
    Entity output_port;
    Entity output_signal;
};

TestGate create_two_input_gate(World& world, const std::string& gate_name) {
    TestGate g;
    
    g.def = world.create();
    world.emplace<ModuleDef>(g.def, gate_name, true);
    
    g.inst = world.create();
    world.emplace<ModuleInst>(g.inst, "u1", g.def);
    
    // Input A
    Entity port_a = world.create();
    world.emplace<Port>(port_a, "A", PortDirection::In, 1u, g.inst, Entity{});
    Entity sig_a = world.create();
    world.emplace<Signal>(sig_a, "sig_a", 1u, Entity{}, std::vector<Entity>{port_a});
    world.emplace<BitValue>(sig_a, 1u);
    world.get<Port>(port_a)->connected_signal = sig_a;
    g.input_ports.push_back(port_a);
    g.input_signals.push_back(sig_a);
    
    // Input B
    Entity port_b = world.create();
    world.emplace<Port>(port_b, "B", PortDirection::In, 1u, g.inst, Entity{});
    Entity sig_b = world.create();
    world.emplace<Signal>(sig_b, "sig_b", 1u, Entity{}, std::vector<Entity>{port_b});
    world.emplace<BitValue>(sig_b, 1u);
    world.get<Port>(port_b)->connected_signal = sig_b;
    g.input_ports.push_back(port_b);
    g.input_signals.push_back(sig_b);
    
    // Output Y
    g.output_port = world.create();
    world.emplace<Port>(g.output_port, "Y", PortDirection::Out, 1u, g.inst, Entity{});
    g.output_signal = world.create();
    world.emplace<Signal>(g.output_signal, "sig_y", 1u, Entity{}, std::vector<Entity>{g.output_port});
    world.get<Port>(g.output_port)->connected_signal = g.output_signal;
    
    return g;
}

void set_inputs(World& world, TestGate& g, bool a, bool b) {
    world.get<BitValue>(g.input_signals[0])->set_bit(0, a);
    world.get<BitValue>(g.input_signals[1])->set_bit(0, b);
}

bool get_output(World& world, TestGate& g) {
    auto* val = world.get<BitValue>(g.output_signal);
    return val ? val->get_bit(0) : false;
}

} // anonymous namespace

TEST(simulation_and_gate) {
    World world;
    auto gate = create_two_input_gate(world, "AND");
    
    Simulation sim(world);
    primitives::register_basic_gates(sim);
    
    set_inputs(world, gate, false, false);
    sim.step();
    ASSERT_EQ(get_output(world, gate), false);
    
    set_inputs(world, gate, true, false);
    sim.step();
    ASSERT_EQ(get_output(world, gate), false);
    
    set_inputs(world, gate, false, true);
    sim.step();
    ASSERT_EQ(get_output(world, gate), false);
    
    set_inputs(world, gate, true, true);
    sim.step();
    ASSERT_EQ(get_output(world, gate), true);
    
    return true;
}

TEST(simulation_or_gate) {
    World world;
    auto gate = create_two_input_gate(world, "OR");
    
    Simulation sim(world);
    primitives::register_basic_gates(sim);
    
    set_inputs(world, gate, false, false);
    sim.step();
    ASSERT_EQ(get_output(world, gate), false);
    
    set_inputs(world, gate, true, false);
    sim.step();
    ASSERT_EQ(get_output(world, gate), true);
    
    set_inputs(world, gate, false, true);
    sim.step();
    ASSERT_EQ(get_output(world, gate), true);
    
    set_inputs(world, gate, true, true);
    sim.step();
    ASSERT_EQ(get_output(world, gate), true);
    
    return true;
}

TEST(simulation_nand_gate) {
    World world;
    auto gate = create_two_input_gate(world, "NAND");
    
    Simulation sim(world);
    primitives::register_basic_gates(sim);
    
    set_inputs(world, gate, false, false);
    sim.step();
    ASSERT_EQ(get_output(world, gate), true);
    
    set_inputs(world, gate, true, false);
    sim.step();
    ASSERT_EQ(get_output(world, gate), true);
    
    set_inputs(world, gate, false, true);
    sim.step();
    ASSERT_EQ(get_output(world, gate), true);
    
    set_inputs(world, gate, true, true);
    sim.step();
    ASSERT_EQ(get_output(world, gate), false);
    
    return true;
}

TEST(simulation_xor_gate) {
    World world;
    auto gate = create_two_input_gate(world, "XOR");
    
    Simulation sim(world);
    primitives::register_basic_gates(sim);
    
    set_inputs(world, gate, false, false);
    sim.step();
    ASSERT_EQ(get_output(world, gate), false);
    
    set_inputs(world, gate, true, false);
    sim.step();
    ASSERT_EQ(get_output(world, gate), true);
    
    set_inputs(world, gate, false, true);
    sim.step();
    ASSERT_EQ(get_output(world, gate), true);
    
    set_inputs(world, gate, true, true);
    sim.step();
    ASSERT_EQ(get_output(world, gate), false);
    
    return true;
}

TEST(simulation_not_gate) {
    World world;
    
    Entity def = world.create();
    world.emplace<ModuleDef>(def, "NOT", true);
    
    Entity inst = world.create();
    world.emplace<ModuleInst>(inst, "inv1", def);
    
    Entity port_a = world.create();
    world.emplace<Port>(port_a, "A", PortDirection::In, 1u, inst, Entity{});
    Entity sig_a = world.create();
    world.emplace<Signal>(sig_a, "sig_a", 1u, Entity{}, std::vector<Entity>{port_a});
    world.emplace<BitValue>(sig_a, 1u);
    world.get<Port>(port_a)->connected_signal = sig_a;
    
    Entity port_y = world.create();
    world.emplace<Port>(port_y, "Y", PortDirection::Out, 1u, inst, Entity{});
    Entity sig_y = world.create();
    world.emplace<Signal>(sig_y, "sig_y", 1u, Entity{}, std::vector<Entity>{port_y});
    world.get<Port>(port_y)->connected_signal = sig_y;
    
    Simulation sim(world);
    primitives::register_basic_gates(sim);
    
    world.get<BitValue>(sig_a)->set_bit(0, false);
    sim.step();
    ASSERT_EQ(world.get<BitValue>(sig_y)->get_bit(0), true);
    
    world.get<BitValue>(sig_a)->set_bit(0, true);
    sim.step();
    ASSERT_EQ(world.get<BitValue>(sig_y)->get_bit(0), false);
    
    return true;
}
