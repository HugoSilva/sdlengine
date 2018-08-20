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
		ecs::ECSManager::getView<SpriteComponent>().each([&, this](auto entity, SpriteComponent &renderable)
		{
			m_renderer->submit(renderable.m_Sprite);
		});
    }
};