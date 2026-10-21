#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace rse {
struct Project;
}  // namespace rse

// The project and scene commands the menus and the keyboard share. Each logs
// its own failures: a menu item has nowhere to return an error to.
//
// The dialogs are SDL's, which are asynchronous -- the chosen path arrives in a
// callback during the event pump, not when the dialog is asked for. On a
// platform with no dialog, or when the platform dialog cannot be shown, the
// command opens a typed-path prompt instead; DrawProjectCommands draws it.
void NewProject();
void OpenProject();
void SaveProject();
void SaveSceneAs();
void ReloadScene();
void NewScene();

// What the Scenes menu draws, built from the manifest and the open scene
// without ImGui: the entries in manifest order, which one is ticked, and the
// neighbours Next/Previous would visit. Pure data, so the order, the tick and
// the wrapping are tested without a window.
struct ScenesMenuModel {
  struct Entry {
    // The manifest path: what SwitchScene takes, spelled as the manifest does.
    std::string path;
    // The file stem: what the menu calls the scene.
    std::string name;
    bool active{false};
  };

  std::vector<Entry> entries;

  // The index of the ticked entry, or entries.size() when the open scene is not
  // in the manifest (a scene outside the project, or none open at all).
  std::size_t active{0};

  static ScenesMenuModel Build(const rse::Project& project, const std::filesystem::path& active_scene);

  // The neighbours, wrapping at the ends. Null when there is nothing to switch
  // to: no project, one scene, or an empty list.
  const Entry* Next() const;
  const Entry* Previous() const;
};

// Scenes > <name>: opens a scene from the manifest. Asking for the scene that
// is already open is a no-op rather than a reload, and a dirty scene asks before
// its edits are lost -- see ConfirmDiscard.
void SwitchScene(const std::string& scene);

// The Ctrl+Tab pair: the next/previous scene in manifest order, wrapping. They
// go through SwitchScene, so the dirty check is the same one.
void NextScene();
void PreviousScene();

// Scenes > Rename/Delete and the content browser's context menu. Rename takes a
// file name, not a path: the scene stays in its folder. Both keep the file and
// the manifest in step, and refuse what would leave the editor without the
// scene it is showing.
void RenameScene(const std::string& scene);
void DeleteScene(const std::string& scene);

// Opens the file's folder in the platform's file manager with the file
// selected. Only Windows has an implementation; elsewhere it says so.
void ShowInExplorer(const std::filesystem::path& file);

// A yes/no question a command asks before losing unsaved work. The command
// returns immediately; DrawProjectCommands draws the question, and the modal's
// buttons call the same ConfirmPending/CancelPending the tests do, so the
// decision is testable without a window. One question at a time.
void ConfirmDiscard(const std::string& message, std::function<void()> on_confirm);

// The same modal with another affirmative label, for a command that is
// destructive without discarding edits -- a delete.
void ConfirmAction(const std::string& message, const std::string& confirm_label, std::function<void()> on_confirm);

bool HasPendingConfirmation();
bool ConfirmPending();
bool CancelPending();

// Edit > Project Settings: the manifest's audio levels, applied as they move
// and written to the manifest when the dialog is saved. A no-op with a warning
// when no project is open, because the levels live in the manifest.
void OpenProjectSettings();

// Prefabs are one entity saved from the open scene and placed into any scene.
// Save takes the entity at click time because the dialog is asynchronous and
// the selection may change before a path arrives.
void CreatePrefab(uint32_t entity);
void InstantiatePrefab();

// The asset registry commands. Import adds a project file to the registry;
// Rename changes an asset's name and the file's name; Delete drops the entry
// after warning when the open scene still references it. Rename prompts for a
// name rather than a path, because the file moves with the entry.
void ImportAsset();
void RenameAsset(const std::string& id);
void DeleteAsset(const std::string& id);

// The ...At forms are what the dialogs, the prompt and the tests all end up
// calling: the command with a path already chosen.
void NewProjectAt(const std::filesystem::path& directory);
void OpenProjectAt(const std::filesystem::path& file);
void SaveSceneAsAt(const std::filesystem::path& file);
void CreateSceneAt(const std::filesystem::path& file);
void RenameSceneAt(const std::filesystem::path& scene, const std::string& name);
void DeleteSceneAt(const std::filesystem::path& scene);
void CreatePrefabAt(uint32_t entity, const std::filesystem::path& file);
void InstantiatePrefabAt(const std::filesystem::path& file);
void ImportAssetAt(const std::filesystem::path& file);
void RenameAssetAt(const std::string& id, const std::string& name);
void DeleteAssetAt(const std::string& id);

// Draws the typed-path prompt and the confirmation, when one is open. Called
// once a frame from the editor layout.
void DrawProjectCommands();
