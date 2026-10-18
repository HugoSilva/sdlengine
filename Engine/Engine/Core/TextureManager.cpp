#include "TextureManager.hpp"

#include "../Components/Texture.hpp"
#include "../Utils/Logger.hpp"

namespace rse
{
	std::vector<Texture*> TextureManager::s_Textures;

	void TextureManager::init()
	{
		// SDL3_image no longer needs IMG_Init()/IMG_Quit(); formats boot
		// themselves on first use, so there is nothing to set up here.
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
	}
}
