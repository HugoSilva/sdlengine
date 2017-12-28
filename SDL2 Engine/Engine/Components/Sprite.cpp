#include "Sprite.h"

using namespace std;

Sprite::Sprite(glm::vec3 position, glm::vec2 size, glm::vec4 color)
	: m_Position(position), m_Size(size), m_Color(color)
{
	texr.x = m_Position.x;
	texr.y = m_Position.y;
	texr.w = m_Size.x;
	texr.h = m_Size.y;
}

Sprite::Sprite(glm::vec3 position, glm::vec2 size, Texture* texture)
	: m_Position(position), m_Size(size), m_Color(glm::vec4(1))
{
	m_Texture = texture;
}

void Sprite::Update()
{
}

void Sprite::Render(SDL_Renderer* renderer)
{
	//SDL_RenderCopy(renderer, NULL, NULL, &texr);
	
	SDL_SetRenderDrawColor(renderer, m_Color.r, m_Color.g, m_Color.b, m_Color.a);
	SDL_RenderFillRect(renderer, &texr);
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