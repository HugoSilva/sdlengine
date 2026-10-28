#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <set>
#include <string>
#include <system_error>
#include <vector>

#include <core/asset_registry.h>

// ---------------------------------------------------------------------------
// The assets a sprite field can point at.
//
// A scene stores an asset *reference*: the registry's id for an imported
// texture or material, or, for an older or unregistered file, a path. The
// inspector's pickers offer both: every registered asset of the requested type,
// plus every file of that type in the project that the registry does not know
// yet. Choosing an unregistered file registers it first (ImportAssetAt), so the
// sprite stores a stable id rather than a path.
//
// TextureChoice keeps its original name because both pickers use the same row
// shape; `BuildMaterialCatalog` is the material-typed half.
// ---------------------------------------------------------------------------

// One row in a picker.
struct TextureChoice {
  // What the sprite stores: an asset id. Empty for an unregistered file, which
  // the picker registers before assigning.
  std::string reference;

  // What the picker shows.
  std::string name;

  // The file's path relative to the project directory, with forward slashes.
  // Used to find the file again and to deduplicate a registered asset against
  // the same file found by the project scan.
  std::string project_path;

  // The path relative to the project's asset root, when the file is inside it.
  // This is the legacy reference a scene would have stored before the registry.
  std::string asset_root_path;

  bool registered{false};
};

// The drag-and-drop payload the content browser sends and the inspector
// accepts. ImGui copies the bytes of a payload, so this is a fixed buffer
// rather than a std::string.
struct TextureDragPayload {
  char path[1024]{};
};

inline constexpr const char* kTextureDragDropPayload = "RSE_TEXTURE";

inline std::string Lowercase(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
  return value;
}

inline bool IsImageFile(const std::filesystem::path& path) {
  return rse::AssetRegistry::TypeForPath(path) == "texture";
}

inline bool IsMaterialFile(const std::filesystem::path& path) {
  return rse::AssetRegistry::TypeForPath(path) == "material";
}

inline bool IsHiddenPath(const std::filesystem::path& path) {
  const std::string name = path.filename().string();
  return !name.empty() && name.front() == '.';
}

inline std::string ProjectRelative(const std::filesystem::path& file, const std::filesystem::path& project_directory) {
  if (project_directory.empty()) {
    return {};
  }

  std::error_code error;
  const std::filesystem::path relative = std::filesystem::relative(file, project_directory, error);

  if (error || relative.empty() || relative.generic_string().rfind("..", 0) == 0) {
    return {};
  }

  return relative.generic_string();
}

inline std::string AssetRootRelative(const std::filesystem::path& file, const std::filesystem::path& project_directory,
                                     const std::string& asset_root) {
  if (project_directory.empty() || asset_root.empty()) {
    return {};
  }

  std::error_code error;
  const std::filesystem::path relative = std::filesystem::relative(file, project_directory / asset_root, error);

  if (error || relative.empty() || relative.generic_string().rfind("..", 0) == 0) {
    return {};
  }

  return relative.generic_string();
}

// Every asset of `type` the inspector can offer. The registry's entries are
// added first, then the project scan adds files of that type the registry does
// not know yet. Hidden directories are skipped, and the scan only runs when a
// project is open.
inline std::vector<TextureChoice> BuildAssetCatalog(const std::filesystem::path& project_directory,
                                                    const std::string& asset_root, const std::string& type) {
  std::vector<TextureChoice> choices;
  std::set<std::string> known_paths;

  for (const rse::AssetRegistry::Asset& asset : rse::AssetRegistry::All()) {
    if (asset.type != type) {
      continue;
    }

    TextureChoice choice;
    choice.reference = asset.id;
    choice.name = asset.name;
    choice.project_path = asset.path;
    choice.asset_root_path = AssetRootRelative(project_directory / asset.path, project_directory, asset_root);
    choice.registered = true;
    choices.push_back(choice);
    known_paths.insert(Lowercase(asset.path));
  }

  if (!project_directory.empty()) {
    std::error_code error;
    std::filesystem::recursive_directory_iterator iterator(
        project_directory, std::filesystem::directory_options::skip_permission_denied, error);
    const std::filesystem::recursive_directory_iterator end;

    while (!error && iterator != end) {
      const std::filesystem::directory_entry& entry = *iterator;

      std::error_code entry_error;

      if (entry.is_directory(entry_error)) {
        if (IsHiddenPath(entry.path())) {
          iterator.disable_recursion_pending();
        }
      } else if (entry.is_regular_file(entry_error) && rse::AssetRegistry::TypeForPath(entry.path()) == type) {
        const std::string project_path = ProjectRelative(entry.path(), project_directory);

        if (!project_path.empty() && known_paths.find(Lowercase(project_path)) == known_paths.end()) {
          TextureChoice choice;
          choice.name = entry.path().filename().string();
          choice.project_path = project_path;
          choice.asset_root_path = AssetRootRelative(entry.path(), project_directory, asset_root);
          choices.push_back(choice);
        }
      }

      iterator.increment(error);
    }
  }

  std::sort(choices.begin(), choices.end(), [](const TextureChoice& a, const TextureChoice& b) {
    const std::string a_name = Lowercase(a.name);
    const std::string b_name = Lowercase(b.name);

    if (a_name != b_name) {
      return a_name < b_name;
    }

    return Lowercase(a.project_path) < Lowercase(b.project_path);
  });

  return choices;
}

inline std::vector<TextureChoice> BuildTextureCatalog(const std::filesystem::path& project_directory,
                                                      const std::string& asset_root) {
  return BuildAssetCatalog(project_directory, asset_root, "texture");
}

inline std::vector<TextureChoice> BuildMaterialCatalog(const std::filesystem::path& project_directory,
                                                       const std::string& asset_root) {
  return BuildAssetCatalog(project_directory, asset_root, "material");
}

inline bool MatchesTextureQuery(const TextureChoice& choice, const std::string& query) {
  if (query.empty()) {
    return true;
  }

  const std::string lowered = Lowercase(query);

  return Lowercase(choice.name).find(lowered) != std::string::npos ||
         Lowercase(choice.reference).find(lowered) != std::string::npos ||
         Lowercase(choice.project_path).find(lowered) != std::string::npos;
}

// The id of the texture asset whose exact project-relative path is `file`, or
// an empty string when the file is not registered. This deliberately does not
// use FindByPath: that helper also matches a bare file name, which would make a
// root-level image inherit the id of a same-named image in a subdirectory.
inline std::string RegisteredAssetIdForFile(const std::filesystem::path& file,
                                            const std::filesystem::path& project_directory, const std::string& type) {
  const std::string project_path = ProjectRelative(file, project_directory);

  if (project_path.empty()) {
    return {};
  }

  const std::string wanted = Lowercase(project_path);

  for (const rse::AssetRegistry::Asset& asset : rse::AssetRegistry::All()) {
    if (asset.type == type && Lowercase(asset.path) == wanted) {
      return asset.id;
    }
  }

  return {};
}

inline std::string RegisteredTextureIdForFile(const std::filesystem::path& file,
                                              const std::filesystem::path& project_directory) {
  return RegisteredAssetIdForFile(file, project_directory, "texture");
}
