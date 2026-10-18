#include "Image.hpp"

#ifdef EMSCRIPTEN
#include <SDL3/SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif // EMSCRIPTEN

#include <cstring>

#include "Logger.hpp"

void Image::checkImageMode(int* mode, SDL_Surface* surface)
{
	// SDL3 replaced the SDL_PixelFormat struct on the surface with an enum plus
	// an SDL_PixelFormatDetails lookup.
	const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(surface->format);
	unsigned int bpp = details->bytes_per_pixel;

	if (bpp == 4) {
		if (details->Rmask == 0x000000ff)
		{
			*mode = GL_RGBA;
		}
		else
		{
			#ifdef EMSCRIPTEN
				*mode = GL_RGBA;
			#else
				*mode = GL_BGRA;
			#endif // EMSCRIPTEN
		}
	}
	else 
	{
		if (details->Rmask == 0x000000ff)
		{
			*mode = GL_RGB;
		}
		else
		{
			#ifdef EMSCRIPTEN
				*mode = GL_RGB;
			#else
				*mode = GL_BGR;
			#endif // EMSCRIPTEN
		}
	}
}

SDL_Surface* Image::loadFont(TTF_Font* font, const char* text, SDL_Color textColor)
{
	// SDL3_ttf takes an explicit length (0 == NUL-terminated) and the colour by value.
	SDL_Surface* surface = TTF_RenderText_Blended(font, text, std::strlen(text), textColor);

	if (surface == NULL)
	{
		Logger::error("Cannot create font text: " + std::string(text));
		return nullptr;
	}

	return surface;
}

SDL_Surface* Image::loadImage(const char* filename)
{
	SDL_Surface* surface = IMG_Load(filename);

	if (surface == NULL)
	{
		Logger::error("Cannot load texture from file: " + std::string(filename));
		return nullptr;
	}

	return surface;
}
