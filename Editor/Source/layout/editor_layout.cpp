#include "editor_layout.h"

#include <core.h>
#include <cstddef>
#include <editor_manager.h>
#include <filesystem>
#include <imgui_internal.h>  // DockBuilder*
#include <project_commands.h>
#include <string>

#include <SDL3/SDL.h>

#include <utils/logger.h>

namespace {
constexpr const char* kDockSpaceId = "RSE_DockSpace";

// Bumped when the default layout changes in a way everybody should see. An
// older editor.cfg rebuilds the layout once; imgui.ini takes over from there.
constexpr int kLayoutVersion = 1;

// The host window is never visible: it just carries the dock space, so it
// has no title bar and no background of its own.
constexpr const char* kDockSpaceHost = "###RSE_DockSpaceHost";

// File > Open Recent. Load already pruned the paths that were gone when the
// settings were read, so a missing entry here is a folder that vanished while
// the editor was running: it is removed and said once rather than opened, and
// removing it now is what keeps it from saying so every time.
void OpenRecentProject(EditorSettings& settings, std::size_t index) {
  if (index >= settings.recent_projects.size()) {
    return;
  }

  const std::string path = settings.recent_projects.at(index);

  if (!std::filesystem::exists(path)) {
    Logger::Warning("The project " + path + " is gone; removing it from the recent list.");
    settings.ForgetRecentProject(index);
    settings.Save();
    return;
  }

  OpenProjectAt(path);
}
}  // namespace

void EditorLayout::AddPanel(std::unique_ptr<EditorPanel> panel) {
  // The saved state is read before the panels are built, so a panel the user
  // had hidden comes back hidden. An id the file does not mention keeps the
  // panel's own default.
  const std::map<std::string, bool>& saved = EditorManager::Settings().panels;
  const std::map<std::string, bool>::const_iterator found = saved.find(panel->Id());

  if (found != saved.end()) {
    panel->visible = found->second;
  }

  m_panels.push_back(std::move(panel));
}

void EditorLayout::ShowPanel(const std::string& id) {
  for (const std::unique_ptr<EditorPanel>& panel : m_panels) {
    if (id != panel->Id()) {
      continue;
    }

    // Shown() only on the transition: a panel whose Cancel restores a snapshot
    // must not take a new one when the menu item merely focuses it again.
    if (!panel->visible) {
      panel->visible = true;
      panel->Shown();
    }

    m_focus_panel = panel->Id();
    return;
  }
}

void EditorLayout::SyncVisibility(EditorSettings& settings) const {
  for (const std::unique_ptr<EditorPanel>& panel : m_panels) {
    settings.panels[panel->Id()] = panel->visible;
  }
}

void EditorLayout::ProjectChanged() {
  for (const std::unique_ptr<EditorPanel>& panel : m_panels) {
    panel->ProjectChanged();
  }
}

void EditorLayout::Draw() {
  // On the first frame the editor opens on the content browser, whether the
  // layout was just built or restored. Panels are submitted in order and the
  // console comes after the browser, so without this the console would take
  // the tab bar on its activation frame.
  if (!m_layout_built) {
    m_select_content_browser = true;
  }

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

  // The windows exist now, so this can select a tab. Keeping the console from
  // being the default is the whole point: ImGui picks the last submitted tab
  // when the node has no selection yet.
  if (m_select_content_browser) {
    for (const std::unique_ptr<EditorPanel>& panel : m_panels) {
      if (std::string(panel->Id()) != "Content Browser" || !panel->visible) {
        continue;
      }

      ImGuiWindow* window = ImGui::FindWindowByName(panel->Id());

      if (window == nullptr) {
        break;
      }

      if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(window->DockId)) {
        // Both: focusing the window makes the tab bar pick it again on the
        // next frame (docking applies NavWindow focus back to the tab bar),
        // and the node keeps the choice if focus wanders.
        ImGui::SetWindowFocus(panel->Id());
        node->SelectedTabId = window->TabId;
        m_select_content_browser = false;
      }

      break;
    }
  }

  if (!m_focus_panel.empty()) {
    // An opened panel was drawn above, so focusing it both raises its window
    // and selects its tab. One shot: the user is free to focus elsewhere next.
    ImGui::SetWindowFocus(m_focus_panel.c_str());
    m_focus_panel.clear();
  }

  DrawProjectCommands();
}

void EditorLayout::DrawMenuBar() {
  if (!ImGui::BeginMainMenuBar()) {
    return;
  }

  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("New Project...")) {
      NewProject();
    }
    if (ImGui::MenuItem("Open Project...")) {
      OpenProject();
    }

    // The list is per machine and Load has already pruned the paths that are
    // gone; an entry that vanishes while the editor runs is dropped when it is
    // chosen. A submenu even with one entry, so File does not grow a row per
    // project.
    const EditorSettings& settings = EditorManager::Settings();
    if (ImGui::BeginMenu("Open Recent", !settings.recent_projects.empty())) {
      for (std::size_t index = 0; index < settings.recent_projects.size(); ++index) {
        // PushID keeps the items distinct by position; the path is the label
        // because two projects called "project.xml" are the normal case.
        ImGui::PushID(static_cast<int>(index));
        const bool clicked = ImGui::MenuItem(settings.recent_projects.at(index).c_str());
        ImGui::PopID();

        if (clicked) {
          OpenRecentProject(EditorManager::Settings(), index);
          break;  // the click may have removed the entry this loop is reading
        }
      }
      ImGui::EndMenu();
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Save", "Ctrl+S")) {
      SaveProject();
    }
    if (ImGui::MenuItem("Save Scene As...")) {
      SaveSceneAs();
    }
    if (ImGui::MenuItem("Reload Scene", "F2")) {
      ReloadScene();
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
    ImGui::Separator();

    if (ImGui::MenuItem("Editor Settings...")) {
      ShowPanel("Editor Settings");
    }

    // A project's audio levels are project data, so the item needs a project.
    if (ImGui::MenuItem("Project Settings...", nullptr, false, Core::GetProject().IsAttached())) {
      ShowPanel("Project Settings");
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Assets")) {
    const uint32_t selected = EditorManager::GetSelectedEntity();

    // Saving a prefab needs an entity; instantiating one needs nothing but a
    // file, so the item stays enabled even in an empty scene.
    if (ImGui::MenuItem("Create Prefab from Selection...", nullptr, false, selected != INVALID_ENTITY_ID)) {
      CreatePrefab(selected);
    }
    if (ImGui::MenuItem("Instantiate Prefab...")) {
      InstantiatePrefab();
    }
    ImGui::Separator();

    // An asset id is project data, so registering a file needs a project. The
    // content browser offers the same command for the file that is selected.
    if (ImGui::MenuItem("Import Asset...", nullptr, false, Core::GetProject().IsAttached())) {
      ImportAsset();
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Scenes", Core::GetProject().IsAttached())) {
    const ScenesMenuModel scenes = ScenesMenuModel::Build(Core::GetProject(), SceneManager::GetActiveScene());

    for (std::size_t index = 0; index < scenes.entries.size(); ++index) {
      const ScenesMenuModel::Entry& entry = scenes.entries.at(index);

      // Two scenes can share a stem in different folders, so the row is what
      // the id is made of, not the name.
      ImGui::PushID(static_cast<int>(index));

      // A tick rather than a disabled row: the menu stays a list of places to
      // go, and the tick says where you are.
      if (ImGui::MenuItem(entry.name.c_str(), nullptr, entry.active)) {
        SwitchScene(entry.path);
      }

      ImGui::PopID();
    }

    if (scenes.entries.empty()) {
      ImGui::TextDisabled("No scenes in the manifest");
    }

    ImGui::Separator();

    // With fewer than two scenes there is nowhere to go: the model says so and
    // the items say it too.
    const bool can_cycle = scenes.entries.size() > 1;
    if (ImGui::MenuItem("Next Scene", "Ctrl+Tab", false, can_cycle)) {
      NextScene();
    }
    if (ImGui::MenuItem("Previous Scene", "Ctrl+Shift+Tab", false, can_cycle)) {
      PreviousScene();
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
      SaveProject();
    }
    if (ImGui::MenuItem("Save Scene As...")) {
      SaveSceneAs();
    }
    if (ImGui::MenuItem("Reload Scene", "F2")) {
      ReloadScene();
    }

    ImGui::Separator();
    if (ImGui::MenuItem("New Scene...")) {
      NewScene();
    }
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("View")) {
    for (const std::unique_ptr<EditorPanel>& panel : m_panels) {
      ImGui::MenuItem(panel->Id(), nullptr, &panel->visible);
    }
    ImGui::Separator();

    if (ImGui::MenuItem("Reset Layout")) {
      RequestResetLayout();
    }
    ImGui::EndMenu();
  }

  // Right-aligned by the text's own width rather than a fixed offset, so the
  // readout stays inside the bar whatever the UI font size is.
  char fps_text[64];
  ImFormatString(fps_text, sizeof(fps_text), "%.2f FPS (%.2f ms)", ImGui::GetIO().Framerate,
                 1000.0f / ImGui::GetIO().Framerate);
  const float fps_width = ImGui::CalcTextSize(fps_text).x;
  ImGui::SameLine(ImGui::GetWindowWidth() - fps_width - ImGui::GetStyle().WindowPadding.x);
  ImGui::TextUnformatted(fps_text);

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

  // A layout saved by a build with a different default is rebuilt once, so the
  // new arrangement does not need View > Reset Layout on every existing
  // install. imgui.ini takes over again from the next run.
  EditorSettings& settings = EditorManager::Settings();
  if (settings.layout_version < kLayoutVersion) {
    m_reset_layout = true;
  }

  // An existing dock node means imgui.ini supplied one, so the user's layout
  // is left alone. Only build the default when there is nothing to restore,
  // when a reset was requested, or when the version says the default moved.
  const bool has_saved_layout = ImGui::DockBuilderGetNode(dockspace_id) != nullptr;
  if (m_reset_layout || (!m_layout_built && !has_saved_layout)) {
    BuildDefaultLayout(dockspace_id);
    m_reset_layout = false;

    if (settings.layout_version < kLayoutVersion) {
      settings.layout_version = kLayoutVersion;
      settings.Save();
    }
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

  // Consumed after the panels have drawn: the content browser has to be the
  // active tab, and ImGui would otherwise pick the last submitted one.
  m_select_content_browser = true;
}
