#pragma once

#include <vector>

#include "Texture.h"

class TextureManager
{
public:
	static void init();
	static void add(Texture* texture);
	static Texture* get(const std::string& name);
	static void clean();

private:
	TextureManager();
	static std::vector<Texture*> m_Textures;
};

