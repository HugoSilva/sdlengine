#pragma once

#include <span>

#include "Core.h"
#include "EditorSettings.hpp"
#define INVALID_ENTITY_ID entt::null

enum class Components { Audio, Rigidbody, Sprite };

class EditorManager {
 public:
  struct CreateEntity {};
  struct SelectEntity {
    uint32_t value;
  };
  struct DestroyEntity {
    uint32_t value;
  };
  struct AddComponent {
    Components component;
  };
  struct RemoveComponent {
    Components component;
  };

  EditorManager() {}
  ~EditorManager() {}

  static void Init();

  // Selection is shared state: the hierarchy panel drives it, the viewport
  // picks into it, the inspector panel reads it.
  //
  // The set is shaped for multi-select even though it cannot hold more than
  // one entity yet. Ctrl is reserved for that follow-up (see the plan's
  // section 3.4), so the outline and the gizmos iterate
  // getSelectedEntities() rather than the single entity, and "the set has
  // more than one element" becomes the only thing that changes.
  static void SetSelection(uint32_t entity) { m_SelectedEntity = entity; }

  // Ctrl-click is its caller. Added now so that epic changes one call site
  // instead of three gizmos and the outline.
  static void ToggleSelection(uint32_t entity) {
    if (entity == INVALID_ENTITY_ID) {
      return;
    }

    m_SelectedEntity = (m_SelectedEntity == entity) ? INVALID_ENTITY_ID : entity;
  }

  static void ClearSelection() { m_SelectedEntity = INVALID_ENTITY_ID; }

  // The *active* entity: the one the inspector edits and the hierarchy
  // highlights, and the anchor a gizmo drag is measured from.
  static uint32_t GetSelectedEntity() { return m_SelectedEntity; }

  // The selection set. One element while multi-select is out of scope.
  static std::span<const uint32_t> GetSelectedEntities();

  // The scene is rendered into the viewport panel's framebuffer rather than
  // the window. The viewport claims this while it draws; Core::run() picks it
  // up for the scene pass, and nullptr means "draw straight to the window".
  static void SetSceneTarget(rse::Framebuffer* target) { m_SceneTarget = target; }
  static rse::Framebuffer* GetSceneTarget() { return m_SceneTarget; }

  // View settings. The View menu toggles them, the systems that draw with them
  // read them, and M7 persists them -- so they need a home the menu, the panels
  // and the systems can all reach, which is this one.
  static EditorSettings& Settings() { return m_Settings; }

  // A one-shot request for the viewport to frame what is selected. The
  // hierarchy's double-click asks for it and the viewport answers it, so it
  // belongs with the rest of the state the two panels share rather than in
  // either one of them.
  static void RequestFrameSelection() { m_FrameRequested = true; }
  static bool ConsumeFrameRequest() {
    const bool requested = m_FrameRequested;
    m_FrameRequested = false;
    return requested;
  }

  template <typename Component>
  static bool ShowComponentWidget() {
    if (m_SelectedEntity != INVALID_ENTITY_ID) {
      return rse::ECSManager::HasComponent<Component>(m_SelectedEntity);
    }
    return false;
  }

  template <typename Component>
  static Component& GetComponent() {
    return rse::ECSManager::GetComponent<Component>(m_SelectedEntity);
  }

  static void HandleCreateEntityEvent(const CreateEntity& event);
  static void HandleSelectEntityEvent(const SelectEntity& event);
  static void HandleDestroyEntityEvent(const DestroyEntity& event);
  static void HandleAddComponentEvent(const AddComponent& event);
  static void HandleRemoveComponentEvent(const RemoveComponent& event);

 private:
  static uint32_t m_SelectedEntity;
  static rse::Framebuffer* m_SceneTarget;
  static EditorSettings m_Settings;
  static bool m_FrameRequested;
};
