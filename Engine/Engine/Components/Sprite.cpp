#include "Sprite.hpp"

namespace rse
{
	Sprite::Sprite() : Renderable2D(glm::vec3(0.0f), glm::vec2(1.0f), 0), position(m_Position)
	{
	}

	Sprite::Sprite(glm::vec3 position, glm::vec2 size, unsigned int color)
		: Renderable2D(position, size, color), position(m_Position)
	{
		texr.x = static_cast<int>(round(m_Position.x));
		texr.y = static_cast<int>(round(m_Position.y));
		texr.w = static_cast<int>(round(m_Size.x));
		texr.h = static_cast<int>(round(m_Size.y));
	}

	Sprite::Sprite(glm::vec3 position, glm::vec2 size, Texture* texture)
		: Renderable2D(position, size, 0xffffffff), position(m_Position)
	{
		m_Texture.reset(texture);
	}
}