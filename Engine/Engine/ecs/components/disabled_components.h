#pragma once

#include <cstdint>

#include <cereal/cereal.hpp>

// Which optional components are switched off on an entity. A missing component
// means every component is enabled, so scenes and prefabs written before this
// group existed load exactly as they did. Disabling a component writes this
// group; the runtime systems skip a component whose bit is set.
struct DisabledComponentsComponent {
  enum Bit : uint32_t {
    kSprite = 1u << 0,
    kAudio = 1u << 1,
    kRigidbody = 1u << 2,
  };

  uint32_t m_mask{0};

  bool IsDisabled(Bit bit) const { return (m_mask & static_cast<uint32_t>(bit)) != 0; }

  void SetEnabled(Bit bit, bool enabled) {
    if (enabled) {
      m_mask &= ~static_cast<uint32_t>(bit);
    } else {
      m_mask |= static_cast<uint32_t>(bit);
    }
  }

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("Mask", m_mask));
  }
};

struct SpriteComponent;
struct AudioComponent;
struct RigidBodyComponent;

// The inspector, the add/remove handlers and the systems all name a component
// by type, so the mapping from type to mask bit lives in one place.
template <typename Component>
struct ComponentDisabledBit;

template <>
struct ComponentDisabledBit<SpriteComponent> {
  static constexpr DisabledComponentsComponent::Bit kValue = DisabledComponentsComponent::kSprite;
};

template <>
struct ComponentDisabledBit<AudioComponent> {
  static constexpr DisabledComponentsComponent::Bit kValue = DisabledComponentsComponent::kAudio;
};

template <>
struct ComponentDisabledBit<RigidBodyComponent> {
  static constexpr DisabledComponentsComponent::Bit kValue = DisabledComponentsComponent::kRigidbody;
};
