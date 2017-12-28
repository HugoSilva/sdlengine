#ifndef SPRITE
#define SPRITE

#include <glm/glm.hpp>
#include <SDL.h>
#include "Texture.h"

class Sprite
{
public:
	Sprite(glm::vec3 position, glm::vec2 size, glm::vec4 color);
	Sprite(glm::vec3 position, glm::vec2 size, Texture* texture);

	void Update();
	void Render(SDL_Renderer* renderer);

	void MoveTo(glm::vec3 v);

	glm::vec3* GetPosition();
	glm::vec2* GetSize();

private:
	glm::vec3 m_Position;
	glm::vec2 m_Size;
	glm::vec4 m_Color;
	Texture* m_Texture;
	SDL_Rect texr;
};

#endif