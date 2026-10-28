#pragma once

#include <filesystem>
#include <string>

#include <cereal/cereal.hpp>
#include <glm/glm.hpp>

namespace rse {
// Reads and writes a scene file: the entities of one scene, the camera it is
// framed with, and the format version that says how to read them.
//
// A scene file holds data only. What a scene *does* lives in the ECS systems;
// which scene is open, and where its file is, is SceneManager's business. The
// functions return false and fill `error` rather than throwing or logging, so
// the caller decides what a failure means to it.
//
// Component groups are written in a fixed order and **appended**, never
// reordered or removed: the reader walks the groups in the same order, so a
// reorder would silently misread every older file instead of failing. A group
// added in version N is read only when the file says it is at least N. The
// camera is scene-root data, not a component group: there is one per scene, and
// it sits beside <entities> in the file rather than inside it.
class SceneIO {
 public:
  // The scene format this build writes, and the newest it can read. Files in
  // [kFirstFormatVersion, kFormatVersion] are read -- older ones through the
  // gates in LoadEntities -- and anything newer is refused.
  static constexpr int kFormatVersion{7};

  // The oldest scene file this build still understands. Version 2 changed what
  // a sprite stores for its texture -- a name instead of the texture itself --
  // so a version 1 file cannot be read as a version 2 one: the field changed
  // shape rather than gaining a tail. The first readable version moves with a
  // break like that, and version 1 is refused instead of half-read. Versions 3
  // and 4 only appended component groups, version 5 added the camera node and
  // version 6 changed what a texture or sound reference *means* -- an asset id
  // rather than a name -- so none of those moved it. Version 6 is not a shape
  // change: the field is a string either way, and a reference that is not a
  // known asset id resolves as a name through the asset registry, then as a
  // path. That fallback is what keeps a version 5 scene opening unchanged.
  // Version 7 appended the sprite appearance group (flip and material); a
  // version 6 scene simply has no such group and gets the defaults.
  static constexpr int kFirstFormatVersion{2};

  // What a scene says about the view. The defaults are what the engine framed
  // before a scene could say: the middle of the reference rectangle, zoom 1,
  // and the magenta the renderer cleared to.
  struct SceneCamera {
    glm::vec2 position{640.0f, 360.0f};
    float zoom{1.0f};
    glm::vec3 background{1.0f, 0.0f, 1.0f};

    template <typename Archive>
    void serialize(Archive& archive) {
      archive(cereal::make_nvp("Position", position), cereal::make_nvp("Zoom", zoom),
              cereal::make_nvp("Background", background));
    }
  };

  // Writes the registry and the scene camera to `file`. The file is written to
  // a scratch path and renamed into place once the archive has closed, so a
  // failed save cannot destroy the previous scene file.
  static bool Save(const std::filesystem::path& file, std::string* error);

  // Replaces the registry and the scene camera with the contents of `file`. The
  // format version is read before the registry is touched, so an unreadable
  // file leaves the open scene alone. A failure after that point leaves an
  // empty scene rather than half of the old one and half of the new one.
  static bool Load(const std::filesystem::path& file, std::string* error);

  // The same payload as Save/Load, held in memory. This is what the editor's
  // play session snapshots the open scene with, so starting and stopping play
  // never writes a project file. The version checks and the load contract are
  // the file functions' own; an error names the source as <snapshot>.
  static bool SaveToString(std::string* contents, std::string* error);
  static bool LoadFromString(const std::string& contents, std::string* error);

  // Empties the registry and returns the camera to its defaults.
  static void Clear();

  // The camera of the open scene: the game reads it after a load to frame its
  // own camera, and the editor edits the background through it. It is the one
  // stored value, so a background change is a scene change and is saved with
  // the scene.
  static SceneCamera& GetCamera() { return m_camera; }

 private:
  SceneIO() {}

  static SceneCamera m_camera;
};
}  // namespace rse
