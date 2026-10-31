#pragma once

namespace rse {
// The fixture-level surface properties a rigid body's shape uses. A physics
// material is shared data: many bodies can reference one asset, and the values
// are copied into the b2Fixture when the body is created.
struct PhysicsMaterial {
  // The defaults are what a body with no material uses. They match the values
  // Rigidbody authored before density and friction moved into this asset.
  static constexpr float kDefaultDensity{1.0f};
  static constexpr float kDefaultFriction{0.3f};
  static constexpr float kDefaultRestitution{0.0f};

  float density{kDefaultDensity};
  float friction{kDefaultFriction};
  float restitution{kDefaultRestitution};
};
}  // namespace rse
