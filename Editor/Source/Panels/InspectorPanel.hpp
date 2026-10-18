#pragma once

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "../Layout/EditorPanel.hpp"
#include "../EditorManager.h"
#include "Core.h"

// Component inspector for the selected entity. Replaces the old floating
// "Entity Components" window.
class InspectorPanel : public EditorPanel
{
public:
	const char* id() const override { return "Inspector"; }
	DockSlot slot() const override { return DockSlot::Right; }

	void draw() override
	{
		if (!ImGui::Begin(id(), &visible))
		{
			ImGui::End();
			return;
		}

		if (EditorManager::showComponentWidget<BasicComponent>())
		{
			if (ImGui::CollapsingHeader("Basic", ImGuiTreeNodeFlags_DefaultOpen))
			{
				BasicComponent& basicData = EditorManager::getComponent<BasicComponent>();

				char str0[128] = "";
				basicData.m_Name.copy(str0, IM_ARRAYSIZE(str0) - 1);

				if (ImGui::InputText("##name", str0, IM_ARRAYSIZE(str0)))
				{
					basicData.m_Name = str0;
				}
				ImGui::SameLine();
				ImGui::Checkbox("Static", &basicData.m_IsStatic);
			}
		}
		if (EditorManager::showComponentWidget<TransformComponent>())
		{
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				TransformComponent& transform = EditorManager::getComponent<TransformComponent>();

				ImGui::InputFloat3("Position", glm::value_ptr(transform.position), "%.2f");

				ImGui::InputFloat3("Rotation", glm::value_ptr(transform.rotation), "%.2f");

				ImGui::InputFloat3("Scale", glm::value_ptr(transform.scale), "%.2f");
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
			if (ImGui::CollapsingHeader("Audio", ImGuiTreeNodeFlags_DefaultOpen))
			{
				AudioComponent& audio = EditorManager::getComponent<AudioComponent>();

				char str0[128] = "";

				strcpy_s(str0, audio.m_Sound->getFileName().c_str());

				ImGui::InputText("Sound", str0, IM_ARRAYSIZE(str0));

				ImGui::Checkbox("Mute", audio.m_Sound->getMute());
				ImGui::Checkbox("Play on play", audio.m_Sound->getPlayTrigger());
				ImGui::Checkbox("Loop", audio.m_Sound->getLoop());
			}
		}
		if (EditorManager::showComponentWidget<RigidBodyComponent>())
		{
			if (ImGui::CollapsingHeader("Rigidbody", ImGuiTreeNodeFlags_DefaultOpen))
			{
				RigidBodyComponent& rigidBody = EditorManager::getComponent<RigidBodyComponent>();

				ImGui::InputFloat("Density", rigidBody.m_Rigidbody->getDensity());
				ImGui::InputFloat("Friction", rigidBody.m_Rigidbody->getFriction());

				const char* items[] = { "b2_staticBody", "b2_kinematicBody", "b2_dynamicBody" };
				int* type = (int*)rigidBody.m_Rigidbody->getBodyType();
				ImGui::Combo("Body Type", type, items, IM_ARRAYSIZE(items));
			}
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
};
