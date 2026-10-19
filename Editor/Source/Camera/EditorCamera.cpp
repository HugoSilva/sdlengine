#include "EditorCamera.hpp"

#include <Core.h>  // rse::Shader
#include <algorithm>
#include <cmath>

#include <glm/gtc/type_ptr.hpp>

namespace {
// One wheel notch.
constexpr float kZoomStep = 1.15f;

// How much room to leave around a focused selection.
constexpr float kFocusPadding = 0.1f;
}  // namespace

EditorCamera::EditorCamera() { Rebuild(); }

void EditorCamera::SetViewportSize(glm::vec2 device_pixels, float dpi_scale) {
  m_viewport_size = glm::max(device_pixels, glm::vec2(1.0f));
  m_dpi_scale = dpi_scale > 0.0f ? dpi_scale : 1.0f;
  Rebuild();
}

void EditorCamera::Rebuild() {
  m_view = rse::MakeOrthoView(m_center, m_zoom, m_viewport_size, kReferenceSize);
  m_projection = rse::MakeOrthoProjection(m_view);
}

void EditorCamera::Apply(rse::Shader* shader) const {
  if (shader == nullptr) {
    return;
  }

  const glm::mat4 view(1.0f);
  shader->SetUniformMat4("view", glm::value_ptr(view));
  shader->SetUniformMat4("projection", glm::value_ptr(m_projection));
}

float EditorCamera::GetWorldPerDevicePixel() const {
  // The view is aspect-correct, so x and y share one scale.
  return m_viewport_size.y > 0.0f ? m_view.Size().y / m_viewport_size.y : 1.0f;
}

float EditorCamera::GetWorldPerPixel() const {
  // One ImGui pixel is dpiScale device pixels; pan() has always made the same
  // conversion, and the gizmo has to make it too or a drag would drift by 50%
  // on a scaled display.
  return GetWorldPerDevicePixel() * m_dpi_scale;
}

glm::vec2 EditorCamera::ScreenToWorld(glm::vec2 pixel) const {
  return rse::ViewportToWorld(m_view, pixel * m_dpi_scale, m_viewport_size);
}

glm::vec2 EditorCamera::WorldToScreen(glm::vec2 world) const {
  return rse::WorldToViewport(m_view, world, m_viewport_size) / m_dpi_scale;
}

void EditorCamera::Pan(glm::vec2 pixel_delta) {
  const float world_per_pixel = GetWorldPerDevicePixel() * m_dpi_scale;

  // Dragging right moves the scene right, so the camera moves left; dragging
  // down (y grows downward) moves it up.
  m_center += glm::vec2(-pixel_delta.x * world_per_pixel, pixel_delta.y * world_per_pixel);
  Rebuild();
}

void EditorCamera::ZoomAt(float wheel_steps, glm::vec2 pixel_anchor) {
  const glm::vec2 world_before = ScreenToWorld(pixel_anchor);

  m_zoom = std::clamp(m_zoom * std::pow(kZoomStep, wheel_steps), kMinZoom, kMaxZoom);
  Rebuild();

  // Leave whatever the cursor was over exactly where it was.
  m_center += world_before - ScreenToWorld(pixel_anchor);
  Rebuild();
}

void EditorCamera::FocusOn(glm::vec2 world_center, glm::vec2 world_size) {
  rse::Bounds2D bounds;
  bounds.min = world_center - world_size * 0.5f;
  bounds.max = world_center + world_size * 0.5f;

  const rse::OrthoView fitted = rse::FitToBounds(m_view, bounds, kFocusPadding);

  // makeOrthoView takes a zoom, so the fitted rectangle has to be turned back
  // into one. Clamping it here is what keeps a zero-sized selection from
  // producing an unbounded zoom.
  m_zoom = std::clamp(kReferenceSize.y / fitted.Size().y, kMinZoom, kMaxZoom);
  m_center = fitted.center;
  Rebuild();
}
