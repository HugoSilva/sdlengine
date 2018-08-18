#pragma once

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "../../EditorManager.h"
#include "Core.h"
#include "MenuBarComponent.hpp"

class PropertyComponent : public DebugElement
{
public:
	PropertyComponent() { }

	void render() const
	{
		bool* bOpen = EditorManager::getShowComponentWidget();
		if (!*bOpen)
		{
			return;
		}
		ImGuiWindowFlags window_flags = 0;
		//window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Entity Components", bOpen, window_flags))
		{
			if (EditorManager::showComponentWidget<TransformComponent>())
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
			}

			ImGui::End();
		}
	}
};