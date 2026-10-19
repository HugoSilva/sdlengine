#include "opengl_renderer.h"

#include <glm/gtc/matrix_transform.hpp>  // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>          // glm::value_ptr
#include <glm/mat4x4.hpp>                // glm::mat4

namespace rse {
OpenGLRenderer::OpenGLRenderer(SDL_Window* win, Shader* shader) : m_window(win), m_shader(shader) {
  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, RENDERER_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(SHADER_VERTEX_INDEX);
  glEnableVertexAttribArray(SHADER_UV_INDEX);
  glEnableVertexAttribArray(SHADER_TID_INDEX);
  glEnableVertexAttribArray(SHADER_COLOR_INDEX);

  glVertexAttribPointer(SHADER_VERTEX_INDEX, 3, GL_FLOAT, GL_FALSE, RENDERER_VERTEX_SIZE, (const GLvoid*)0);
  glVertexAttribPointer(SHADER_UV_INDEX, 2, GL_FLOAT, GL_FALSE, RENDERER_VERTEX_SIZE,
                        (const GLvoid*)(offsetof(VertexData, uv)));
  glVertexAttribPointer(SHADER_TID_INDEX, 1, GL_FLOAT, GL_FALSE, RENDERER_VERTEX_SIZE,
                        (const GLvoid*)(offsetof(VertexData, tid)));
  glVertexAttribPointer(SHADER_COLOR_INDEX, 4, GL_UNSIGNED_BYTE, GL_TRUE, RENDERER_VERTEX_SIZE,
                        (const GLvoid*)(offsetof(VertexData, color)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  unsigned int* indices = new unsigned int[RENDERER_INDICES_SIZE];

  int offset = 0;
  for (int i = 0; i < RENDERER_INDICES_SIZE; i += 6) {
    indices[i] = offset + 0;
    indices[i + 1] = offset + 1;
    indices[i + 2] = offset + 2;

    indices[i + 3] = offset + 2;
    indices[i + 4] = offset + 3;
    indices[i + 5] = offset + 0;

    offset += 4;
  }

  m_ibo = new IndexBuffer(indices, RENDERER_INDICES_SIZE);

  glBindVertexArray(0);

  m_clear_color = new test::TestClearColor();
#ifdef EMSCRIPTEN
  m_BufferBase = new VertexData[RENDERER_MAX_SPRITES * 4];
#endif  // EMSCRIPTEN
}

OpenGLRenderer::~OpenGLRenderer() {
  delete m_ibo;
  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
}

void OpenGLRenderer::Begin() {
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
#ifdef EMSCRIPTEN
  m_Buffer = m_BufferBase;
#else
  m_buffer = (VertexData*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
#endif  // EMSCRIPTEN

  m_clear_color->OnRender();
}

float* OpenGLRenderer::GetClearColor() { return m_clear_color->GetClearColor(); }

void OpenGLRenderer::Submit(TransformComponent& transform, const Renderable2D* renderable) {
  // const glm::vec3& position = renderable->getPosition();
  const glm::vec2& size = renderable->GetSize();
  const unsigned int& color = renderable->GetColor();
  const std::vector<glm::vec2>& uv = renderable->GetUv();
  const unsigned int tid = renderable->GetTId();

  float ts = 0.0f;
  if (tid > 0) {
    bool found = false;
    for (unsigned int i = 0; i < m_texture_slots.size(); i++) {
      if (m_texture_slots[i] == tid) {
        ts = (float)(i + 1);
        found = true;
        break;
      }
    }

    if (!found) {
      if (m_texture_slots.size() >= RENDERER_MAX_TEXTURES) {
        End();
        Flush();
        Begin();
      }
      m_texture_slots.push_back(tid);
      ts = (float)(m_texture_slots.size());
    }
  }

  // TODO Create a proper anchor concept for the sprites this hard codes center points
  glm::vec3 up_right = glm::vec3(1.f * size.x, 1.f * size.y, 0.f);
  glm::vec3 up_left = glm::vec3(-1.f * size.x, 1.f * size.y, 0.f);
  glm::vec3 down_right = glm::vec3(1.f * size.x, -1.f * size.y, 0.f);
  glm::vec3 down_left = glm::vec3(-1.f * size.x, -1.f * size.y, 0.f);

  // TODO Move the vertex calculation outside of the renderer for better performance
  glm::mat4 model = glm::translate(glm::mat4(1.f), transform.position);
  model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.f, 0.f, 0.f));
  model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.f, 1.f, 0.f));
  model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.f, 0.f, 1.f));
  model = glm::scale(model, transform.scale);

  m_buffer->vertex = model * glm::vec4(down_left, 1.f);
  m_buffer->uv = uv[0];
  m_buffer->tid = ts;
  m_buffer->color = color;
  m_buffer++;

  m_buffer->vertex = model * glm::vec4(up_left, 1.f);
  m_buffer->uv = uv[1];
  m_buffer->tid = ts;
  m_buffer->color = color;
  m_buffer++;

  m_buffer->vertex = model * glm::vec4(up_right, 1.f);
  m_buffer->uv = uv[2];
  m_buffer->tid = ts;
  m_buffer->color = color;
  m_buffer++;

  m_buffer->vertex = model * glm::vec4(down_right, 1.f);
  m_buffer->uv = uv[3];
  m_buffer->tid = ts;
  m_buffer->color = color;
  m_buffer++;

  m_index_count += 6;
}

void OpenGLRenderer::SubmitQuad(TransformComponent& transform, glm::vec2 half_extent, unsigned int color) {
  // Same transform and corner order as submit(), so the index buffer's
  // winding applies unchanged; the difference is that tid stays 0, which is
  // what makes Default.frag fall through to fs_in.color instead of sampling
  // a texture. That is why a grid line needs no shader change.
  glm::mat4 model = glm::translate(glm::mat4(1.f), transform.position);
  model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.f, 0.f, 0.f));
  model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.f, 1.f, 0.f));
  model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.f, 0.f, 1.f));
  model = glm::scale(model, transform.scale);

  const glm::vec3 down_left = glm::vec3(-1.f * half_extent.x, -1.f * half_extent.y, 0.f);
  const glm::vec3 up_left = glm::vec3(-1.f * half_extent.x, 1.f * half_extent.y, 0.f);
  const glm::vec3 up_right = glm::vec3(1.f * half_extent.x, 1.f * half_extent.y, 0.f);
  const glm::vec3 down_right = glm::vec3(1.f * half_extent.x, -1.f * half_extent.y, 0.f);

  m_buffer->vertex = model * glm::vec4(down_left, 1.f);
  m_buffer->uv = glm::vec2(0.f, 1.f);
  m_buffer->tid = 0.0f;
  m_buffer->color = color;
  m_buffer++;

  m_buffer->vertex = model * glm::vec4(up_left, 1.f);
  m_buffer->uv = glm::vec2(0.f, 0.f);
  m_buffer->tid = 0.0f;
  m_buffer->color = color;
  m_buffer++;

  m_buffer->vertex = model * glm::vec4(up_right, 1.f);
  m_buffer->uv = glm::vec2(1.f, 0.f);
  m_buffer->tid = 0.0f;
  m_buffer->color = color;
  m_buffer++;

  m_buffer->vertex = model * glm::vec4(down_right, 1.f);
  m_buffer->uv = glm::vec2(1.f, 1.f);
  m_buffer->tid = 0.0f;
  m_buffer->color = color;
  m_buffer++;

  m_index_count += 6;
}

void OpenGLRenderer::End() {
#ifdef EMSCRIPTEN
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, (m_Buffer - m_BufferBase) * RENDERER_VERTEX_SIZE, m_BufferBase);
  m_Buffer = m_BufferBase;
#else
  glUnmapBuffer(GL_ARRAY_BUFFER);
#endif  // EMSCRIPTEN

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGLRenderer::Flush() {
  for (unsigned int i = 0; i < m_texture_slots.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, m_texture_slots[i]);
  }

  glBindVertexArray(m_vao);
  m_ibo->Bind();

  glDrawElements(GL_TRIANGLES, m_index_count, GL_UNSIGNED_INT, NULL);

  m_ibo->Unbind();
  glBindVertexArray(0);

  m_index_count = 0;
  m_texture_slots.clear();
}
}  // namespace rse
