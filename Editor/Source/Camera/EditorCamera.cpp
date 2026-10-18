#include "EditorCamera.hpp"

#include <algorithm>
#include <cmath>

#include <glm/gtc/type_ptr.hpp>

#include <Core.h> // rse::Shader

namespace
{
	// One wheel notch.
	constexpr float kZoomStep = 1.15f;

	// How much room to leave around a focused selection.
	constexpr float kFocusPadding = 0.1f;
}

EditorCamera::EditorCamera()
{
	rebuild();
}

void EditorCamera::setViewportSize(glm::vec2 devicePixels, float dpiScale)
{
	m_ViewportSize = glm::max(devicePixels, glm::vec2(1.0f));
	m_DpiScale = dpiScale > 0.0f ? dpiScale : 1.0f;
	rebuild();
}

void EditorCamera::rebuild()
{
	m_View = rse::makeOrthoView(m_Center, m_Zoom, m_ViewportSize, kReferenceSize);
	m_Projection = rse::makeOrthoProjection(m_View);
}

void EditorCamera::apply(rse::Shader* shader) const
{
	if (shader == nullptr)
	{
		return;
	}

	const glm::mat4 view(1.0f);
	shader->setUniformMat4("view", glm::value_ptr(view));
	shader->setUniformMat4("projection", glm::value_ptr(m_Projection));
}

float EditorCamera::getWorldPerDevicePixel() const
{
	// The view is aspect-correct, so x and y share one scale.
	return m_ViewportSize.y > 0.0f ? m_View.size().y / m_ViewportSize.y : 1.0f;
}

glm::vec2 EditorCamera::screenToWorld(glm::vec2 pixel) const
{
	return rse::viewportToWorld(m_View, pixel * m_DpiScale, m_ViewportSize);
}

glm::vec2 EditorCamera::worldToScreen(glm::vec2 world) const
{
	return rse::worldToViewport(m_View, world, m_ViewportSize) / m_DpiScale;
}

void EditorCamera::pan(glm::vec2 pixelDelta)
{
	const float worldPerPixel = getWorldPerDevicePixel() * m_DpiScale;

	// Dragging right moves the scene right, so the camera moves left; dragging
	// down (y grows downward) moves it up.
	m_Center += glm::vec2(-pixelDelta.x * worldPerPixel, pixelDelta.y * worldPerPixel);
	rebuild();
}

void EditorCamera::zoomAt(float wheelSteps, glm::vec2 pixelAnchor)
{
	const glm::vec2 worldBefore = screenToWorld(pixelAnchor);

	m_Zoom = std::clamp(m_Zoom * std::pow(kZoomStep, wheelSteps), kMinZoom, kMaxZoom);
	rebuild();

	// Leave whatever the cursor was over exactly where it was.
	m_Center += worldBefore - screenToWorld(pixelAnchor);
	rebuild();
}

void EditorCamera::focusOn(glm::vec2 worldCenter, glm::vec2 worldSize)
{
	rse::Bounds2D bounds;
	bounds.min = worldCenter - worldSize * 0.5f;
	bounds.max = worldCenter + worldSize * 0.5f;

	const rse::OrthoView fitted = rse::fitToBounds(m_View, bounds, kFocusPadding);

	// makeOrthoView takes a zoom, so the fitted rectangle has to be turned back
	// into one. Clamping it here is what keeps a zero-sized selection from
	// producing an unbounded zoom.
	m_Zoom = std::clamp(kReferenceSize.y / fitted.size().y, kMinZoom, kMaxZoom);
	m_Center = fitted.center;
	rebuild();
}
