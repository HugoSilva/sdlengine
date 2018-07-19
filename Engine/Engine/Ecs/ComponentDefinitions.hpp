#pragma once

#include "Component.hpp"
#include "System.hpp"
#include "../Components/Texture.h"
#include "../Audio/Sound.h"
#include "../Graphics/Renderer2D.h"
#include "../Components/Sprite.h"

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
		Texture* Sprite{ nullptr };
		unsigned int Color{ 0x00 };
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
		SpriteRenderSystem(graphics::Renderer2D& renderer) : SystemBase(), m_renderer(renderer)
		{
			addComponentType(PositionComponent::ID);
			addComponentType(SpriteComponent::ID);
		}

		virtual void updateComponents(float delta, ComponentBase** components)
		{
			PositionComponent* transform = (PositionComponent*)components[0];
			SpriteComponent* sprite = (SpriteComponent*)components[1];

			//TODO Render logic needs to be held where instead of the layer class
			if (sprite->Sprite != nullptr)
			{
				m_renderer.submit(new Sprite(glm::vec3(transform->x, transform->y, transform->z), glm::vec2(40, 40), sprite->Sprite));
			}
			else
			{
				m_renderer.submit(new Sprite(glm::vec3(transform->x, transform->y, transform->z), glm::vec2(40, 40), sprite->Color));
			}
		}
	private:
		graphics::Renderer2D& m_renderer;
	};
}