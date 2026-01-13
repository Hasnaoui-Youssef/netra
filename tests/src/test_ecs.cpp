#include "test_framework.hpp"
#include <core/world.hpp>
#include <components/components.hpp>

using namespace netra;

TEST(entity_creation) {
    World world;
    
    Entity e1 = world.create();
    Entity e2 = world.create();
    
    ASSERT(e1.valid());
    ASSERT(e2.valid());
    ASSERT(e1 != e2);
    ASSERT(world.alive(e1));
    ASSERT(world.alive(e2));
    ASSERT_EQ(world.entity_count(), 2u);
    
    return true;
}

TEST(entity_destruction) {
    World world;
    
    Entity e1 = world.create();
    Entity e2 = world.create();
    
    world.destroy(e1);
    
    ASSERT(!world.alive(e1));
    ASSERT(world.alive(e2));
    ASSERT_EQ(world.entity_count(), 1u);
    
    return true;
}

TEST(entity_id_reuse) {
    World world;
    
    Entity e1 = world.create();
    EntityID id1 = e1.id();
    world.destroy(e1);
    
    Entity e2 = world.create();
    ASSERT_EQ(e2.id(), id1);
    
    return true;
}

TEST(component_add_get) {
    World world;
    Entity e = world.create();
    
    world.emplace<Transform>(e, 10.0f, 20.0f, 100.0f, 80.0f);
    
    ASSERT(world.has<Transform>(e));
    
    auto* t = world.get<Transform>(e);
    ASSERT(t != nullptr);
    ASSERT_EQ(t->x, 10.0f);
    ASSERT_EQ(t->y, 20.0f);
    
    return true;
}

TEST(component_remove) {
    World world;
    Entity e = world.create();
    
    world.emplace<Transform>(e, 0.0f, 0.0f, 0.0f, 0.0f);
    ASSERT(world.has<Transform>(e));
    
    world.remove<Transform>(e);
    ASSERT(!world.has<Transform>(e));
    ASSERT(world.get<Transform>(e) == nullptr);
    
    return true;
}

TEST(component_cleanup_on_destroy) {
    World world;
    Entity e = world.create();
    
    world.emplace<Transform>(e, 0.0f, 0.0f, 0.0f, 0.0f);
    world.emplace<ModuleDef>(e, "test", false);
    
    world.destroy(e);
    
    ASSERT(!world.has<Transform>(e));
    ASSERT(!world.has<ModuleDef>(e));
    
    return true;
}

TEST(view_single_component) {
    World world;
    
    Entity e1 = world.create();
    Entity e2 = world.create();
    Entity e3 = world.create();
    
    world.emplace<Transform>(e1, 1.0f, 0.0f, 0.0f, 0.0f);
    world.emplace<Transform>(e2, 2.0f, 0.0f, 0.0f, 0.0f);
    // e3 has no Transform
    
    int count = 0;
    float sum = 0.0f;
    
    world.view<Transform>().each([&](Entity e, Transform& t) {
        ++count;
        sum += t.x;
    });
    
    ASSERT_EQ(count, 2);
    ASSERT_EQ(sum, 3.0f);
    
    return true;
}

TEST(view_multiple_components) {
    World world;
    
    Entity e1 = world.create();
    Entity e2 = world.create();
    Entity e3 = world.create();
    
    world.emplace<Transform>(e1, 1.0f, 0.0f, 0.0f, 0.0f);
    world.emplace<ModuleInst>(e1, "inst1", Entity{});
    
    world.emplace<Transform>(e2, 2.0f, 0.0f, 0.0f, 0.0f);
    // e2 has no ModuleInst
    
    world.emplace<ModuleInst>(e3, "inst3", Entity{});
    // e3 has no Transform
    
    int count = 0;
    bool correct = true;
    
    world.view<Transform, ModuleInst>().each([&](Entity e, Transform& t, ModuleInst& m) {
        ++count;
        if (t.x != 1.0f) correct = false;
        if (m.instance_name != "inst1") correct = false;
    });
    
    ASSERT_EQ(count, 1);
    ASSERT(correct);
    
    return true;
}

TEST(bitvalue_operations) {
    BitValue val(8);
    
    ASSERT_EQ(val.width(), 8u);
    ASSERT_EQ(val.get_bit(0), false);
    
    val.set_bit(0, true);
    val.set_bit(7, true);
    
    ASSERT_EQ(val.get_bit(0), true);
    ASSERT_EQ(val.get_bit(1), false);
    ASSERT_EQ(val.get_bit(7), true);
    
    val.clear();
    ASSERT_EQ(val.get_bit(0), false);
    ASSERT_EQ(val.get_bit(7), false);
    
    return true;
}
