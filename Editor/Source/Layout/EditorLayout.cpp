#include "EditorLayout.hpp"

#include <imgui_internal.h> // DockBuilder*
#include <SDL3/SDL.h>

#include <Core.h>

namespace
{
	constexpr const char* kDockSpaceId = "RSE_DockSpace";

	// The host window is never visible: it just carries the dock space, so it
	// has no title bar and no background of its own.
	constexpr const char* kDockSpaceHost = "###RSE_DockSpaceHost";
}

void EditorLayout::addPanel(std::unique_ptr<EditorPanel> panel)
{
	m_Panels.push_back(std::move(panel));
}

void EditorLayout::draw()
{
	// The scene target is deliberately *not* reset here. The viewport panel owns
	// it and publishes it whenever it is given the chance to draw, so hiding or
	// collapsing the panel keeps the scene offscreen instead of letting it fall
	// through to the window -- which the dock space, having no background of its
	// own, would then show through.

	// The menu bar has to be submitted first: it is what shrinks the viewport
	// work area that the dock space then fills.
	drawMenuBar();
	drawDockSpace();

	for (const std::unique_ptr<EditorPanel>& panel : m_Panels)
	{
		if (panel->visible)
		{
			panel->draw();
		}
	}
}

void EditorLayout::drawMenuBar()
{
	if (!ImGui::BeginMainMenuBar())
	{
		return;
	}

	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("New")) { }
		ImGui::Separator();
		if (ImGui::MenuItem("Save", "F1")) { rse::ECSManager::save(); }
		if (ImGui::MenuItem("Load", "F2")) { rse::ECSManager::load(); }
		ImGui::Separator();
		if (ImGui::MenuItem("Quit", "Alt+F4"))
		{
			SDL_Event sdlevent;
			sdlevent.type = SDL_EVENT_QUIT;
			SDL_PushEvent(&sdlevent);
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
		if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}
		ImGui::Separator();
		if (ImGui::MenuItem("Cut", "CTRL+X")) {}
		if (ImGui::MenuItem("Copy", "CTRL+C")) {}
		if (ImGui::MenuItem("Paste", "CTRL+V")) {}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("View"))
	{
		for (const std::unique_ptr<EditorPanel>& panel : m_Panels)
		{
			ImGui::MenuItem(panel->id(), nullptr, &panel->visible);
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Reset Layout"))
		{
			requestResetLayout();
		}
		ImGui::EndMenu();
	}

	ImGui::SameLine(ImGui::GetWindowWidth() - 160);
	ImGui::Text("%.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

	ImGui::EndMainMenuBar();
}

void EditorLayout::drawDockSpace()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	const ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin(kDockSpaceHost, nullptr, flags);
	ImGui::PopStyleVar(3);

	const ImGuiID dockspaceId = ImGui::GetID(kDockSpaceId);

	// An existing dock node means imgui.ini supplied one, so the user's layout
	// is left alone. Only build the default when there is nothing to restore,
	// or when a reset was requested.
	const bool hasSavedLayout = ImGui::DockBuilderGetNode(dockspaceId) != nullptr;
	if (m_ResetLayout || (!m_LayoutBuilt && !hasSavedLayout))
	{
		buildDefaultLayout(dockspaceId);
		m_ResetLayout = false;
	}
	m_LayoutBuilt = true;

	ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	ImGui::End();
}

void EditorLayout::buildDefaultLayout(ImGuiID dockspaceId)
{
	// Viewport fills the centre, the console spans the bottom, the hierarchy
	// column sits on the left and the inspector hugs the right edge. Which panel
	// asks for which slot is up to the panel (EditorPanel::slot()).
	bool needsLeft = false;
	bool needsRight = false;
	bool needsBottom = false;

	for (const std::unique_ptr<EditorPanel>& panel : m_Panels)
	{
		switch (panel->slot())
		{
			case DockSlot::Left:   needsLeft = true; break;
			case DockSlot::Right:  needsRight = true; break;
			case DockSlot::Bottom: needsBottom = true; break;
			case DockSlot::Center: break;
		}
	}

	const ImGuiViewport* viewport = ImGui::GetMainViewport();

	ImGui::DockBuilderRemoveNode(dockspaceId);
	ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);

	ImGuiID center = dockspaceId;
	ImGuiID left = 0;
	ImGuiID right = 0;
	ImGuiID bottom = 0;

	// The bottom strip is split off first so it spans the full width rather than
	// being boxed into one of the columns.
	if (needsBottom)
	{
		bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.25f, nullptr, &center);
	}
	if (needsLeft)
	{
		left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.22f, nullptr, &center);
	}
	if (needsRight)
	{
		right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.32f, nullptr, &center);
	}

	for (const std::unique_ptr<EditorPanel>& panel : m_Panels)
	{
		ImGuiID target = center;

		switch (panel->slot())
		{
			case DockSlot::Left:   target = left;   break;
			case DockSlot::Right:  target = right;  break;
			case DockSlot::Bottom: target = bottom; break;
			case DockSlot::Center: target = center; break;
		}

		if (target != 0)
		{
			ImGui::DockBuilderDockWindow(panel->id(), target);
		}
	}

	ImGui::DockBuilderFinish(dockspaceId);
}
