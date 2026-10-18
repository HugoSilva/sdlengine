#pragma once

#include <imgui.h>
#include <glm/glm.hpp>
#include <Core.h>

#include "../Camera/EditorCamera.hpp"
#include "../Layout/EditorPanel.hpp"
#include "../EditorManager.h"

// ---------------------------------------------------------------------------
// The main scene viewport.
//
// The scene is not drawn into this window -- it is drawn into an offscreen
// framebuffer (handed to Core through EditorManager::setSceneTarget) and the
// resulting texture is shown here as an image. That indirection is what lets
// the scene behave like any other dockable panel, and it is the same shape a
// game view or a second viewport would take later on.
//
// The panel publishes its framebuffer's real pixel size to the camera every
// frame: the projection is built from the *shape of the target*, so a panel
// that is not 16:9 shows more world rather than a stretched one.
// ---------------------------------------------------------------------------
class ViewportPanel : public EditorPanel
{
public:
	explicit ViewportPanel(EditorCamera* camera) : m_Camera(camera) {}

	const char* id() const override { return "Viewport"; }

	void draw() override
	{
		if (!ImGui::Begin(id(), &visible))
		{
			// Collapsed. There is nothing to present, but the scene still needs
			// a target: without one it would be drawn into the window and show
			// through the dock space, whose host window has no background of
			// its own. The framebuffer keeps its last size instead.
			claimSceneTarget();
			ImGui::End();
			return;
		}

		const ImVec2 available = ImGui::GetContentRegionAvail();
		const float dpiScale = displayScale();

		m_Framebuffer.resize(
			static_cast<int>(available.x * dpiScale),
			static_cast<int>(available.y * dpiScale));

		if (m_Camera != nullptr)
		{
			m_Camera->setViewportSize(
				glm::vec2(static_cast<float>(m_Framebuffer.getWidth()), static_cast<float>(m_Framebuffer.getHeight())),
				dpiScale);
		}

		// GL textures are bottom-up, ImGui images are top-down.
		ImGui::Image(
			(ImTextureID)(intptr_t)m_Framebuffer.getColorTexture(),
			available,
			ImVec2(0.0f, 1.0f),
			ImVec2(1.0f, 0.0f));

		handleNavigation(available);

		// Tells Core::run() where the scene pass should draw this frame.
		claimSceneTarget();

		ImGui::End();
	}

private:
	static float displayScale()
	{
		const float scale = ImGui::GetIO().DisplayFramebufferScale.x;
		return scale > 0.0f ? scale : 1.0f;
	}

	void claimSceneTarget()
	{
		EditorManager::setSceneTarget(&m_Framebuffer);
	}

	void handleNavigation(const ImVec2& size)
	{
		if (m_Camera == nullptr)
		{
			return;
		}

		// Only steal input while the cursor is over the image itself, so the
		// panel's own scrollbars and the rest of the UI stay usable.
		if (!ImGui::IsItemHovered())
		{
			return;
		}

		const ImGuiIO& io = ImGui::GetIO();

		if (io.MouseWheel != 0.0f)
		{
			// Anchored on the middle of the panel, which is what the old
			// rse::Camera::AddZoom did. Zooming at the cursor is M3.
			m_Camera->zoomAt(io.MouseWheel, glm::vec2(size.x * 0.5f, size.y * 0.5f));
		}

		// Middle-drag pans, as it does in most editors.
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
		{
			m_Camera->pan(glm::vec2(io.MouseDelta.x, io.MouseDelta.y));
		}
	}

	EditorCamera* m_Camera{ nullptr };
	rse::Framebuffer m_Framebuffer;
};
