#include "editor_manager.h"

#include <utils/logger.h>

uint32_t EditorManager::m_selected_entity{INVALID_ENTITY_ID};
rse::Framebuffer* EditorManager::m_scene_target{nullptr};
EditorSettings EditorManager::m_settings;
bool EditorManager::m_frame_requested{false};

void EditorManager::Init() {
  // Before anything can read a setting or draw a panel with one. Called from
  // the setup view's constructor, so it happens once.
  m_settings.Load();

  EventManager::GetEventDispatcher()->sink<CreateEntity>().connect<&EditorManager::HandleCreateEntityEvent>();
  EventManager::GetEventDispatcher()->sink<SelectEntity>().connect<&EditorManager::HandleSelectEntityEvent>();
  EventManager::GetEventDispatcher()->sink<DestroyEntity>().connect<&EditorManager::HandleDestroyEntityEvent>();
  EventManager::GetEventDispatcher()->sink<AddComponent>().connect<&EditorManager::HandleAddComponentEvent>();
  EventManager::GetEventDispatcher()->sink<RemoveComponent>().connect<&EditorManager::HandleRemoveComponentEvent>();
}

void EditorManager::HandleCreateEntityEvent(const CreateEntity& event) {
  m_selected_entity = rse::ECSManager::CreateEntity();
  rse::ECSManager::AddComponent<BasicComponent>(m_selected_entity);
  rse::ECSManager::AddComponent<TransformComponent>(m_selected_entity);
  // Untextured: the editor used to hand every new entity a sample project's
  // image by name, which meant new entities in any other project pointed at a
  // texture that project did not have.
  rse::ECSManager::AddComponent<SpriteComponent>(m_selected_entity,
                                                 new rse::Sprite(glm::vec3(0, 0, 0), glm::vec2(40, 40), 0xffffffffu));

  SceneManager::MarkDirty();
}

void EditorManager::HandleSelectEntityEvent(const SelectEntity& event) { SetSelection(event.value); }

std::span<const uint32_t> EditorManager::GetSelectedEntities() {
  // One element, materialised on demand: the storage is still the single
  // active entity, so there is no second copy of the selection to keep in
  // step with the first. The span is valid until the selection changes.
  static uint32_t active[1];

  if (m_selected_entity == INVALID_ENTITY_ID) {
    return {};
  }

  active[0] = m_selected_entity;
  return std::span<const uint32_t>(active, 1);
}

void EditorManager::HandleDestroyEntityEvent(const DestroyEntity& event) {
  if (event.value == INVALID_ENTITY_ID) {
    return;
  }

  // Drop the selection first: the inspector must not keep reading a dead id.
  if (m_selected_entity == event.value) {
    m_selected_entity = INVALID_ENTITY_ID;
  }

  rse::ECSManager::DestroyEntity(event.value);
  SceneManager::MarkDirty();
}

void EditorManager::HandleAddComponentEvent(const AddComponent& event) {
  switch (event.component) {
    case Components::kAudio:
      if (rse::ECSManager::HasComponent<AudioComponent>(m_selected_entity)) {
        Logger::Error("Already existing audio component");
      } else {
        rse::ECSManager::AddComponent<AudioComponent>(m_selected_entity, rse::SoundManager::GetSound("eff"));
        SceneManager::MarkDirty();
      }
      break;
    case Components::kRigidbody:
      if (rse::ECSManager::HasComponent<RigidBodyComponent>(m_selected_entity)) {
        Logger::Error("Already existing rigidbody component");
      } else {
        glm::vec2 position{0.f, 0.f};
        glm::vec2 size{0.f, 0.f};
        if (rse::ECSManager::HasComponent<TransformComponent>(m_selected_entity)) {
          position = rse::ECSManager::GetComponent<TransformComponent>(m_selected_entity).position;
        }
        if (rse::ECSManager::HasComponent<SpriteComponent>(m_selected_entity)) {
          size = rse::ECSManager::GetComponent<SpriteComponent>(m_selected_entity).m_sprite->GetSize();
        }
        rse::ECSManager::AddComponent<RigidBodyComponent>(m_selected_entity, new Rigidbody(position, size));
        SceneManager::MarkDirty();
      }
      break;
    case Components::kSprite:
      if (rse::ECSManager::HasComponent<SpriteComponent>(m_selected_entity)) {
        Logger::Error("Already existing sprite component");
      } else {
        rse::ECSManager::AddComponent<SpriteComponent>(m_selected_entity);
        SceneManager::MarkDirty();
      }
      break;
    default:
      break;
  }
}

void EditorManager::HandleRemoveComponentEvent(const RemoveComponent& event) {
  if (m_selected_entity == INVALID_ENTITY_ID) {
    return;
  }

  // Only the optional components can go: the hierarchy lists entities by their
  // BasicComponent and the renderer needs a transform, so those two stay.
  switch (event.component) {
    case Components::kAudio:
      if (!rse::ECSManager::HasComponent<AudioComponent>(m_selected_entity)) {
        Logger::Error("Missing audio component");
        break;
      }
      // Components hold their payload in a shared_ptr, so removing one
      // releases it properly.
      rse::ECSManager::RemoveComponent<AudioComponent>(m_selected_entity);
      SceneManager::MarkDirty();
      break;
    case Components::kRigidbody:
      if (!rse::ECSManager::HasComponent<RigidBodyComponent>(m_selected_entity)) {
        Logger::Error("Missing rigidbody component");
        break;
      }
      rse::ECSManager::RemoveComponent<RigidBodyComponent>(m_selected_entity);
      SceneManager::MarkDirty();
      break;
    case Components::kSprite:
      if (!rse::ECSManager::HasComponent<SpriteComponent>(m_selected_entity)) {
        Logger::Error("Missing sprite component");
        break;
      }
      rse::ECSManager::RemoveComponent<SpriteComponent>(m_selected_entity);
      SceneManager::MarkDirty();
      break;
    default:
      break;
  }
}

void EditorManager::PlayScene() {
  std::string error;

  if (!SceneManager::Play(&error)) {
    Logger::Error("Could not play the scene: " + error);
  }
}

void EditorManager::PauseScene() { SceneManager::Pause(); }

void EditorManager::StopScene() {
  std::string error;

  if (!SceneManager::Stop(&error)) {
    Logger::Error("Could not stop the scene: " + error);
  }

  ValidateSelection();
}

void EditorManager::ValidateSelection() {
  if (m_selected_entity != INVALID_ENTITY_ID && !rse::ECSManager::IsValidEntity(m_selected_entity)) {
    m_selected_entity = INVALID_ENTITY_ID;
  }
}
