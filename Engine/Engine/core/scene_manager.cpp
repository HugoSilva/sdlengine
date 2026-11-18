#include "scene_manager.h"

#include <system_error>

#include <audio/sound_manager.h>
#include <ecs/scene_io.h>

std::filesystem::path SceneManager::m_project_directory;
std::filesystem::path SceneManager::m_active_scene;
bool SceneManager::m_dirty{false};
SceneManager::PlayState SceneManager::m_play_state{SceneManager::PlayState::kStopped};
bool SceneManager::m_dirty_on_play{false};
rse::Scene SceneManager::m_scene;
rse::Scene SceneManager::m_play_scene;

void SceneManager::Init(const std::filesystem::path& project_directory) {
  DiscardPlaySession();

  m_project_directory = project_directory;
  m_active_scene.clear();
  m_dirty = false;
}

rse::Scene& SceneManager::ActiveScene() { return IsPlayActive() ? m_play_scene : m_scene; }

rse::Scene& SceneManager::OpenScene() { return m_scene; }

void SceneManager::Clean() {
  DiscardPlaySession();

  m_project_directory.clear();
  m_active_scene.clear();
  m_dirty = false;
}

bool SceneManager::Open(const std::filesystem::path& scene, std::string* error) {
  // The scene that is open -- and any copy of it -- is about to be replaced.
  // There is nothing worth keeping, so the session is dropped rather than
  // stopped.
  DiscardPlaySession();

  const std::filesystem::path resolved = Resolve(scene);

  if (!rse::SceneIO::Load(m_scene, resolved, error)) {
    return false;
  }

  m_active_scene = resolved;
  m_dirty = false;
  return true;
}

bool SceneManager::Save(std::string* error) {
  if (IsPlayActive()) {
    if (error != nullptr) {
      *error = "stop the playing scene before saving";
    }
    return false;
  }

  if (m_active_scene.empty()) {
    if (error != nullptr) {
      *error = "no scene is open";
    }
    return false;
  }

  if (!rse::SceneIO::Save(m_scene, m_active_scene, error)) {
    return false;
  }

  m_dirty = false;
  return true;
}

bool SceneManager::Create(const std::filesystem::path& scene, std::string* error) {
  DiscardPlaySession();

  const std::filesystem::path resolved = Resolve(scene);

  // SceneIO writes to a scratch file next to the target, so the directory has
  // to exist before the first save.
  if (!resolved.parent_path().empty()) {
    std::error_code error_code;
    std::filesystem::create_directories(resolved.parent_path(), error_code);
  }

  m_scene.Clear();

  if (!rse::SceneIO::Save(m_scene, resolved, error)) {
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

void SceneManager::Play() {
  if (m_play_state == PlayState::kPlaying) {
    return;
  }

  if (m_play_state == PlayState::kPaused) {
    Resume();
    return;
  }

  // The copy comes from the live scene, not from the file: unsaved edits are
  // part of the state Play starts from. The open scene is not simulated and is
  // not written to, so Stop has nothing to restore.
  m_play_scene.CopyFrom(m_scene);
  m_dirty_on_play = m_dirty;
  m_play_state = PlayState::kPlaying;
}

void SceneManager::Pause() {
  if (m_play_state != PlayState::kPlaying) {
    return;
  }

  m_play_state = PlayState::kPaused;
  rse::SoundManager::PauseAll();
}

void SceneManager::Resume() {
  if (m_play_state != PlayState::kPaused) {
    return;
  }

  m_play_state = PlayState::kPlaying;
  rse::SoundManager::ResumeAll();
}

void SceneManager::Stop() {
  if (m_play_state == PlayState::kStopped) {
    return;
  }

  // A paused or playing scene may be making noise; the open scene will start
  // its own sounds when it is played again.
  rse::SoundManager::StopAll();

  // Throwing the copy away releases what it owns -- its sprites, its bodies --
  // and leaves the open scene active again, exactly as Play found it.
  m_play_scene.Clear();
  m_play_state = PlayState::kStopped;

  // The open scene was never simulated, so nothing it held was lost; the flag
  // simply goes back to what it was before Play.
  m_dirty = m_dirty_on_play;
  m_dirty_on_play = false;
}

SceneManager::PlayState SceneManager::GetPlayState() { return m_play_state; }

bool SceneManager::IsPlaying() { return m_play_state == PlayState::kPlaying; }

bool SceneManager::IsPaused() { return m_play_state == PlayState::kPaused; }

bool SceneManager::IsPlayActive() { return m_play_state != PlayState::kStopped; }

void SceneManager::DiscardPlaySession() {
  // SoundManager may not be initialised yet when Init runs; only a session
  // that actually started something has anything to stop.
  if (m_play_state != PlayState::kStopped) {
    rse::SoundManager::StopAll();
  }

  m_play_scene.Clear();
  m_play_state = PlayState::kStopped;
  m_dirty_on_play = false;
}

std::filesystem::path SceneManager::Resolve(const std::filesystem::path& scene) {
  if (scene.is_absolute() || m_project_directory.empty()) {
    return scene;
  }

  return m_project_directory / scene;
}
