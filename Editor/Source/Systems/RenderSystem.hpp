#pragma once

#include <Core.h>
#include "../Components/MenuBarComponent.hpp"
#include "../Components/EntityViewerComponent.hpp"

class RenderSystem : public BaseSystem
{
public:
    RenderSystem(graphics::Renderer2D* renderer) : BaseSystem(renderer) {}

    void update(const float alpha)
    {
		ecs::ECSManager::getView<MenuBarComponent>().each([&](auto entity, MenuBarComponent &renderable)
        {
			renderable.submit(m_renderer);
        });
		ecs::ECSManager::getView<EntityViewerComponent>().each([&](auto entity, EntityViewerComponent &viewer)
		{
			viewer.submit(m_renderer);
		});
    }
};