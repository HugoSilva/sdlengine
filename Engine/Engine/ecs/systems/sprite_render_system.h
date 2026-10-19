#pragma once

#include "base_system.h"
#include <ecs/ecs_manager.h>
#include <render/renderer_2d.h>

class SpriteRenderSystem : public BaseSystem {
 public:
  SpriteRenderSystem(rse::Renderer2D* renderer) : BaseSystem(), m_renderer(renderer) {}

  void Update(const float alpha) {
    rse::ECSManager::GetView<TransformComponent, SpriteComponent>().each(
        [&, this](auto entity, TransformComponent& transform, SpriteComponent& sprite) {
          m_renderer->Submit(transform, sprite.m_sprite.get());
        });
  }

 private:
  rse::Renderer2D* m_renderer;
};