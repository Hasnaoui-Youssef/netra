#include "core/entity.hpp"

namespace netra {

Entity::Entity() : m_id(NullEntity) {}

Entity::Entity(EntityID id) : m_id(id) {}

EntityID Entity::id() const { return m_id; }

bool Entity::valid() const { return m_id != NullEntity; }

Entity::operator bool() const { return valid(); }

bool Entity::operator==(const Entity& other) const { return m_id == other.m_id; }

bool Entity::operator!=(const Entity& other) const { return m_id != other.m_id; }

bool Entity::operator<(const Entity& other) const { return m_id < other.m_id; }

} // namespace netra

std::size_t std::hash<netra::Entity>::operator()(const netra::Entity& e) const noexcept {
    return std::hash<netra::EntityID>{}(e.id());
}
