#pragma once

#include <string>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>

#include <core/uuid.h>

// The stable identity of an entity.
//
// An entt id names a slot in one registry: it is reused after a destroy and it
// is not written to a file. This component is what a save, a play-session copy
// and a prefab placement can all agree on, because the uuid inside it is made
// once and travels with the entity instead of being handed out again.
//
// The scene writes the canonical text form. A uuid that does not parse -- a
// hand-edited file, a group written by something else -- is replaced with a
// fresh one rather than failing the load: the scene has to stay usable, and
// giving the entity a new name is the only outcome that keeps identities
// unique.
struct UuidComponent {
  UuidComponent() : m_uuid(rse::Uuid::Generate()) {}

  explicit UuidComponent(const rse::Uuid& uuid) : m_uuid(uuid) {}

  rse::Uuid m_uuid;

  template <typename Archive>
  void serialize(Archive& archive) {
    if constexpr (Archive::is_loading::value) {
      std::string text;
      archive(cereal::make_nvp("Value", text));

      if (!rse::Uuid::Parse(text, &m_uuid) || m_uuid.IsNil()) {
        m_uuid = rse::Uuid::Generate();
      }
    } else {
      const std::string text = m_uuid.ToString();
      archive(cereal::make_nvp("Value", text));
    }
  }
};
