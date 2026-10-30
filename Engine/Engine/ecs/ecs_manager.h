#pragma once

#include <vector>

#include <entity/registry.hpp>

#include <ecs/components/disabled_components.h>

class BaseSystem;  // forward declaration for BaseSystem

namespace rse {
class SceneIO;   // forward declaration for SceneIO
class PrefabIO;  // forward declaration for PrefabIO

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

  static uint32_t CreateEntity() { return registry.create(); }

  static void DestroyEntity(uint32_t entity) { registry.destroy(entity); }

  // Whether an id still names a live entity. Selection outlives a scene
  // restore, and the restore may have removed an entity that was created while
  // the scene was playing.
  static bool IsValidEntity(uint32_t entity) { return registry.valid(entity); }

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

  // An entity with no EntityEnabledComponent is enabled: that is what keeps a
  // scene or prefab written before the component existed fully playable.
  static bool IsEntityEnabled(uint32_t entity);

  // Adds the component when it is missing and writes the requested state. The
  // inspector calls this, so toggling Enabled is one edit and one dirty mark.
  static void SetEntityEnabled(uint32_t entity, bool enabled);

  // A component with no disabled bit is enabled: that is what keeps a scene or
  // prefab written before component state existed fully enabled. The bit lives
  // in DisabledComponentsComponent, which is only present while something on
  // the entity is off.
  template <typename Component>
  static bool IsComponentEnabled(uint32_t entity) {
    if (!HasComponent<DisabledComponentsComponent>(entity)) {
      return true;
    }

    return !GetComponent<DisabledComponentsComponent>(entity).IsDisabled(ComponentDisabledBit<Component>::kValue);
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

  // Runs the frame's systems. `run_simulation` is false in the editor while the
  // scene is stopped or paused: pre-scene and scene render systems still run,
  // so the scene keeps drawing, but physics and audio wait.
  static void Update(float delta, bool run_simulation = true);
  static void UpdateUi();
  static void Clean();

 private:
  // SceneIO reads and writes the registry as a whole; it is the only thing
  // that should touch the buffer behind it directly.
  friend class SceneIO;

  // PrefabIO reads one entity out of the registry and adds a copy of one back
  // into it. The copy code lives there rather than here because only a prefab
  // knows that a placed entity owns new sprites and bodies instead of sharing
  // them.
  friend class PrefabIO;

  static entt::DefaultRegistry registry;
  static std::vector<BaseSystem*> m_systems_list;
  static std::vector<BaseSystem*> m_pre_scene_systems;
  static std::vector<BaseSystem*> m_simulation_systems;
};
}  // namespace rse
