#include "Label.h"

Label::Label(std::string text, glm::vec3 position, graphics::Font* font, unsigned int color)
	: Renderable2D(position, glm::vec2(117, 28), color), text(text), m_Font(font)
{
	m_Texture = new Texture(font, text, color);
}

void Label::submit(graphics::Renderer2D* renderer) const
{
	renderer->drawString(this);
}