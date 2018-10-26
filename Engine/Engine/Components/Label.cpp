#include "Label.h"

#include <memory>
#include "Texture.h"

Label::Label(std::string text, glm::vec3 position, graphics::Font* font, unsigned int color)
	: Renderable2D(position, glm::vec2(0), color), text(text), m_Font(font)
{
	m_Texture = std::make_shared<Texture>(font->getName()+text, font, text, color);
	m_Size = glm::vec2(m_Texture.get()->getWidth(), m_Texture.get()->getHeight());
}

void Label::submit(graphics::Renderer2D* renderer) const
{
	//TODO Fix label support
	//renderer->submit(static_cast<glm::vec3>(this->getPosition()), this);
}