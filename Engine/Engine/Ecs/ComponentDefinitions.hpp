#pragma once

#include "Component.hpp"
#include "System.hpp"
#include "../Components/Texture.h"
#include "../Audio/Sound.h"

namespace ecs
{
	struct PositionComponent : public Component<PositionComponent>
	{
		float x;
		float y;
		float z;
	};

	struct SpriteComponent : public Component<SpriteComponent>
	{
		Texture* Sprite;
		unsigned int Color;
	};

	struct AudioComponent : public Component<AudioComponent>
	{
		audio::Sound* AudioClip;
	};

	struct RigidbodyComponent : public Component<RigidbodyComponent>
	{
		float Mass;
		float Drag;
	};

	struct BoxColliderComponent : public Component<BoxColliderComponent>
	{
		glm::vec3 Center;
		glm::vec3 Size;
	};

	class SpriteRenderSystem : public SystemBase
	{
	public:
		SpriteRenderSystem() : SystemBase()
		{
			addComponentType(PositionComponent::ID);
			addComponentType(SpriteComponent::ID);
		}

		virtual void updateComponents(float delta, ComponentBase** components)
		{
			PositionComponent * transform = (PositionComponent*)components[0];
			SpriteComponent * mesh = (SpriteComponent*)components[1];

			//TODO Render logic needs to be held where instead of the layer class
		}
	};
}