#include "Framebuffer.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "../Utils/Logger.hpp"

namespace rse
{
	Framebuffer::~Framebuffer()
	{
		destroy();
	}

	void Framebuffer::destroy()
	{
		if (m_ColorTexture != 0)
		{
			glDeleteTextures(1, &m_ColorTexture);
			m_ColorTexture = 0;
		}
		if (m_DepthBuffer != 0)
		{
			glDeleteRenderbuffers(1, &m_DepthBuffer);
			m_DepthBuffer = 0;
		}
		if (m_Framebuffer != 0)
		{
			glDeleteFramebuffers(1, &m_Framebuffer);
			m_Framebuffer = 0;
		}

		m_Width = 0;
		m_Height = 0;
	}

	void Framebuffer::resize(int width, int height)
	{
		// A docked panel can be dragged to zero width; a zero sized attachment
		// is a GL error, not something worth showing.
		width = std::max(width, 1);
		height = std::max(height, 1);

		if (m_Framebuffer != 0 && width == m_Width && height == m_Height)
		{
			return;
		}

		destroy();

		m_Width = width;
		m_Height = height;

	#ifdef EMSCRIPTEN
		const GLint colorFormat = GL_RGBA;
		const GLenum depthFormat = GL_DEPTH_COMPONENT16;
	#else
		const GLint colorFormat = GL_RGBA8;
		const GLenum depthFormat = GL_DEPTH_COMPONENT24;
	#endif

		glGenFramebuffers(1, &m_Framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

		glGenTextures(1, &m_ColorTexture);
		glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTexture, 0);

		glGenRenderbuffers(1, &m_DepthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, depthFormat, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBuffer);

		const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::ostringstream message;
			message << "Viewport framebuffer incomplete (0x" << std::hex << status << std::dec << ")";
			Logger::error(message.str());
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);
		glViewport(0, 0, m_Width, m_Height);
	}

	void Framebuffer::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}
