#pragma once

#include <cstddef>
#include <cstdint>

#include <glm/glm.hpp>

#include <ecs/components/transform.h>

namespace rse {
class MaterialShader;
class Renderable2D;
class Shader;

// Per-sprite presentation that is not part of the renderable itself. The
// sprite's UV rectangle still comes from the texture (an atlas region, not the
// whole page); flip mirrors within that rectangle, and a material replaces the
// program used for this sprite's draw batch.
struct SpriteAppearance {
  bool flip_x{false};
  bool flip_y{false};
  MaterialShader* material{nullptr};
};

enum class RenderBackend {
  kUnknown,
  kOpenGL,
  kCitro3D,
  kLibNDS,
};

// What a backend can do. The editor and game only need the common subset, but
// exposing the limits makes it possible to choose a path (for example, instanced
// sprites on desktop and plain batches on a fixed-function console) without
// leaking GL queries into gameplay code.
struct RenderCapabilities {
  bool instancing{false};
  bool indirect_draw{false};
  bool compute{false};
  bool texture_arrays{false};
  std::uint32_t max_texture_units{1};
  std::uint32_t max_texture_size{0};
  std::uint32_t max_batch_vertices{0};
};

struct Renderer2DStats {
  std::size_t submitted{0};
  std::size_t culled{0};
  std::size_t batches{0};
  std::size_t vertices{0};
  std::size_t indices{0};
  std::size_t draw_calls{0};
  double cpu_ms{0.0};
};

class Renderer2D {
 protected:
  Renderer2D() = default;

 public:
  virtual ~Renderer2D() = default;

  virtual void Begin() {}

  // `layer` is the coarse sort key: lower layers draw first. The editor's grid
  // uses the default background layer; sprites use their BasicComponent layer
  // so a scene can put foreground effects over world geometry without relying
  // on submission order.
  virtual void Submit(TransformComponent& position, const Renderable2D* renderable, std::int32_t layer = 0) = 0;

  // The same submission with the sprite's flip and material. Backends that do
  // not know about appearance fall back to the plain submission, which is what
  // keeps the interface usable by a fixed-function console backend.
  virtual void Submit(TransformComponent& position, const Renderable2D* renderable, std::int32_t layer,
                      const SpriteAppearance& appearance) {
    (void)appearance;
    Submit(position, renderable, layer);
  }

  // A flat, untextured quad in world space. Same transform and corner
  // convention as Submit(), but with no texture bound, so the shader falls
  // through to the vertex colour and `color` is used as-is. The editor's grid
  // and gizmos are built out of these.
  virtual void SubmitQuad(TransformComponent& transform, glm::vec2 half_extent, unsigned int color,
                          std::int32_t layer = -1) {}

  virtual void End() {}

  virtual void Flush() = 0;

  // The program a batch uses when its sprite has no material. The application
  // still enables the shader before submitting -- existing backends and
  // callers rely on that -- but a backend that switches to a material needs to
  // know which program to return to, and which camera uniforms to re-upload
  // after the switch.
  virtual void SetSceneShader(Shader* shader) { (void)shader; }
  virtual void SetCameraUniforms(const glm::mat4& view, const glm::mat4& projection) {
    (void)view;
    (void)projection;
  }

  // The colour every frame is cleared to. The open scene owns the background,
  // so core sets this once per frame; a backend that never clears ignores it.
  virtual void SetClearColor(const glm::vec3& color) {}

  // The world rectangle the camera can see. Backends use it to reject sprites
  // before building vertices; an empty view disables culling.
  virtual void SetViewBounds(const glm::vec2& min, const glm::vec2& max) {}
  virtual void ClearViewBounds() {}

  virtual Renderer2DStats GetStats() const { return {}; }
  virtual RenderBackend GetBackend() const { return RenderBackend::kUnknown; }
  virtual RenderCapabilities GetCapabilities() const { return {}; }
};
}  // namespace rse
