#pragma once

#include <vector>

#include <entity/registry.hpp>

class BaseSystem;  // forward declaration for BaseSystem

namespace rse {
// When a system runs relative to everything else.
//
// There is no depth to lean on in the 2D scene pass -- every sprite sits at
// the same depth -- so anything that has to end up *under* the sprites, the
// editor's grid, has to be submitted before the systems that draw them.
enum class SystemPhase { PreScene, Scene };

class ECSManager {
 public:
  ECSManager() {}

  static void init();

  static uint32_t createEntity() { return registry.create(); }

  static void destroyEntity(uint32_t entity) { registry.destroy(entity); }

  template <typename... Components, typename... TArgs>
  static void addComponent(uint32_t entity, TArgs... arguments) {
    registry.assign<Components...>(entity, arguments...);
  }

  template <typename Component>
  static void removeComponent(uint32_t entity) {
    registry.remove<Component>(entity);
  }

  template <typename... Components>
  static bool hasComponent(uint32_t entity) {
    return registry.has<Components...>(entity);
  }

  template <typename Component>
  static Component& getComponent(uint32_t entity) {
    return registry.get<Component>(entity);
  }

  template <typename... Components>
  static entt::View<uint32_t, Components...> getView() {
    return registry.view<Components...>();
  }

  static void addSystem(BaseSystem* system, SystemPhase phase = SystemPhase::Scene);

  static void update(float delta);
  static void updateUI();
  static void clean();

  static bool save();
  static bool load();

 private:
  static entt::DefaultRegistry registry;
  static std::vector<BaseSystem*> m_systemsList;
  static std::vector<BaseSystem*> m_preSceneSystems;
};
}  // namespace rse