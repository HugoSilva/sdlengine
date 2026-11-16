#pragma once

#include <algorithm>
#include <memory>

#include "base_system.h"
#include <core/material_manager.h>
#include <ecs/components/basic.h>
#include <ecs/components/rigid_body.h>
#include <ecs/components/sprite_appearance.h>
#include <ecs/ecs_manager.h>
#include <render/renderer_2d.h>

class SpriteRenderSystem : public BaseSystem {
 public:
  SpriteRenderSystem(rse::Renderer2D* renderer) : BaseSystem(), m_renderer(renderer) {}

  void Update(const float alpha) override {
    // The simulation steps whole sixtieths of a second; this is how far the
    // frame is into the next one.
    const float interp = std::clamp(alpha, 0.0f, 1.0f);

    rse::ECSManager::GetView<TransformComponent, SpriteComponent>().each([&, this](auto entity,
                                                                                   TransformComponent& transform,
                                                                                   SpriteComponent& sprite) {
      if (!rse::ECSManager::IsEntityEnabled(entity) || !rse::ECSManager::IsComponentEnabled<SpriteComponent>(entity)) {
        return;
      }

      const int layer = rse::ECSManager::HasComponent<BasicComponent>(entity)
                            ? rse::ECSManager::GetComponent<BasicComponent>(entity).m_layer
                            : 0;

      rse::SpriteAppearance appearance;

      if (rse::ECSManager::HasComponent<SpriteAppearanceComponent>(entity)) {
        SpriteAppearanceComponent& component = rse::ECSManager::GetComponent<SpriteAppearanceComponent>(entity);
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

      if (rse::ECSManager::HasComponent<RigidBodyComponent>(entity)) {
        const std::shared_ptr<Rigidbody>& body = rse::ECSManager::GetComponent<RigidBodyComponent>(entity).m_rigidbody;

        if (body != nullptr && body->m_init) {
          draw.position = glm::vec3(glm::mix(body->GetPreviousPosition(), body->GetCurrentPosition(), interp), 0.0f);
        }
      }

      m_renderer->Submit(draw, sprite.m_sprite.get(), layer, appearance);
    });
  }

 private:
  rse::Renderer2D* m_renderer;
};
