#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>

#include <glm/glm.hpp>

#include "renderer_2d.h"
#include "sprite_batch.h"

struct SDL_Window;

namespace rse {
class Shader;

// The desktop OpenGL implementation of the 2D renderer. It deliberately keeps
// GL out of this header: gameplay and editor code see Renderer2D, while the only
// translation unit that knows about buffers and draw calls is the .cpp.
class OpenGLRenderer : public Renderer2D {
 public:
  OpenGLRenderer(SDL_Window* window, Shader* shader);
  ~OpenGLRenderer() override;

  void Begin() override;
  void Submit(TransformComponent& transform, const Renderable2D* renderable, std::int32_t layer = 0) override;
  void SubmitQuad(TransformComponent& transform, glm::vec2 half_extent, unsigned int color,
                  std::int32_t layer = -1) override;
  void End() override;
  void Flush() override;

  void SetClearColor(const glm::vec3& color) override;
  void SetViewBounds(const glm::vec2& min, const glm::vec2& max) override;
  void ClearViewBounds() override;

  Renderer2DStats GetStats() const override { return m_stats; }
  RenderBackend GetBackend() const override { return RenderBackend::kOpenGL; }
  RenderCapabilities GetCapabilities() const override { return m_capabilities; }

 private:
  void DrawBatch(const SpriteBatch& batch);

  unsigned int m_vao{0};
  unsigned int m_vbo{0};
  unsigned int m_ibo{0};
  std::size_t m_vbo_capacity{0};
  std::size_t m_ibo_capacity{0};

  Shader* m_shader{nullptr};
  SpriteBatchBuilder m_builder;
  Renderer2DStats m_stats;
  RenderCapabilities m_capabilities;
  glm::vec3 m_clear_color{0.0f};
  std::chrono::steady_clock::time_point m_frame_start;
};
}  // namespace rse
