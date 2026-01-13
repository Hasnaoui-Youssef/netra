#pragma once

#include <types.hpp>
#include <functional>

namespace netra {

class Entity {
public:
    Entity();
    explicit Entity(EntityID id);

    EntityID id() const;
    bool valid() const;
    explicit operator bool() const;

    bool operator==(const Entity& other) const;
    bool operator!=(const Entity& other) const;
    bool operator<(const Entity& other) const;

private:
    EntityID m_id;
};

} // namespace netra

template<>
struct std::hash<netra::Entity> {
    std::size_t operator()(const netra::Entity& e) const noexcept;
};
