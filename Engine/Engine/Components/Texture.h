#pragma once

#include <string>
#ifdef EMSCRIPTEN
	#include <SDL_opengles2.h>
#else
	#include <GL/glew.h>
#endif // EMSCRIPTEN
#include "../Graphics/Font.h"
#include "../Utils/Image.h"

class Texture
{
public:
	Texture() {};
	Texture(const std::string& name, const std::string& filename);
	Texture(const std::string& name, graphics::Font* font, const std::string text, unsigned int color);
	~Texture();

	void bind() const;
	void unbind() const;

	inline const unsigned int getID() const { return m_Id; }
	inline const std::string& getName() const { return m_Name; }
	inline const unsigned int getWidth() const { return m_Width; }
	inline const unsigned int getHeight() const { return m_Height; }

	template<typename Archive>
	void serialize(Archive &archive)
	{
		archive(cereal::make_nvp("Id", m_Id), cereal::make_nvp("Name", m_Name), cereal::make_nvp("Filename", m_Filename),
			cereal::make_nvp("Width", m_Width), cereal::make_nvp("Height", m_Height));
	}

private:
	unsigned int m_Id;
	std::string m_Name;
	std::string m_Filename;
	int m_Width, m_Height;
};