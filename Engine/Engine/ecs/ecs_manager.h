#pragma once

#include <vector>

#include <entity/registry.hpp>

#include <ecs/scene.h>

class BaseSystem;  // forward declaration for BaseSystem

namespace rse {
// When a system runs relative to everything else.
//
// There is no depth to lean on in the 2D scene pass -- every sprite sits at
// the same depth -- so anything that has to end up *under* the sprites, the
// editor's grid, has to be submitted before the systems that draw them.
//
// kSimulation is the scene's logic rather than its picture. The editor skips
// this phase unless the scene is playing, while the render phases keep running
// so a stopped scene still shows what it contains.
enum class SystemPhase { kPreScene, kSimulation, kScene };

class ECSManager {
 public:
  ECSManager() {}

  static void Init();

  // The scene the engine is working on: SceneManager's active scene, which is
  // the play copy while a session runs and the open scene otherwise. Temporary
  // until the systems and the editor take the scene from SceneManager directly.
  static Scene& ActiveScene();

  // The active scene's registry, for the code that still takes one: scene IO,
  // prefab IO and the editor's ids. The systems now walk the scene they are
  // handed. Temporary, for the same reason as ActiveScene().
  static entt::DefaultRegistry& Registry();

  // Creates an entity with a fresh identity. Everything that makes an entity
  // goes through here -- the editor, a prefab placement, a test -- so every
  // entity in a scene has a UuidComponent whether its file wrote one or not.
  static uint32_t CreateEntity();

  static void DestroyEntity(uint32_t entity) { Registry().destroy(entity); }

  // Whether an id still names a live entity. Selection outlives a scene
  // restore, and the restore may have removed an entity that was created while
  // the scene was playing.
  static bool IsValidEntity(uint32_t entity) { return Registry().valid(entity); }

  template <typename... Components, typename... TArgs>
  static void AddComponent(uint32_t entity, TArgs... arguments) {
    Registry().assign<Components...>(entity, arguments...);
  }

  template <typename Component>
  static void RemoveComponent(uint32_t entity) {
    Registry().remove<Component>(entity);
  }

  template <typename... Components>
  static bool HasComponent(uint32_t entity) {
    return Registry().has<Components...>(entity);
  }

  template <typename Component>
  static Component& GetComponent(uint32_t entity) {
    return Registry().get<Component>(entity);
  }

  template <typename... Components>
  static entt::View<uint32_t, Components...> GetView() {
    return Registry().view<Components...>();
  }

  // An entity with no EntityEnabledComponent is enabled: that is what keeps a
  // scene or prefab written before the component existed fully playable.
  static bool IsEntityEnabled(uint32_t entity) { return ActiveScene().IsEntityEnabled(entity); }

  // Adds the component when it is missing and writes the requested state. The
  // inspector calls this, so toggling Enabled is one edit and one dirty mark.
  static void SetEntityEnabled(uint32_t entity, bool enabled);

  // A component with no disabled bit is enabled: that is what keeps a scene or
  // prefab written before component state existed fully enabled. The bit lives
  // in DisabledComponentsComponent, which is only present while something on
  // the entity is off.
  template <typename Component>
  static bool IsComponentEnabled(uint32_t entity) {
    return ActiveScene().IsComponentEnabled<Component>(entity);
  }

  // Disabling writes the mask group; enabling clears the bit and drops the
  // group once nothing is disabled. The inspector calls this, so a checkbox is
  // one edit and one dirty mark.
  template <typename Component>
  static void SetComponentEnabled(uint32_t entity, bool enabled) {
    if (enabled) {
      if (!HasComponent<DisabledComponentsComponent>(entity)) {
        return;
      }

      DisabledComponentsComponent& state = GetComponent<DisabledComponentsComponent>(entity);
      state.SetEnabled(ComponentDisabledBit<Component>::kValue, true);

      if (state.m_mask == 0) {
        RemoveComponent<DisabledComponentsComponent>(entity);
      }

      return;
    }

    if (!HasComponent<DisabledComponentsComponent>(entity)) {
      AddComponent<DisabledComponentsComponent>(entity);
    }

    GetComponent<DisabledComponentsComponent>(entity).SetEnabled(ComponentDisabledBit<Component>::kValue, false);
  }

  static void AddSystem(BaseSystem* system, SystemPhase phase = SystemPhase::kScene);

  // Runs the frame's drawing phases -- pre-scene, then scene -- with the
  // frame's interpolation alpha. Simulation is not run here: it advances in
  // fixed steps, through UpdateSimulation, before this is called.
  static void Update(float alpha);

  // Runs the simulation phase once, advancing by `step` seconds. Core calls
  // this zero or more times per frame, however many whole steps the frame owes,
  // and only while the scene is playing.
  static void UpdateSimulation(float step);
  static void UpdateUi();
  static void Clean();

 private:
  static std::vector<BaseSystem*> m_systems_list;
  static std::vector<BaseSystem*> m_pre_scene_systems;
  static std::vector<BaseSystem*> m_simulation_systems;
};
}  // namespace rse
