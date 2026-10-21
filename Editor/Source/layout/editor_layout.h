#pragma once

#include <imgui.h>
#include <memory>
#include <vector>

#include "editor_panel.h"

// Only a reference in SyncVisibility's signature; the layout does not own or
// read the settings itself.
struct EditorSettings;

// ---------------------------------------------------------------------------
// Owns the editor's dock space and the set of dockable panels.
//
// The layout itself -- which panel sits where, node sizes, floating windows --
// is stored by ImGui in imgui.ini and restored automatically. The default
// layout is only generated when there is nothing to restore, or when the user
// asks for it from View > Reset Layout.
// ---------------------------------------------------------------------------
class EditorLayout {
 public:
  void AddPanel(std::unique_ptr<EditorPanel> panel);

  // Copies every panel's current visibility into the settings, so the next run
  // starts with the same set. Called when the editor shuts down.
  void SyncVisibility(EditorSettings& settings) const;

  // Draws the whole editor UI for the current frame: menu bar, dock space,
  // then every visible panel.
  void Draw();

  // Tells every panel that a different project is open. Called once per change
  // from EditorApp, not once a frame.
  void ProjectChanged();

  void RequestResetLayout() { m_reset_layout = true; }

 private:
  void DrawMenuBar();
  void DrawDockSpace();
  void BuildDefaultLayout(ImGuiID dockspace_id);

  std::vector<std::unique_ptr<EditorPanel>> m_panels;
  bool m_layout_built{false};
  bool m_reset_layout{false};
};
