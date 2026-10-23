#pragma once

#include <filesystem>
#include <string>

// The scene that is open right now.
//
// A scene is a file: the entity list SceneIO reads and writes. What a scene
// *does* is the ECS systems; this class is only the small amount of state
// around the data -- which file is open, where the project directory is, and
// whether the open scene has unsaved edits.
class SceneManager {
 public:
  // The editor's play session. A game is always playing and never uses these;
  // the editor starts stopped and Core asks IsPlaying() before it runs the
  // scene's simulation systems.
  enum class PlayState { kStopped, kPlaying, kPaused };

  // Sets the directory relative scene paths resolve against, and forgets the
  // open scene.
  static void Init(const std::filesystem::path& project_directory);
  static void Clean();

  // Opens a scene and makes it the active one. A relative path resolves
  // against the project directory. On failure the active scene is unchanged.
  static bool Open(const std::filesystem::path& scene, std::string* error);

  // Saves the active scene back to its own file.
  static bool Save(std::string* error);

  // Replaces the open scene with an empty one, written to `scene`.
  static bool Create(const std::filesystem::path& scene, std::string* error);

  static const std::filesystem::path& GetActiveScene();

  // The file stem, which is what the UI calls a scene.
  static std::string GetActiveSceneName();

  // Called by anything that changes the entities of the open scene. The editor
  // uses it to say there is work to lose and to mark the title bar.
  static void MarkDirty();
  static bool IsDirty();

  // Play snapshots the open scene and starts its logic. Stop restores that
  // snapshot exactly -- entities, components, camera and the dirty flag -- and
  // ends the session. Pause holds the scene where it is; Resume continues it.
  // All four are no-ops (or failures) from a state they do not apply to, so the
  // UI can call them without guarding first.
  static bool Play(std::string* error);
  static void Pause();
  static void Resume();
  static bool Stop(std::string* error);

  static PlayState GetPlayState();
  static bool IsPlaying();
  static bool IsPaused();

  // Playing or paused: the state in which saving is refused and the snapshot
  // still has to be discarded before the scene is replaced.
  static bool IsPlayActive();

 private:
  SceneManager() {}

  static std::filesystem::path Resolve(const std::filesystem::path& scene);

  // The scene is being replaced or torn down, so there is nothing to restore:
  // stop any sound the session started and forget the snapshot.
  static void DiscardPlaySession();

  static std::filesystem::path m_project_directory;
  static std::filesystem::path m_active_scene;
  static bool m_dirty;

  static PlayState m_play_state;
  static std::string m_snapshot;
  static bool m_dirty_on_play;
};
