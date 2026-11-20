#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>

#include <glm/glm.hpp>

#include "base_system.h"
#include <ecs/components/camera.h>
#include <ecs/components/transform.h>
#include <ecs/scene.h>
#include <render/ortho_view.h>

// Turns the best camera entity into the view the scene is framed with.
//
// A camera entity is one with CameraComponent and TransformComponent: the
// transform is the pose, the component is zoom and priority. The system writes
// the resolved pose into the scene's camera, which is what the game reads
// before it uploads its view and projection. A scene with no camera entity
// keeps the camera it was saved with, so older scenes and the editor's free
// camera behave exactly as they did.
//
// The game resolves the component again at upload time rather than reading the
// scene's copy, so a camera that moved during the last simulation step does
// not lag the view by a frame.
class CameraSystem : public BaseSystem {
 public:
  struct View {
    glm::vec2 position{0.0f};
    float zoom{1.0f};
  };

  // The pose of the active camera, or false when the scene has no enabled
  // camera entity. `view` may be null when only the answer matters. The zoom is
  // clamped to the range the projection supports, so a zero or absurd zoom
  // still produces a well-defined view.
  static bool Resolve(rse::Scene& scene, View* view) {
    uint32_t best = entt::null;
    int best_priority = std::numeric_limits<int>::min();
    View resolved;

    scene.Registry().view<CameraComponent, TransformComponent>().each(
        [&scene, &best, &best_priority, &resolved](auto entity, const CameraComponent& camera,
                                                   const TransformComponent& transform) {
          if (!scene.IsEntityEnabled(entity) || !scene.IsComponentEnabled<CameraComponent>(entity)) {
            return;
          }

          // Lowest id wins a tie, not registry order: two cameras added in the
          // reverse order must still resolve to the same one.
          if (best == entt::null || camera.priority > best_priority ||
              (camera.priority == best_priority && entity < best)) {
            best = entity;
            best_priority = camera.priority;
            resolved.position = glm::vec2(transform.position.x, transform.position.y);
            resolved.zoom = camera.zoom;
          }
        });

    if (best == entt::null) {
      return false;
    }

    resolved.zoom = std::clamp(resolved.zoom, rse::kOrthoMinZoom, rse::kOrthoMaxZoom);

    if (view != nullptr) {
      *view = resolved;
    }

    return true;
  }

  // Applies the resolved pose to the scene camera in the simulation phase, so
  // the scene's copy follows the same fixed steps as physics. The game uses
  // Resolve directly for the frame it is about to draw.
  void UpdateSimulation(rse::Scene& scene, const float step) override {
    (void)step;

    View view;

    if (!Resolve(scene, &view)) {
      return;
    }

    rse::Scene::Camera& camera = scene.GetCamera();
    camera.position = view.position;
    camera.zoom = view.zoom;
  }
};
