#pragma once

#include "../ECSManager.hpp"
#include "../Components/IncludeList.hpp"

class RenderSystem
{
public:
    RenderSystem() {}

    void update(const float alpha)
    {
        ecs::ECSManager::getView<TransformComponent>().each([&](auto entity, TransformComponent &renderable)
        {
            renderable->submit(m_Renderer);
        });
    }
};