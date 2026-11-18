#include "scene.h"

#include <box2d/box2d.h>

#include <core/uuid.h>
#include <ecs/components/entity_enabled.h>
#include <ecs/components/uuid.h>
#include <ecs/entity.h>

namespace rse {
Scene::Scene() : m_world(std::make_unique<b2World>(b2Vec2{0.0f, -10.0f})) {}

// The registry is destroyed before the world even without this: both are
// members, and the world is declared first. Keeping the destructor here is what
// lets b2World stay incomplete in the header.
Scene::~Scene() = default;

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

bool Scene::IsEntityEnabled(uint32_t entity) const {
  if (!m_registry.has<EntityEnabledComponent>(entity)) {
    return true;
  }

  return m_registry.get<EntityEnabledComponent>(entity).m_enabled;
}

void Scene::Clear() {
  m_registry.reset();
  m_camera = Camera{};
}
}  // namespace rse
