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
    registry.snapshot().entities(archive).component<BasicComponent, TransformComponent, SpriteComponent>(archive);
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

    // Groups added after version 1 are chained here, in the order SaveEntities
    // writes them, each read only from the version that introduced it so a
    // file that predates the group keeps the default:
    //
    //   if (format_version >= 2) loader.component<AudioComponent>(archive);
  }
};

void SetError(std::string* error, const std::string& message) {
  if (error != nullptr) {
    *error = message;
  }
}

// Components that have cereal hooks but are not in the saved set yet, so an
// entity that carries one loses it on the next load. The check and the warning
// both come from this list: deleting a row when the component starts
// round-tripping updates both, and keeps the message from naming something the
// save actually keeps.
struct UnsavedComponent {
  const char* name;
  bool (*has)(entt::DefaultRegistry& registry, entt::DefaultRegistry::entity_type entity);
};

template <typename Component>
bool HasComponent(entt::DefaultRegistry& registry, entt::DefaultRegistry::entity_type entity) {
  return registry.has<Component>(entity);
}

constexpr UnsavedComponent kUnsavedComponents[] = {
    {"AudioComponent", &HasComponent<AudioComponent>},
    {"RigidBodyComponent", &HasComponent<RigidBodyComponent>},
};

void WarnAboutComponentsThatAreNotSaved(entt::DefaultRegistry& registry) {
  int entities = 0;

  registry.each([&entities, &registry](const auto entity) {
    for (const UnsavedComponent& component : kUnsavedComponents) {
      if (component.has(registry, entity)) {
        ++entities;
        return;
      }
    }
  });

  if (entities == 0) {
    return;
  }

  std::string names;

  for (const UnsavedComponent& component : kUnsavedComponents) {
    if (!names.empty()) {
      names += ", ";
    }

    names += component.name;
  }

  Logger::Warning(std::to_string(entities) + " entities carry components that are not saved (" + names + ")");
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

    if (format_version < kFirstFormatVersion) {
      SetError(error, "not a scene file: format version " + std::to_string(format_version) +
                          " is not a scene version: " + file.string());
      return false;
    }

    if (format_version > kFormatVersion) {
      SetError(error, "scene format version " + std::to_string(format_version) +
                          " is newer than this build supports (" + std::to_string(kFormatVersion) +
                          "): " + file.string());
      return false;
    }

    // The header was readable, so this really is a scene file: replacing the
    // open scene is now the right thing to do.
    Clear();
    cleared = true;

    LoadEntities entities{ECSManager::registry, format_version};
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
