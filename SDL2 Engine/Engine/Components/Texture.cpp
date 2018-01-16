#include "Texture.h"

void* LoadImage(const char* filename, int* width, int* height, int* mode)
{
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		std::string imageError("SDL_image Error : %s\n", IMG_GetError());
		Logger::error("SDL_image could not initialize. " + imageError);

		return nullptr;
	}

	SDL_Surface* surface = IMG_Load(filename);

	if (surface == NULL)
	{
		Logger::error("Cannot load texture from file: " + (std::string)filename);
		return nullptr;
	}

	unsigned int bpp = surface->format->BytesPerPixel;

	if (bpp == 4)
	{
		if (surface->format->Rmask == 0x000000ff)
			*mode = GL_RGBA;
		else
			*mode = GL_BGRA;
	}
	else if (bpp == 3)
	{
		if (surface->format->Rmask == 0x000000ff)
			*mode = GL_RGB;
		else
			*mode = GL_BGR;
	}

	*width = surface->w;
	*height = surface->h;

	SDL_FreeSurface(surface);

	return surface->pixels;
}

Texture::Texture(std::string pathname)
	:m_Filename(pathname)
{
	int Mode = GL_RGB;
	void* pixels = LoadImage(m_Filename.c_str(), &m_Width, &m_Height, &Mode);

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
