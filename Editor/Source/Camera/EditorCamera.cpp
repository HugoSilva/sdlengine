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

void EditorCamera::SetViewportSize(glm::vec2 devicePixels, float dpiScale) {
  m_ViewportSize = glm::max(devicePixels, glm::vec2(1.0f));
  m_DpiScale = dpiScale > 0.0f ? dpiScale : 1.0f;
  Rebuild();
}

void EditorCamera::Rebuild() {
  m_View = rse::MakeOrthoView(m_Center, m_Zoom, m_ViewportSize, kReferenceSize);
  m_Projection = rse::MakeOrthoProjection(m_View);
}

void EditorCamera::Apply(rse::Shader* shader) const {
  if (shader == nullptr) {
    return;
  }

  const glm::mat4 view(1.0f);
  shader->SetUniformMat4("view", glm::value_ptr(view));
  shader->SetUniformMat4("projection", glm::value_ptr(m_Projection));
}

float EditorCamera::GetWorldPerDevicePixel() const {
  // The view is aspect-correct, so x and y share one scale.
  return m_ViewportSize.y > 0.0f ? m_View.Size().y / m_ViewportSize.y : 1.0f;
}

float EditorCamera::GetWorldPerPixel() const {
  // One ImGui pixel is dpiScale device pixels; pan() has always made the same
  // conversion, and the gizmo has to make it too or a drag would drift by 50%
  // on a scaled display.
  return GetWorldPerDevicePixel() * m_DpiScale;
}

glm::vec2 EditorCamera::ScreenToWorld(glm::vec2 pixel) const {
  return rse::ViewportToWorld(m_View, pixel * m_DpiScale, m_ViewportSize);
}

glm::vec2 EditorCamera::WorldToScreen(glm::vec2 world) const {
  return rse::WorldToViewport(m_View, world, m_ViewportSize) / m_DpiScale;
}

void EditorCamera::Pan(glm::vec2 pixelDelta) {
  const float worldPerPixel = GetWorldPerDevicePixel() * m_DpiScale;

  // Dragging right moves the scene right, so the camera moves left; dragging
  // down (y grows downward) moves it up.
  m_Center += glm::vec2(-pixelDelta.x * worldPerPixel, pixelDelta.y * worldPerPixel);
  Rebuild();
}

void EditorCamera::ZoomAt(float wheelSteps, glm::vec2 pixelAnchor) {
  const glm::vec2 worldBefore = ScreenToWorld(pixelAnchor);

  m_Zoom = std::clamp(m_Zoom * std::pow(kZoomStep, wheelSteps), kMinZoom, kMaxZoom);
  Rebuild();

  // Leave whatever the cursor was over exactly where it was.
  m_Center += worldBefore - ScreenToWorld(pixelAnchor);
  Rebuild();
}

void EditorCamera::FocusOn(glm::vec2 worldCenter, glm::vec2 worldSize) {
  rse::Bounds2D bounds;
  bounds.min = worldCenter - worldSize * 0.5f;
  bounds.max = worldCenter + worldSize * 0.5f;

  const rse::OrthoView fitted = rse::FitToBounds(m_View, bounds, kFocusPadding);

  // makeOrthoView takes a zoom, so the fitted rectangle has to be turned back
  // into one. Clamping it here is what keeps a zero-sized selection from
  // producing an unbounded zoom.
  m_Zoom = std::clamp(kReferenceSize.y / fitted.Size().y, kMinZoom, kMaxZoom);
  m_Center = fitted.center;
  Rebuild();
}
