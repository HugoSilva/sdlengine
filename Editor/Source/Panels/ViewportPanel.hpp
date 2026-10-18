#pragma once

#include <array>
#include <cmath>
#include <cstdio>
#include <optional>

#include <imgui.h>
#include <glm/glm.hpp>
#include <Core.h>

#include "../Camera/EditorCamera.hpp"
#include "../Gizmo/Gizmo.hpp"
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

		// The gizmo gets the press before the picking does: a click on a handle
		// is a drag, not a selection.
		const bool gizmoOwnsMouse = handleGizmo(imagePos);

		handlePicking(imagePos, gizmoOwnsMouse);
		handleShortcuts();
		drawSelection(imagePos);
		drawGizmo(imagePos);
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

	// The gizmo's mouse work, and whether it took the press. Guarded here rather
	// than in the gizmo so that a panel with no camera does not have to know
	// about one.
	bool handleGizmo(const ImVec2& imagePos)
	{
		if (m_Camera == nullptr)
		{
			return false;
		}

		return m_Gizmo.handleInput(*m_Camera, imagePos, ImGui::IsItemHovered());
	}

	void drawGizmo(const ImVec2& imagePos)
	{
		if (m_Camera == nullptr)
		{
			return;
		}

		m_Gizmo.draw(*m_Camera, imagePos);
	}

	// A left click that does not turn into a drag selects what is under it;
	// clicking empty space deselects. Latched on press and judged on release, so
	// a drag -- a pan, or one of the gizmo's own -- is never mistaken for a
	// click. `taken` is this frame's answer from the gizmo: pressing a handle
	// starts a drag, and the same press must not also re-pick.
	void handlePicking(const ImVec2& imagePos, bool taken)
	{
		if (m_Camera == nullptr)
		{
			return;
		}

		const ImGuiIO& io = ImGui::GetIO();

		if (!taken && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			m_ClickArmed = true;
			m_ClickPos = io.MousePos;
		}

		if (!m_ClickArmed || !ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			return;
		}

		m_ClickArmed = false;

		const float travelled = glm::max(
			std::abs(io.MousePos.x - m_ClickPos.x),
			std::abs(io.MousePos.y - m_ClickPos.y));

		if (travelled > kClickSlop)
		{
			return;
		}

		pickAt(glm::vec2(io.MousePos.x - imagePos.x, io.MousePos.y - imagePos.y));
	}

	// Whatever is under the cursor; the last hit wins, because the renderer has
	// no depth to sort by and "on top" means "drawn later".
	void pickAt(glm::vec2 pixel)
	{
		const glm::vec2 world = m_Camera->screenToWorld(pixel);

		uint32_t hit = INVALID_ENTITY_ID;

		rse::ECSManager::getView<TransformComponent, SpriteComponent>().each(
			[&hit, &world](uint32_t entity, TransformComponent&, SpriteComponent&)
			{
				const std::optional<EntityBounds> bounds = EditorBounds::ofEntity(entity);

				if (bounds && rse::pointInOrientedBox(world, bounds->center, bounds->rotationDegrees, bounds->halfExtent))
				{
					hit = entity;
				}
			});

		EditorManager::setSelection(hit);
	}

	// One box per selected entity, over the scene. It is the same rectangle the
	// renderer draws -- EntityBounds mirrors submit() -- so it lands on the
	// sprite's own pixels rather than near them. An entity with no sprite has no
	// rectangle to draw, and gets a cross at its position instead of nothing.
	void drawSelection(const ImVec2& imagePos)
	{
		if (m_Camera == nullptr)
		{
			return;
		}

		ImDrawList* drawList = ImGui::GetWindowDrawList();

		for (const uint32_t entity : EditorManager::getSelectedEntities())
		{
			// Only one entity can be selected today, so this is always the
			// active colour; the other one is what multi-select will use to
			// tell the rest of the set from the entity the inspector edits.
			const ImU32 colour = entity == EditorManager::getSelectedEntity() ? kActiveOutline : kOutline;

			const std::optional<EntityBounds> bounds = EditorBounds::ofEntity(entity);

			if (!bounds)
			{
				const std::optional<glm::vec2> position = EditorBounds::positionOf(entity);

				if (position)
				{
					const glm::vec2 screen = m_Camera->worldToScreen(*position);
					const ImVec2 at(imagePos.x + screen.x, imagePos.y + screen.y);

					drawList->AddLine(ImVec2(at.x - kCrossSize, at.y), ImVec2(at.x + kCrossSize, at.y), kOutlineHalo, 4.0f);
					drawList->AddLine(ImVec2(at.x, at.y - kCrossSize), ImVec2(at.x, at.y + kCrossSize), kOutlineHalo, 4.0f);
					drawList->AddLine(ImVec2(at.x - kCrossSize, at.y), ImVec2(at.x + kCrossSize, at.y), colour, 2.0f);
					drawList->AddLine(ImVec2(at.x, at.y - kCrossSize), ImVec2(at.x, at.y + kCrossSize), colour, 2.0f);
				}

				continue;
			}

			const std::array<glm::vec2, 4> corners = EditorBounds::corners(*bounds);
			ImVec2 points[4];

			for (int i = 0; i < 4; i++)
			{
				const glm::vec2 screen = m_Camera->worldToScreen(corners[i]);
				points[i] = ImVec2(imagePos.x + screen.x, imagePos.y + screen.y);
			}

			// Halo first, colour over it. An outline is one colour and the scene
			// is whatever colour it likes: measured against the test sprite, the
			// amber line landed on a pixel of (200,95,21) and the two oranges
			// washed into each other.
			drawList->AddPolyline(points, 4, kOutlineHalo, 4.0f, ImDrawFlags_Closed);
			drawList->AddPolyline(points, 4, colour, 2.0f, ImDrawFlags_Closed);
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

	// A click is a press and a release that did not travel. The slop is wide
	// enough for an unsteady hand and narrow enough that a pan, or one of M5's
	// gizmo drags, can never be mistaken for one.
	static constexpr float kClickSlop{ 4.0f };

	// Half the arm of the cross drawn for a selected entity with no sprite. It
	// is deliberately bigger than the gizmo's centre square, which lands on the
	// same point -- but the gizmo's arrows run along the cross's +X and +Y arms,
	// so what actually shows is the marker's other two halves. That is enough
	// for what it is for: saying that the selected thing is here, and has no
	// size, when there is no rectangle to outline.
	static constexpr float kCrossSize{ 12.0f };

	static constexpr ImU32 kActiveOutline{ IM_COL32(255, 176, 0, 255) };
	static constexpr ImU32 kOutline{ IM_COL32(190, 190, 190, 255) };

	// Drawn under the colour and one pixel wider on each side, so the outline
	// reads against content of its own colour as well as against the scene.
	static constexpr ImU32 kOutlineHalo{ IM_COL32(0, 0, 0, 200) };

	EditorCamera* m_Camera{ nullptr };
	Gizmo m_Gizmo;
	rse::Framebuffer m_Framebuffer;

	// Latched on middle-press and cleared on release, so a pan survives the
	// cursor leaving the panel mid-drag.
	bool m_Panning{ false };

	// The same latch for the left button, so a click can be judged by how far
	// the cursor travelled rather than by whether it is still over the image.
	bool m_ClickArmed{ false };
	ImVec2 m_ClickPos{ 0.0f, 0.0f };
};
