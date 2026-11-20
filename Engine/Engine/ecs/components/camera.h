#pragma once

#include <cereal/cereal.hpp>

// A camera attached to an entity. The pose is the entity's TransformComponent,
// so moving or parenting the entity moves the view; this component carries only
// what is specific to the camera.
//
// A scene may hold several cameras. CameraSystem picks the enabled one with the
// highest priority and, on a tie, the lower entity id, so the choice does not
// depend on registry iteration order. A disabled entity or a disabled component
// is skipped, which lets the inspector switch a camera off without deleting it.
struct CameraComponent {
  // How much of the 1280x720 reference rectangle the view shows: 1 shows all of
  // it, 2 shows half. The resolved value is clamped to the same range the
  // editor camera uses.
  float zoom{1.0f};

  // Higher wins when more than one camera is enabled.
  int priority{0};

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("Zoom", zoom), cereal::make_nvp("Priority", priority));
  }
};
