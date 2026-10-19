#pragma once

// ---------------------------------------------------------------------------
// EditorCamera -- the editor's own 2D camera for the scene viewport, plus the
// screen/world conversions everything else in the viewport is built on.
//
// Deliberately standalone rather than built on rse::Camera: the editor pans,
// zooms and converts between screen and world space every frame, and sharing
// state with the engine's game camera is what tangled the two together in the
// first place (the layout epic bolted ortho controls onto rse::Camera purely to
// make the viewport work).
//
// Sizes are in *device* pixels -- the framebuffer's real size, which is what the
// projection has to match. Pixel arguments to screenToWorld / worldToScreen /
// pan are in ImGui's logical pixels, y down, i.e. relative to the top-left of
// the viewport image.
// ---------------------------------------------------------------------------

#include <glm/glm.hpp>

#include <render/ortho_view.h>

namespace rse {
class Shader;
}

class EditorCamera {
 public:
  // 720 world units tall at zoom 1, i.e. exactly what the renderer framed
  // before this camera existed, so the default view is unchanged.
  static constexpr glm::vec2 kReferenceSize{1280.0f, 720.0f};
  static constexpr float kMinZoom = rse::kOrthoMinZoom;
  static constexpr float kMaxZoom = rse::kOrthoMaxZoom;

  EditorCamera();

  // Published by the viewport panel every frame.
  void SetViewportSize(glm::vec2 device_pixels, float dpi_scale);

  const rse::OrthoView& GetView() const { return m_view; }
  const glm::mat4& GetProjection() const { return m_projection; }

  // Uploads the frame's uniforms: identity view (the ortho rectangle already
  // carries the camera position) plus the projection.
  void Apply(rse::Shader* shader) const;

  glm::vec2 GetCenter() const { return m_center; }
  float GetZoom() const { return m_zoom; }

  // World units per *device* pixel, so a grid line can be exactly one pixel
  // thick at any zoom.
  float GetWorldPerDevicePixel() const;

  // World units per *ImGui* pixel. That is the unit a mouse delta arrives in,
  // so it is what turns a gizmo drag of N pixels into a world offset.
  float GetWorldPerPixel() const;

  glm::vec2 ScreenToWorld(glm::vec2 pixel) const;
  glm::vec2 WorldToScreen(glm::vec2 world) const;

  void Pan(glm::vec2 pixel_delta);
  void ZoomAt(float wheel_steps, glm::vec2 pixel_anchor);
  void FocusOn(glm::vec2 world_center, glm::vec2 world_size);

 private:
  void Rebuild();

  rse::OrthoView m_view;
  glm::mat4 m_projection{1.0f};

  glm::vec2 m_center{640.0f, 360.0f};
  glm::vec2 m_viewport_size{kReferenceSize};  // device pixels
  float m_dpi_scale{1.0f};
  float m_zoom{1.0f};
};
