#pragma once

#include <algorithm>
#include <memory>

#include "base_system.h"
#include <core/material_manager.h>
#include <ecs/components/basic.h>
#include <ecs/components/rigid_body.h>
#include <ecs/components/sprite_appearance.h>
#include <ecs/scene.h>
#include <render/renderer_2d.h>

class SpriteRenderSystem : public BaseSystem {
 public:
  SpriteRenderSystem(rse::Renderer2D* renderer) : BaseSystem(), m_renderer(renderer) {}

  void Update(rse::Scene& scene, const float alpha) override {
    // The simulation steps whole sixtieths of a second; this is how far the
    // frame is into the next one.
    const float interp = std::clamp(alpha, 0.0f, 1.0f);

    scene.Registry().view<TransformComponent, SpriteComponent>().each(
        [&, this](auto entity, TransformComponent& transform, SpriteComponent& sprite) {
          if (!scene.IsEntityEnabled(entity) || !scene.IsComponentEnabled<SpriteComponent>(entity)) {
            return;
          }

          const int layer =
              scene.Registry().has<BasicComponent>(entity) ? scene.Registry().get<BasicComponent>(entity).m_layer : 0;

          rse::SpriteAppearance appearance;

          if (scene.Registry().has<SpriteAppearanceComponent>(entity)) {
            SpriteAppearanceComponent& component = scene.Registry().get<SpriteAppearanceComponent>(entity);
            appearance.flip_x = component.flip_x;
            appearance.flip_y = component.flip_y;

            if (!component.material.empty()) {
              // Cached by the manager, so this is a lookup rather than a load
              // after the first frame.
              appearance.material = rse::MaterialManager::GetOrLoad(component.material);
            }
          }

          // A body's transform is where the last step left it, so it is drawn
          // between the last two steps: at 60 Hz on a faster display, snapping
          // to the newest step is the stutter interpolation exists to remove.
          // Everything else -- anything without a body -- is drawn where it is.
          TransformComponent draw = transform;

          if (scene.Registry().has<RigidBodyComponent>(entity)) {
            const std::shared_ptr<Rigidbody>& body = scene.Registry().get<RigidBodyComponent>(entity).m_rigidbody;

            if (body != nullptr && body->m_init) {
              draw.position =
                  glm::vec3(glm::mix(body->GetPreviousPosition(), body->GetCurrentPosition(), interp), 0.0f);
              draw.rotation.z = glm::mix(body->GetPreviousRotation(), body->GetCurrentRotation(), interp);
            }
          }

          m_renderer->Submit(draw, sprite.m_sprite.get(), layer, appearance);
        });
  }

 private:
  rse::Renderer2D* m_renderer;
};
