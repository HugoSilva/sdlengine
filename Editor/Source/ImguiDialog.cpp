#include "ImguiDialog.h"

#include <imgui.h>

ImguiDialog::ImguiDialog() : Renderable2D(glm::vec3(0), glm::vec2(0), 0)
{
}

void ImguiDialog::submit(graphics::Renderer2D* renderer) const
{
	if (ImGui::Begin("Most recent projects"))
	{
		ImGui::Columns(3, "mixed");

		if (ImGui::Button("New"))
		{
		}
		ImGui::NextColumn();

		if (ImGui::Button("Open"))
		{
		}
		ImGui::NextColumn();

		if (ImGui::Button("Close"))
		{
		}
		ImGui::NextColumn();

		ImGui::End();
	}
}
