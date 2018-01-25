#include "Label.h"

Label::Label(std::string text, glm::vec3 position, graphics::Font* font, unsigned int color)
	: Renderable2D(position, glm::vec2(0), color), text(text), m_Font(font)
{
	m_Texture = new Texture(font->getName()+text, font, text, color);
	m_Size = glm::vec2(m_Texture->getWidth(), m_Texture->getHeight());
}

void Label::submit(graphics::Renderer2D* renderer) const
{
	renderer->submit(this);
}