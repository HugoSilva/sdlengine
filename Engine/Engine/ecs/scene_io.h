#pragma once

#include <filesystem>
#include <string>

namespace rse {
// Reads and writes a scene file: the entities of one scene and the format
// version that says how to read them.
//
// A scene file holds data only. What a scene *does* lives in the ECS systems;
// which scene is open, and where its file is, is SceneManager's business. The
// functions return false and fill `error` rather than throwing or logging, so
// the caller decides what a failure means to it.
class SceneIO {
 public:
  static constexpr int kFormatVersion{1};

  // Writes the registry to `file`. The file is written to a scratch path and
  // renamed into place once the archive has closed, so a failed save cannot
  // destroy the previous scene file.
  static bool Save(const std::filesystem::path& file, std::string* error);

  // Replaces the registry with the contents of `file`. The format version is
  // read before the registry is touched, so an unreadable file leaves the open
  // scene alone. A failure after that point leaves an empty scene rather than
  // half of the old one and half of the new one.
  static bool Load(const std::filesystem::path& file, std::string* error);

  // Empties the registry.
  static void Clear();

 private:
  SceneIO() {}
};
}  // namespace rse
