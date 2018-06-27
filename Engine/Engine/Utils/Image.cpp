#include "Image.h"

void Image::CheckImageMode(int* mode, SDL_Surface* surface)
{
	unsigned int bpp = surface->format->BytesPerPixel;

	if (bpp == 4) {
		if (surface->format->Rmask == 0x000000ff)
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
		if (surface->format->Rmask == 0x000000ff)
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

void* Image::LoadFont(TTF_Font* font, const char* text, SDL_Color textColor, int* width, int* height, int* mode)
{
	SDL_Surface* surface = TTF_RenderText_Blended(font, text, textColor);

	if (surface == NULL)
	{
		Logger::error("Cannot create font text: " + (std::string)text);
		return nullptr;
	}

	CheckImageMode(mode, surface);

	*width = surface->w;
	*height = surface->h;

	SDL_FreeSurface(surface);

	return surface->pixels;
}

void* Image::LoadImage(const char* filename, int* width, int* height, int* mode)
{
	SDL_Surface* surface = IMG_Load(filename);

	if (surface == NULL)
	{
		Logger::error("Cannot load texture from file: " + (std::string)filename);
		return nullptr;
	}

	CheckImageMode(mode, surface);

	*width = surface->w;
	*height = surface->h;

	SDL_FreeSurface(surface);

	return surface->pixels;
}