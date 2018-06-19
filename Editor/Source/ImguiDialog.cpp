#include "ImguiDialog.h"

#include <imgui.h>

ImguiDialog::ImguiDialog() : Renderable2D(glm::vec3(0), glm::vec2(0), 0)
{
}

void ImguiDialog::submit(graphics::Renderer2D* renderer) const
{
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;

	ImGui::SetNextWindowSize(ImVec2(500, 200), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Most recent projects", NULL, window_flags))
	{
		ImGui::Text("Recent Projects");
		ImGui::Columns(2, "recentProjects");
		ImGui::Separator();
		ImGui::Text("Name"); ImGui::NextColumn();
		ImGui::Text("Path"); ImGui::NextColumn();
		ImGui::Separator();
		const char* names[3] = { "One", "Two", "Three" };
		const char* paths[3] = { "/path/one", "/path/two", "/path/three" };
		static int selected = -1;
		for (int i = 0; i < 3; i++)
		{
			if (ImGui::Selectable(names[i], selected == i, ImGuiSelectableFlags_SpanAllColumns))
				selected = i;
			ImGui::NextColumn();

			ImGui::Text(paths[i]); ImGui::NextColumn();
		}
		ImGui::Columns(1);
		ImGui::Separator();

		if (ImGui::Button("New"))
		{
		}
		ImGui::SameLine();

		if (ImGui::Button("Open"))
		{
		}

		ImGui::End();
	}
}
