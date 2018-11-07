#include "FontManager.hpp"

#include "Font.hpp"
#include "../Utils/Logger.hpp"

namespace graphics
{
	std::vector<Font*> FontManager::m_Fonts;
	
	void FontManager::init()
	{
		if (TTF_Init() == -1)
		{
			std::string fontError("error: %s\n", SDL_GetError());
			Logger::error("SDL_ttf could not initialize, " + fontError);
		}
	}

	void FontManager::add(Font* font)
	{
		m_Fonts.push_back(font);
	}

	Font* FontManager::get()
	{
		return m_Fonts[0];
	}

	Font* FontManager::get(const std::string& name)
	{
		for (Font* font : m_Fonts)
		{
			if (font->getName() == name)
				return font;
		}
		// TODO: Maybe return a default font instead?
		return nullptr;
	}

	Font* FontManager::get(const std::string& name, unsigned int size)
	{
		for (Font* font : m_Fonts)
		{
			if (font->getSize() == size && font->getName() == name)
				return font;
		}
		// TODO: Maybe return a default font instead?
		return nullptr;
	}

	void FontManager::clean()
	{
		for (unsigned int i = 0; i < m_Fonts.size(); i++)
			delete m_Fonts[i];

		TTF_Quit();
	}
}