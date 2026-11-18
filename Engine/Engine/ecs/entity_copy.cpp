#include "entity_copy.h"

#include <ecs/components/include_list.h>

namespace rse {
entt::DefaultRegistry::entity_type CopyEntity(const entt::DefaultRegistry& source,
                                              entt::DefaultRegistry::entity_type entity, entt::DefaultRegistry& target,
                                              const Uuid& uuid) {
  const entt::DefaultRegistry::entity_type copy = target.create();

  target.assign<UuidComponent>(copy, UuidComponent{uuid});

  if (source.has<BasicComponent>(entity)) {
    target.assign<BasicComponent>(copy, source.get<BasicComponent>(entity));
  }

  if (source.has<TransformComponent>(entity)) {
    target.assign<TransformComponent>(copy, source.get<TransformComponent>(entity));
  }

  if (source.has<SpriteComponent>(entity)) {
    const SpriteComponent& sprite = source.get<SpriteComponent>(entity);
    Sprite* sprite_copy = sprite.m_sprite == nullptr ? nullptr : new Sprite{*sprite.m_sprite};
    target.assign<SpriteComponent>(copy, SpriteComponent{sprite_copy});
  }

  if (source.has<SpriteAppearanceComponent>(entity)) {
    target.assign<SpriteAppearanceComponent>(copy, source.get<SpriteAppearanceComponent>(entity));
  }

  if (source.has<EntityEnabledComponent>(entity)) {
    target.assign<EntityEnabledComponent>(copy, source.get<EntityEnabledComponent>(entity));
  }

  if (source.has<DisabledComponentsComponent>(entity)) {
    target.assign<DisabledComponentsComponent>(copy, source.get<DisabledComponentsComponent>(entity));
  }

  if (source.has<AudioComponent>(entity)) {
    target.assign<AudioComponent>(copy, source.get<AudioComponent>(entity));
  }

  if (source.has<RigidBodyComponent>(entity)) {
    const RigidBodyComponent& rigidbody = source.get<RigidBodyComponent>(entity);
    Rigidbody* body_copy = rigidbody.m_rigidbody == nullptr ? nullptr : rigidbody.m_rigidbody->Clone();
    target.assign<RigidBodyComponent>(copy, RigidBodyComponent{body_copy});
  }

  return copy;
}
}  // namespace rse
