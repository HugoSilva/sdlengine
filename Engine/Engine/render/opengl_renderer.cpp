#include "opengl_renderer.h"

#include <algorithm>
#include <cstddef>

#include <glm/glm.hpp>

#ifdef EMSCRIPTEN
#include <SDL3/SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif  // EMSCRIPTEN

#include <components/shader.h>
#include <render/renderable_2d.h>

namespace rse {
OpenGLRenderer::OpenGLRenderer(SDL_Window* window, Shader* shader) : m_shader(shader) {
  // The window is part of the constructor because backends need it to create
  // their context; the batched 2D path itself has no SDL dependency.
  (void)window;

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ibo);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (const void*)offsetof(SpriteVertex, position));
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (const void*)offsetof(SpriteVertex, uv));
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (const void*)offsetof(SpriteVertex, tid));
  glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(SpriteVertex),
                        (const void*)offsetof(SpriteVertex, color));

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  if (m_shader != nullptr) {
    m_shader->Enable();
    m_shader->SetUniform1i("texture0", 0);
    m_shader->Disable();
  }

  GLint texture_units = 0;
  GLint texture_size = 0;
  glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texture_size);
  m_capabilities.max_texture_units = static_cast<std::uint32_t>(std::max(1, texture_units));
  m_capabilities.max_texture_size = static_cast<std::uint32_t>(std::max(0, texture_size));
  m_capabilities.max_batch_vertices = static_cast<std::uint32_t>(SpriteBatchBuilder::kMaxVerticesPerBatch);
}

OpenGLRenderer::~OpenGLRenderer() {
  glDeleteBuffers(1, &m_ibo);
  glDeleteBuffers(1, &m_vbo);
  glDeleteVertexArrays(1, &m_vao);
}

void OpenGLRenderer::Begin() {
  m_builder.Reset();
  m_stats = {};
  m_frame_start = std::chrono::steady_clock::now();

  glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::Submit(TransformComponent& transform, const Renderable2D* renderable, std::int32_t layer) {
  if (renderable == nullptr) return;

  SpriteDraw draw;
  draw.uv = renderable->GetUv();
  draw.position = transform.position;
  draw.rotation = transform.rotation;
  draw.size = renderable->GetSize();
  draw.scale = transform.scale;
  draw.color = renderable->GetColor();
  draw.texture = renderable->GetTId();
  draw.layer = layer;
  draw.depth = transform.position.z;
  m_builder.Add(draw);
}

void OpenGLRenderer::SubmitQuad(TransformComponent& transform, glm::vec2 half_extent, unsigned int color,
                                std::int32_t layer) {
  SpriteDraw draw;
  draw.position = transform.position;
  draw.rotation = transform.rotation;
  draw.size = half_extent;
  draw.scale = transform.scale;
  draw.color = color;
  draw.texture = 0;
  draw.layer = layer;
  draw.depth = transform.position.z;
  m_builder.Add(draw);
}

void OpenGLRenderer::End() {
  // Nothing is mapped or copied here any more. The queue is built in Flush,
  // after every system has submitted its frame, which is what makes sorting and
  // texture grouping possible.
}

void OpenGLRenderer::Flush() {
  m_builder.Build();
  const SpriteBatchStats& batch_stats = m_builder.GetStats();
  m_stats.submitted = batch_stats.submitted;
  m_stats.culled = batch_stats.culled;
  m_stats.batches = batch_stats.batches;
  m_stats.vertices = batch_stats.vertices;
  m_stats.indices = batch_stats.indices;
  m_stats.draw_calls = batch_stats.batches;

  if (m_shader != nullptr) m_shader->Enable();
  glBindVertexArray(m_vao);
  glActiveTexture(GL_TEXTURE0);

  // One upload for the frame, not one per batch. The batch index ranges are
  // local to the batch, so each draw is an offset into that single index
  // buffer and no base-vertex support is needed.
  const auto& vertices = m_builder.GetVertices();
  const auto& indices = m_builder.GetIndices();
  const std::size_t vertex_bytes = vertices.size() * sizeof(SpriteVertex);
  const std::size_t index_bytes = indices.size() * sizeof(std::uint16_t);

  if (vertex_bytes > 0) {
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    if (vertex_bytes > m_vbo_capacity) {
      glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertex_bytes), nullptr, GL_STREAM_DRAW);
      m_vbo_capacity = vertex_bytes;
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(vertex_bytes), vertices.data());
  }

  if (index_bytes > 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    if (index_bytes > m_ibo_capacity) {
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(index_bytes), nullptr, GL_STREAM_DRAW);
      m_ibo_capacity = index_bytes;
    }
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(index_bytes), indices.data());
  }

  for (const SpriteBatch& batch : m_builder.GetBatches()) {
    DrawBatch(batch);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);

  const auto elapsed = std::chrono::steady_clock::now() - m_frame_start;
  m_stats.cpu_ms = std::chrono::duration<double, std::milli>(elapsed).count();
}

void OpenGLRenderer::DrawBatch(const SpriteBatch& batch) {
  const auto* index_offset = reinterpret_cast<const void*>(batch.first_index * sizeof(std::uint16_t));
  glBindTexture(GL_TEXTURE_2D, batch.texture);
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(batch.index_count), GL_UNSIGNED_SHORT, index_offset);
}

void OpenGLRenderer::SetClearColor(const glm::vec3& color) { m_clear_color = color; }

void OpenGLRenderer::SetViewBounds(const glm::vec2& min, const glm::vec2& max) { m_builder.SetViewBounds(min, max); }

void OpenGLRenderer::ClearViewBounds() { m_builder.ClearViewBounds(); }
}  // namespace rse
