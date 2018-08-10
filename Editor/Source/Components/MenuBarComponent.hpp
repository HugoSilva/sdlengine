#pragma once

#include <imgui.h>
#include <imgui_internal.h>

#include "../../EditorManager.h"

class MenuBarComponent
{
public:

	MenuBarComponent()
	{
	}

	void submit(graphics::Renderer2D* renderer) const
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New"))
				{
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Quit", "Alt+F4")) {
					SDL_Event sdlevent;
					sdlevent.type = SDL_QUIT;
					SDL_PushEvent(&sdlevent);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
				if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "CTRL+X")) {}
				if (ImGui::MenuItem("Copy", "CTRL+C")) {}
				if (ImGui::MenuItem("Paste", "CTRL+V")) {}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Debug"))
			{
				//TODO read values from the editor main file
				bool* bSceneValue = EditorManager::getShowSceneWidget();
				bool* bEntityValue = EditorManager::getShowComponentWidget();
				ImGui::Checkbox("Scene Viewer", bSceneValue);
				ImGui::Checkbox("Entity Components", bEntityValue);
				ImGui::EndMenu();
			}

			ImGui::SameLine(ImGui::GetWindowWidth() - 160);
			ImGui::Text("%.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

			ImGui::EndMainMenuBar();
		}
	}

	bool opened{ false };
};
