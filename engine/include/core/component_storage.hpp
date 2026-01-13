#pragma once

#include <types.hpp>
#include <vector>
#include <cassert>

namespace netra {

// Sparse set for O(1) lookup and cache-friendly iteration
template<typename T>
class ComponentStorage {
public:
    static constexpr EntityID INVALID = NullEntity;

    void insert(EntityID entity, T component) {
        if (entity >= m_sparse.size()) {
            m_sparse.resize(entity + 1, INVALID);
        }
        
        if (m_sparse[entity] != INVALID) {
            m_dense_components[m_sparse[entity]] = std::move(component);
            return;
        }

        m_sparse[entity] = static_cast<EntityID>(m_dense_entities.size());
        m_dense_entities.push_back(entity);
        m_dense_components.push_back(std::move(component));
    }

    void remove(EntityID entity) {
        if (!contains(entity)) return;

        EntityID dense_idx = m_sparse[entity];
        EntityID last_entity = m_dense_entities.back();

        m_dense_entities[dense_idx] = last_entity;
        m_dense_components[dense_idx] = std::move(m_dense_components.back());
        m_sparse[last_entity] = dense_idx;

        m_dense_entities.pop_back();
        m_dense_components.pop_back();
        m_sparse[entity] = INVALID;
    }

    bool contains(EntityID entity) const {
        return entity < m_sparse.size() && m_sparse[entity] != INVALID;
    }

    T* get(EntityID entity) {
        if (!contains(entity)) return nullptr;
        return &m_dense_components[m_sparse[entity]];
    }

    const T* get(EntityID entity) const {
        if (!contains(entity)) return nullptr;
        return &m_dense_components[m_sparse[entity]];
    }

    std::size_t size() const { return m_dense_entities.size(); }
    bool empty() const { return m_dense_entities.empty(); }

    // Iteration support
    auto begin() { return m_dense_components.begin(); }
    auto end() { return m_dense_components.end(); }
    auto begin() const { return m_dense_components.begin(); }
    auto end() const { return m_dense_components.end(); }

    const std::vector<EntityID>& entities() const { return m_dense_entities; }

    // Iterate with entity ID
    template<typename Func>
    void each(Func&& func) {
        for (std::size_t i = 0; i < m_dense_entities.size(); ++i) {
            func(Entity(m_dense_entities[i]), m_dense_components[i]);
        }
    }

    void clear() {
        m_sparse.clear();
        m_dense_entities.clear();
        m_dense_components.clear();
    }

private:
    std::vector<EntityID> m_sparse;           // entity -> dense index
    std::vector<EntityID> m_dense_entities;   // dense index -> entity
    std::vector<T> m_dense_components;        // dense index -> component
};

} // namespace netra
