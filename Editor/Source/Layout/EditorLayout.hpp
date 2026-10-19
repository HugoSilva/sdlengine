#pragma once

#include <imgui.h>
#include <memory>
#include <vector>

#include "EditorPanel.hpp"

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

  // Draws the whole editor UI for the current frame: menu bar, dock space,
  // then every visible panel.
  void Draw();

  void RequestResetLayout() { m_ResetLayout = true; }

 private:
  void DrawMenuBar();
  void DrawDockSpace();
  void BuildDefaultLayout(ImGuiID dockspaceId);

  std::vector<std::unique_ptr<EditorPanel>> m_Panels;
  bool m_LayoutBuilt{false};
  bool m_ResetLayout{false};
};
