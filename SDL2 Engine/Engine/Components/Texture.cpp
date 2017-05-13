#include "Texture.h"

Texture::Texture()
{
	id = -1;
}

Texture::Texture(int _id)
{
	id = _id;
	if (id < 0)
	{
		cout << "Error loading image with ID: " << id << endl;
	}
}

Texture::Texture(string path)
{
	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
	{
		std::cout << "Could not create window: " << IMG_GetError() << std::endl;
		return;
	}

	surface = IMG_Load(path.c_str());
	texture = SDL_CreateTextureFromSurface(Engine::GetRenderer(), surface);

	if (surface == nullptr)
	{
		cout << "Error loading image: " << path << endl;
	}
}

Texture::~Texture()
{
	//SDL_DestroyTexture(texture);
	//SDL_FreeSurface(surface);
}

int Texture::GetID()
{
	return id;
}

int Texture::GetWidth()
{
	return width;
}

int Texture::GetHeight()
{
	return height;
}


SDL_Surface* Texture::GetSurface()
{
	return surface;
}

SDL_Texture* Texture::GetTexture()
{
	return texture;
}