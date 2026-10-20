#include "scene_io.h"

#include <exception>
#include <fstream>
#include <system_error>

#include <cereal/archives/xml.hpp>

#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>
#include <utils/logger.h>

namespace rse {
namespace {
// What a scene file stores is spelled out once per direction below. Audio and
// rigid-body components have cereal hooks of their own but are not round-trip
// safe yet, so they are deliberately absent and warned about on save.
//
// entt writes its snapshot as unnamed nodes, which cereal names value0,
// value1, ... at the archive root. Wrapping it in a named node keeps a scene
// file readable: <formatVersion> and <entities> are siblings, and everything
// entt writes stays inside the second one.
struct SaveEntities {
  entt::DefaultRegistry& registry;

  template <typename Archive>
  void serialize(Archive& archive) {
    registry.snapshot()
        .entities(archive)
        .component<BasicComponent, TransformComponent, SpriteComponent>(archive);
  }
};

struct LoadEntities {
  entt::DefaultRegistry& registry;

  template <typename Archive>
  void serialize(Archive& archive) {
    registry.restore()
        .entities(archive)
        .component<BasicComponent, TransformComponent, SpriteComponent>(archive);
  }
};

void SetError(std::string* error, const std::string& message) {
  if (error != nullptr) {
    *error = message;
  }
}

// Audio and rigid-body components are not part of the saved set yet, so an
// entity that carries one loses it on the next load. Say so once per save
// instead of dropping it silently.
void WarnAboutComponentsThatAreNotSaved(entt::DefaultRegistry& registry) {
  int entities = 0;

  registry.each([&entities, &registry](const auto entity) {
    if (registry.has<AudioComponent>(entity) || registry.has<RigidBodyComponent>(entity)) {
      ++entities;
    }
  });

  if (entities > 0) {
    Logger::Warning(std::to_string(entities) +
                    " entities carry components that are not saved (AudioComponent, RigidBodyComponent)");
  }
}
}  // namespace

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

    const int format_version{kFormatVersion};
    archive(cereal::make_nvp("formatVersion", format_version));

    SaveEntities entities{ECSManager::registry};
    archive(cereal::make_nvp("entities", entities));
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

  WarnAboutComponentsThatAreNotSaved(ECSManager::registry);
  return true;
}

bool SceneIO::Load(const std::filesystem::path& file, std::string* error) {
  std::ifstream stream{file};

  if (!stream.is_open()) {
    SetError(error, "not found: " + file.string());
    return false;
  }

  int format_version{0};
  bool cleared = false;

  try {
    cereal::XMLInputArchive archive{stream};

    archive(cereal::make_nvp("formatVersion", format_version));

    if (format_version != kFormatVersion) {
      SetError(error, "unsupported scene format version " + std::to_string(format_version) + " (expected " +
                          std::to_string(kFormatVersion) + "): " + file.string());
      return false;
    }

    // The header was readable, so this really is a scene file: replacing the
    // open scene is now the right thing to do.
    Clear();
    cleared = true;

    LoadEntities entities{ECSManager::registry};
    archive(cereal::make_nvp("entities", entities));
  } catch (const std::exception& exception) {
    if (cleared) {
      Clear();
    }

    SetError(error, exception.what());
    return false;
  } catch (...) {
    if (cleared) {
      Clear();
    }

    SetError(error, "unknown error");
    return false;
  }

  return true;
}

void SceneIO::Clear() { ECSManager::registry.reset(); }
}  // namespace rse
