#pragma once

#include <imgui.h>
#include <glm/glm.hpp>
#include <Core.h>

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
// ---------------------------------------------------------------------------
class ViewportPanel : public EditorPanel
{
public:
	explicit ViewportPanel(rse::Camera* camera) : m_Camera(camera) {}

	const char* id() const override { return "Viewport"; }

	void draw() override
	{
		if (!ImGui::Begin(id(), &visible))
		{
			// Collapsed: it still exists, but there is nothing to render into.
			ImGui::End();
			return;
		}

		const ImVec2 available = ImGui::GetContentRegionAvail();
		const float dpiScale = ImGui::GetIO().DisplayFramebufferScale.x;

		m_Framebuffer.resize(
			static_cast<int>(available.x * dpiScale),
			static_cast<int>(available.y * dpiScale));

		// GL textures are bottom-up, ImGui images are top-down.
		ImGui::Image(
			(ImTextureID)(intptr_t)m_Framebuffer.getColorTexture(),
			available,
			ImVec2(0.0f, 1.0f),
			ImVec2(1.0f, 0.0f));

		handleNavigation(available);

		// Tells Core::run() where the scene pass should draw this frame.
		EditorManager::setSceneTarget(&m_Framebuffer);

		ImGui::End();
	}

private:
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
			m_Camera->AddZoom(io.MouseWheel);
		}

		// Middle-drag pans, as it does in most editors.
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
		{
			const float worldPerPixelX = m_Camera->getViewWidth() / (size.x > 0.0f ? size.x : 1.0f);
			const float worldPerPixelY = m_Camera->getViewHeight() / (size.y > 0.0f ? size.y : 1.0f);

			// Dragging right moves the scene right, so the camera moves left.
			m_Camera->Pan(glm::vec2(-io.MouseDelta.x * worldPerPixelX, io.MouseDelta.y * worldPerPixelY));
		}
	}

	rse::Camera* m_Camera{ nullptr };
	rse::Framebuffer m_Framebuffer;
};
