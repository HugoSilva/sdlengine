#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include <entity/registry.hpp>

#include <ecs/components/disabled_components.h>

class b2World;

namespace rse {
class ECSManager;
class Entity;

// The world one scene is made of: every entity and every component it holds,
// plus the physics world those entities' bodies live in.
//
// The engine runs on one active Scene, reached through ECSManager until the
// systems take a scene argument. Owning the registry rather than borrowing a
// global is what lets two worlds exist at once -- an editor stopped on the saved
// scene and a play session running a copy -- and gives the physics world a
// lifetime to hang from: a body belongs to the world that built it, so the
// entities must die before the world does.
//
// A scene is its registry, so it cannot be copied: a copy would be a second
// world holding the same entity ids, and every handle would name an entity in
// both. A play session will duplicate entities with an explicit copy operation
// instead. Handles that were taken before the scene is destroyed belong to it
// and become invalid with it.
class Scene {
 public:
  Scene();
  ~Scene();
  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;

  // Creates an entity with a fresh identity. The scene owns it until it is
  // destroyed, and the returned handle stops being valid when it is.
  Entity CreateEntity();

  // Destroys the entity if it belongs to this scene and is still alive. A
  // handle from another scene, one already destroyed, or a default handle is
  // ignored: a handle is a name, and being asked for one it does not know is
  // not an error.
  void DestroyEntity(const Entity& entity);

  // Whether `entity` names a live entity of this scene. A handle from another
  // scene is never contained, even if the other scene has a live entity with
  // the same numeric id.
  bool Contains(const Entity& entity) const;

  // How many entities are alive in the scene.
  std::size_t EntityCount() const;

  // The entities and components themselves. A system walks components through
  // this -- registry().view<...>() -- because a view is entt's API and wrapping
  // it would only add a name. SceneIO and PrefabIO read and write it the same
  // way. The registry is the scene's, so a view taken from one scene never
  // sees another's entities.
  entt::DefaultRegistry& Registry() { return m_registry; }
  const entt::DefaultRegistry& Registry() const { return m_registry; }

  // An entity with no EntityEnabledComponent is enabled: that is what keeps a
  // scene or prefab written before the component existed fully playable.
  bool IsEntityEnabled(uint32_t entity) const;

  // A component with no disabled bit is enabled: that is what keeps a scene or
  // prefab written before component state existed fully enabled. The bit lives
  // in DisabledComponentsComponent, which is only present while something on
  // the entity is off.
  template <typename Component>
  bool IsComponentEnabled(uint32_t entity) const {
    if (!m_registry.has<DisabledComponentsComponent>(entity)) {
      return true;
    }

    return !m_registry.get<DisabledComponentsComponent>(entity).IsDisabled(ComponentDisabledBit<Component>::kValue);
  }

  // Destroys every entity, keeping the scene object. The camera and the file a
  // scene manager is holding are not part of this type; the physics world stays
  // alive, empty, so bodies are destroyed before it and the scene can be filled
  // again without rebuilding it.
  void Clear();

  // The world the scene's rigid bodies are built in and stepped by. Every scene
  // has its own, so a body created in one is not visible to another, and the
  // world outlives every entity the scene owns.
  b2World* PhysicsWorld() const { return m_world.get(); }

 private:
  // Entity reaches the registry through the scene it was made by; nothing
  // outside the pair gets to touch it directly. ECSManager is the temporary
  // forwarder that gives the rest of the engine the active scene's registry
  // until the systems take a scene argument.
  friend class Entity;
  friend class ECSManager;

  // Declared before the registry so that the registry -- and with it every
  // Rigidbody, which destroys its body through this world -- is destroyed
  // first. The world is the last thing to go.
  std::unique_ptr<b2World> m_world;

  entt::DefaultRegistry m_registry;
};
}  // namespace rse
