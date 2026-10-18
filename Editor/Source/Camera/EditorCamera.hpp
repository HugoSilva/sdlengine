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

#include <Render/OrthoView.hpp>

namespace rse { class Shader; }

class EditorCamera
{
public:
	// 720 world units tall at zoom 1, i.e. exactly what the renderer framed
	// before this camera existed, so the default view is unchanged.
	static constexpr glm::vec2 kReferenceSize{ 1280.0f, 720.0f };
	static constexpr float kMinZoom = rse::kOrthoMinZoom;
	static constexpr float kMaxZoom = rse::kOrthoMaxZoom;

	EditorCamera();

	// Published by the viewport panel every frame.
	void setViewportSize(glm::vec2 devicePixels, float dpiScale);

	const rse::OrthoView& getView() const { return m_View; }
	const glm::mat4& getProjection() const { return m_Projection; }

	// Uploads the frame's uniforms: identity view (the ortho rectangle already
	// carries the camera position) plus the projection.
	void apply(rse::Shader* shader) const;

	glm::vec2 getCenter() const { return m_Center; }
	float getZoom() const { return m_Zoom; }

	// World units per *device* pixel, so a grid line can be exactly one pixel
	// thick at any zoom.
	float getWorldPerDevicePixel() const;

	// World units per *ImGui* pixel. That is the unit a mouse delta arrives in,
	// so it is what turns a gizmo drag of N pixels into a world offset.
	float getWorldPerPixel() const;

	glm::vec2 screenToWorld(glm::vec2 pixel) const;
	glm::vec2 worldToScreen(glm::vec2 world) const;

	void pan(glm::vec2 pixelDelta);
	void zoomAt(float wheelSteps, glm::vec2 pixelAnchor);
	void focusOn(glm::vec2 worldCenter, glm::vec2 worldSize);

private:
	void rebuild();

	rse::OrthoView m_View;
	glm::mat4 m_Projection{ 1.0f };

	glm::vec2 m_Center{ 640.0f, 360.0f };
	glm::vec2 m_ViewportSize{ kReferenceSize };   // device pixels
	float m_DpiScale{ 1.0f };
	float m_Zoom{ 1.0f };
};
