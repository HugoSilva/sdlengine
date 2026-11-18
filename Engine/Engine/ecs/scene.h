#pragma once

#include <cstddef>
#include <cstdint>

#include <entity/registry.hpp>

namespace rse {
class Entity;

// The world one scene is made of: every entity and every component it holds.
//
// The engine still runs on ECSManager's static registry; this type is the
// destination the rest of Stage 2 moves onto. Owning the registry rather than
// borrowing a global is what lets two worlds exist at once -- an editor stopped
// on the saved scene and a play session running a copy -- and gives the physics
// world a lifetime to hang from.
//
// A scene is its registry, so it cannot be copied: a copy would be a second
// world holding the same entity ids, and every handle would name an entity in
// both. A play session will duplicate entities with an explicit copy operation
// instead. Handles that were taken before the scene is destroyed belong to it
// and become invalid with it.
class Scene {
 public:
  Scene() = default;
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

 private:
  // Entity reaches the registry through the scene it was made by; nothing
  // outside the pair gets to touch it directly.
  friend class Entity;

  entt::DefaultRegistry m_registry;
};
}  // namespace rse
