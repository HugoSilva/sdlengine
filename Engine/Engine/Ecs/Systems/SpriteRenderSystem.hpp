#pragma once

#include "BaseSystem.hpp"
#include "../ECSManager.hpp"
#include "../../Graphics/Renderer2D.h"

class SpriteRenderSystem : public BaseSystem
{
public:
	SpriteRenderSystem(graphics::Renderer2D* renderer) : BaseSystem(renderer) {}

    void update(const float alpha)
    {
		ecs::ECSManager::getView<TransformComponent, SpriteComponent>().each([&, this](auto entity, 
			TransformComponent &transform, SpriteComponent &sprite)
		{
			m_renderer->submit(transform, sprite.m_Sprite);
		});
    }
};