#include "Texture.hpp"

#ifdef EMSCRIPTEN
#include <SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif // EMSCRIPTEN

#include "../Render/Font.hpp"
#include "../Utils/Image.hpp"

Texture::Texture(const std::string& name, const std::string& filename)
	: m_Name(name), m_Filename(filename)
{
	int Mode = GL_RGB;
	void* pixels = Image::loadImage(m_Filename.c_str(), &m_Width, &m_Height, &Mode);

	unsigned int result;
	glGenTextures(1, &result);
	glBindTexture(GL_TEXTURE_2D, result);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, Mode, m_Width, m_Height, 0, Mode, GL_UNSIGNED_BYTE, pixels);

	glBindTexture(GL_TEXTURE_2D, 0);

	m_Id = result;
}

Texture::Texture(const std::string& name, graphics::Font* font, const std::string text, unsigned int color)
	: m_Name(name)
{
	SDL_Color textColor = { static_cast<unsigned char>(color), static_cast<unsigned char>(color>>8), static_cast<unsigned char>(color>>16), static_cast<unsigned char>(color>>24) };
	int Mode = GL_RGB;
	void* pixels = Image::loadFont(font->getFTFont(), text.c_str(), textColor, &m_Width, &m_Height, &Mode);

	unsigned int result;
	glGenTextures(1, &result);
	glBindTexture(GL_TEXTURE_2D, result);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, Mode, GL_UNSIGNED_BYTE, pixels);

	glBindTexture(GL_TEXTURE_2D, 0);

	m_Id = result;
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_Id);
}

void Texture::bind() const
{
	glBindTexture(GL_TEXTURE_2D, m_Id);
}

void Texture::unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
