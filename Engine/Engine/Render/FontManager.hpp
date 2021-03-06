#pragma once

#include <vector>

namespace rse
{
	class Font; //forward declaration for Font

	class FontManager
	{
	public:
		static void init();
		static void add(Font* font);
		static Font* get();
		static Font* get(const std::string& name);
		static Font* get(const std::string& name, unsigned int size);
		static void clean();

	private:
		FontManager() { }
		static std::vector<Font*> m_Fonts;
	};
}