#pragma once

#include <entity/registry.hpp>

#include <core/uuid.h>

namespace rse {
// Adds a copy of `entity` from `source` to `target`, names the copy with `uuid`
// and returns the new id.
//
// The copy is deep where an entity owns something. A sprite is per entity, so
// the copy gets its own Sprite; the texture inside stays borrowed from
// TextureManager, which is what sharing a texture name means. A rigid body is
// per entity and cannot be copied, so it gets a Clone() with the same inputs,
// uninitialised until PhysicsSystem sees it. A sound is borrowed from
// SoundManager by design, so the copied handle points at that one sound.
//
// The identity is the caller's to choose rather than copied: a prefab placement
// is a new entity and gets a fresh uuid, while a play session's copy is the same
// entity seen in another world and keeps the uuid it had.
entt::DefaultRegistry::entity_type CopyEntity(const entt::DefaultRegistry& source,
                                              entt::DefaultRegistry::entity_type entity, entt::DefaultRegistry& target,
                                              const Uuid& uuid);
}  // namespace rse
