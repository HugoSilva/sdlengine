#pragma once

#include <imgui.h>
#include <memory>
#include <string>
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
// layout is only generated when there is nothing to restore, when the layout
// version in editor.cfg is older than this build, or when the user asks for it
// from View > Reset Layout.
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

  // Shows the panel with `id` and focuses it, or does nothing if no panel has
  // that id. The default layout docks a hidden panel too, so this brings it up
  // as a tab rather than as a floating window.
  void ShowPanel(const std::string& id);

  std::vector<std::unique_ptr<EditorPanel>> m_panels;
  bool m_layout_built{false};
  bool m_reset_layout{false};

  // Set on the first frame and when the default layout is built, consumed
  // after the panels have drawn, when their windows exist. ImGui selects the
  // last submitted tab in a node, which would otherwise leave the console in
  // front of the content browser.
  bool m_select_content_browser{false};

  // A panel opened from the Edit menu, focused after it has drawn this frame.
  std::string m_focus_panel;
};
