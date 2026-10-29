#pragma once

#include <cereal/cereal.hpp>

// Whether an entity takes part in the scene. A missing component means enabled,
// so every entity -- including one loaded from a scene or prefab written before
// this component existed -- starts enabled. Disabling an entity writes this
// group in the scene and prefab formats; the runtime systems skip it, so it is
// neither drawn nor simulated.
struct EntityEnabledComponent {
  EntityEnabledComponent() {}

  explicit EntityEnabledComponent(bool enabled) : m_enabled(enabled) {}

  bool m_enabled{true};

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("Enabled", m_enabled));
  }
};
