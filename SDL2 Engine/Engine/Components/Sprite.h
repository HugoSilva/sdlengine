#ifndef SPRITE
#define SPRITE

#include <glm/glm.hpp>
#include <SDL.h>
#include "../Graphics/Renderable2D.h"
#include "../Graphics/Renderer2D.h"

class Sprite : public graphics::Renderable2D
{
public:
	Sprite(glm::vec3 position, glm::vec2 size, glm::vec4 color);
	Sprite(glm::vec3 position, glm::vec2 size, Texture* texture);

	void Update();
	
	void submit(graphics::Renderer2D* renderer) {};

private:
	Texture* m_Texture;
};

#endif