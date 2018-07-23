#pragma once

#include "Component.hpp"
#include "System.hpp"
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
		
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar(x, y, z);
		}
	};

	struct SpriteComponent : public Component<SpriteComponent>
	{
		Sprite* Sprite{ nullptr };
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
			m_renderer.submit(sprite->Sprite);
		}
	private:
		graphics::Renderer2D& m_renderer;
	};
}