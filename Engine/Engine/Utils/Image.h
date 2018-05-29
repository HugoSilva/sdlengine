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
	static void CheckImageMode(int* mode, SDL_Surface* surface);

	static void* LoadFont(TTF_Font* font, const char* text, SDL_Color textColor, int* width, int* height, int* mode);

	static void* LoadImage(const char* filename, int* width, int* height, int* mode);
};