#include "Sprite.h"

Sprite::Sprite(glm::vec3 position, glm::vec2 size, unsigned int color)
	: Renderable2D(position, size, color)
{
	texr.x = m_Position.x;
	texr.y = m_Position.y;
	texr.w = m_Size.x;
	texr.h = m_Size.y;
}

Sprite::Sprite(glm::vec3 position, glm::vec2 size, Texture* texture)
	: Renderable2D(position, size, 0xffffffff)
{
	m_Texture = texture;
}