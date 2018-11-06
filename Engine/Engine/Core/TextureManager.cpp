#include "TextureManager.hpp"

#include <SDL_image.h>

#include "../Components/Texture.hpp"
#include "../Utils/Logger.h"

std::vector<Texture*> TextureManager::s_Textures;

void TextureManager::init()
{
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		std::string imageError("error: %s\n", IMG_GetError());
		Logger::error("SDL_mixer could not initialize, " + imageError);
	}
}

void TextureManager::add(Texture* texture)
{
	s_Textures.push_back(texture);
}

Texture* TextureManager::get(const std::string& name)
{
	for (Texture* texture : s_Textures)
	{
		if (texture->getName() == name)
			return texture;
	}
	return nullptr;
}

void TextureManager::clean()
{
	for (unsigned int i = 0; i < s_Textures.size(); i++)
		delete s_Textures[i];

	IMG_Quit();
}