#pragma once

#include <string>
#include <GL/glew.h>
#include "../Utils/Image.h"

class Texture
{
public:
	Texture(const std::string pathname);
	~Texture();

	void bind() const;
	void unbind() const;

	inline const unsigned int getID() const { return m_Id; }
	inline const unsigned int getWidth() const { return m_Width; }
	inline const unsigned int getHeight() const { return m_Height; }

private:
	unsigned int m_Id;
	std::string m_Filename;
	int m_Width, m_Height;
};