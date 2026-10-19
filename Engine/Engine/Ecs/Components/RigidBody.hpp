#pragma once

#include <cereal/cereal.hpp>

#include "../../Physics/Rigidbody.h"

struct RigidBodyComponent {
  RigidBodyComponent() {};

  RigidBodyComponent(Rigidbody* rigidbody) : m_rigidbody(rigidbody) {};

  std::shared_ptr<Rigidbody> m_rigidbody;

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("Rigidbody", m_rigidbody));
  }
};