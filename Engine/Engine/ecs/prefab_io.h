#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace rse {
// Reads and writes a prefab: the components of one entity, saved as a project
// file so the same entity can be placed into any scene.
//
// A prefab is not linked to what it places. Instantiating copies the entity
// into the open scene -- new sprites, a new rigid body, fresh ids -- and the
// copy knows nothing about the file it came from. Editing the file later does
// not change anything already placed, and editing a placed entity does not
// change the file.
//
// A prefab file is a formatVersion header and one <entity> using the same
// component serialisers as a scene, so every component a scene can store is a
// prefab component for free.
class PrefabIO {
 public:
  // The prefab format this build writes, and the newest it can read.
  static constexpr int kFormatVersion{2};

  // The oldest prefab format this build still understands.
  static constexpr int kFirstFormatVersion{1};

  // Writes `entity` from the open scene to `file`. The file is written to a
  // scratch path and renamed into place once the archive has closed, so a
  // failed save cannot destroy an existing prefab. An entity that no longer
  // exists is refused.
  static bool Save(const std::filesystem::path& file, uint32_t entity, std::string* error);

  // Copies the prefab into the open scene with fresh entity ids and returns the
  // first new entity, or entt::null when the file cannot be read. The file is
  // read into a scratch registry first, so a failure leaves the open scene
  // untouched.
  //
  // The entity ids a prefab file carries are deliberately not restored: entt's
  // restore() keeps them, which would overwrite whatever the scene already has
  // at those ids.
  static uint32_t Instantiate(const std::filesystem::path& file, std::string* error);

 private:
  PrefabIO() {}
};
}  // namespace rse
