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
			if (EditorManager::showComponentWidget<BasicComponent>())
			{
				if (ImGui::CollapsingHeader("Basic", ImGuiTreeNodeFlags_DefaultOpen))
				{
					BasicComponent& basicData = EditorManager::getComponent<BasicComponent>();

					char str0[128] = "";
					
					strcpy_s(str0, basicData.m_Name);

					ImGui::InputText("", str0, IM_ARRAYSIZE(str0));
					ImGui::SameLine();
					ImGui::Checkbox("Static", &basicData.m_IsStatic);
				}
			}
			if (EditorManager::showComponentWidget<TransformComponent>())
			{
				if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
				{
					TransformComponent& transform = EditorManager::getComponent<TransformComponent>();

					ImGui::InputFloat3("Position", glm::value_ptr(transform.position), 2);

					ImGui::InputFloat3("Rotation", glm::value_ptr(transform.rotation), 2);

					ImGui::InputFloat3("Scale", glm::value_ptr(transform.scale), 2);
				}
			}

			ImGui::End();
		}
	}
};