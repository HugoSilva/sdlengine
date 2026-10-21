#include "project_commands.h"

#include <algorithm>
#include <core.h>
#include <imgui.h>
#include <string>
#include <system_error>

#include <SDL3/SDL.h>
#include <entity/entity.hpp>

#include <core/project.h>
#include <core/scene_manager.h>
#include <ecs/prefab_io.h>
#include <ecs/scene_io.h>
#include <utils/logger.h>

namespace {
// What a chosen path is for. The dialog callback carries one of these through
// its userdata, so a single callback serves every dialog.
enum class PathPromptKind { kNone, kNewProjectFolder, kOpenProject, kSaveSceneAs, kCreatePrefab, kInstantiatePrefab };

// The typed-path fallback. One at a time: a dialog can only be started from a
// menu, and a modal prompt blocks the menu.
struct PathPrompt {
  PathPromptKind kind{PathPromptKind::kNone};
  char path[512]{};
  bool needs_focus{false};
  // Only a prefab save uses it: the entity selected when the command ran.
  uint32_t entity{entt::null};
};

PathPrompt g_prompt;

constexpr const char* kPromptTitle = "Path";
constexpr int kPromptWidth = 520;

// File-static so the pointer handed to the asynchronous dialog stays valid.
constexpr SDL_DialogFileFilter kProjectFilters[]{{"RSE project", "xml"}};
constexpr SDL_DialogFileFilter kSceneFilters[]{{"RSE scene", "xml"}};
constexpr SDL_DialogFileFilter kPrefabFilters[]{{"RSE prefab", "xml"}};

void BeginPrompt(PathPromptKind kind, uint32_t entity = entt::null) {
  g_prompt.kind = kind;
  g_prompt.path[0] = '\0';
  g_prompt.needs_focus = true;
  g_prompt.entity = entity;
}

const char* PromptText(PathPromptKind kind) {
  switch (kind) {
    case PathPromptKind::kNewProjectFolder:
      return "Folder to create the project in:";
    case PathPromptKind::kOpenProject:
      return "Path to a project.xml:";
    case PathPromptKind::kSaveSceneAs:
      return "Path for the scene file:";
    case PathPromptKind::kCreatePrefab:
      return "Path for the prefab file:";
    case PathPromptKind::kInstantiatePrefab:
      return "Path to a prefab file:";
    case PathPromptKind::kNone:
      break;
  }

  return "";
}

void ApplyChosenPath(PathPromptKind kind, const std::filesystem::path& path, uint32_t entity) {
  switch (kind) {
    case PathPromptKind::kNewProjectFolder:
      NewProjectAt(path);
      break;
    case PathPromptKind::kOpenProject:
      OpenProjectAt(path);
      break;
    case PathPromptKind::kSaveSceneAs:
      SaveSceneAsAt(path);
      break;
    case PathPromptKind::kCreatePrefab:
      CreatePrefabAt(entity, path);
      break;
    case PathPromptKind::kInstantiatePrefab:
      InstantiatePrefabAt(path);
      break;
    case PathPromptKind::kNone:
      break;
  }
}

#ifndef EMSCRIPTEN
struct DialogRequest {
  PathPromptKind kind;
  uint32_t entity;
};

// Runs during the event pump. Anything done here happens between frames, which
// is exactly where a scene swap is safe.
void OnPathChosen(void* userdata, const char* const* filelist, int filter) {
  const DialogRequest* request = static_cast<DialogRequest*>(userdata);
  const PathPromptKind kind = request->kind;
  const uint32_t entity = request->entity;
  delete request;

  if (filelist == nullptr) {
    const char* error = SDL_GetError();

    if (error != nullptr && error[0] != '\0') {
      Logger::Error(std::string("The file dialog could not be shown: ") + error);
      BeginPrompt(kind, entity);
    }

    // No error message means the user cancelled, which is not a failure.
    return;
  }

  if (filelist[0] == nullptr) {
    return;
  }

  ApplyChosenPath(kind, std::filesystem::path{filelist[0]}, entity);
}
#endif
}  // namespace

void NewProject() {
#ifdef EMSCRIPTEN
  BeginPrompt(PathPromptKind::kNewProjectFolder);
#else
  SDL_ShowOpenFolderDialog(OnPathChosen, new DialogRequest{PathPromptKind::kNewProjectFolder, entt::null},
                           Core::GetWindow()->GetWindow(), nullptr, false);
#endif
}

void OpenProject() {
#ifdef EMSCRIPTEN
  BeginPrompt(PathPromptKind::kOpenProject);
#else
  SDL_ShowOpenFileDialog(OnPathChosen, new DialogRequest{PathPromptKind::kOpenProject, entt::null},
                         Core::GetWindow()->GetWindow(), kProjectFilters, 1, nullptr, false);
#endif
}

void SaveProject() {
  std::string error;

  if (!Core::SaveProject(&error)) {
    Logger::Error("Failed to save the project: " + error);
  }
}

void SaveSceneAs() {
#ifdef EMSCRIPTEN
  BeginPrompt(PathPromptKind::kSaveSceneAs);
#else
  SDL_ShowSaveFileDialog(OnPathChosen, new DialogRequest{PathPromptKind::kSaveSceneAs, entt::null},
                         Core::GetWindow()->GetWindow(), kSceneFilters, 1, nullptr);
#endif
}

void CreatePrefab(uint32_t entity) {
#ifdef EMSCRIPTEN
  BeginPrompt(PathPromptKind::kCreatePrefab, entity);
#else
  SDL_ShowSaveFileDialog(OnPathChosen, new DialogRequest{PathPromptKind::kCreatePrefab, entity},
                         Core::GetWindow()->GetWindow(), kPrefabFilters, 1, nullptr);
#endif
}

void InstantiatePrefab() {
#ifdef EMSCRIPTEN
  BeginPrompt(PathPromptKind::kInstantiatePrefab);
#else
  SDL_ShowOpenFileDialog(OnPathChosen, new DialogRequest{PathPromptKind::kInstantiatePrefab, entt::null},
                         Core::GetWindow()->GetWindow(), kPrefabFilters, 1, nullptr, false);
#endif
}

void ReloadScene() {
  if (SceneManager::GetActiveScene().empty()) {
    Logger::Log("No scene is open.");
    return;
  }

  std::string error;

  if (!SceneManager::Open(SceneManager::GetActiveScene(), &error)) {
    Logger::Error("Failed to reload the scene: " + error);
    return;
  }

  Logger::Log("Reloaded " + SceneManager::GetActiveScene().string());
}

void NewProjectAt(const std::filesystem::path& directory) {
  std::error_code error_code;
  std::filesystem::create_directories(directory / "scenes", error_code);
  std::filesystem::create_directories(directory / "assets", error_code);
  std::filesystem::create_directories(directory / "prefabs", error_code);

  // An empty first scene, written where the manifest will look for it.
  rse::SceneIO::Clear();

  std::string error;

  if (!rse::SceneIO::Save(directory / "scenes" / "start.xml", &error)) {
    Logger::Error("Failed to write the new project's first scene: " + error);
    return;
  }

  rse::Project project;
  project.SetFile(directory / "project.xml");
  project.name = directory.filename().string().empty() ? "Untitled" : directory.filename().string();
  project.window.title = project.name;
  project.startup_scene = "scenes/start.xml";
  project.scenes = {"scenes/start.xml"};

  if (!project.Save(&error)) {
    Logger::Error("Failed to write the new project: " + error);
    return;
  }

  Logger::Log("Created project " + project.name + " in " + directory.string());
  OpenProjectAt(project.File());
}

void OpenProjectAt(const std::filesystem::path& file) {
  std::string error;

  if (!Core::OpenProject(file, &error)) {
    Logger::Error("Failed to open the project: " + error);
  }
}

void SaveSceneAsAt(const std::filesystem::path& file) {
  rse::Project& project = Core::GetProject();

  if (!project.IsAttached()) {
    Logger::Error("No project is open, so a scene has nowhere to belong. Use File > New Project.");
    return;
  }

  // Manifest paths are relative to the manifest, so a scene outside the
  // project directory could not be listed without an absolute path.
  std::error_code error_code;
  const std::filesystem::path relative = std::filesystem::relative(file, project.Directory(), error_code);

  if (error_code || relative.empty() || relative.generic_string().rfind("..", 0) == 0) {
    Logger::Error("A scene has to live inside the project directory: " + file.string());
    return;
  }

  std::string error;

  if (!rse::SceneIO::Save(file, &error)) {
    Logger::Error("Failed to save the scene: " + error);
    return;
  }

  if (!SceneManager::Open(file, &error)) {
    Logger::Error("Failed to open the saved scene: " + error);
    return;
  }

  const std::string manifest_path = relative.generic_string();

  if (std::find(project.scenes.begin(), project.scenes.end(), manifest_path) == project.scenes.end()) {
    project.scenes.push_back(manifest_path);
  }

  if (!Core::SaveProject(&error)) {
    Logger::Error("Failed to save the project: " + error);
  }
}

void CreatePrefabAt(uint32_t entity, const std::filesystem::path& file) {
  rse::Project& project = Core::GetProject();

  if (!project.IsAttached()) {
    Logger::Error("No project is open, so a prefab has nowhere to belong. Use File > New Project.");
    return;
  }

  // A prefab is not listed in the manifest, but it is still a project file: a
  // manifest-relative path is what lets the project directory be copied or
  // moved, so the file has to stay inside it.
  std::error_code error_code;
  const std::filesystem::path relative = std::filesystem::relative(file, project.Directory(), error_code);

  if (error_code || relative.empty() || relative.generic_string().rfind("..", 0) == 0) {
    Logger::Error("A prefab has to live inside the project directory: " + file.string());
    return;
  }

  std::string error;

  if (!rse::PrefabIO::Save(file, entity, &error)) {
    Logger::Error("Failed to save the prefab: " + error);
    return;
  }

  Logger::Log("Saved prefab " + file.filename().string());
}

void InstantiatePrefabAt(const std::filesystem::path& file) {
  std::string error;

  if (rse::PrefabIO::Instantiate(file, &error) == entt::null) {
    Logger::Error("Failed to instantiate the prefab: " + error);
    return;
  }

  SceneManager::MarkDirty();
  Logger::Log("Placed prefab " + file.stem().string());
}

void DrawProjectCommands() {
  if (g_prompt.kind == PathPromptKind::kNone) {
    return;
  }

  if (!ImGui::IsPopupOpen(kPromptTitle)) {
    ImGui::OpenPopup(kPromptTitle);
  }

  if (!ImGui::BeginPopupModal(kPromptTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    return;
  }

  ImGui::TextUnformatted(PromptText(g_prompt.kind));

  ImGui::SetNextItemWidth(kPromptWidth);

  if (g_prompt.needs_focus) {
    ImGui::SetKeyboardFocusHere();
    g_prompt.needs_focus = false;
  }

  const bool submitted =
      ImGui::InputText("##path", g_prompt.path, sizeof(g_prompt.path), ImGuiInputTextFlags_EnterReturnsTrue);
  const bool accepted = ImGui::Button("OK") || submitted;

  ImGui::SameLine();

  if (ImGui::Button("Cancel")) {
    g_prompt.kind = PathPromptKind::kNone;
    ImGui::CloseCurrentPopup();
  } else if (accepted) {
    const PathPromptKind kind = g_prompt.kind;
    const uint32_t entity = g_prompt.entity;
    g_prompt.kind = PathPromptKind::kNone;
    ImGui::CloseCurrentPopup();
    ApplyChosenPath(kind, std::filesystem::path{g_prompt.path}, entity);
  }

  ImGui::EndPopup();
}
