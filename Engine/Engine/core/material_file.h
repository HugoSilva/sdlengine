#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>

namespace rse {
// The data side of a material shader asset. A material is two shader sources --
// a vertex stage and a fragment stage -- named so a sprite can refer to the
// asset rather than to the files, and versioned so a later format can add
// parameters without an older build guessing at them.
//
// This type only reads and writes the file. The compiled program belongs to
// MaterialManager, which keeps one per asset and hands it to the renderer.
struct MaterialFile {
  // The material format this build writes, and the newest it can read.
  static constexpr int kFormatVersion{1};

  // The oldest material format this build still understands.
  static constexpr int kFirstFormatVersion{1};

  int format_version{kFormatVersion};

  // The name the inspector shows. Empty in a file is allowed; the manager then
  // falls back to the asset name or the file stem.
  std::string name;

  // Shader files, relative to the project directory. They use the same uniform
  // contract as the default scene shader (view, projection, texture0), so a
  // material can replace the program without replacing the camera path.
  std::string vertex_shader;
  std::string fragment_shader;

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("formatVersion", format_version), cereal::make_nvp("name", name),
            cereal::make_nvp("vertexShader", vertex_shader), cereal::make_nvp("fragmentShader", fragment_shader));
  }

  // Reads `file` into `material`. Returns false and fills `error` when the file
  // is missing, is not a material, is older than this build reads, or is newer
  // than this build understands.
  static bool Load(const std::filesystem::path& file, MaterialFile* material, std::string* error);

  // Writes `material` to `file` through a scratch file, so a failed write cannot
  // destroy the previous material.
  static bool Save(const std::filesystem::path& file, const MaterialFile& material, std::string* error);
};
}  // namespace rse
