#pragma once

#include "BaseSystem.hpp"
#include "../ECSManager.hpp"

class AudioSystem : public BaseSystem
{
public:
	AudioSystem() : BaseSystem() {}

	void update(const float alpha)
	{
		ecs::ECSManager::getView<TransformComponent, AudioComponent>().each([&, this](auto entity,
			TransformComponent &transform, AudioComponent &audio)
		{
			//TODO Remove the logic from the old Audio object and do it here
			//transform, audio
		});
	}
};