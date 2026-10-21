#pragma once

#include <cstdint>
#include <filesystem>

// The project and scene commands the File menu and the keyboard share. Each
// logs its own failures: a menu item has nowhere to return an error to.
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
void CreatePrefabAt(uint32_t entity, const std::filesystem::path& file);
void InstantiatePrefabAt(const std::filesystem::path& file);
void ImportAssetAt(const std::filesystem::path& file);
void RenameAssetAt(const std::string& id, const std::string& name);
void DeleteAssetAt(const std::string& id);

// Draws the typed-path prompt, when one is open. Called once a frame from the
// editor layout.
void DrawProjectCommands();
