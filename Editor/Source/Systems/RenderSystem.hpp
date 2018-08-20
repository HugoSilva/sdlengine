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
		ecs::ECSManager::getView<DebugComponent>().each([&](auto entity, DebugComponent &renderable)
        {
			renderable.render();
        });
    }
};