#pragma once

#include <glm\glm.hpp>
#include "../Graphics/Renderable2D.h"
#include "../Graphics/FontManager.h"

class Label : public graphics::Renderable2D
{
public:
	Label(std::string text, glm::vec3 position, graphics::Font* font, unsigned int color);
	void submit(graphics::Renderer2D* renderer) const override;

private:
	unsigned int m_Id;
	int m_Width, m_Height;
	graphics::Font* m_Font;
	std::string text;
};