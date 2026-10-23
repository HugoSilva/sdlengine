#include "scene_manager.h"

#include <system_error>
#include <utility>

#include <audio/sound_manager.h>
#include <ecs/scene_io.h>

std::filesystem::path SceneManager::m_project_directory;
std::filesystem::path SceneManager::m_active_scene;
bool SceneManager::m_dirty{false};
SceneManager::PlayState SceneManager::m_play_state{SceneManager::PlayState::kStopped};
std::string SceneManager::m_snapshot;
bool SceneManager::m_dirty_on_play{false};

void SceneManager::Init(const std::filesystem::path& project_directory) {
  DiscardPlaySession();

  m_project_directory = project_directory;
  m_active_scene.clear();
  m_dirty = false;
}

void SceneManager::Clean() {
  DiscardPlaySession();

  m_project_directory.clear();
  m_active_scene.clear();
  m_dirty = false;
}

bool SceneManager::Open(const std::filesystem::path& scene, std::string* error) {
  // The scene that is open -- and any snapshot of it -- is about to be
  // replaced. There is nothing worth restoring, so the session is dropped
  // rather than stopped.
  DiscardPlaySession();

  const std::filesystem::path resolved = Resolve(scene);

  if (!rse::SceneIO::Load(resolved, error)) {
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

  if (!rse::SceneIO::Save(m_active_scene, error)) {
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

bool SceneManager::Play(std::string* error) {
  if (m_play_state == PlayState::kPlaying) {
    return true;
  }

  if (m_play_state == PlayState::kPaused) {
    Resume();
    return true;
  }

  // The snapshot is taken from the live registry, not from the file: unsaved
  // edits are part of the state Play starts from, so Stop returns to exactly
  // what the user was looking at when they pressed Play.
  std::string snapshot;
  if (!rse::SceneIO::SaveToString(&snapshot, error)) {
    return false;
  }

  m_snapshot = std::move(snapshot);
  m_dirty_on_play = m_dirty;
  m_play_state = PlayState::kPlaying;
  return true;
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

bool SceneManager::Stop(std::string* error) {
  if (m_play_state == PlayState::kStopped) {
    return true;
  }

  // A paused or playing scene may be making noise; the restored scene will
  // start its own sounds when it is played again.
  rse::SoundManager::StopAll();

  const bool restored = rse::SceneIO::LoadFromString(m_snapshot, error);

  m_snapshot.clear();
  m_play_state = PlayState::kStopped;

  // Edits made while playing are gone with the restored snapshot, so the flag
  // goes back to what it was before Play. A failed restore leaves the live
  // scene in place, and then its dirty flag is still telling the truth.
  if (restored) {
    m_dirty = m_dirty_on_play;
  }

  m_dirty_on_play = false;
  return restored;
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

  m_play_state = PlayState::kStopped;
  m_snapshot.clear();
  m_dirty_on_play = false;
}

std::filesystem::path SceneManager::Resolve(const std::filesystem::path& scene) {
  if (scene.is_absolute() || m_project_directory.empty()) {
    return scene;
  }

  return m_project_directory / scene;
}
