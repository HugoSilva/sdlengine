#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>

#include <glm/glm.hpp>

#include "renderer_2d.h"
#include "sprite_batch.h"

struct SDL_Window;

namespace rse {
// The desktop OpenGL implementation of the 2D renderer. It deliberately keeps
// GL out of this header: gameplay and editor code see Renderer2D, while the only
// translation unit that knows about buffers and draw calls is the .cpp.
//
// The renderer does not own the scene shader. The application still enables it
// and uploads the camera uniforms before submitting, which is the path every
// existing caller uses. The renderer records that program and the camera
// matrices so it can switch to a sprite's material and back without losing the
// uniforms: every switch re-uploads both.
class OpenGLRenderer : public Renderer2D {
 public:
  explicit OpenGLRenderer(SDL_Window* window);
  ~OpenGLRenderer() override;

  void Begin() override;
  void Submit(TransformComponent& transform, const Renderable2D* renderable, std::int32_t layer = 0) override;
  void Submit(TransformComponent& transform, const Renderable2D* renderable, std::int32_t layer,
              const SpriteAppearance& appearance) override;
  void SubmitQuad(TransformComponent& transform, glm::vec2 half_extent, unsigned int color,
                  std::int32_t layer = -1) override;
  void End() override;
  void Flush() override;

  void SetSceneShader(Shader* shader) override;
  void SetCameraUniforms(const glm::mat4& view, const glm::mat4& projection) override;

  void SetClearColor(const glm::vec3& color) override;
  void SetViewBounds(const glm::vec2& min, const glm::vec2& max) override;
  void ClearViewBounds() override;

  Renderer2DStats GetStats() const override { return m_stats; }
  RenderBackend GetBackend() const override { return RenderBackend::kOpenGL; }
  RenderCapabilities GetCapabilities() const override { return m_capabilities; }

 private:
  void ApplyFrameUniforms(Shader* shader);
  void DrawBatch(const SpriteBatch& batch, int base_vertex, std::size_t first_index);

  unsigned int m_vao{0};
  unsigned int m_vbo{0};
  unsigned int m_ibo{0};
  std::size_t m_vbo_capacity{0};
  std::size_t m_ibo_capacity{0};

  SpriteBatchBuilder m_builder;
  Renderer2DStats m_stats;
  RenderCapabilities m_capabilities;
  glm::vec3 m_clear_color{0.0f};
  glm::mat4 m_view{1.0f};
  glm::mat4 m_projection{1.0f};
  Shader* m_scene_shader{nullptr};
  std::chrono::steady_clock::time_point m_frame_start;
};
}  // namespace rse
