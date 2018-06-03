#include "Font.h"

namespace graphics
{
	Font::Font(std::string name, std::string filename, int size)
		: m_Name(name), m_Filename(filename), m_Size(size)
	{
		m_Font = TTF_OpenFont(filename.c_str(), size);
	}

	Font::~Font()
	{
		TTF_CloseFont(m_Font);
	}
}