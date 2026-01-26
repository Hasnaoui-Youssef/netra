#pragma once

#include "component_storage.hpp"
#include "entity.hpp"
#include <any>
#include <optional>
#include <typeindex>
#include <unordered_map>
#include <concepts>

namespace netra {

class World {
public:
  World() = default;
  ~World() = default;

  World(const World &) = delete;
  World &operator=(const World &) = delete;
  World(World &&) = default;
  World &operator=(World &&) = default;

  // Entity management
  Entity create() {
    EntityID id;
    if (!m_free_ids.empty()) {
      id = m_free_ids.back();
      m_free_ids.pop_back();
    } else {
      id = m_next_id++;
    }
    m_alive.insert(id, true);
    return Entity(id);
  }

  void destroy(Entity entity) {
    if (!alive(entity))
      return;

    m_alive.remove(entity.id());
    m_free_ids.push_back(entity.id());

    // Remove all components for this entity
    for (auto &[type, remover] : m_component_removers) {
      remover(entity.id());
    }
  }

  bool alive(Entity entity) const { return m_alive.contains(entity.id()); }

  // Component management
  template <typename T, typename... Args>
  T &emplace(Entity entity, Args &&...args) {
    auto &storage = get_or_create_storage<T>();
    storage.insert(entity.id(), T{std::forward<Args>(args)...});
    return *storage.get(entity.id());
  }

  template <typename T> void remove(Entity entity) {
    if (auto *storage = get_storage<T>()) {
      storage->remove(entity.id());
    }
  }

  template <typename T> T *get(Entity entity) {
    if (auto *storage = get_storage<T>()) {
      return storage->get(entity.id());
    }
    return nullptr;
  }

  template <typename T> const T *get(Entity entity) const {
    if (auto *storage = get_storage<T>()) {
      return storage->get(entity.id());
    }
    return nullptr;
  }

  template <typename T> bool has(Entity entity) const {
    if (auto *storage = get_storage<T>()) {
      return storage->contains(entity.id());
    }
    return false;
  }

  // Iteration - single component
  template <typename T, typename Func> void each(Func &&func) {
    if (auto *storage = get_storage<T>()) {
      storage->each(std::forward<Func>(func));
    }
  }

  // View - iterate entities with specific components
  template <typename... Components> class View {
  public:
    View(World &world) : m_world(world) {}

    template <typename Func> void each(Func &&func) {
      auto *first_storage = m_world.get_storage<
          std::tuple_element_t<0, std::tuple<Components...>>>();
      if (!first_storage)
        return;

      for (EntityID id : first_storage->entities()) {
        Entity entity(id);
        if ((m_world.has<Components>(entity) && ...)) {
          func(entity, *m_world.get<Components>(entity)...);
        }
      }
    }
    template <typename Pred>
        requires std::is_invocable_r_v<bool, Pred, Entity, Components&...>
        std::optional<Entity> find_first(Pred&& predicate) {
            auto* first_storage = m_world.get_storage
                <std::tuple_element_t<0, std::tuple<Components...>>>();
            if (!first_storage) return std::nullopt;
            for(EntityID id : first_storage->entities()) {
                Entity entity(id);
                if((m_world.has<Components>(entity) && ...)) {
                    if(predicate(entity, *m_world.get<Components>(entity)...)) {
                        return entity;
                    }
                }
            }
            return std::nullopt;
        }
    template <typename Pred>
        requires std::is_invocable_r_v<bool, Pred, Components&...>
        std::optional<Entity> find_first(Pred&& predicate) {
            auto* first_storage = m_world.get_storage
                <std::tuple_element_t<0, std::tuple<Components...>>>();
            if (!first_storage) return std::nullopt;
            for(EntityID id : first_storage->entities()) {
                Entity entity(id);
                if((m_world.has<Components>(entity) && ...)) {
                    if(predicate(*m_world.get<Components>(entity)...)) {
                        return entity;
                    }
                }
            }
            return std::nullopt;
        }

  private:
    World &m_world;
  };

  template <typename... Components> View<Components...> view() {
    return View<Components...>(*this);
  }

  // Storage access
  template <typename T> ComponentStorage<T> *get_storage() {
    auto it = m_storages.find(std::type_index(typeid(T)));
    if (it == m_storages.end())
      return nullptr;
    return std::any_cast<ComponentStorage<T>>(&it->second);
  }

  template <typename T> const ComponentStorage<T> *get_storage() const {
    auto it = m_storages.find(std::type_index(typeid(T)));
    if (it == m_storages.end())
      return nullptr;
    return std::any_cast<ComponentStorage<T>>(&it->second);
  }

  std::size_t entity_count() const { return m_alive.size(); }

private:
  template <typename T> ComponentStorage<T> &get_or_create_storage() {
    auto type = std::type_index(typeid(T));
    auto it = m_storages.find(type);
    if (it == m_storages.end()) {
      m_storages[type] = ComponentStorage<T>{};
      m_component_removers[type] = [this](EntityID id) {
        if (auto *s = get_storage<T>())
          s->remove(id);
      };
    }
    return *std::any_cast<ComponentStorage<T>>(&m_storages[type]);
  }

  EntityID m_next_id = 0;
  std::vector<EntityID> m_free_ids;
  ComponentStorage<bool> m_alive;

  std::unordered_map<std::type_index, std::any> m_storages;
  std::unordered_map<std::type_index, std::function<void(EntityID)>>
      m_component_removers;
};

} // namespace netra
