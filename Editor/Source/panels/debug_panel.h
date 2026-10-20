#pragma once

#include <core.h>
#include <imgui.h>

#include <core/scene_manager.h>
#include <ecs/scene_io.h>
#include <layout/editor_panel.h>

// ---------------------------------------------------------------------------
// Home for the engine's debug gizmos.
//
// The scene owns its background now, so this panel edits the value the scene
// stores rather than a renderer-global clear colour: the colour shown here is
// what the next saved scene will carry, and what the renderer clears to every
// frame.
// ---------------------------------------------------------------------------
class DebugPanel : public EditorPanel {
 public:
  const char* Id() const override { return "Debug"; }
  DockSlot Slot() const override { return DockSlot::kLeft; }

  void Draw() override {
    if (!ImGui::Begin(Id(), &visible)) {
      ImGui::End();
      return;
    }

    rse::SceneIO::SceneCamera& camera = rse::SceneIO::GetCamera();
    float background[3] = {camera.background.r, camera.background.g, camera.background.b};

    if (ImGui::ColorEdit3("Background", background)) {
      camera.background = glm::vec3(background[0], background[1], background[2]);
      SceneManager::MarkDirty();
    }

    ImGui::SameLine();
    ImGui::TextDisabled("(the open scene)");

    ImGui::End();
  }
};
