#pragma once

#include "BaseSystem.hpp"
#include "../ECSManager.hpp"
#include "../../Render/Renderer2D.hpp"

class SpriteRenderSystem : public BaseSystem
{
public:
	SpriteRenderSystem(rse::Renderer2D* renderer) : BaseSystem(), m_renderer(renderer) {}

    void update(const float alpha)
    {
		rse::ECSManager::getView<TransformComponent, SpriteComponent>().each([&, this](auto entity, 
			TransformComponent &transform, SpriteComponent &sprite)
		{
			m_renderer->submit(transform, sprite.m_Sprite.get());
		});
		}

private:
	rse::Renderer2D* m_renderer;
};