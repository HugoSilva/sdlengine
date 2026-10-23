#pragma once

#include <cstddef>
#include <cstdint>

#include <glm/glm.hpp>

#include <ecs/components/transform.h>

namespace rse {
class Renderable2D;

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

  // A flat, untextured quad in world space. Same transform and corner
  // convention as Submit(), but with no texture bound, so the shader falls
  // through to the vertex colour and `color` is used as-is. The editor's grid
  // and gizmos are built out of these.
  virtual void SubmitQuad(TransformComponent& transform, glm::vec2 half_extent, unsigned int color,
                          std::int32_t layer = -1) {}

  virtual void End() {}

  virtual void Flush() = 0;

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
