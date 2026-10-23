#include "project_commands.h"

#include <algorithm>
#include <core.h>
#include <imgui.h>
#include <string>
#include <system_error>
#include <utility>

#include <SDL3/SDL.h>
#include <entity/entity.hpp>

#ifdef _WIN32
#include <shellapi.h>
#endif

#include <core/asset_registry.h>
#include <core/project.h>
#include <core/scene_manager.h>
#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>
#include <ecs/prefab_io.h>
#include <ecs/scene_io.h>
#include <input/input_map.h>
#include <utils/logger.h>

namespace {
// What a chosen path is for. The dialog callback carries one of these through
// its userdata, so a single callback serves every dialog.
enum class PathPromptKind {
  kNone,
  kNewProjectFolder,
  kOpenProject,
  kSaveSceneAs,
  kNewScene,
  kCreatePrefab,
  kInstantiatePrefab,
  kImportAsset,
  kRenameAsset,
  kRenameScene
};

// The typed-path fallback. One at a time: a dialog can only be started from a
// menu, and a modal prompt blocks the menu.
struct PathPrompt {
  PathPromptKind kind{PathPromptKind::kNone};
  char path[512]{};
  bool needs_focus{false};
  // Only a prefab save uses it: the entity selected when the command ran.
  uint32_t entity{entt::null};
  // Only a rename uses it: the asset id or the manifest scene path.
  std::string subject;
};

PathPrompt g_prompt;

constexpr const char* kPromptTitle = "Path";
constexpr int kPromptWidth = 520;

// File-static so the pointer handed to the asynchronous dialog stays valid.
constexpr SDL_DialogFileFilter kProjectFilters[]{{"RSE project", "xml"}};
constexpr SDL_DialogFileFilter kSceneFilters[]{{"RSE scene", "xml"}};
constexpr SDL_DialogFileFilter kPrefabFilters[]{{"RSE prefab", "xml"}};

void BeginPrompt(PathPromptKind kind, uint32_t entity = entt::null, const std::string& subject = "") {
  g_prompt.kind = kind;
  g_prompt.path[0] = '\0';
  g_prompt.needs_focus = true;
  g_prompt.entity = entity;
  g_prompt.subject = subject;
}

const char* PromptText(PathPromptKind kind) {
  switch (kind) {
    case PathPromptKind::kNewProjectFolder:
      return "Folder to create the project in:";
    case PathPromptKind::kOpenProject:
      return "Path to a project.xml:";
    case PathPromptKind::kSaveSceneAs:
      return "Path for the scene file:";
    case PathPromptKind::kNewScene:
      return "Path for the new scene file:";
    case PathPromptKind::kCreatePrefab:
      return "Path for the prefab file:";
    case PathPromptKind::kInstantiatePrefab:
      return "Path to a prefab file:";
    case PathPromptKind::kImportAsset:
      return "Path to a file to register as an asset:";
    case PathPromptKind::kRenameAsset:
      return "New name for the asset:";
    case PathPromptKind::kRenameScene:
      return "New name for the scene:";
    case PathPromptKind::kNone:
      break;
  }

  return "";
}

void ApplyChosenPath(PathPromptKind kind, const std::filesystem::path& path, uint32_t entity,
                     const std::string& subject) {
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
    case PathPromptKind::kNewScene:
      CreateSceneAt(path);
      break;
    case PathPromptKind::kCreatePrefab:
      CreatePrefabAt(entity, path);
      break;
    case PathPromptKind::kInstantiatePrefab:
      InstantiatePrefabAt(path);
      break;
    case PathPromptKind::kImportAsset:
      ImportAssetAt(path);
      break;
    case PathPromptKind::kRenameAsset:
      RenameAssetAt(subject, path.string());
      break;
    case PathPromptKind::kRenameScene:
      RenameSceneAt(subject, path.string());
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

  ApplyChosenPath(kind, std::filesystem::path{filelist[0]}, entity, "");
}
#endif

// True when a component of the open scene still references the asset id. This
// is the warning DeleteAssetAt turns into a message: the entry goes away, but a
// scene that already stores the id has nothing to resolve it to until the file
// is imported again.
bool SceneReferencesAsset(const std::string& id) {
  bool referenced = false;

  rse::ECSManager::GetView<SpriteComponent>().each([&referenced, &id](auto, SpriteComponent& component) {
    if (component.m_sprite != nullptr && component.m_sprite->GetTextureAssetId() == id) {
      referenced = true;
    }
  });

  rse::ECSManager::GetView<AudioComponent>().each([&referenced, &id](auto, AudioComponent& component) {
    if (component.m_sound != nullptr && component.m_sound->GetAssetId() == id) {
      referenced = true;
    }
  });

  return referenced;
}

// The path of `file` relative to the project directory, or false with the
// reason. Every registry path travels with the project, so a file outside it
// cannot be registered.
bool ProjectRelativePath(const std::filesystem::path& file, std::string* relative, std::string* error) {
  rse::Project& project = Core::GetProject();

  if (!project.IsAttached()) {
    *error = "no project is open";
    return false;
  }

  std::error_code error_code;
  const std::filesystem::path resolved = std::filesystem::relative(file, project.Directory(), error_code);

  if (error_code || resolved.empty() || resolved.generic_string().rfind("..", 0) == 0) {
    *error = "an asset has to live inside the project directory: " + file.string();
    return false;
  }

  *relative = resolved.generic_string();
  return true;
}
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

void NewScene() {
#ifdef EMSCRIPTEN
  BeginPrompt(PathPromptKind::kNewScene);
#else
  SDL_ShowSaveFileDialog(OnPathChosen, new DialogRequest{PathPromptKind::kNewScene, entt::null},
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

void ImportAsset() {
#ifdef EMSCRIPTEN
  BeginPrompt(PathPromptKind::kImportAsset);
#else
  SDL_ShowOpenFileDialog(OnPathChosen, new DialogRequest{PathPromptKind::kImportAsset, entt::null},
                         Core::GetWindow()->GetWindow(), nullptr, 0, nullptr, false);
#endif
}

void RenameAsset(const std::string& id) {
  // Renaming moves the file, so the prompt asks for a name rather than a path;
  // there is no file dialog for it on any platform.
  BeginPrompt(PathPromptKind::kRenameAsset, entt::null, id);
}

void DeleteAsset(const std::string& id) { DeleteAssetAt(id); }

void RenameScene(const std::string& scene) {
  // A scene is renamed in place: the prompt takes a file name, and the browser
  // and the Scenes menu both hand in the manifest's spelling.
  BeginPrompt(PathPromptKind::kRenameScene, entt::null, scene);
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

namespace {
// Where a manifest path lands on disk, with the spelling the active scene uses:
// the project directory for a relative path, and normalised either way.
std::filesystem::path ResolveAgainst(const std::filesystem::path& directory, const std::string& scene) {
  const std::filesystem::path path{scene};

  if (path.is_absolute()) {
    return path.lexically_normal();
  }

  return (directory / path).lexically_normal();
}

void OpenScene(const std::string& scene) {
  std::string error;

  if (!SceneManager::Open(scene, &error)) {
    Logger::Error("Failed to open the scene: " + error);
    return;
  }

  Logger::Log("Opened scene " + SceneManager::GetActiveScene().string());
}

// The confirmed half of CreateSceneAt: an empty scene written to `file`, listed
// in the manifest, and open.
void CreateSceneNow(const std::filesystem::path& file) {
  std::string error;

  if (!SceneManager::Create(file, &error)) {
    Logger::Error("Failed to create the scene: " + error);
    return;
  }

  rse::Project& project = Core::GetProject();

  std::error_code error_code;
  const std::filesystem::path relative = std::filesystem::relative(file, project.Directory(), error_code);

  if (error_code || relative.empty()) {
    Logger::Error("Failed to list the new scene in the project: " + file.string());
    return;
  }

  project.scenes.push_back(relative.generic_string());

  if (!Core::SaveProject(&error)) {
    Logger::Error("Failed to save the project: " + error);
    return;
  }

  Logger::Log("Created scene " + SceneManager::GetActiveScene().string());
}

struct Confirmation {
  std::string title;
  std::string message;
  std::string confirm_label;
  std::function<void()> on_confirm;
};

Confirmation g_confirmation;
bool g_confirmation_requested{false};

// The popup id of the question being drawn. It outlives the pending question,
// because the frame that closes the modal still has to name it.
std::string g_confirmation_title;

void AskConfirmation(const std::string& title, const std::string& message, const std::string& confirm_label,
                     std::function<void()> on_confirm) {
  // One at a time: the modal blocks the menus that could start another, and
  // overwriting a question nobody answered would lose the command behind it.
  if (HasPendingConfirmation()) {
    Logger::Warning("Another confirmation is still open; not asking: " + message);
    return;
  }

  g_confirmation.title = title;
  g_confirmation.message = message;
  g_confirmation.confirm_label = confirm_label;
  g_confirmation.on_confirm = std::move(on_confirm);
  g_confirmation_requested = true;
}
}  // namespace

ScenesMenuModel ScenesMenuModel::Build(const rse::Project& project, const std::filesystem::path& active_scene) {
  ScenesMenuModel model;

  if (!project.IsAttached()) {
    return model;
  }

  const std::filesystem::path active = active_scene.lexically_normal();
  bool found = false;

  for (const std::string& path : project.scenes) {
    ScenesMenuModel::Entry entry;
    entry.path = path;
    entry.name = std::filesystem::path{path}.stem().string();
    entry.active = ResolveAgainst(project.Directory(), path) == active;
    model.entries.push_back(entry);

    if (entry.active && !found) {
      model.active = model.entries.size() - 1;
      found = true;
    }
  }

  // No tick when the open scene is not in the manifest; the callers treat that
  // as "nothing is current" rather than ticking the first row by accident.
  if (!found) {
    model.active = model.entries.size();
  }

  return model;
}

const ScenesMenuModel::Entry* ScenesMenuModel::Next() const {
  if (entries.size() < 2) {
    return nullptr;
  }

  // With no tick to move from, "next" starts at the top of the list.
  if (active >= entries.size()) {
    return &entries.front();
  }

  return &entries.at((active + 1) % entries.size());
}

const ScenesMenuModel::Entry* ScenesMenuModel::Previous() const {
  if (entries.size() < 2) {
    return nullptr;
  }

  if (active >= entries.size()) {
    return &entries.back();
  }

  return &entries.at((active + entries.size() - 1) % entries.size());
}

void SwitchScene(const std::string& scene) {
  if (!Core::GetProject().IsAttached()) {
    Logger::Warning("No project is open, so there is no scene to switch to.");
    return;
  }

  if (scene.empty()) {
    return;
  }

  const std::filesystem::path requested = ResolveAgainst(Core::GetProject().Directory(), scene);

  // The menu ticks what is open and Reload is its own command: asking for the
  // active scene must not quietly throw edits away.
  if (requested == SceneManager::GetActiveScene().lexically_normal()) {
    return;
  }

  if (SceneManager::IsDirty()) {
    ConfirmDiscard("Discard the unsaved changes to " + SceneManager::GetActiveSceneName() + " and open " +
                       std::filesystem::path{scene}.stem().string() + "?",
                   [scene]() { OpenScene(scene); });
    return;
  }

  OpenScene(scene);
}

void NextScene() {
  if (!Core::GetProject().IsAttached()) {
    Logger::Warning("No project is open, so there is no next scene.");
    return;
  }

  const ScenesMenuModel model = ScenesMenuModel::Build(Core::GetProject(), SceneManager::GetActiveScene());

  if (const ScenesMenuModel::Entry* next = model.Next(); next != nullptr) {
    SwitchScene(next->path);
  }
}

void PreviousScene() {
  if (!Core::GetProject().IsAttached()) {
    Logger::Warning("No project is open, so there is no previous scene.");
    return;
  }

  const ScenesMenuModel model = ScenesMenuModel::Build(Core::GetProject(), SceneManager::GetActiveScene());

  if (const ScenesMenuModel::Entry* previous = model.Previous(); previous != nullptr) {
    SwitchScene(previous->path);
  }
}

void ConfirmDiscard(const std::string& message, std::function<void()> on_confirm) {
  AskConfirmation("Unsaved changes", message, "Discard", std::move(on_confirm));
}

void ConfirmAction(const std::string& message, const std::string& confirm_label, std::function<void()> on_confirm) {
  AskConfirmation("Confirm", message, confirm_label, std::move(on_confirm));
}

bool HasPendingConfirmation() { return g_confirmation.on_confirm != nullptr; }

bool ConfirmPending() {
  if (!HasPendingConfirmation()) {
    return false;
  }

  // The callback may open a scene, so it runs after the state is cleared: a
  // command that asks again from inside it is a new question, not this one.
  std::function<void()> on_confirm = std::move(g_confirmation.on_confirm);
  g_confirmation = Confirmation{};
  on_confirm();
  return true;
}

bool CancelPending() {
  if (!HasPendingConfirmation()) {
    return false;
  }

  g_confirmation = Confirmation{};
  return true;
}

void CreateSceneAt(const std::filesystem::path& file) {
  rse::Project& project = Core::GetProject();

  if (!project.IsAttached()) {
    Logger::Error("No project is open, so a scene has nowhere to belong. Use File > New Project.");
    return;
  }

  std::error_code error_code;
  const std::filesystem::path relative = std::filesystem::relative(file, project.Directory(), error_code);

  if (error_code || relative.empty() || relative.generic_string().rfind("..", 0) == 0) {
    Logger::Error("A scene has to live inside the project directory: " + file.string());
    return;
  }

  const std::string manifest_path = relative.generic_string();

  // Creating over a scene the manifest already lists would replace a file the
  // project is using. A second "new" at the same path is a mistake, not an
  // overwrite.
  if (std::find(project.scenes.begin(), project.scenes.end(), manifest_path) != project.scenes.end()) {
    Logger::Warning("The project already lists that scene: " + manifest_path);
    return;
  }

  if (SceneManager::IsDirty()) {
    ConfirmDiscard("Discard the unsaved changes to " + SceneManager::GetActiveSceneName() + " and create " +
                       std::filesystem::path{manifest_path}.stem().string() + "?",
                   [file]() { CreateSceneNow(file); });
    return;
  }

  CreateSceneNow(file);
}

namespace {
// The manifest spelling of a scene, with a relative path resolved against the
// project directory first. Empty when the file is outside the project, because
// a manifest cannot describe it there.
std::string SceneManifestPath(const std::filesystem::path& scene) {
  rse::Project& project = Core::GetProject();

  if (!project.IsAttached()) {
    return {};
  }

  std::filesystem::path absolute = scene;
  if (absolute.is_relative()) {
    absolute = project.Directory() / absolute;
  }

  std::error_code error_code;
  const std::filesystem::path relative = std::filesystem::relative(absolute, project.Directory(), error_code);

  if (error_code || relative.empty() || relative.generic_string().rfind("..", 0) == 0) {
    return {};
  }

  return relative.generic_string();
}

// Why a scene cannot be deleted right now, or empty when it can. Startup and
// open are the two the editor would miss immediately.
std::string DeleteRefusal(const std::string& manifest_path) {
  rse::Project& project = Core::GetProject();

  if (manifest_path == project.startup_scene) {
    return "The startup scene cannot be deleted: " + manifest_path;
  }

  const std::filesystem::path active = SceneManager::GetActiveScene().lexically_normal();

  if (!active.empty() && ResolveAgainst(project.Directory(), manifest_path) == active) {
    return "The open scene cannot be deleted: " + manifest_path;
  }

  return {};
}
}  // namespace

void RenameSceneAt(const std::filesystem::path& scene, const std::string& name) {
  rse::Project& project = Core::GetProject();

  if (!project.IsAttached()) {
    Logger::Error("No project is open, so there is no scene to rename.");
    return;
  }

  const std::string manifest_path = SceneManifestPath(scene);

  if (manifest_path.empty()) {
    Logger::Error("A scene has to live inside the project directory: " + scene.string());
    return;
  }

  const std::vector<std::string>::iterator entry =
      std::find(project.scenes.begin(), project.scenes.end(), manifest_path);

  if (entry == project.scenes.end()) {
    Logger::Warning("The project does not list that scene: " + manifest_path);
    return;
  }

  // A name is a file name: a separator would move the scene to another folder
  // without updating anything that expects it there.
  if (name.empty() || name == "." || name == ".." || name.find('/') != std::string::npos ||
      name.find('\\') != std::string::npos) {
    Logger::Error("A scene name cannot be empty or contain a path: " + name);
    return;
  }

  std::filesystem::path target_name{name};

  if (target_name.extension().empty()) {
    target_name += ".xml";
  }

  const std::filesystem::path old_file = project.Directory() / manifest_path;
  const std::filesystem::path new_file = old_file.parent_path() / target_name;
  const std::string new_manifest_path = SceneManifestPath(new_file);

  if (new_manifest_path.empty()) {
    Logger::Error("A scene has to live inside the project directory: " + new_file.string());
    return;
  }

  if (new_manifest_path == manifest_path) {
    Logger::Log("The scene is already called " + manifest_path);
    return;
  }

  std::error_code error_code;

  if (std::filesystem::exists(new_file, error_code) ||
      std::find(project.scenes.begin(), project.scenes.end(), new_manifest_path) != project.scenes.end()) {
    Logger::Error("A scene with that name already exists: " + new_manifest_path);
    return;
  }

  const bool was_open = SceneManager::GetActiveScene().lexically_normal() == old_file.lexically_normal();

  // Renaming the open scene: save it first so the edits move with the file.
  // The editor forgets the scene below -- the path it held is gone -- but the
  // work is not.
  if (was_open) {
    std::string save_error;

    if (!Core::SaveProject(&save_error)) {
      Logger::Error("Failed to save the open scene before renaming it: " + save_error);
      return;
    }
  }

  std::filesystem::rename(old_file, new_file, error_code);

  if (error_code) {
    Logger::Error("Failed to rename the scene: " + error_code.message());
    return;
  }

  *entry = new_manifest_path;

  if (project.startup_scene == manifest_path) {
    project.startup_scene = new_manifest_path;
  }

  std::string error;

  if (!project.Save(&error)) {
    Logger::Error("Failed to save the project: " + error);
    return;
  }

  if (was_open) {
    // The open scene is a path, and that path no longer exists. Opening the
    // renamed file is a separate decision -- Scenes > <name> does it -- and the
    // entities of the scene that is no longer open go with it.
    SceneManager::Init(project.Directory());
    rse::SceneIO::Clear();
  }

  Logger::Log("Renamed scene " + manifest_path + " to " + new_manifest_path);
}

void DeleteScene(const std::string& scene) {
  if (!Core::GetProject().IsAttached()) {
    Logger::Warning("No project is open, so there is no scene to delete.");
    return;
  }

  const std::string manifest_path = SceneManifestPath(scene);

  if (manifest_path.empty()) {
    Logger::Error("A scene has to live inside the project directory: " + scene);
    return;
  }

  // Refused before the question, not after: confirming and then being told no
  // is worse than not being asked.
  const std::string refusal = DeleteRefusal(manifest_path);

  if (!refusal.empty()) {
    Logger::Warning(refusal);
    return;
  }

  ConfirmAction("Delete the scene " + std::filesystem::path{manifest_path}.stem().string() +
                    "? Its file is removed from the project.",
                "Delete", [manifest_path]() { DeleteSceneAt(manifest_path); });
}

void DeleteSceneAt(const std::filesystem::path& scene) {
  rse::Project& project = Core::GetProject();

  if (!project.IsAttached()) {
    Logger::Error("No project is open, so there is no scene to delete.");
    return;
  }

  const std::string manifest_path = SceneManifestPath(scene);

  if (manifest_path.empty()) {
    Logger::Error("A scene has to live inside the project directory: " + scene.string());
    return;
  }

  const std::vector<std::string>::iterator entry =
      std::find(project.scenes.begin(), project.scenes.end(), manifest_path);

  if (entry == project.scenes.end()) {
    Logger::Warning("The project does not list that scene: " + manifest_path);
    return;
  }

  const std::string refusal = DeleteRefusal(manifest_path);

  if (!refusal.empty()) {
    Logger::Warning(refusal);
    return;
  }

  std::error_code error_code;
  const bool removed = std::filesystem::remove(project.Directory() / manifest_path, error_code);

  if (error_code) {
    Logger::Error("Failed to delete the scene file: " + error_code.message());
    return;
  }

  if (!removed) {
    // The manifest is still the place to fix: drop the entry even though the
    // file had already been removed outside the editor.
    Logger::Warning("The scene file was already gone: " + manifest_path);
  }

  project.scenes.erase(entry);

  std::string error;

  if (!project.Save(&error)) {
    Logger::Error("Failed to save the project: " + error);
    return;
  }

  Logger::Log("Deleted scene " + manifest_path);
}

void ShowInExplorer(const std::filesystem::path& file) {
#ifdef _WIN32
  // Explorer takes the selection as one argument: /select,"C:\path\scene.xml".
  const std::wstring argument = L"/select,\"" + file.wstring() + L"\"";
  const HINSTANCE result = ShellExecuteW(nullptr, L"open", L"explorer.exe", argument.c_str(), nullptr, SW_SHOWNORMAL);

  if (reinterpret_cast<std::intptr_t>(result) <= 32) {
    Logger::Error("Could not open Explorer at " + file.string());
  }
#else
  Logger::Warning("Showing a file in the file manager is only implemented on Windows: " + file.string());
#endif
}

void NewProjectAt(const std::filesystem::path& directory) {
  std::error_code error_code;
  std::filesystem::create_directories(directory / "scenes", error_code);
  std::filesystem::create_directories(directory / "assets", error_code);
  std::filesystem::create_directories(directory / "prefabs", error_code);

  std::string error;

  // An empty asset registry, so the project has the table a scene's asset ids
  // are looked up in from the start.
  rse::AssetRegistry::Init(directory);

  if (!rse::AssetRegistry::Save(&error)) {
    Logger::Error("Failed to write the new project's asset registry: " + error);
    return;
  }

  // An empty first scene, written where the manifest will look for it.
  rse::SceneIO::Clear();

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

  // The compiled defaults, written out: a new manifest is where somebody looks
  // to find out what the editor and the game answer to. The map is ordered, so
  // the file starts the same way every time.
  for (const std::pair<const std::string, rse::Binding>& binding : rse::InputMap::Defaults()) {
    project.input.push_back({binding.first, rse::InputMap::Format(binding.second)});
  }

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
    return;
  }

  // The mix belongs to the project, so opening one means hearing it.
  const rse::Project& project = Core::GetProject();
  rse::SoundManager::SetVolumes(project.audio.master, project.audio.music, project.audio.sfx);
}

void SaveSceneAsAt(const std::filesystem::path& file) {
  rse::Project& project = Core::GetProject();

  if (!project.IsAttached()) {
    Logger::Error("No project is open, so a scene has nowhere to belong. Use File > New Project.");
    return;
  }

  if (SceneManager::IsPlayActive()) {
    Logger::Error("Stop the playing scene before saving it.");
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

void ImportAssetAt(const std::filesystem::path& file) {
  std::string relative;
  std::string error;

  if (!ProjectRelativePath(file, &relative, &error)) {
    Logger::Error("Failed to import the asset: " + error);
    return;
  }

  if (rse::AssetRegistry::FindByPath(relative) != nullptr) {
    Logger::Warning("The asset is already registered: " + relative);
    return;
  }

  const std::string type = rse::AssetRegistry::TypeForPath(file);

  if (type.empty()) {
    Logger::Error("The file is not an asset this engine knows: " + relative);
    return;
  }

  const std::string name = file.filename().string();
  rse::AssetRegistry::Add(name, type, relative);

  if (!rse::AssetRegistry::Save(&error)) {
    Logger::Error("Failed to save the asset registry: " + error);
    return;
  }

  Logger::Log("Registered asset " + name + " (" + type + ")");
}

void RenameAssetAt(const std::string& id, const std::string& name) {
  std::string error;

  if (!rse::AssetRegistry::Rename(id, name, &error)) {
    Logger::Error("Failed to rename the asset: " + error);
    return;
  }

  if (!rse::AssetRegistry::Save(&error)) {
    Logger::Error("Failed to save the asset registry: " + error);
    return;
  }

  Logger::Log("Renamed asset to " + rse::AssetRegistry::Name(id));
}

void DeleteAssetAt(const std::string& id) {
  const std::string name = rse::AssetRegistry::Name(id);

  if (name.empty()) {
    Logger::Error("There is no asset with that id.");
    return;
  }

  if (SceneReferencesAsset(id)) {
    Logger::Warning("The open scene still references the asset '" + name +
                    "'; a scene that stores its id will load it as nothing.");
  }

  if (!rse::AssetRegistry::Remove(id)) {
    Logger::Error("Failed to remove the asset.");
    return;
  }

  std::string error;

  if (!rse::AssetRegistry::Save(&error)) {
    Logger::Error("Failed to save the asset registry: " + error);
    return;
  }

  Logger::Log("Removed asset " + name);
}

namespace {
void DrawConfirmation() {
  if (g_confirmation_requested) {
    g_confirmation_title = g_confirmation.title;
    ImGui::OpenPopup(g_confirmation_title.c_str());
    g_confirmation_requested = false;
  }

  if (g_confirmation_title.empty() || !ImGui::IsPopupOpen(g_confirmation_title.c_str())) {
    return;
  }

  if (!ImGui::BeginPopupModal(g_confirmation_title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    return;
  }

  // The answer can also come from outside the modal -- the tests call the same
  // ConfirmPending the buttons do -- so a window with no question left is
  // closed rather than left showing stale text.
  if (!HasPendingConfirmation()) {
    ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    return;
  }

  ImGui::TextUnformatted(g_confirmation.message.c_str());
  ImGui::Separator();

  if (ImGui::Button(g_confirmation.confirm_label.c_str())) {
    ImGui::CloseCurrentPopup();
    ConfirmPending();
  }

  ImGui::SameLine();

  if (ImGui::Button("Cancel")) {
    ImGui::CloseCurrentPopup();
    CancelPending();
  }

  ImGui::EndPopup();
}
}  // namespace

void DrawProjectCommands() {
  // The confirmation is independent of the path prompt, which is why it is
  // drawn before the early return.
  DrawConfirmation();

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
    const std::string subject = g_prompt.subject;
    g_prompt.kind = PathPromptKind::kNone;
    ImGui::CloseCurrentPopup();
    ApplyChosenPath(kind, std::filesystem::path{g_prompt.path}, entity, subject);
  }

  ImGui::EndPopup();
}
