#include "scene_io.h"

#include <exception>
#include <fstream>
#include <sstream>
#include <system_error>

#include <cereal/archives/xml.hpp>

#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>

namespace rse {
namespace {
// What a scene file stores is spelled out once per direction below: every
// component a scene can carry has a cereal hook that writes data, never a
// pointer, so nothing is dropped on save.
//
// entt writes its snapshot as unnamed nodes, which cereal names value0,
// value1, ... at the archive root. Wrapping it in a named node keeps a scene
// file readable: <formatVersion>, <camera> and <entities> are siblings, and
// everything entt writes stays inside the last one.
struct SaveEntities {
  entt::DefaultRegistry& registry;

  template <typename Archive>
  void serialize(Archive& archive) {
    registry.snapshot()
        .entities(archive)
        .component<BasicComponent, TransformComponent, SpriteComponent, AudioComponent, RigidBodyComponent,
                   SpriteAppearanceComponent, EntityEnabledComponent>(archive);
  }
};

struct LoadEntities {
  entt::DefaultRegistry& registry;
  int format_version{0};

  template <typename Archive>
  void serialize(Archive& archive) {
    // One loader for the whole file: restore() asserts the registry is empty,
    // so a second one would fire that assert on every load.
    auto loader = registry.restore();

    loader.entities(archive);
    loader.component<BasicComponent, TransformComponent, SpriteComponent>(archive);

    // Groups added after version 2 are chained here, in the order SaveEntities
    // writes them, each read only from the version that introduced it:
    //
    //   if (format_version >= 7) loader.component<...>(archive);
    if (format_version >= 3) {
      loader.component<AudioComponent>(archive);
    }

    if (format_version >= 4) {
      loader.component<RigidBodyComponent>(archive);
    }

    if (format_version >= 7) {
      loader.component<SpriteAppearanceComponent>(archive);
    }

    if (format_version >= 8) {
      loader.component<EntityEnabledComponent>(archive);
    }
  }
};

void SetError(std::string* error, const std::string& message) {
  if (error != nullptr) {
    *error = message;
  }
}

// One writer for both destinations: the file functions and the in-memory
// snapshot have to produce the same payload, or a scene would load differently
// depending on which route it travelled.
template <typename Archive>
void WriteScene(Archive& archive, entt::DefaultRegistry& registry) {
  const int format_version{SceneIO::kFormatVersion};
  archive(cereal::make_nvp("formatVersion", format_version));

  // The camera is scene-root data, written before the entities it frames.
  archive(cereal::make_nvp("camera", SceneIO::GetCamera()));

  SaveEntities entities{registry};
  archive(cereal::make_nvp("entities", entities));
}

// One reader for both sources. `source` is what an error names: a path, or
// <snapshot> for the play session.
template <typename Archive>
bool ReadScene(Archive& archive, entt::DefaultRegistry& registry, const std::string& source, std::string* error) {
  int format_version{0};
  bool cleared = false;

  try {
    archive(cereal::make_nvp("formatVersion", format_version));

    // Three cases, three messages: below 1 is not a scene at all, below the
    // first version this build reads is one of ours that is too old to read,
    // and above the current one is newer than this build.
    if (format_version < 1) {
      SetError(error, "not a scene file: format version " + std::to_string(format_version) +
                          " is not a scene version: " + source);
      return false;
    }

    if (format_version < SceneIO::kFirstFormatVersion) {
      SetError(error, "scene format version " + std::to_string(format_version) +
                          " is older than this build supports (" + std::to_string(SceneIO::kFirstFormatVersion) +
                          "): " + source);
      return false;
    }

    if (format_version > SceneIO::kFormatVersion) {
      SetError(error, "scene format version " + std::to_string(format_version) +
                          " is newer than this build supports (" + std::to_string(SceneIO::kFormatVersion) +
                          "): " + source);
      return false;
    }

    // The header was readable, so this really is a scene file: replacing the
    // open scene is now the right thing to do.
    SceneIO::Clear();
    cleared = true;

    // Version 5 added the camera node. An older scene has none, so the value
    // Clear() left is the default framing, which is what it was drawn with.
    if (format_version >= 5) {
      archive(cereal::make_nvp("camera", SceneIO::GetCamera()));
    }

    LoadEntities entities{registry, format_version};
    archive(cereal::make_nvp("entities", entities));
  } catch (const std::exception& exception) {
    if (cleared) {
      SceneIO::Clear();
    }

    SetError(error, exception.what());
    return false;
  } catch (...) {
    if (cleared) {
      SceneIO::Clear();
    }

    SetError(error, "unknown error");
    return false;
  }

  return true;
}
}  // namespace

SceneIO::SceneCamera SceneIO::m_camera;

bool SceneIO::Save(const std::filesystem::path& file, std::string* error) {
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
    WriteScene(archive, ECSManager::registry);
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

bool SceneIO::Load(const std::filesystem::path& file, std::string* error) {
  std::ifstream stream{file};

  if (!stream.is_open()) {
    SetError(error, "not found: " + file.string());
    return false;
  }

  try {
    cereal::XMLInputArchive archive{stream};
    return ReadScene(archive, ECSManager::registry, file.string(), error);
  } catch (const std::exception& exception) {
    SetError(error, exception.what());
    return false;
  } catch (...) {
    SetError(error, "unknown error");
    return false;
  }
}

bool SceneIO::SaveToString(std::string* contents, std::string* error) {
  if (contents == nullptr) {
    SetError(error, "no string to save the scene into");
    return false;
  }

  std::ostringstream stream;

  try {
    // The archive closes on destruction, which is what writes the closing
    // tags: take the string after the archive is gone.
    {
      cereal::XMLOutputArchive archive{stream};
      WriteScene(archive, ECSManager::registry);
    }

    *contents = stream.str();
  } catch (const std::exception& exception) {
    SetError(error, exception.what());
    return false;
  } catch (...) {
    SetError(error, "unknown error");
    return false;
  }

  return true;
}

bool SceneIO::LoadFromString(const std::string& contents, std::string* error) {
  std::istringstream stream{contents};

  try {
    cereal::XMLInputArchive archive{stream};
    return ReadScene(archive, ECSManager::registry, "<snapshot>", error);
  } catch (const std::exception& exception) {
    SetError(error, exception.what());
    return false;
  } catch (...) {
    SetError(error, "unknown error");
    return false;
  }
}

void SceneIO::Clear() {
  ECSManager::registry.reset();
  m_camera = SceneCamera{};
}
}  // namespace rse
