#include "EditorManager.h"

uint32_t EditorManager::m_SelectedEntity{INVALID_ENTITY_ID};
rse::Framebuffer* EditorManager::m_SceneTarget{nullptr};
EditorSettings EditorManager::m_Settings;
bool EditorManager::m_FrameRequested{false};

void EditorManager::Init() {
  // Before anything can read a setting or draw a panel with one. Called from
  // the setup view's constructor, so it happens once.
  m_Settings.Load();

  EventManager::GetEventDispatcher()->sink<CreateEntity>().connect<&EditorManager::HandleCreateEntityEvent>();
  EventManager::GetEventDispatcher()->sink<SelectEntity>().connect<&EditorManager::HandleSelectEntityEvent>();
  EventManager::GetEventDispatcher()->sink<DestroyEntity>().connect<&EditorManager::HandleDestroyEntityEvent>();
  EventManager::GetEventDispatcher()->sink<AddComponent>().connect<&EditorManager::HandleAddComponentEvent>();
  EventManager::GetEventDispatcher()->sink<RemoveComponent>().connect<&EditorManager::HandleRemoveComponentEvent>();
}

void EditorManager::HandleCreateEntityEvent(const CreateEntity& event) {
  m_SelectedEntity = rse::ECSManager::CreateEntity();
  rse::ECSManager::AddComponent<BasicComponent>(m_SelectedEntity);
  rse::ECSManager::AddComponent<TransformComponent>(m_SelectedEntity);
  rse::ECSManager::AddComponent<SpriteComponent>(
      m_SelectedEntity, new rse::Sprite(glm::vec3(0, 0, 0), glm::vec2(40, 40), rse::TextureManager::Get("test00")));
}

void EditorManager::HandleSelectEntityEvent(const SelectEntity& event) { SetSelection(event.value); }

std::span<const uint32_t> EditorManager::GetSelectedEntities() {
  // One element, materialised on demand: the storage is still the single
  // active entity, so there is no second copy of the selection to keep in
  // step with the first. The span is valid until the selection changes.
  static uint32_t active[1];

  if (m_SelectedEntity == INVALID_ENTITY_ID) {
    return {};
  }

  active[0] = m_SelectedEntity;
  return std::span<const uint32_t>(active, 1);
}

void EditorManager::HandleDestroyEntityEvent(const DestroyEntity& event) {
  if (event.value == INVALID_ENTITY_ID) {
    return;
  }

  // Drop the selection first: the inspector must not keep reading a dead id.
  if (m_SelectedEntity == event.value) {
    m_SelectedEntity = INVALID_ENTITY_ID;
  }

  rse::ECSManager::DestroyEntity(event.value);
}

void EditorManager::HandleAddComponentEvent(const AddComponent& event) {
  switch (event.component) {
    case Components::Audio:
      if (rse::ECSManager::HasComponent<AudioComponent>(m_SelectedEntity)) {
        Logger::Error("Already existing audio component");
      } else {
        rse::ECSManager::AddComponent<AudioComponent>(m_SelectedEntity, rse::SoundManager::GetSound("eff"));
      }
      break;
    case Components::Rigidbody:
      if (rse::ECSManager::HasComponent<RigidBodyComponent>(m_SelectedEntity)) {
        Logger::Error("Already existing rigidbody component");
      } else {
        glm::vec2 position{0.f, 0.f};
        glm::vec2 size{0.f, 0.f};
        if (rse::ECSManager::HasComponent<TransformComponent>(m_SelectedEntity)) {
          position = rse::ECSManager::GetComponent<TransformComponent>(m_SelectedEntity).position;
        }
        if (rse::ECSManager::HasComponent<SpriteComponent>(m_SelectedEntity)) {
          size = rse::ECSManager::GetComponent<SpriteComponent>(m_SelectedEntity).m_Sprite->GetSize();
        }
        rse::ECSManager::AddComponent<RigidBodyComponent>(m_SelectedEntity, new Rigidbody(position, size));
      }
      break;
    case Components::Sprite:
      if (rse::ECSManager::HasComponent<SpriteComponent>(m_SelectedEntity)) {
        Logger::Error("Already existing sprite component");
      } else {
        rse::ECSManager::AddComponent<SpriteComponent>(m_SelectedEntity);
      }
      break;
    default:
      break;
  }
}

void EditorManager::HandleRemoveComponentEvent(const RemoveComponent& event) {
  if (m_SelectedEntity == INVALID_ENTITY_ID) {
    return;
  }

  // Only the optional components can go: the hierarchy lists entities by their
  // BasicComponent and the renderer needs a transform, so those two stay.
  switch (event.component) {
    case Components::Audio:
      if (!rse::ECSManager::HasComponent<AudioComponent>(m_SelectedEntity)) {
        Logger::Error("Missing audio component");
        break;
      }
      // Components hold their payload in a shared_ptr, so removing one
      // releases it properly.
      rse::ECSManager::RemoveComponent<AudioComponent>(m_SelectedEntity);
      break;
    case Components::Rigidbody:
      if (!rse::ECSManager::HasComponent<RigidBodyComponent>(m_SelectedEntity)) {
        Logger::Error("Missing rigidbody component");
        break;
      }
      rse::ECSManager::RemoveComponent<RigidBodyComponent>(m_SelectedEntity);
      break;
    case Components::Sprite:
      if (!rse::ECSManager::HasComponent<SpriteComponent>(m_SelectedEntity)) {
        Logger::Error("Missing sprite component");
        break;
      }
      rse::ECSManager::RemoveComponent<SpriteComponent>(m_SelectedEntity);
      break;
    default:
      break;
  }
}