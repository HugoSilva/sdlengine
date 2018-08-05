#include "ImguiCreate.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

ImguiCreate::ImguiCreate() : Renderable2D(glm::vec3(0), glm::vec2(0), 0)
{
}

void ImguiCreate::submit(graphics::Renderer2D* renderer) const
{
	ImGuiWindowFlags window_flags = 0;
	//window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;

	ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Entity Components", NULL, window_flags))
	{
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			static glm::vec3 pos{ 0.f, 0.f, 0.f };
			ImGui::InputFloat3("Position", glm::value_ptr(pos), 2);

			static glm::vec3 rot{ 0.f, 0.f, 0.f };
			ImGui::InputFloat3("Rotation", glm::value_ptr(rot), 2);

			static glm::vec3 sca{ 1.f, 1.f, 1.f };
			ImGui::InputFloat3("Scale", glm::value_ptr(sca), 2);
		}

		ImGui::End();
	}
}
