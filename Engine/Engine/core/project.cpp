#include "project.h"

#include <algorithm>
#include <exception>
#include <fstream>
#include <system_error>

#include <cereal/archives/xml.hpp>

namespace rse {
namespace {
// Only a slice of the manifest: enough to decide whether the rest can be read
// at all. A newer format may have renamed fields, so the version has to be
// checked before the reader is allowed to fail on it.
struct ManifestHeader {
  int format_version{0};

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("formatVersion", format_version));
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
}  // namespace

void Project::NormalisePaths() {
  asset_root = WithForwardSlashes(asset_root);
  startup_scene = WithForwardSlashes(startup_scene);

  for (std::string& scene : scenes) {
    scene = WithForwardSlashes(scene);
  }
}

bool Project::Load(const std::filesystem::path& file, Project& project, std::string* error) {
  int format_version{0};

  {
    std::ifstream stream{file};

    if (!stream.is_open()) {
      SetError(error, "not found: " + file.string());
      return false;
    }

    try {
      cereal::XMLInputArchive archive{stream};
      ManifestHeader header;
      archive(cereal::make_nvp("project", header));
      format_version = header.format_version;
    } catch (const std::exception& exception) {
      SetError(error, std::string("not a project file: ") + exception.what() + ": " + file.string());
      return false;
    } catch (...) {
      SetError(error, "not a project file: unknown error: " + file.string());
      return false;
    }
  }

  if (format_version < kFirstFormatVersion) {
    // Version 0 and below were never manifests: a file that claims one is not
    // a project file, whatever it contains.
    SetError(error, "not a project file: format version " + std::to_string(format_version) +
                        " is not a project version: " + file.string());
    return false;
  }

  if (format_version > kFormatVersion) {
    SetError(error, "project format version " + std::to_string(format_version) +
                        " is newer than this build supports (" + std::to_string(kFormatVersion) +
                        "): " + file.string());
    return false;
  }

  Project loaded;

  try {
    std::ifstream stream{file};

    if (!stream.is_open()) {
      SetError(error, "not found: " + file.string());
      return false;
    }

    cereal::XMLInputArchive archive{stream};
    archive(cereal::make_nvp("project", loaded));
  } catch (const std::exception& exception) {
    SetError(error, std::string("failed to read the project: ") + exception.what() + ": " + file.string());
    return false;
  } catch (...) {
    SetError(error, "failed to read the project: unknown error: " + file.string());
    return false;
  }

  loaded.m_file = std::filesystem::absolute(file);
  Migrate(loaded, format_version);
  project = loaded;
  return true;
}

void Project::Migrate(Project& project, int from_version) {
  // No published version has changed the meaning of existing data yet, so
  // every version this build reads is already in the current shape. When one
  // does, it gets one block per version, oldest first, each gated on the
  // version it upgrades *from*:
  //
  //   if (from_version < 2) { project.audio = ...; }   // #46b
  //
  // Additive fields need no block: serialize() already skips them for an
  // older file and the defaults stay in place.
  (void)project;
  (void)from_version;
}

bool Project::Save(std::string* error) const {
  if (!IsAttached()) {
    SetError(error, "no project file is attached");
    return false;
  }

  std::filesystem::path scratch{m_file};
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

    Project normalised{*this};
    normalised.NormalisePaths();

    // The file is written in *this* build's shape, not in the shape it was
    // read in: a manifest loaded as an older version and saved again must not
    // keep claiming the old one. The engine version is informational, but it
    // has the same problem -- it would otherwise keep naming the build that
    // wrote the file first.
    normalised.format_version = kFormatVersion;
    normalised.engine_version = kEngineVersion;

    archive(cereal::make_nvp("project", normalised));
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

  std::filesystem::rename(scratch, m_file, error_code);

  if (error_code) {
    std::filesystem::remove(scratch, error_code);
    SetError(error, error_code.message());
    return false;
  }

  return true;
}

std::filesystem::path Project::Resolve(const std::string& relative) const {
  std::filesystem::path path{WithForwardSlashes(relative)};

  if (path.is_absolute() || !IsAttached()) {
    return path;
  }

  return Directory() / path;
}
}  // namespace rse
