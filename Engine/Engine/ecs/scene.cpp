#include "scene.h"

#include <core/uuid.h>
#include <ecs/components/uuid.h>
#include <ecs/entity.h>

namespace rse {
Entity Scene::CreateEntity() {
  const uint32_t handle = m_registry.create();
  m_registry.assign<UuidComponent>(handle, Uuid::Generate());
  return Entity{this, handle};
}

void Scene::DestroyEntity(const Entity& entity) {
  if (!Contains(entity)) {
    return;
  }

  m_registry.destroy(entity.m_handle);
}

bool Scene::Contains(const Entity& entity) const { return entity.m_scene == this && m_registry.valid(entity.m_handle); }

std::size_t Scene::EntityCount() const { return m_registry.alive(); }
}  // namespace rse
