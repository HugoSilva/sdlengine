#pragma once

#include <cstdint>

#include <entity/registry.hpp>

#include <core/uuid.h>
#include <ecs/scene.h>

namespace rse {
// A handle to one entity in one scene.
//
// An entt id on its own is an index into a registry: it is reused after a
// destroy, it means something different in another registry, and it has no
// name a file can store. This pairs the id with the scene that gave it out and
// with the entity's uuid, so a handle can answer whether it is still alive and
// can reach the entity's components without knowing which registry is active.
//
// A default handle names nothing. IsValid, GetScene, Handle and Destroy are
// safe on a stale or default handle; reaching its components is not.
class Entity {
 public:
  Entity() = default;

  // Whether the scene that made this handle still has a live entity under it.
  // A destroyed entity's slot may have been recycled since; the recycled id
  // carries a new version, so this stays false.
  bool IsValid() const;

  // Destroys the entity in its scene. Destroying twice, or destroying a
  // default handle, does nothing.
  void Destroy();

  // The entity's stable identity: the uuid written to the scene file and kept
  // by a copy. It is the one name that outlives this handle.
  Uuid GetUuid() const;

  Scene* GetScene() const { return m_scene; }

  // The raw entt id, for the code that still speaks in ids. It is only
  // meaningful together with GetScene(), and it is not stable across a save.
  uint32_t Handle() const { return m_handle; }

  bool operator==(const Entity&) const = default;

 private:
  // Only a scene hands these out; a handle forged with an arbitrary id would
  // be a name nothing answers to.
  friend class Scene;

  Entity(Scene* scene, uint32_t handle) : m_scene(scene), m_handle(handle) {}

  Scene* m_scene{nullptr};
  uint32_t m_handle{static_cast<uint32_t>(entt::null)};
};
}  // namespace rse
