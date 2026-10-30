#pragma once

#include "base_system.h"
#include <core/material_manager.h>
#include <ecs/components/basic.h>
#include <ecs/components/sprite_appearance.h>
#include <ecs/ecs_manager.h>
#include <render/renderer_2d.h>

class SpriteRenderSystem : public BaseSystem {
 public:
  SpriteRenderSystem(rse::Renderer2D* renderer) : BaseSystem(), m_renderer(renderer) {}

  void Update(const float alpha) {
    rse::ECSManager::GetView<TransformComponent, SpriteComponent>().each(
        [&, this](auto entity, TransformComponent& transform, SpriteComponent& sprite) {
          if (!rse::ECSManager::IsEntityEnabled(entity) ||
              !rse::ECSManager::IsComponentEnabled<SpriteComponent>(entity)) {
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

          m_renderer->Submit(transform, sprite.m_sprite.get(), layer, appearance);
        });
  }

 private:
  rse::Renderer2D* m_renderer;
};
