#pragma once

#include <cstdio>
#include <optional>

#include <imgui.h>
#include <glm/glm.hpp>
#include <Core.h>

#include "../Camera/EditorCamera.hpp"
#include "../Layout/EditorPanel.hpp"
#include "../Scene/EntityBounds.hpp"
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

		// The image's own screen position: mouse input arrives in window
		// coordinates and has to be made relative to this.
		const ImVec2 imagePos = ImGui::GetCursorScreenPos();

		// GL textures are bottom-up, ImGui images are top-down.
		ImGui::Image(
			(ImTextureID)(intptr_t)m_Framebuffer.getColorTexture(),
			available,
			ImVec2(0.0f, 1.0f),
			ImVec2(1.0f, 0.0f));

		handleNavigation(imagePos);
		handleShortcuts();
		drawHud(imagePos);

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

	void handleNavigation(const ImVec2& imagePos)
	{
		if (m_Camera == nullptr)
		{
			return;
		}

		// The hover test only decides whether a gesture *starts*. Once a pan is
		// latched it is the button, not the cursor position, that keeps it
		// going -- otherwise the pan stops the moment a drag leaves the panel.
		const bool hovered = ImGui::IsItemHovered();
		const ImGuiIO& io = ImGui::GetIO();

		if (hovered && io.MouseWheel != 0.0f)
		{
			// Anchored on the cursor, so whatever is under it stays under it.
			// The mouse and the image are both in ImGui pixels, so this is a
			// subtraction; the camera turns it into device pixels.
			const glm::vec2 anchor(io.MousePos.x - imagePos.x, io.MousePos.y - imagePos.y);
			m_Camera->zoomAt(io.MouseWheel, anchor);
		}

		// Middle-drag pans, as it does in most editors.
		if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
		{
			m_Panning = true;
		}

		if (!ImGui::IsMouseDown(ImGuiMouseButton_Middle))
		{
			m_Panning = false;
		}

		if (m_Panning)
		{
			m_Camera->pan(glm::vec2(io.MouseDelta.x, io.MouseDelta.y));
		}
	}

	// F frames the selection, or the whole scene when nothing is selected. The
	// hierarchy's double-click asks for the same thing, through EditorManager,
	// because it happens in another panel.
	void handleShortcuts()
	{
		if (m_Camera == nullptr)
		{
			return;
		}

		const ImGuiIO& io = ImGui::GetIO();
		const bool viewportActive =
			ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) || ImGui::IsItemHovered();

		const bool pressed = viewportActive && !io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_F);

		// Consumed even when the key was the reason, so a request cannot be
		// left pending and fire a second time later.
		const bool requested = EditorManager::consumeFrameRequest();

		if (pressed || requested)
		{
			frameSelection();
		}
	}

	void frameSelection()
	{
		// The selection first, then everything there is. An empty scene has
		// nothing to frame, so the view is left alone rather than zoomed to its
		// maximum on the origin.
		std::optional<rse::Bounds2D> bounds;

		const std::optional<EntityBounds> selection = EditorBounds::ofEntity(EditorManager::getSelectedEntity());
		if (selection)
		{
			// The box that frames it, which is not the box that gets drawn: a
			// rotated entity needs its corners boxed first.
			bounds = EditorBounds::aabb(*selection);
		}

		if (!bounds)
		{
			bounds = EditorBounds::ofScene();
		}

		if (!bounds)
		{
			return;
		}

		m_Camera->focusOn(bounds->center(), bounds->size());
	}

	// Zoom, camera centre and gizmo mode, over the top-left of the scene. Drawn
	// into the draw list rather than as widgets: widgets here would add to the
	// window's content size and could give the panel scrollbars.
	void drawHud(const ImVec2& imagePos)
	{
		if (m_Camera == nullptr)
		{
			return;
		}

		char text[192];
		snprintf(text, sizeof(text), "Zoom %.0f%%\nCentre %.0f, %.0f\nGizmo %s",
			m_Camera->getZoom() * 100.0f,
			m_Camera->getCenter().x,
			m_Camera->getCenter().y,
			gizmoModeName(EditorManager::settings().gizmo.mode));

		const ImVec2 textSize = ImGui::CalcTextSize(text);
		const ImVec2 blockMin(imagePos.x + 6.0f, imagePos.y + 6.0f);

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(blockMin,
			ImVec2(blockMin.x + textSize.x + 8.0f, blockMin.y + textSize.y + 8.0f),
			IM_COL32(0, 0, 0, 140), 3.0f);
		drawList->AddText(ImVec2(blockMin.x + 4.0f, blockMin.y + 4.0f), IM_COL32(235, 235, 235, 255), text);
	}

	EditorCamera* m_Camera{ nullptr };
	rse::Framebuffer m_Framebuffer;

	// Latched on middle-press and cleared on release, so a pan survives the
	// cursor leaving the panel mid-drag.
	bool m_Panning{ false };
};
