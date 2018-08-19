#pragma once

#include <glm\glm.hpp>
#include "../../Components/Sprite.h"

struct SpriteComponent
{
	SpriteComponent() { };

	SpriteComponent(Sprite* sprite) : m_Sprite(sprite)
	{
	};

	Sprite* m_Sprite;
};