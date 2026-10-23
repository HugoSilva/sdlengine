#pragma once

#include <core.h>
#include <span>

#include "editor_settings.h"
#define INVALID_ENTITY_ID entt::null

enum class Components { kAudio, kRigidbody, kSprite };

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
  static void SetSelection(uint32_t entity) { m_selected_entity = entity; }

  // Ctrl-click is its caller. Added now so that epic changes one call site
  // instead of three gizmos and the outline.
  static void ToggleSelection(uint32_t entity) {
    if (entity == INVALID_ENTITY_ID) {
      return;
    }

    m_selected_entity = (m_selected_entity == entity) ? INVALID_ENTITY_ID : entity;
  }

  static void ClearSelection() { m_selected_entity = INVALID_ENTITY_ID; }

  // The play session, as one action each. The toolbar, the Scenes menu and the
  // shortcuts all come through here so error handling and the post-stop
  // selection cleanup live in one place.
  static void PlayScene();
  static void PauseScene();
  static void StopScene();

  // Drops a selection whose entity the current scene no longer contains. A
  // restore or a scene switch can leave the id pointing at an entity that is
  // gone; every panel that reads the selection would then be reading a dead id.
  static void ValidateSelection();

  // The *active* entity: the one the inspector edits and the hierarchy
  // highlights, and the anchor a gizmo drag is measured from.
  static uint32_t GetSelectedEntity() { return m_selected_entity; }

  // The selection set. One element while multi-select is out of scope.
  static std::span<const uint32_t> GetSelectedEntities();

  // The scene is rendered into the viewport panel's framebuffer rather than
  // the window. The viewport claims this while it draws; Core::run() picks it
  // up for the scene pass, and nullptr means "draw straight to the window".
  static void SetSceneTarget(rse::Framebuffer* target) { m_scene_target = target; }
  static rse::Framebuffer* GetSceneTarget() { return m_scene_target; }

  // View settings. The View menu toggles them, the systems that draw with them
  // read them, and M7 persists them -- so they need a home the menu, the panels
  // and the systems can all reach, which is this one.
  static EditorSettings& Settings() { return m_settings; }

  // A one-shot request for the viewport to frame what is selected. The
  // hierarchy's double-click asks for it and the viewport answers it, so it
  // belongs with the rest of the state the two panels share rather than in
  // either one of them.
  static void RequestFrameSelection() { m_frame_requested = true; }
  static bool ConsumeFrameRequest() {
    const bool requested = m_frame_requested;
    m_frame_requested = false;
    return requested;
  }

  template <typename Component>
  static bool ShowComponentWidget() {
    if (m_selected_entity != INVALID_ENTITY_ID) {
      return rse::ECSManager::HasComponent<Component>(m_selected_entity);
    }
    return false;
  }

  template <typename Component>
  static Component& GetComponent() {
    return rse::ECSManager::GetComponent<Component>(m_selected_entity);
  }

  static void HandleCreateEntityEvent(const CreateEntity& event);
  static void HandleSelectEntityEvent(const SelectEntity& event);
  static void HandleDestroyEntityEvent(const DestroyEntity& event);
  static void HandleAddComponentEvent(const AddComponent& event);
  static void HandleRemoveComponentEvent(const RemoveComponent& event);

 private:
  static uint32_t m_selected_entity;
  static rse::Framebuffer* m_scene_target;
  static EditorSettings m_settings;
  static bool m_frame_requested;
};
