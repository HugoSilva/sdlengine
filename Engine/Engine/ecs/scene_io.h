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
//
// Component groups are written in a fixed order and **appended**, never
// reordered or removed: the reader walks the groups in the same order, so a
// reorder would silently misread every older file instead of failing. A group
// added in version N is read only when the file says it is at least N.
class SceneIO {
 public:
  // The scene format this build writes, and the newest it can read. Files in
  // [kFirstFormatVersion, kFormatVersion] are read -- older ones through the
  // gates in LoadEntities -- and anything newer is refused.
  static constexpr int kFormatVersion{4};

  // The oldest scene file this build still understands. Version 2 changed what
  // a sprite stores for its texture -- a name instead of the texture itself --
  // so a version 1 file cannot be read as a version 2 one: the field changed
  // shape rather than gaining a tail. The first readable version moves with a
  // break like that, and version 1 is refused instead of half-read. Versions 3
  // and 4 only appended component groups, so it does not move.
  static constexpr int kFirstFormatVersion{2};

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
