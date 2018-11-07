#pragma once

#include "BaseSystem.hpp"
#include "../ECSManager.hpp"
#include "../../Render/Renderer2D.hpp"

class SpriteRenderSystem : public BaseSystem
{
public:
	SpriteRenderSystem(graphics::Renderer2D* renderer) : BaseSystem(), m_renderer(renderer) {}

    void update(const float alpha)
    {
		ecs::ECSManager::getView<TransformComponent, SpriteComponent>().each([&, this](auto entity, 
			TransformComponent &transform, SpriteComponent &sprite)
		{
			m_renderer->submit(transform, sprite.m_Sprite.get());
		});
    }

	graphics::Renderer2D* m_renderer;
};