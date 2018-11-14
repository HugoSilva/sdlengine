#pragma once

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "../../EditorManager.h"
#include "Core.h"

class PropertyComponent : public EditorElement
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
			if (EditorManager::showComponentWidget<SpriteComponent>())
			{
				if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
				{
					SpriteComponent& sprite = EditorManager::getComponent<SpriteComponent>();

					const unsigned int& color = sprite.m_Sprite->getColor();
					
					SDL_Color sdlColor = { static_cast<unsigned char>(color), static_cast<unsigned char>(color >> 8), static_cast<unsigned char>(color >> 16), static_cast<unsigned char>(color >> 24) };

					ImVec4 vec4Color = ImVec4(sdlColor.r / 255.f, sdlColor.g / 255.f, sdlColor.b / 255.f, sdlColor.a / 255.f);

					ImGui::ColorEdit4("Color", (float *)&vec4Color);
					
					char str0[128] = "";

					strcpy_s(str0, sprite.m_Sprite->getTextureName().c_str());

					ImGui::InputText("Texture", str0, IM_ARRAYSIZE(str0));
				}
			}
			if (EditorManager::showComponentWidget<AudioComponent>())
			{
				ImGui::Separator();
				ImGui::Button("Audio");
			}
			if (EditorManager::showComponentWidget<RigidBodyComponent>())
			{
				ImGui::Separator();
				ImGui::Button("Rigid");
			}

			if (EditorManager::showComponentWidget<BasicComponent>())
			{
				ImGui::Separator();

				if (ImGui::Button("Add Component"))
					ImGui::OpenPopup("toggle");
				if (ImGui::BeginPopup("toggle"))
				{
					if (ImGui::MenuItem("Audio"))
					{
						EventManager::getEventDispatcher()->trigger<EditorManager::AddComponent>(eComponents::Audio);
					}
					if (ImGui::MenuItem("Ridigbody"))
					{
						EventManager::getEventDispatcher()->trigger<EditorManager::AddComponent>(eComponents::Rigidbody);
					}
					if (ImGui::MenuItem("Sprite"))
					{
						EventManager::getEventDispatcher()->trigger<EditorManager::AddComponent>(eComponents::Sprite);
					}
					ImGui::EndPopup();
				}
			}

			ImGui::End();
		}
	}
};