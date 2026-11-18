#pragma once

#include <filesystem>
#include <string>

#include <ecs/scene.h>

// The scene that is open right now.
//
// A scene is a file: the entity list SceneIO reads and writes. What a scene
// *does* is the ECS systems; this class is only the small amount of state
// around the data -- which file is open, where the project directory is, and
// whether the open scene has unsaved edits.
//
// It also owns the Scene itself. The world used to live in ECSManager, which
// made "open a different scene" mean "reset the one registry"; owning it here
// is what lets a play session run a copy while the editor keeps the original.
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

  // The scene the systems run on and the editor shows. While a play session is
  // active this is the play copy, so playing never touches the open scene.
  static rse::Scene& ActiveScene();

  // The open scene itself: what Open, Create and Save read and write. It is the
  // scene a play session copies, and it does not change when Play starts.
  static rse::Scene& OpenScene();

  // Opens a scene and makes it the active one. A relative path resolves
  // against the project directory. A file whose header cannot be read leaves
  // the open scene alone; a failure part-way through leaves it empty rather
  // than half of the old one and half of the new one.
  static bool Open(const std::filesystem::path& scene, std::string* error);

  // Saves the open scene back to its own file. Refused while a play session is
  // active: the open scene is not the one being simulated.
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

  // Play copies the open scene and runs the copy. Stop throws the copy away and
  // makes the open scene active again -- untouched, because it was never
  // simulated -- and restores the dirty flag Play found. Pause holds the copy
  // where it is; Resume continues it. All four are no-ops from a state they do
  // not apply to, so the UI can call them without guarding first.
  static void Play();
  static void Pause();
  static void Resume();
  static void Stop();

  static PlayState GetPlayState();
  static bool IsPlaying();
  static bool IsPaused();

  // Playing or paused: the state in which saving is refused and the play copy
  // is what ActiveScene() returns.
  static bool IsPlayActive();

 private:
  SceneManager() {}

  static std::filesystem::path Resolve(const std::filesystem::path& scene);

  // The open scene is being replaced or the session is being torn down: stop
  // any sound the session started and throw the play copy away.
  static void DiscardPlaySession();

  static std::filesystem::path m_project_directory;
  static std::filesystem::path m_active_scene;
  static bool m_dirty;

  // The open scene: the entities the editor edits and the file Open/Save use.
  static rse::Scene m_scene;

  // What ActiveScene() returns while a session runs: a copy of the open scene,
  // simulated instead of it. Empty while stopped.
  static rse::Scene m_play_scene;

  static PlayState m_play_state;
  static bool m_dirty_on_play;
};
