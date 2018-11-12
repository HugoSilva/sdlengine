#pragma once

#include <vector>

namespace rse
{
	class Texture; //forward declaration for Texture

	class TextureManager
	{
	public:
		static void init();
		static void add(Texture* texture);
		static Texture* get(const std::string& name);
		static void clean();

	private:
		TextureManager();
		static std::vector<Texture*> s_Textures;
	};
}