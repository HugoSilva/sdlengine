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

  static void Init();

  static uint32_t CreateEntity() { return registry.create(); }

  static void DestroyEntity(uint32_t entity) { registry.destroy(entity); }

  template <typename... Components, typename... TArgs>
  static void AddComponent(uint32_t entity, TArgs... arguments) {
    registry.assign<Components...>(entity, arguments...);
  }

  template <typename Component>
  static void RemoveComponent(uint32_t entity) {
    registry.remove<Component>(entity);
  }

  template <typename... Components>
  static bool HasComponent(uint32_t entity) {
    return registry.has<Components...>(entity);
  }

  template <typename Component>
  static Component& GetComponent(uint32_t entity) {
    return registry.get<Component>(entity);
  }

  template <typename... Components>
  static entt::View<uint32_t, Components...> GetView() {
    return registry.view<Components...>();
  }

  static void AddSystem(BaseSystem* system, SystemPhase phase = SystemPhase::Scene);

  static void Update(float delta);
  static void UpdateUi();
  static void Clean();

  static bool Save();
  static bool Load();

 private:
  static entt::DefaultRegistry registry;
  static std::vector<BaseSystem*> m_systems_list;
  static std::vector<BaseSystem*> m_pre_scene_systems;
};
}  // namespace rse