#include "Framebuffer.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "../Utils/Logger.hpp"

namespace rse {
Framebuffer::~Framebuffer() { Destroy(); }

void Framebuffer::Destroy() {
  if (m_color_texture != 0) {
    glDeleteTextures(1, &m_color_texture);
    m_color_texture = 0;
  }
  if (m_depth_buffer != 0) {
    glDeleteRenderbuffers(1, &m_depth_buffer);
    m_depth_buffer = 0;
  }
  if (m_framebuffer != 0) {
    glDeleteFramebuffers(1, &m_framebuffer);
    m_framebuffer = 0;
  }

  m_width = 0;
  m_height = 0;
}

void Framebuffer::Resize(int width, int height) {
  // A docked panel can be dragged to zero width; a zero sized attachment
  // is a GL error, not something worth showing.
  width = std::max(width, 1);
  height = std::max(height, 1);

  if (m_framebuffer != 0 && width == m_width && height == m_height) {
    return;
  }

  Destroy();

  m_width = width;
  m_height = height;

#ifdef EMSCRIPTEN
  const GLint colorFormat = GL_RGBA;
  const GLenum depthFormat = GL_DEPTH_COMPONENT16;
#else
  const GLint color_format = GL_RGBA8;
  const GLenum depth_format = GL_DEPTH_COMPONENT24;
#endif

  glGenFramebuffers(1, &m_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

  glGenTextures(1, &m_color_texture);
  glBindTexture(GL_TEXTURE_2D, m_color_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, color_format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, 0);

  glGenRenderbuffers(1, &m_depth_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, m_depth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, depth_format, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_buffer);

  const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    std::ostringstream message;
    message << "Viewport framebuffer incomplete (0x" << std::hex << status << std::dec << ")";
    Logger::Error(message.str());
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
  glViewport(0, 0, m_width, m_height);
}

void Framebuffer::Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
}  // namespace rse
