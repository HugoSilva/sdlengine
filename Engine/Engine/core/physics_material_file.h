#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>

#include <physics/physics_material.h>

namespace rse {
// The data side of a physics material asset: the fixture-level surface
// properties a rigid body uses. It is a different asset from the render
// `.material`: that one names two shader files, this one holds numbers.
struct PhysicsMaterialFile {
  // The physics material format this build writes, and the newest it can read.
  static constexpr int kFormatVersion{1};

  // The oldest physics material format this build still understands.
  static constexpr int kFirstFormatVersion{1};

  int format_version{kFormatVersion};

  // The name the inspector shows. Empty in a file is allowed; the manager then
  // falls back to the asset name or the file stem.
  std::string name;

  PhysicsMaterial material;

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("formatVersion", format_version), cereal::make_nvp("name", name),
            cereal::make_nvp("density", material.density), cereal::make_nvp("friction", material.friction),
            cereal::make_nvp("restitution", material.restitution));
  }

  // Reads `file` into `material`. Returns false and fills `error` when the file
  // is missing, is not a physics material, is older than this build reads, or is
  // newer than this build understands.
  static bool Load(const std::filesystem::path& file, PhysicsMaterialFile* material, std::string* error);

  // Writes `material` to `file` through a scratch file, so a failed write cannot
  // destroy the previous physics material.
  static bool Save(const std::filesystem::path& file, const PhysicsMaterialFile& material, std::string* error);
};
}  // namespace rse
