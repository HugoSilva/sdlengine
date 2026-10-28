#include "prefab_io.h"

#include <exception>
#include <fstream>
#include <system_error>

#include <cereal/archives/xml.hpp>
#include <entity/registry.hpp>

#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>

namespace rse {
namespace {
using Registry = entt::DefaultRegistry;

// A prefab file stores one entity snapshot, in the same shape a scene's
// <entities> block has: a count, the ids entt wrote, and the component groups
// in the scene's order. A group appended to prefabs later is read only from the
// version that introduced it, exactly like a scene group.
struct SaveEntity {
  Registry& registry;

  template <typename Archive>
  void serialize(Archive& archive) {
    registry.snapshot()
        .entities(archive)
        .component<BasicComponent, TransformComponent, SpriteComponent, AudioComponent, RigidBodyComponent,
                   SpriteAppearanceComponent>(archive);
  }
};

struct LoadEntity {
  Registry& registry;
  int format_version{0};

  template <typename Archive>
  void serialize(Archive& archive) {
    // restore() requires an empty registry, which the scratch one always is
    // when a prefab is loaded. It keeps the file's ids; Instantiate copies the
    // entities into the scene and lets the scene hand out its own.
    auto loader = registry.restore();

    loader.entities(archive);
    loader.component<BasicComponent, TransformComponent, SpriteComponent, AudioComponent, RigidBodyComponent>(archive);

    // Format 1 stores the five groups above. Groups appended later are chained
    // here, each read only from the version that introduced it.
    if (format_version >= 2) {
      loader.component<SpriteAppearanceComponent>(archive);
    }
  }
};

void SetError(std::string* error, const std::string& message) {
  if (error != nullptr) {
    *error = message;
  }
}

// Adds a copy of `entity` to `target` with a fresh id and returns that id.
//
// The copy is deep where an entity owns something. A sprite is per entity, so
// the copy gets its own Sprite; the texture inside stays borrowed from
// TextureManager, which is what sharing a texture name means. A rigid body is
// per entity and cannot be copied, so it gets a Clone() with the same inputs,
// uninitialised until PhysicsSystem sees it. A sound is borrowed from
// SoundManager by design, so the copied handle points at that one sound.
Registry::entity_type CopyEntity(Registry& source, Registry::entity_type entity, Registry& target) {
  const Registry::entity_type copy = target.create();

  if (source.has<BasicComponent>(entity)) {
    target.assign<BasicComponent>(copy, source.get<BasicComponent>(entity));
  }

  if (source.has<TransformComponent>(entity)) {
    target.assign<TransformComponent>(copy, source.get<TransformComponent>(entity));
  }

  if (source.has<SpriteComponent>(entity)) {
    const SpriteComponent& sprite = source.get<SpriteComponent>(entity);
    Sprite* sprite_copy = sprite.m_sprite == nullptr ? nullptr : new Sprite{*sprite.m_sprite};
    target.assign<SpriteComponent>(copy, SpriteComponent{sprite_copy});
  }

  if (source.has<SpriteAppearanceComponent>(entity)) {
    target.assign<SpriteAppearanceComponent>(copy, source.get<SpriteAppearanceComponent>(entity));
  }

  if (source.has<AudioComponent>(entity)) {
    target.assign<AudioComponent>(copy, source.get<AudioComponent>(entity));
  }

  if (source.has<RigidBodyComponent>(entity)) {
    const RigidBodyComponent& rigidbody = source.get<RigidBodyComponent>(entity);
    Rigidbody* body_copy = rigidbody.m_rigidbody == nullptr ? nullptr : rigidbody.m_rigidbody->Clone();
    target.assign<RigidBodyComponent>(copy, RigidBodyComponent{body_copy});
  }

  return copy;
}
}  // namespace

bool PrefabIO::Save(const std::filesystem::path& file, uint32_t entity, std::string* error) {
  // A selection can outlive the entity it names. Writing a prefab of an entity
  // that is not there would write a file with no entity in it.
  if (!ECSManager::registry.valid(entity)) {
    SetError(error, "entity does not exist in the open scene");
    return false;
  }

  // The writer is a registry snapshot, so the entity is copied into a scratch
  // registry first. The copy is independent of the scene and the scene itself
  // is only read.
  entt::DefaultRegistry scratch;
  CopyEntity(ECSManager::registry, entity, scratch);

  std::filesystem::path scratch_file{file};
  scratch_file += ".tmp";

  std::ofstream stream{scratch_file};

  if (!stream.is_open()) {
    SetError(error, "could not be opened for writing: " + scratch_file.string());
    return false;
  }

  bool succeeded = true;
  std::string message;

  try {
    cereal::XMLOutputArchive archive{stream};

    const int format_version{kFormatVersion};
    archive(cereal::make_nvp("formatVersion", format_version));

    SaveEntity save{scratch};
    archive(cereal::make_nvp("entity", save));
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
    std::filesystem::remove(scratch_file, error_code);
    SetError(error, message);
    return false;
  }

  std::filesystem::rename(scratch_file, file, error_code);

  if (error_code) {
    std::filesystem::remove(scratch_file, error_code);
    SetError(error, error_code.message());
    return false;
  }

  return true;
}

uint32_t PrefabIO::Instantiate(const std::filesystem::path& file, std::string* error) {
  std::ifstream stream{file};

  if (!stream.is_open()) {
    SetError(error, "not found: " + file.string());
    return entt::null;
  }

  int format_version{0};
  entt::DefaultRegistry scratch;

  try {
    cereal::XMLInputArchive archive{stream};

    archive(cereal::make_nvp("formatVersion", format_version));

    // Three cases, three messages, the same contract as a scene file: below 1
    // is not a prefab at all, below the first version this build reads is one
    // of ours that is too old, and above the current one is newer than this
    // build.
    if (format_version < 1) {
      SetError(error, "not a prefab file: format version " + std::to_string(format_version) +
                          " is not a prefab version: " + file.string());
      return entt::null;
    }

    if (format_version < kFirstFormatVersion) {
      SetError(error, "prefab format version " + std::to_string(format_version) +
                          " is older than this build supports (" + std::to_string(kFirstFormatVersion) +
                          "): " + file.string());
      return entt::null;
    }

    if (format_version > kFormatVersion) {
      SetError(error, "prefab format version " + std::to_string(format_version) +
                          " is newer than this build supports (" + std::to_string(kFormatVersion) +
                          "): " + file.string());
      return entt::null;
    }

    LoadEntity load{scratch, format_version};
    archive(cereal::make_nvp("entity", load));
  } catch (const std::exception& exception) {
    SetError(error, exception.what());
    return entt::null;
  } catch (...) {
    SetError(error, "unknown error");
    return entt::null;
  }

  // The whole file was read, so the open scene can now be touched. Every entity
  // is copied in with an id the scene hands out -- never the id from the file.
  Registry& scene = ECSManager::registry;
  uint32_t first = entt::null;

  scratch.each([&first, &scratch, &scene](Registry::entity_type entity) {
    const Registry::entity_type copy = CopyEntity(scratch, entity, scene);

    if (first == entt::null) {
      first = copy;
    }
  });

  if (first == entt::null) {
    SetError(error, "prefab contains no entity: " + file.string());
  }

  return first;
}
}  // namespace rse
