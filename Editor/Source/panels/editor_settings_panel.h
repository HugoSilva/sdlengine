#pragma once

#include <core.h>
#include <editor_manager.h>
#include <imgui.h>

#include <core/scene_manager.h>
#include <ecs/scene_io.h>
#include <layout/editor_panel.h>

// ---------------------------------------------------------------------------
// Edit > Editor Settings: options that describe how the editor looks.
//
// The grid is the editor's own setting and lives in editor.cfg. The background
// is the open scene's value, not an editor preference: changing it marks the
// scene dirty and saving the scene stores it, so the game clears to the same
// colour.
//
// Starts hidden and opens from the Edit menu; the default layout docks it next
// to the inspector.
// ---------------------------------------------------------------------------
class EditorSettingsPanel : public EditorPanel {
 public:
  EditorSettingsPanel() { visible = false; }

  const char* Id() const override { return "Editor Settings"; }
  DockSlot Slot() const override { return DockSlot::kRight; }

  void Draw() override {
    if (!ImGui::Begin(Id(), &visible)) {
      ImGui::End();
      return;
    }

    EditorSettings& settings = EditorManager::Settings();

    // MenuItem used to flip the bool and report the click; a checkbox does the
    // same, so saving immediately still cannot write the old value.
    if (ImGui::Checkbox("Show Grid", &settings.grid.visible)) {
      settings.Save();
    }

    ImGui::Separator();

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
