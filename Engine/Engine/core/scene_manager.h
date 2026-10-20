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

 private:
  SceneManager() {}

  static std::filesystem::path Resolve(const std::filesystem::path& scene);

  static std::filesystem::path m_project_directory;
  static std::filesystem::path m_active_scene;
  static bool m_dirty;
};
