#include "entity.h"

#include <ecs/components/uuid.h>

namespace rse {
bool Entity::IsValid() const { return m_scene != nullptr && m_scene->m_registry.valid(m_handle); }

void Entity::Destroy() {
  if (m_scene != nullptr) {
    m_scene->DestroyEntity(*this);
  }
}

Uuid Entity::GetUuid() const { return m_scene->m_registry.get<UuidComponent>(m_handle).m_uuid; }
}  // namespace rse
