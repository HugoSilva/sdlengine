#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

namespace rse {
// The project's asset registry: the table that gives every asset a stable id
// and remembers where it lives.
//
// A scene or a prefab stores the **id**, never the path. The id is generated
// when the asset is imported and never derived from the file, so renaming or
// moving the file is an entry update rather than a scene edit: the id in every
// scene that uses the asset still resolves, and the inspector still has a name
// to show. That is the whole point of the table -- a path in a scene breaks the
// moment the file moves, an id does not.
//
// The table lives in `assets.xml` at the project root, one `<asset>` per file:
// `{id, name, type, path}`. The file is loaded when a project opens and written
// when its entries change. A project without one is simply a project whose
// assets have no ids yet; a scene that still names its texture by path keeps
// loading, through the same fallback an older scene file uses.
class AssetRegistry {
 public:
  // The registry format this build writes, and the newest it can read.
  static constexpr int kFormatVersion{1};

  // The oldest registry this build still understands.
  static constexpr int kFirstFormatVersion{1};

  // The file, at the project root.
  static constexpr const char* kFileName{"assets.xml"};

  // One asset. The id is 16 hex characters and is never derived from `path`;
  // `name` is what the inspector shows and what an older scene may still refer
  // to; `type` is one of texture, sound, font or prefab; `path` is relative to
  // the project directory and stored with forward slashes.
  struct Asset {
    std::string id;
    std::string name;
    std::string type;
    std::string path;

    template <typename Archive>
    void serialize(Archive& archive) {
      archive(cereal::make_nvp("id", id), cereal::make_nvp("name", name), cereal::make_nvp("type", type),
              cereal::make_nvp("path", path));
    }
  };

  // Points the registry at a project directory and reads its `assets.xml` when
  // one is there. A missing file is not an error: the registry is then empty
  // and entries appear as assets are imported. A file that cannot be read is
  // logged and leaves an empty registry, so a bad table does not stop a project
  // from opening.
  static void Init(const std::filesystem::path& project_directory);

  // Forgets the directory and every entry.
  static void Clean();

  // Replaces the entries with the contents of `file`. Leaves the entries alone
  // and returns false, with `error` filled in, when the file is missing,
  // malformed, or written by an incompatible format version.
  static bool Load(const std::filesystem::path& file, std::string* error);

  // Writes the entries to `<directory>/assets.xml`. Written to a scratch path
  // and renamed into place, so a failed save cannot destroy the previous table.
  static bool Save(std::string* error);

  static const std::vector<Asset>& All();

  // An entry by id, by name, or by path. FindByPath also matches a bare file
  // name, so "tex3.png" finds "assets/tex3.png".
  static const Asset* Find(const std::string& id);
  static const Asset* FindByName(const std::string& name);
  static const Asset* FindByPath(const std::string& path);

  // What a scene or prefab field refers to: an id, a name, or a legacy path.
  // Returns nullptr when nothing matches, which is the caller's signal to fall
  // back to treating the text as a path.
  static const Asset* FindReference(const std::string& reference);

  // The stored path for `id`, or an empty string. `Name` and `Type` are the
  // same for the other two columns.
  static std::string Resolve(const std::string& id);
  static std::string Name(const std::string& id);
  static std::string Type(const std::string& id);

  // Adds an entry with a freshly generated, collision-checked id and returns
  // it. The caller decides when to Save.
  static std::string Add(const std::string& name, const std::string& type, const std::string& path);

  // Changes an entry's display name without touching the file.
  static bool SetName(const std::string& id, const std::string& name);

  // Renames an asset: the entry's name and path are updated and the file is
  // renamed on disk. The id does not move, so every scene that references it
  // keeps resolving.
  static bool Rename(const std::string& id, const std::string& new_name, std::string* error);

  // Drops an entry. The file is left alone: re-importing it brings it back, and
  // nothing a user authored is deleted by removing it from the table.
  static bool Remove(const std::string& id);

  // A new id: 16 hex characters from a random engine. Never path-derived.
  static std::string GenerateId();

  // Whether `text` has the shape of an id: 16 hex characters.
  static bool IsId(const std::string& text);

  // The asset type a path implies, or an empty string for a file the registry
  // does not know how to classify.
  static std::string TypeForPath(const std::filesystem::path& path);

  // The directory the registry was opened with, and the file it saves to.
  static const std::filesystem::path& Directory();
  static std::filesystem::path File();

 private:
  AssetRegistry() {}

  // Find, without the const, for the operations that edit an entry.
  static Asset* FindMutable(const std::string& id);

  static std::vector<Asset> s_assets;
  static std::filesystem::path s_directory;
};
}  // namespace rse
