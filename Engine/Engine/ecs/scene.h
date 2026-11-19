#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include <cereal/cereal.hpp>
#include <entity/registry.hpp>
#include <glm/glm.hpp>

#include <ecs/components/disabled_components.h>

class b2World;

namespace rse {
class Entity;

// The world one scene is made of: every entity and every component it holds,
// plus the physics world those entities' bodies live in.
//
// Whoever owns the scene decides which one is active: SceneManager holds the
// open scene and the play copy, and hands the active one to the systems.
// Owning the registry rather than borrowing a global is what lets two worlds
// exist at once -- an editor stopped on the saved scene and a play session
// running a copy -- and gives the physics world a lifetime to hang from: a body
// belongs to the world that built it, so the entities must die before the world
// does.
//
// A scene is its registry, so it cannot be copied: a copy would be a second
// world holding the same entity ids, and every handle would name an entity in
// both. A play session will duplicate entities with an explicit copy operation
// instead. Handles that were taken before the scene is destroyed belong to it
// and become invalid with it.
class Scene {
 public:
  // What the scene says about the view. The defaults are what the engine framed
  // before a scene could say: the middle of the reference rectangle, zoom 1,
  // and the magenta the renderer cleared to. The camera is scene-root data --
  // there is one per scene -- so a copy carries its own and a play session that
  // moves the view cannot touch the open scene's.
  struct Camera {
    glm::vec2 position{640.0f, 360.0f};
    float zoom{1.0f};
    glm::vec3 background{1.0f, 0.0f, 1.0f};

    template <typename Archive>
    void serialize(Archive& archive) {
      archive(cereal::make_nvp("Position", position), cereal::make_nvp("Zoom", zoom),
              cereal::make_nvp("Background", background));
    }
  };

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

  // Adds the component when it is missing and writes the requested state. This
  // is what the inspector calls, so toggling Enabled is one edit and one dirty
  // mark.
  void SetEntityEnabled(uint32_t entity, bool enabled);

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

  // Disabling writes the mask group; enabling clears the bit and drops the
  // group once nothing is disabled. This is what the inspector calls, so a
  // checkbox is one edit and one dirty mark.
  template <typename Component>
  void SetComponentEnabled(uint32_t entity, bool enabled) {
    if (enabled) {
      if (!m_registry.has<DisabledComponentsComponent>(entity)) {
        return;
      }

      DisabledComponentsComponent& state = m_registry.get<DisabledComponentsComponent>(entity);
      state.SetEnabled(ComponentDisabledBit<Component>::kValue, true);

      if (state.m_mask == 0) {
        m_registry.remove<DisabledComponentsComponent>(entity);
      }

      return;
    }

    if (!m_registry.has<DisabledComponentsComponent>(entity)) {
      m_registry.assign<DisabledComponentsComponent>(entity);
    }

    m_registry.get<DisabledComponentsComponent>(entity).SetEnabled(ComponentDisabledBit<Component>::kValue, false);
  }

  // Destroys every entity and puts the camera back to its defaults, keeping the
  // scene object. The physics world stays alive, empty, so bodies are destroyed
  // before it and the scene can be filled again without rebuilding it. Which
  // file the scene came from is SceneManager's business, not the scene's.
  void Clear();

  // The view the scene is framed with. It is the scene's, so a play session's
  // copy pans and zooms on its own, and the editor's background edit is a scene
  // change that saves with the scene.
  Camera& GetCamera() { return m_camera; }
  const Camera& GetCamera() const { return m_camera; }

  // Fills this scene with a copy of `source`: its camera, then every entity
  // with the uuid it has there. What an entity owns is copied rather than
  // shared -- a Sprite, a Rigidbody -- so writing through one scene never
  // changes the other. The bodies are clones, so they belong to this scene's
  // world and are built in it the first time PhysicsSystem sees them.
  //
  // This is what a play session starts from: the open scene stays untouched
  // while the copy is simulated, and Stop throws the copy away.
  void CopyFrom(const Scene& source);

  // The world the scene's rigid bodies are built in and stepped by. Every scene
  // has its own, so a body created in one is not visible to another, and the
  // world outlives every entity the scene owns.
  b2World* PhysicsWorld() const { return m_world.get(); }

 private:
  // Entity reaches the registry through the scene it was made by; nothing
  // outside the pair gets to touch it directly.
  friend class Entity;

  // Declared before the registry so that the registry -- and with it every
  // Rigidbody, which destroys its body through this world -- is destroyed
  // first. The world is the last thing to go.
  std::unique_ptr<b2World> m_world;

  entt::DefaultRegistry m_registry;

  Camera m_camera;
};
}  // namespace rse
