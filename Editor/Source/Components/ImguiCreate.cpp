#include "ImguiCreate.h"

#include <imgui.h>

ImguiCreate::ImguiCreate() : Renderable2D(glm::vec3(0), glm::vec2(0), 0)
{
}

void ImguiCreate::submit(graphics::Renderer2D* renderer) const
{
	ImGuiWindowFlags window_flags = 0;
	//window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;

	ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Scene Viewer", NULL, window_flags))
	{
		static char name[64] = "Name";
		ImGui::Text("Project Name:");
		ImGui::InputText("##name", name, IM_ARRAYSIZE(name));

		static char path[64] = "C:\\Projects\\";
		ImGui::Text("Project Path:");
		ImGui::InputText("##path", path, IM_ARRAYSIZE(path));

		if (ImGui::Button("Cancel"))
		{
		}
		ImGui::SameLine();

		if (ImGui::Button("Create"))
		{
		}

		ImGui::End();
	}
}
