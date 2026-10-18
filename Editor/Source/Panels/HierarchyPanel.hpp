#pragma once

#include <imgui.h>
#include <Core.h>

#include "../Layout/EditorPanel.hpp"
#include "../EditorManager.h"

// Entity list for the current scene. Replaces the old floating "Scene Entities"
// window.
class HierarchyPanel : public EditorPanel
{
public:
	const char* id() const override { return "Hierarchy"; }
	DockSlot slot() const override { return DockSlot::Left; }

	void draw() override
	{
		if (!ImGui::Begin(id(), &visible))
		{
			ImGui::End();
			return;
		}

		static int selection_mask = (1 << 2); // Dumb representation of what may be user-side selection state. You may carry selection state inside or outside your objects in whatever format you see fit.
		int node_clicked = -1;                // Temporary storage of what node we have clicked to process selection at the end of the loop. May be a pointer to your own node type, etc.
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3); // Increase spacing to differentiate leaves from expanded contents.

		rse::ECSManager::getView<BasicComponent>().each([&](auto entity, BasicComponent& basic)
		{
			// Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
			ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((selection_mask & (1 << entity)) ? ImGuiTreeNodeFlags_Selected : 0);

			node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
			ImGui::TreeNodeEx((void*)(intptr_t)entity, node_flags, basic.m_Name.c_str());
			if (ImGui::IsItemClicked())
			{
				node_clicked = entity;
				EventManager::getEventDispatcher()->trigger<EditorManager::SelectEntity>(entity);
			}
		});

		if (node_clicked != -1)
		{
			// Update selection state. Process outside of tree loop to avoid visual inconsistencies during the clicking-frame.
			if (ImGui::GetIO().KeyCtrl)
				selection_mask ^= (1 << node_clicked);          // CTRL+click to toggle
			else
				selection_mask = (1 << node_clicked);           // Click to single-select
		}
		ImGui::PopStyleVar();

		if (ImGui::Button("Create"))
		{
			EventManager::getEventDispatcher()->trigger<EditorManager::CreateEntity>();
		}

		ImGui::End();
	}
};
