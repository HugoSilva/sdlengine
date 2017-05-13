#ifndef TEXTURE
#define TEXTURE

#include "../../Libs/SDL2/include/SDL.h"
#include "../../Libs/SDL2/include/SDL_opengl.h"
#include "../../Libs/SDL2_image/include/SDL_image.h"
#include "../Engine.h"
#include <iostream>
#include <string>
using namespace std;

class Texture
{
public:
	Texture();
	Texture(int _id);
	Texture(string path);
	~Texture();

	int GetID();
	int GetWidth();
	int GetHeight();
	SDL_Surface* GetSurface();
	SDL_Texture* GetTexture();

private:
	SDL_Surface* surface;
	SDL_Texture* texture;
	int id;
	int width;
	int height;
};

#endif