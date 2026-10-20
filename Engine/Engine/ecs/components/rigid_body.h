#pragma once

#include <cereal/cereal.hpp>

#include <physics/rigidbody.h>

// A body belongs to one entity, so the component owns it -- unlike a texture or
// a sound, there is no manager to share it with. The archive keeps cereal's
// shared_ptr wrapper, but what is inside is only data: Rigidbody::save writes
// the inputs, and load_and_construct rebuilds a body that has no b2Body yet.
struct RigidBodyComponent {
  RigidBodyComponent() {};

  RigidBodyComponent(Rigidbody* rigidbody) : m_rigidbody(rigidbody) {};

  std::shared_ptr<Rigidbody> m_rigidbody;

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("Rigidbody", m_rigidbody));
  }
};
