#pragma once

#include <algorithm>
#include <core.h>
#include <imgui.h>
#include <string>

#include <layout/editor_panel.h>
#include <render/window.h>
#include <utils/logger.h>

// ---------------------------------------------------------------------------
// Edit > Project Settings: the manifest's audio levels and frame pacing.
//
// A panel rather than a modal. Both apply as they change, so a level is set by
// ear and VSync is judged by the fan; Save writes just the manifest and Cancel
// puts everything back. The snapshot Cancel restores is taken when the panel is
// shown, so reopening it starts from what is on screen now.
//
// Starts hidden and opens from the Edit menu; the default layout docks it next
// to the inspector.
// ---------------------------------------------------------------------------
class ProjectSettingsPanel : public EditorPanel {
 public:
  ProjectSettingsPanel() { visible = false; }

  const char* Id() const override { return "Project Settings"; }
  DockSlot Slot() const override { return DockSlot::kRight; }

  void Shown() override { m_snapshot_taken = false; }

  void Draw() override {
    if (!ImGui::Begin(Id(), &visible)) {
      ImGui::End();
      return;
    }

    rse::Project& project = Core::GetProject();

    if (!project.IsAttached()) {
      ImGui::TextDisabled("No project is open.");
      ImGui::End();
      return;
    }

    // Taken lazily as well as from Shown(): a saved panel.* entry can bring the
    // panel up at launch without going through the Edit menu.
    if (!m_snapshot_taken) {
      m_before = project.audio;
      m_before_pacing = project.frame_pacing;
      m_snapshot_taken = true;
    }

    bool changed = false;

    ImGui::TextUnformatted("Audio");
    changed |= ImGui::SliderInt("Master", &project.audio.master, 0, rse::SoundManager::kMaxVolume);
    changed |= ImGui::SliderInt("Music", &project.audio.music, 0, rse::SoundManager::kMaxVolume);
    changed |= ImGui::SliderInt("Sound effects", &project.audio.sfx, 0, rse::SoundManager::kMaxVolume);

    if (changed) {
      // Heard while the slider moves, not after a restart: the panel writes the
      // manifest only on Save, but a level you cannot hear is a level you
      // cannot set.
      rse::SoundManager::SetVolumes(project.audio.master, project.audio.music, project.audio.sfx);
    }

    ImGui::Separator();

    ImGui::TextUnformatted("Frame pacing");

    // Applied on the spot, like the audio levels: VSync is judged by the fan and
    // the cap by the frame time next to it in the Performance panel, not after a
    // restart.
    bool vsync = project.frame_pacing.vsync;

    if (ImGui::Checkbox("VSync", &vsync)) {
      project.frame_pacing.vsync = vsync;
      Core::GetWindow()->SetVSync(vsync);
    }

    ImGui::InputInt("Frame cap", &project.frame_pacing.frame_cap);
    ImGui::SameLine();
    ImGui::TextDisabled("0 = uncapped");

    // The manifest is clamped on load; clamping here keeps the live value in the
    // same range instead of showing a number the file would not keep.
    project.frame_pacing.frame_cap = std::clamp(project.frame_pacing.frame_cap, 0, 1000);

    ImGui::Separator();

    if (ImGui::Button("Save")) {
      std::string error;

      if (!project.Save(&error)) {
        Logger::Error("Failed to save the project settings: " + error);
      } else {
        Logger::Log("Saved the project settings to " + project.File().string());
      }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel")) {
      project.audio = m_before;
      project.frame_pacing = m_before_pacing;
      rse::SoundManager::SetVolumes(project.audio.master, project.audio.music, project.audio.sfx);
      Core::GetWindow()->SetVSync(project.frame_pacing.vsync);
      m_snapshot_taken = false;
    }

    ImGui::End();
  }

 private:
  rse::Project::Audio m_before;
  rse::Project::FramePacing m_before_pacing;
  bool m_snapshot_taken{false};
};
