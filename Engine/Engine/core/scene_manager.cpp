#include "scene_manager.h"

#include <system_error>

#include <ecs/scene_io.h>

std::filesystem::path SceneManager::m_project_directory;
std::filesystem::path SceneManager::m_active_scene;
bool SceneManager::m_dirty{false};

void SceneManager::Init(const std::filesystem::path& project_directory) {
  m_project_directory = project_directory;
  m_active_scene.clear();
  m_dirty = false;
}

void SceneManager::Clean() {
  m_project_directory.clear();
  m_active_scene.clear();
  m_dirty = false;
}

bool SceneManager::Open(const std::filesystem::path& scene, std::string* error) {
  const std::filesystem::path resolved = Resolve(scene);

  if (!rse::SceneIO::Load(resolved, error)) {
    return false;
  }

  m_active_scene = resolved;
  m_dirty = false;
  return true;
}

bool SceneManager::Save(std::string* error) {
  if (m_active_scene.empty()) {
    if (error != nullptr) {
      *error = "no scene is open";
    }
    return false;
  }

  if (!rse::SceneIO::Save(m_active_scene, error)) {
    return false;
  }

  m_dirty = false;
  return true;
}

bool SceneManager::Create(const std::filesystem::path& scene, std::string* error) {
  const std::filesystem::path resolved = Resolve(scene);

  // SceneIO writes to a scratch file next to the target, so the directory has
  // to exist before the first save.
  if (!resolved.parent_path().empty()) {
    std::error_code error_code;
    std::filesystem::create_directories(resolved.parent_path(), error_code);
  }

  rse::SceneIO::Clear();

  if (!rse::SceneIO::Save(resolved, error)) {
    return false;
  }

  m_active_scene = resolved;
  m_dirty = false;
  return true;
}

const std::filesystem::path& SceneManager::GetActiveScene() { return m_active_scene; }

std::string SceneManager::GetActiveSceneName() {
  if (m_active_scene.empty()) {
    return "";
  }

  return m_active_scene.stem().string();
}

void SceneManager::MarkDirty() { m_dirty = true; }

bool SceneManager::IsDirty() { return m_dirty; }

std::filesystem::path SceneManager::Resolve(const std::filesystem::path& scene) {
  if (scene.is_absolute() || m_project_directory.empty()) {
    return scene;
  }

  return m_project_directory / scene;
}
