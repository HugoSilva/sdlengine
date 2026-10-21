#include "asset_registry.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <system_error>

#include <cereal/archives/xml.hpp>

#include <utils/logger.h>

namespace rse {
namespace {
// Only a slice of the file: enough to decide whether the rest can be read at
// all. A newer format may have renamed fields, so the version has to be checked
// before the reader is allowed to fail on it.
struct RegistryHeader {
  int format_version{0};

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("formatVersion", format_version));
  }
};

// The whole file. `assets` is a cereal container, so it is written as a list of
// <valueN> nodes under one <assets> wrapper, exactly like the manifest's scene
// list.
struct RegistryFile {
  int format_version{AssetRegistry::kFormatVersion};
  std::vector<AssetRegistry::Asset> assets;

  template <typename Archive>
  void serialize(Archive& archive) {
    // The version is read before anything it governs: on the way in, cereal has
    // assigned format_version by the time the next call runs, which is what
    // lets a later section be gated on it.
    archive(cereal::make_nvp("formatVersion", format_version));

    if (format_version >= 1) {
      archive(cereal::make_nvp("asset", assets));
    }
  }
};

void SetError(std::string* error, const std::string& message) {
  if (error != nullptr) {
    *error = message;
  }
}

std::string WithForwardSlashes(const std::string& path) {
  std::string normalised{path};
  std::replace(normalised.begin(), normalised.end(), '\\', '/');
  return normalised;
}

std::string Lowercase(const std::string& value) {
  std::string lowered{value};
  std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                 [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
  return lowered;
}

std::mt19937_64& RandomEngine() {
  static std::mt19937_64 engine{std::random_device{}()};
  return engine;
}
}  // namespace

std::vector<AssetRegistry::Asset> AssetRegistry::s_assets;
std::filesystem::path AssetRegistry::s_directory;

void AssetRegistry::Init(const std::filesystem::path& project_directory) {
  Clean();
  s_directory = std::filesystem::absolute(project_directory);

  const std::filesystem::path file = File();

  if (!std::filesystem::exists(file)) {
    return;
  }

  std::string error;

  if (!Load(file, &error)) {
    Logger::Error("Could not read " + file.string() + ": " + error);
    s_assets.clear();
  }
}

void AssetRegistry::Clean() {
  s_assets.clear();
  s_directory.clear();
}

bool AssetRegistry::Load(const std::filesystem::path& file, std::string* error) {
  int format_version{0};

  {
    std::ifstream stream{file};

    if (!stream.is_open()) {
      SetError(error, "not found: " + file.string());
      return false;
    }

    try {
      cereal::XMLInputArchive archive{stream};
      RegistryHeader header;
      archive(cereal::make_nvp("assets", header));
      format_version = header.format_version;
    } catch (const std::exception& exception) {
      SetError(error, std::string("not an asset registry: ") + exception.what() + ": " + file.string());
      return false;
    } catch (...) {
      SetError(error, "not an asset registry: unknown error: " + file.string());
      return false;
    }
  }

  // Below 1 it was never a registry, between 1 and the oldest this build reads
  // it is one of ours that is too old, and above the current one it is newer.
  if (format_version < 1) {
    SetError(error, "not an asset registry: format version " + std::to_string(format_version) +
                        " is not a registry version: " + file.string());
    return false;
  }

  if (format_version < kFirstFormatVersion) {
    SetError(error, "asset registry format version " + std::to_string(format_version) +
                        " is older than this build supports (" + std::to_string(kFirstFormatVersion) +
                        "): " + file.string());
    return false;
  }

  if (format_version > kFormatVersion) {
    SetError(error, "asset registry format version " + std::to_string(format_version) +
                        " is newer than this build supports (" + std::to_string(kFormatVersion) +
                        "): " + file.string());
    return false;
  }

  RegistryFile loaded;

  try {
    std::ifstream stream{file};

    if (!stream.is_open()) {
      SetError(error, "not found: " + file.string());
      return false;
    }

    cereal::XMLInputArchive archive{stream};
    archive(cereal::make_nvp("assets", loaded));
  } catch (const std::exception& exception) {
    SetError(error, std::string("failed to read the asset registry: ") + exception.what() + ": " + file.string());
    return false;
  } catch (...) {
    SetError(error, "failed to read the asset registry: unknown error: " + file.string());
    return false;
  }

  s_assets = std::move(loaded.assets);
  return true;
}

bool AssetRegistry::Save(std::string* error) {
  if (s_directory.empty()) {
    SetError(error, "no project directory is attached");
    return false;
  }

  const std::filesystem::path file = File();
  std::filesystem::path scratch{file};
  scratch += ".tmp";

  std::ofstream stream{scratch};

  if (!stream.is_open()) {
    SetError(error, "could not be opened for writing: " + scratch.string());
    return false;
  }

  bool succeeded = true;
  std::string message;

  try {
    cereal::XMLOutputArchive archive{stream};

    RegistryFile written;
    written.format_version = kFormatVersion;
    written.assets = s_assets;

    archive(cereal::make_nvp("assets", written));
  } catch (const std::exception& exception) {
    message = exception.what();
    succeeded = false;
  } catch (...) {
    message = "unknown error";
    succeeded = false;
  }

  stream.close();

  std::error_code error_code;

  if (!succeeded) {
    std::filesystem::remove(scratch, error_code);
    SetError(error, message);
    return false;
  }

  std::filesystem::rename(scratch, file, error_code);

  if (error_code) {
    std::filesystem::remove(scratch, error_code);
    SetError(error, error_code.message());
    return false;
  }

  return true;
}

const std::vector<AssetRegistry::Asset>& AssetRegistry::All() { return s_assets; }

AssetRegistry::Asset* AssetRegistry::FindMutable(const std::string& id) {
  for (Asset& asset : s_assets) {
    if (asset.id == id) {
      return &asset;
    }
  }

  return nullptr;
}

const AssetRegistry::Asset* AssetRegistry::Find(const std::string& id) { return FindMutable(id); }

const AssetRegistry::Asset* AssetRegistry::FindByName(const std::string& name) {
  for (const Asset& asset : s_assets) {
    if (asset.name == name) {
      return &asset;
    }
  }

  return nullptr;
}

const AssetRegistry::Asset* AssetRegistry::FindByPath(const std::string& path) {
  const std::string wanted = WithForwardSlashes(path);

  for (const Asset& asset : s_assets) {
    if (asset.path == wanted) {
      return &asset;
    }

    // A scene written before the registry stored the path relative to the asset
    // root, so a bare file name is also a match: "tex3.png" finds
    // "assets/tex3.png", and "sprites/crate.png" finds
    // "assets/sprites/crate.png".
    if (std::filesystem::path{asset.path}.filename().string() == wanted) {
      return &asset;
    }

    const std::string suffix = "/" + wanted;
    if (asset.path.size() > suffix.size() &&
        asset.path.compare(asset.path.size() - suffix.size(), suffix.size(), suffix) == 0) {
      return &asset;
    }
  }

  return nullptr;
}

const AssetRegistry::Asset* AssetRegistry::FindReference(const std::string& reference) {
  if (reference.empty()) {
    return nullptr;
  }

  if (const Asset* asset = Find(reference)) {
    return asset;
  }

  if (const Asset* asset = FindByName(reference)) {
    return asset;
  }

  return FindByPath(reference);
}

std::string AssetRegistry::Resolve(const std::string& id) {
  const Asset* asset = Find(id);
  return asset == nullptr ? std::string{} : asset->path;
}

std::string AssetRegistry::Name(const std::string& id) {
  const Asset* asset = Find(id);
  return asset == nullptr ? std::string{} : asset->name;
}

std::string AssetRegistry::Type(const std::string& id) {
  const Asset* asset = Find(id);
  return asset == nullptr ? std::string{} : asset->type;
}

std::string AssetRegistry::Add(const std::string& name, const std::string& type, const std::string& path) {
  std::string id = GenerateId();

  // The space is 2^64, so a collision is astronomically unlikely; the check
  // costs nothing and makes the invariant ("ids are unique") true by
  // construction rather than by probability.
  while (Find(id) != nullptr) {
    id = GenerateId();
  }

  Asset asset;
  asset.id = id;
  asset.name = name;
  asset.type = type;
  asset.path = WithForwardSlashes(path);
  s_assets.push_back(std::move(asset));

  return id;
}

bool AssetRegistry::SetName(const std::string& id, const std::string& name) {
  Asset* asset = FindMutable(id);

  if (asset == nullptr) {
    return false;
  }

  asset->name = name;
  return true;
}

bool AssetRegistry::Rename(const std::string& id, const std::string& new_name, std::string* error) {
  Asset* asset = FindMutable(id);

  if (asset == nullptr) {
    SetError(error, "no asset with id " + id);
    return false;
  }

  const std::string folded = WithForwardSlashes(new_name);

  if (folded.empty() || folded.find('/') != std::string::npos) {
    SetError(error, "an asset name cannot be empty or contain a path: " + new_name);
    return false;
  }

  std::filesystem::path file_name{folded};

  // A name with no extension keeps the file's own: renaming "crate" must not
  // silently strip ".png" and turn the file into one nothing can load.
  if (!file_name.has_extension()) {
    file_name += std::filesystem::path{asset->path}.extension();
  }

  const std::filesystem::path old_path{asset->path};
  const std::filesystem::path new_path = old_path.parent_path() / file_name;

  if (new_path.generic_string() == old_path.generic_string()) {
    asset->name = file_name.string();
    return true;
  }

  if (!s_directory.empty()) {
    std::error_code error_code;
    const std::filesystem::path old_absolute = s_directory / old_path;
    const std::filesystem::path new_absolute = s_directory / new_path;

    if (std::filesystem::exists(new_absolute, error_code)) {
      SetError(error, "there is already a file at " + new_path.generic_string());
      return false;
    }

    // A file that is not there is not fatal: the entry is still renamed, which
    // is what a registry with a stale path needs.
    if (std::filesystem::exists(old_absolute, error_code)) {
      std::filesystem::rename(old_absolute, new_absolute, error_code);

      if (error_code) {
        SetError(error, error_code.message());
        return false;
      }
    }
  }

  asset->name = file_name.string();
  asset->path = new_path.generic_string();
  return true;
}

bool AssetRegistry::Remove(const std::string& id) {
  const std::vector<Asset>::iterator found =
      std::find_if(s_assets.begin(), s_assets.end(), [&id](const Asset& asset) { return asset.id == id; });

  if (found == s_assets.end()) {
    return false;
  }

  s_assets.erase(found);
  return true;
}

std::string AssetRegistry::GenerateId() {
  std::uniform_int_distribution<std::uint64_t> distribution;

  std::ostringstream out;
  out << std::hex << std::setw(16) << std::setfill('0') << distribution(RandomEngine());
  return out.str();
}

bool AssetRegistry::IsId(const std::string& text) {
  if (text.size() != 16) {
    return false;
  }

  return std::all_of(text.begin(), text.end(), [](unsigned char character) { return std::isxdigit(character) != 0; });
}

std::string AssetRegistry::TypeForPath(const std::filesystem::path& path) {
  const std::string extension = Lowercase(path.extension().string());

  if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp" ||
      extension == ".gif" || extension == ".tga" || extension == ".webp" || extension == ".avif" ||
      extension == ".qoi") {
    return "texture";
  }

  if (extension == ".wav" || extension == ".ogg" || extension == ".mp3" || extension == ".flac") {
    return "sound";
  }

  if (extension == ".ttf" || extension == ".otf") {
    return "font";
  }

  if (extension == ".xml") {
    return "prefab";
  }

  return "";
}

const std::filesystem::path& AssetRegistry::Directory() { return s_directory; }

std::filesystem::path AssetRegistry::File() {
  if (s_directory.empty()) {
    return std::filesystem::path{kFileName};
  }

  return s_directory / kFileName;
}
}  // namespace rse
