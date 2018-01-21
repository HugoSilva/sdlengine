#include "Texture.h"

Texture::Texture(std::string pathname)
	:m_Filename(pathname)
{
	int Mode = GL_RGB;
	void* pixels = Image::LoadImage(m_Filename.c_str(), &m_Width, &m_Height, &Mode);

	unsigned int result;
	glGenTextures(1, &result);
	glBindTexture(GL_TEXTURE_2D, result);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, Mode, m_Width, m_Height, 0, Mode, GL_UNSIGNED_BYTE, pixels);

	glBindTexture(GL_TEXTURE_2D, 0);

	m_Id = result;
}

Texture::~Texture()
{

}

void Texture::bind() const
{
	glBindTexture(GL_TEXTURE_2D, m_Id);
}

void Texture::unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
