#pragma once

#ifdef EMSCRIPTEN
#define GL_GLEXT_PROTOTYPES
#include <SDL3/SDL_opengles2.h>

// OES_vertex_array_object
#define glGenVertexArrays glGenVertexArraysOES
#define glBindVertexArray glBindVertexArrayOES
#define glDeleteVertexArrays glDeleteVertexArraysOES

// OES_mapbuffer
#define glMapBuffer glMapBufferOES
#define glUnmapBuffer glUnmapBufferOES
#define GL_WRITE_ONLY GL_WRITE_ONLY_OES

#else
#include <GL/glew.h>
#endif  // EMSCRIPTEN

#include <vector>

#include <glm/glm.hpp>

#include "index_buffer.h"
#include "renderable_2d.h"
#include "renderer_2d.h"
#include <components/shader.h>
#include <tests/test_clear_color.h>

namespace rse {
#define RENDERER_MAX_SPRITES 60000
#define RENDERER_VERTEX_SIZE sizeof(VertexData)
#define RENDERER_SPRITE_SIZE RENDERER_VERTEX_SIZE * 4
#define RENDERER_BUFFER_SIZE RENDERER_SPRITE_SIZE* RENDERER_MAX_SPRITES
#define RENDERER_INDICES_SIZE RENDERER_MAX_SPRITES * 6
#define RENDERER_MAX_TEXTURES 32

#define SHADER_VERTEX_INDEX 0
#define SHADER_UV_INDEX 1
#define SHADER_TID_INDEX 2
#define SHADER_COLOR_INDEX 3

class OpenGLRenderer : public Renderer2D {
 public:
  OpenGLRenderer(SDL_Window* win, Shader* shader);
  ~OpenGLRenderer();

  void Begin();
  void Submit(TransformComponent& position, const Renderable2D* renderable);
  void SubmitQuad(TransformComponent& transform, glm::vec2 half_extent, unsigned int color) override;
  void End();
  void Flush();

  // Writes the scene's background into the clear colour TestClearColor owns
  // and Begin() applies, so the clear stays in one place.
  void SetClearColor(const glm::vec3& color) override;

 private:
  unsigned int m_vao, m_vbo;
  int m_index_count;
  VertexData* m_buffer;
  IndexBuffer* m_ibo;
  SDL_Window* m_window;
  std::vector<unsigned int> m_texture_slots;
  test::TestClearColor* m_clear_color;
  Shader* m_shader;
#ifdef EMSCRIPTEN
  VertexData* m_BufferBase;
#endif  // EMSCRIPTEN
};
}  // namespace rse
