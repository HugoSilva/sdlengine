#pragma once

// ---------------------------------------------------------------------------
// Gizmo -- the on-screen handles for moving the selection.
//
// Hand-rolled rather than ImGuizmo (a decision recorded in the plan): the
// handles are a handful of lines and triangles in the panel's draw list and the
// hit test is a rectangle test, which is a fraction of the machinery a
// third-party gizmo brings along -- and it draws through the same camera and
// the same draw list as the outline it sits next to.
//
// The handles are drawn at a *constant screen size*: 60 px of arm whatever the
// zoom, because a gizmo that shrinks with the view is unusable exactly when it
// is needed most. What that does not mean is that a drag is measured in world
// units. A drag is "the cursor moved 34 pixels along the arrow", and what world
// offset that corresponds to is the camera's business, not the gizmo's.
//
// Every frame of a drag is computed from where the transform was when the drag
// started rather than accumulated frame to frame: a latched drag that leaves
// the panel and comes back has to land where the cursor is, not where the sum
// of the deltas happened to end up.
//
// M5 draws and drags the translate handles. The drag plumbing here -- the latch,
// the grab snapshot, the axis projection, the snapping -- is deliberately the
// shared part, because the rotate and scale handles need all of it.
// ---------------------------------------------------------------------------

#include <optional>
#include <vector>

#include <imgui.h>
#include <glm/glm.hpp>
#include <Core.h>

#include "../Camera/EditorCamera.hpp"
#include "../Scene/EntityBounds.hpp"
#include "../EditorManager.h"

class Gizmo
{
public:
	// Which handle a drag is attached to. Both is the centre square, which moves
	// in the plane rather than along one axis.
	enum class Axis { None, X, Y, Both };

	// Reads the mouse for this frame and returns true when a handle owns the
	// press, so the viewport knows the click was not aimed at the scene.
	bool handleInput(const EditorCamera& camera, const ImVec2& imagePos, bool hovered);

	void draw(const EditorCamera& camera, const ImVec2& imagePos) const;

private:
	// Screen pixels. The arrows are the same size at every zoom by design.
	static constexpr float kArmLength{ 60.0f };
	static constexpr float kArrowLength{ 12.0f };
	static constexpr float kArrowHalfWidth{ 5.0f };

	// Half the thickness of an axis's grab band, and half the width of the
	// centre square. Generous enough to hit without aiming.
	static constexpr float kGrabWidth{ 6.0f };
	static constexpr float kCentreSize{ 6.0f };

	// Axis colours, dim and lit; lit means hovered, or being dragged.
	static constexpr ImU32 kXDim{ IM_COL32(198, 62, 62, 255) };
	static constexpr ImU32 kXLit{ IM_COL32(255, 128, 128, 255) };
	static constexpr ImU32 kYDim{ IM_COL32(78, 186, 78, 255) };
	static constexpr ImU32 kYLit{ IM_COL32(140, 255, 140, 255) };
	static constexpr ImU32 kPlaneDim{ IM_COL32(228, 228, 228, 255) };
	static constexpr ImU32 kPlaneLit{ IM_COL32(255, 240, 150, 255) };

	// Drawn under every part of the gizmo, for the reason the selection outline
	// has one: the scene behind it is free to be any colour, including the
	// gizmo's own.
	static constexpr ImU32 kHalo{ IM_COL32(0, 0, 0, 190) };

	// A transform as it was when the drag started.
	struct Grabbed
	{
		uint32_t entity{ INVALID_ENTITY_ID };
		glm::vec3 position{ 0.0f };
	};

	// Where the handles sit, in screen pixels. Fails when the selection has no
	// position at all, which is also when there is no gizmo to draw.
	bool anchor(const EditorCamera& camera, const ImVec2& imagePos, ImVec2& out) const;

	Axis hitTest(glm::vec2 mouse, const ImVec2& origin) const;

	void beginDrag(glm::vec2 mouse, Axis axis);
	void applyDrag(const EditorCamera& camera);

	Axis m_Active{ Axis::None };
	Axis m_Hovered{ Axis::None };

	// The cursor where the drag started, in ImGui pixels. The offset applied is
	// always measured from here.
	glm::vec2 m_GrabScreen{ 0.0f };
	std::vector<Grabbed> m_Grabbed;
};

inline bool Gizmo::anchor(const EditorCamera& camera, const ImVec2& imagePos, ImVec2& out) const
{
	// The active entity anchors the drag; with one entity selected that is the
	// only one there is.
	const std::optional<glm::vec2> position = EditorBounds::positionOf(EditorManager::getSelectedEntity());

	if (!position)
	{
		return false;
	}

	const glm::vec2 screen = camera.worldToScreen(*position);
	out = ImVec2(imagePos.x + screen.x, imagePos.y + screen.y);
	return true;
}

inline Gizmo::Axis Gizmo::hitTest(glm::vec2 mouse, const ImVec2& origin) const
{
	const glm::vec2 at(origin.x, origin.y);

	// The centre square is tested first: it sits on top of both axes where they
	// meet, and it is the handle most likely to be aimed at.
	if (rse::pointInBox(mouse, at - glm::vec2(kCentreSize), at + glm::vec2(kCentreSize)))
	{
		return Axis::Both;
	}

	// The bands run the length of the arrows, including the heads, so the whole
	// thing is grabbable and not just the shaft.
	const glm::vec2 xEnd(at.x + kArmLength + kArrowLength, at.y);
	if (rse::pointInBox(mouse, glm::vec2(at.x, at.y - kGrabWidth), glm::vec2(xEnd.x, at.y + kGrabWidth)))
	{
		return Axis::X;
	}

	const glm::vec2 yEnd(at.x, at.y - kArmLength - kArrowLength);
	if (rse::pointInBox(mouse, glm::vec2(at.x - kGrabWidth, yEnd.y), glm::vec2(at.x + kGrabWidth, at.y)))
	{
		return Axis::Y;
	}

	return Axis::None;
}

inline void Gizmo::beginDrag(glm::vec2 mouse, Axis axis)
{
	m_Active = axis;
	m_GrabScreen = mouse;
	m_Grabbed.clear();

	for (const uint32_t entity : EditorManager::getSelectedEntities())
	{
		if (!rse::ECSManager::hasComponent<TransformComponent>(entity))
		{
			continue;
		}

		m_Grabbed.push_back({ entity, rse::ECSManager::getComponent<TransformComponent>(entity).position });
	}
}

inline void Gizmo::applyDrag(const EditorCamera& camera)
{
	const ImGuiIO& io = ImGui::GetIO();
	const glm::vec2 travel(io.MousePos.x - m_GrabScreen.x, io.MousePos.y - m_GrabScreen.y);

	const bool dragX = m_Active == Axis::X || m_Active == Axis::Both;
	const bool dragY = m_Active == Axis::Y || m_Active == Axis::Both;

	// Screen y grows downwards and world y grows upwards, so a drag upwards is
	// a positive move; an axis that is not being dragged contributes nothing,
	// which is what makes the handle a constraint rather than a suggestion.
	const glm::vec2 projected(dragX ? travel.x : 0.0f, dragY ? -travel.y : 0.0f);
	const glm::vec2 offset = projected * camera.getWorldPerPixel();

	const float step = EditorManager::settings().grid.base;
	const bool snapping = io.KeyShift;

	for (const Grabbed& grabbed : m_Grabbed)
	{
		TransformComponent& transform = rse::ECSManager::getComponent<TransformComponent>(grabbed.entity);

		glm::vec3 moved = grabbed.position;
		moved.x += offset.x;
		moved.y += offset.y;

		if (snapping)
		{
			// Only the axes being dragged are snapped: the other one is where
			// it was, and rounding it would move the entity sideways.
			if (dragX)
			{
				moved.x = rse::snapToStep(moved.x, step);
			}

			if (dragY)
			{
				moved.y = rse::snapToStep(moved.y, step);
			}
		}

		transform.position = moved;
	}
}

inline bool Gizmo::handleInput(const EditorCamera& camera, const ImVec2& imagePos, bool hovered)
{
	ImVec2 origin;

	if (!anchor(camera, imagePos, origin))
	{
		// Nothing to move. Any drag in flight is dropped rather than carried on:
		// the entity it was moving may be the one that just went away.
		m_Active = Axis::None;
		m_Hovered = Axis::None;
		m_Grabbed.clear();
		return false;
	}

	const ImGuiIO& io = ImGui::GetIO();

	if (m_Active != Axis::None)
	{
		if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			// Released. One last apply, in case the cursor moved and let go
			// between frames, and then the drag is over.
			applyDrag(camera);
			m_Active = Axis::None;
			m_Grabbed.clear();
			return false;
		}

		applyDrag(camera);
		return true;
	}

	// Hovering is only meaningful while the cursor is over the image: the
	// handles are drawn there, and off the panel they are not on screen.
	m_Hovered = hovered ? hitTest(glm::vec2(io.MousePos.x, io.MousePos.y), origin) : Axis::None;

	if (m_Hovered != Axis::None && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		beginDrag(glm::vec2(io.MousePos.x, io.MousePos.y), m_Hovered);
		return true;
	}

	return false;
}

inline void Gizmo::draw(const EditorCamera& camera, const ImVec2& imagePos) const
{
	ImVec2 origin;

	if (!anchor(camera, imagePos, origin))
	{
		return;
	}

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	// Being dragged beats being hovered: once the pointer is latched it can be
	// anywhere, and the axis being moved is the one that should be lit.
	const Axis lit = m_Active != Axis::None ? m_Active : m_Hovered;

	const ImU32 xColour = lit == Axis::X || lit == Axis::Both ? kXLit : kXDim;
	const ImU32 yColour = lit == Axis::Y || lit == Axis::Both ? kYLit : kYDim;
	const ImU32 planeColour = lit == Axis::Both ? kPlaneLit : kPlaneDim;

	const ImVec2 xTip(origin.x + kArmLength, origin.y);
	const ImVec2 yTip(origin.x, origin.y - kArmLength);
	const ImVec2 xHead(xTip.x + kArrowLength, xTip.y);
	const ImVec2 yHead(yTip.x, yTip.y - kArrowLength);

	const auto stroke = [drawList](const ImVec2& from, const ImVec2& to, ImU32 colour)
	{
		drawList->AddLine(from, to, kHalo, 4.0f);
		drawList->AddLine(from, to, colour, 2.0f);
	};

	const auto head = [drawList](const ImVec2& tip, const ImVec2& left, const ImVec2& right, ImU32 colour)
	{
		drawList->AddTriangleFilled(tip, left, right, colour);
		drawList->AddTriangle(tip, left, right, kHalo, 2.0f);
	};

	stroke(origin, xTip, xColour);
	stroke(origin, yTip, yColour);

	head(xHead, ImVec2(xTip.x, xTip.y - kArrowHalfWidth), ImVec2(xTip.x, xTip.y + kArrowHalfWidth), xColour);
	head(yHead, ImVec2(yTip.x + kArrowHalfWidth, yTip.y), ImVec2(yTip.x - kArrowHalfWidth, yTip.y), yColour);

	// The centre square, for dragging in the plane: a halo with the fill inset
	// into it, which is a border without needing two rectangles of the same
	// size drawn in the right order.
	const ImVec2 squareMin(origin.x - kCentreSize, origin.y - kCentreSize);
	const ImVec2 squareMax(origin.x + kCentreSize, origin.y + kCentreSize);

	drawList->AddRectFilled(squareMin, squareMax, kHalo);
	drawList->AddRectFilled(ImVec2(squareMin.x + 2.0f, squareMin.y + 2.0f), ImVec2(squareMax.x - 2.0f, squareMax.y - 2.0f), planeColour);
}
