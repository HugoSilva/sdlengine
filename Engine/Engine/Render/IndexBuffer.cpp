#include "IndexBuffer.hpp"

#ifdef EMSCRIPTEN
#include <SDL3/SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif  // EMSCRIPTEN

namespace rse {
IndexBuffer::IndexBuffer(unsigned short* data, int count) : m_count(count) {
  glGenBuffers(1, &m_buffer_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned short), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IndexBuffer::IndexBuffer(unsigned int* data, int count) : m_count(count) {
  glGenBuffers(1, &m_buffer_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer_id);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IndexBuffer::~IndexBuffer() { glDeleteBuffers(1, &m_buffer_id); }

void IndexBuffer::Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffer_id); }

void IndexBuffer::Unbind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
}  // namespace rse