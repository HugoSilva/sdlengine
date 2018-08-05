#include "ImguiMenuBar.h"

#include <imgui.h>
#include <imgui_internal.h>

ImguiMenuBar::ImguiMenuBar()
	: Renderable2D(glm::vec3(0), glm::vec2(0), 0)
{
}

void ImguiMenuBar::submit(graphics::Renderer2D* renderer) const
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
			static bool read_only = false;
			ImGui::Checkbox("Scene Viewer", &read_only);
			ImGui::Checkbox("Entity Components", &read_only);
			ImGui::EndMenu();
		}

		ImGui::SameLine(ImGui::GetWindowWidth() - 160);
		ImGui::Text("%.2f FPS (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

		ImGui::EndMainMenuBar();
	}

	if (ImGui::GetIO().MouseClicked[1] || opened)
	{
		//TODO change the const to open menus update func maybe?
		//opened = opened ? false : true;

		if(opened)
			ImGui::OpenPopup("FilePopup");

		if (ImGui::BeginPopup("FilePopup"))
		{
			ImGui::Button("Test");
			ImGui::EndPopup();
		}
	}
}