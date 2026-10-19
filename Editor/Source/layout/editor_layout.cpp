#include "editor_layout.h"

#include <core.h>
#include <editor_manager.h>
#include <imgui_internal.h>  // DockBuilder*

#include <SDL3/SDL.h>

namespace {
constexpr const char* kDockSpaceId = "RSE_DockSpace";

// The host window is never visible: it just carries the dock space, so it
// has no title bar and no background of its own.
constexpr const char* kDockSpaceHost = "###RSE_DockSpaceHost";
}  // namespace

void EditorLayout::AddPanel(std::unique_ptr<EditorPanel> panel) { m_panels.push_back(std::move(panel)); }

void EditorLayout::Draw() {
  // The scene target is deliberately *not* reset here. The viewport panel owns
  // it and publishes it whenever it is given the chance to draw, so hiding or
  // collapsing the panel keeps the scene offscreen instead of letting it fall
  // through to the window -- which the dock space, having no background of its
  // own, would then show through.

  // The menu bar has to be submitted first: it is what shrinks the viewport
  // work area that the dock space then fills.
  DrawMenuBar();
  DrawDockSpace();

  for (const std::unique_ptr<EditorPanel>& panel : m_panels) {
    if (panel->visible) {
      panel->Draw();
    }
  }
}

void EditorLayout::DrawMenuBar() {
  if (!ImGui::BeginMainMenuBar()) {
    return;
  }

  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("New")) {
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Save", "F1")) {
      rse::ECSManager::Save();
    }
    if (ImGui::MenuItem("Load", "F2")) {
      rse::ECSManager::Load();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Quit", "Alt+F4")) {
      SDL_Event sdlevent;
      sdlevent.type = SDL_EVENT_QUIT;
      SDL_PushEvent(&sdlevent);
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Edit")) {
    if (ImGui::MenuItem("Undo", "CTRL+Z")) {
    }
    if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Cut", "CTRL+X")) {
    }
    if (ImGui::MenuItem("Copy", "CTRL+C")) {
    }
    if (ImGui::MenuItem("Paste", "CTRL+V")) {
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("View")) {
    for (const std::unique_ptr<EditorPanel>& panel : m_panels) {
      ImGui::MenuItem(panel->Id(), nullptr, &panel->visible);
    }
    ImGui::Separator();

    // MenuItem flips the bool itself and reports the click, so by the time
    // this returns, the setting is already the new value and saving cannot
    // write the old one.
    if (ImGui::MenuItem("Show Grid", nullptr, &EditorManager::Settings().grid.visible)) {
      EditorManager::Settings().Save();
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Reset Layout")) {
      RequestResetLayout();
    }
    ImGui::EndMenu();
  }

  ImGui::SameLine(ImGui::GetWindowWidth() - 160);
  ImGui::Text("%.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

  ImGui::EndMainMenuBar();
}

void EditorLayout::DrawDockSpace() {
  const ImGuiViewport* viewport = ImGui::GetMainViewport();

  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);

  const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                 ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoDocking |
                                 ImGuiWindowFlags_NoBackground;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  ImGui::Begin(kDockSpaceHost, nullptr, flags);
  ImGui::PopStyleVar(3);

  const ImGuiID dockspace_id = ImGui::GetID(kDockSpaceId);

  // An existing dock node means imgui.ini supplied one, so the user's layout
  // is left alone. Only build the default when there is nothing to restore,
  // or when a reset was requested.
  const bool has_saved_layout = ImGui::DockBuilderGetNode(dockspace_id) != nullptr;
  if (m_reset_layout || (!m_layout_built && !has_saved_layout)) {
    BuildDefaultLayout(dockspace_id);
    m_reset_layout = false;
  }
  m_layout_built = true;

  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

  ImGui::End();
}

void EditorLayout::BuildDefaultLayout(ImGuiID dockspace_id) {
  // Viewport fills the centre, the console spans the bottom, the hierarchy
  // column sits on the left and the inspector hugs the right edge. Which panel
  // asks for which slot is up to the panel (EditorPanel::slot()).
  bool needs_left = false;
  bool needs_right = false;
  bool needs_bottom = false;

  for (const std::unique_ptr<EditorPanel>& panel : m_panels) {
    switch (panel->Slot()) {
      case DockSlot::kLeft:
        needs_left = true;
        break;
      case DockSlot::kRight:
        needs_right = true;
        break;
      case DockSlot::kBottom:
        needs_bottom = true;
        break;
      case DockSlot::kCenter:
        break;
    }
  }

  const ImGuiViewport* viewport = ImGui::GetMainViewport();

  ImGui::DockBuilderRemoveNode(dockspace_id);
  ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->WorkSize);

  ImGuiID center = dockspace_id;
  ImGuiID left = 0;
  ImGuiID right = 0;
  ImGuiID bottom = 0;

  // The bottom strip is split off first so it spans the full width rather than
  // being boxed into one of the columns.
  if (needs_bottom) {
    bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.25f, nullptr, &center);
  }
  if (needs_left) {
    left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.22f, nullptr, &center);
  }
  if (needs_right) {
    right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.32f, nullptr, &center);
  }

  for (const std::unique_ptr<EditorPanel>& panel : m_panels) {
    ImGuiID target = center;

    switch (panel->Slot()) {
      case DockSlot::kLeft:
        target = left;
        break;
      case DockSlot::kRight:
        target = right;
        break;
      case DockSlot::kBottom:
        target = bottom;
        break;
      case DockSlot::kCenter:
        target = center;
        break;
    }

    if (target != 0) {
      ImGui::DockBuilderDockWindow(panel->Id(), target);
    }
  }

  ImGui::DockBuilderFinish(dockspace_id);
}
