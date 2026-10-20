#include "scene_commands.h"

#include <string>

#include <core/scene_manager.h>
#include <utils/logger.h>

void SaveScene() {
  std::string error;

  if (!SceneManager::Save(&error)) {
    Logger::Error("Failed to save the scene: " + error);
  }
}

void ReloadScene() {
  if (SceneManager::GetActiveScene().empty()) {
    Logger::Log("No scene is open.");
    return;
  }

  std::string error;

  if (!SceneManager::Open(SceneManager::GetActiveScene(), &error)) {
    Logger::Error("Failed to open the scene: " + error);
  }
}
