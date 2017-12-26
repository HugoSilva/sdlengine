#include "Sprite.h"

using namespace std;

Sprite::Sprite(glm::vec3 position, glm::vec2 size, glm::vec4 color)
	: m_Position(position), m_Size(size), m_Color(color)
{
}

Sprite::Sprite(glm::vec3 position, glm::vec2 size, Texture* texture)
	: m_Position(position), m_Size(size), m_Color(glm::vec4(1))
{
	m_Texture = texture;
}

void Sprite::Update()
{
}

void Sprite::Render()
{
}

void Sprite::MoveTo(glm::vec3 v)
{
	m_Position = v;
}

glm::vec3* Sprite::GetPosition()
{
	return &m_Position;
}

glm::vec2* Sprite::GetSize()
{
	return &m_Size;
}