#include "Component.hpp"
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
}