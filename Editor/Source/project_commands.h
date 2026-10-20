#pragma once

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

// The ...At forms are what the dialogs, the prompt and the tests all end up
// calling: the command with a path already chosen.
void NewProjectAt(const std::filesystem::path& directory);
void OpenProjectAt(const std::filesystem::path& file);
void SaveSceneAsAt(const std::filesystem::path& file);

// Draws the typed-path prompt, when one is open. Called once a frame from the
// editor layout.
void DrawProjectCommands();
