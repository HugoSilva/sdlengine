#pragma once

#include <glm/glm.hpp>
#include "../Graphics/Renderable2D.h"
#include "../Graphics/Renderer2D.h"

class Sprite : public graphics::Renderable2D
{
public:
	Sprite();
	Sprite(glm::vec3 position, glm::vec2 size, unsigned int color);
	Sprite(glm::vec3 position, glm::vec2 size, Texture* texture);
	
	void submit(graphics::Renderer2D* renderer) {};
	glm::vec3& position;
};