#pragma once

#include <string>
#include <glm/glm.hpp>
#include <SDL2_ttf\SDL_ttf.h>

namespace graphics {

	class Font
	{
	public:
		Font(std::string name, std::string filename, int size);
		~Font();

		inline TTF_Font* getFTFont() const { return m_Font; }
		inline const std::string& getName() const { return m_Name; }
		inline const std::string& getFileName() const { return m_Filename; }
		inline const int getSize() const { return m_Size; }

	private:
		TTF_Font* m_Font;
		unsigned int m_Size;
		std::string m_Name;
		std::string m_Filename;
	};
}