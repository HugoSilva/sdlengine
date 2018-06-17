#include "ImguiDialog.h"

#include <imgui.h>

ImguiDialog::ImguiDialog() : Renderable2D(glm::vec3(0), glm::vec2(0), 0)
{
}

void ImguiDialog::submit(graphics::Renderer2D* renderer) const
{
	ImGui::Begin("Most recent projects");
	if (ImGui::Button("New"))
	{
	}
	if (ImGui::Button("Open"))
	{
	}
	if (ImGui::Button("Close"))
	{
	}
	ImGui::End();
}
