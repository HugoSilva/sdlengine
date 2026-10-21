#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

namespace rse {
// The engine version written into a project manifest. Informational: a project
// from another engine version still opens, because only the format version
// decides whether the file can be read.
inline constexpr const char* kEngineVersion{"0.13"};

// The manifest of a game project: what the project is called, how its window
// opens, where its assets live and which scenes it is made of.
//
// A project is a directory. Every path in here is relative to the manifest's
// own directory and stored with forward slashes, so the directory can be
// copied or moved and still opens. The entities of a scene are *not* in here:
// they live in the scene files this manifest lists.
struct Project {
  // The manifest format this build writes, and the newest it can read. A file
  // is readable when its version is in [kFirstFormatVersion, kFormatVersion]:
  // older files are migrated forward by Migrate(), newer ones are refused
  // because this build cannot know what they mean.
  static constexpr int kFormatVersion{2};

  // The oldest manifest this build still understands. It only moves when a
  // migration stops being possible at all, not when a version is added.
  static constexpr int kFirstFormatVersion{1};

  struct Window {
    std::string title{"RockSlide Engine"};
    int width{1280};
    int height{720};

    template <typename Archive>
    void serialize(Archive& archive) {
      archive(cereal::make_nvp("title", title), cereal::make_nvp("width", width), cereal::make_nvp("height", height));
    }
  };

  // The project's mix, on the same 0-128 scale as Sound::SetVolume: the sound
  // effects and the music each sit under a master, so lowering the master
  // lowers both. Version 2 appended it; a version 1 manifest opens at full
  // volume, which is what it played at.
  struct Audio {
    int master{128};
    int music{128};
    int sfx{128};

    template <typename Archive>
    void serialize(Archive& archive) {
      archive(cereal::make_nvp("master", master), cereal::make_nvp("music", music), cereal::make_nvp("sfx", sfx));
    }
  };

  int format_version{kFormatVersion};
  std::string name{"Untitled"};
  std::string engine_version{kEngineVersion};
  std::string asset_root{"assets"};
  Window window;
  std::string startup_scene{"scenes/start.xml"};
  std::vector<std::string> scenes{"scenes/start.xml"};
  Audio audio;

  // Reads `file` into `project`. `project` is left untouched and false is
  // returned, with `error` filled in, when the file is missing, unreadable,
  // malformed, or written by an incompatible format version.
  static bool Load(const std::filesystem::path& file, Project& project, std::string* error);

  // Writes the manifest back to the path it was loaded from (or the one set
  // with SetFile). The file is written to a scratch path and renamed into
  // place, so a failed save cannot destroy the previous manifest.
  bool Save(std::string* error) const;

  // Resolves a manifest or scene path against the project directory. An
  // unattached project returns the path unchanged, which is what a run without
  // a project needs.
  std::filesystem::path Resolve(const std::string& relative) const;

  std::filesystem::path File() const { return m_file; }
  std::filesystem::path Directory() const { return m_file.parent_path(); }
  bool IsAttached() const { return !m_file.empty(); }
  void SetFile(const std::filesystem::path& file) { m_file = file; }

  template <typename Archive>
  void serialize(Archive& archive) {
    // The version is read before anything it governs: on the way in, cereal
    // has assigned format_version by the time the next call runs, which is
    // what lets the sections below be gated on it. On the way out the field
    // is already kFormatVersion (Save() sets it), so the same gates hold.
    archive(cereal::make_nvp("formatVersion", format_version));

    // Everything version 1 knows about. A field added later is appended in a
    // separate call guarded by the version that introduced it, so older files
    // keep opening:
    //
    //   if (format_version >= 3) archive(cereal::make_nvp("input", input));
    archive(cereal::make_nvp("name", name), cereal::make_nvp("engineVersion", engine_version),
            cereal::make_nvp("assetRoot", asset_root), cereal::make_nvp("window", window),
            cereal::make_nvp("startupScene", startup_scene), cereal::make_nvp("scene", scenes));

    if (format_version >= 2) {
      archive(cereal::make_nvp("audio", audio));
    }
  }

 private:
  // Brings a manifest read as `from_version` up to kFormatVersion's shape.
  // Runs after the file has been read and before it is handed to the caller,
  // so nothing outside this class ever sees an old version.
  static void Migrate(Project& project, int from_version);

  // Stored paths use forward slashes whatever the platform; a backslash a
  // caller typed is turned into one on the way out.
  void NormalisePaths();

  std::filesystem::path m_file;  // not part of the manifest
};
}  // namespace rse
