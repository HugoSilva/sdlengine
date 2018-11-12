#pragma once

#include <SDL_image.h>
#include <SDL_ttf.h>

class Image
{
public:
	static void checkImageMode(int* mode, SDL_Surface* surface);

	static void* loadFont(TTF_Font* font, const char* text, SDL_Color textColor, int* width, int* height, int* mode);

	static void* loadImage(const char* filename, int* width, int* height, int* mode);
};