#pragma once

#include <SDL_image.h>
#include <SDL_ttf.h>
#ifdef EMSCRIPTEN
	#include <SDL_opengles2.h>
#else
	#include <GL/glew.h>
#endif // EMSCRIPTEN
#include "Logger.h"

class Image
{
public:
	static void* LoadFont(TTF_Font* font, const char* text, SDL_Color textColor, int* width, int* height, int* mode)
	{
		SDL_Surface* surface = TTF_RenderText_Blended(font, text, textColor);

		if (surface == NULL)
		{
			Logger::error("Cannot create font text: " + (std::string)text);
			return nullptr;
		}

		unsigned int bpp = surface->format->BytesPerPixel;

		if (bpp == 4) {
			if (surface->format->Rmask == 0x000000ff)
				*mode = GL_RGBA;
			else
				*mode = GL_BGRA;
		}
		else {
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

	static void* LoadImage(const char* filename, int* width, int* height, int* mode)
	{
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
};