#pragma once

#include <imgui.h>
#include <Core.h>

#include "../../EditorManager.h" // for events

class EntityViewerComponent
{
public:
	EntityViewerComponent() { }

	void submit(graphics::Renderer2D* renderer) const
	{
		bool* bOpen = EditorManager::getShowSceneWidget();
		if (!*bOpen)
		{
			return;
		}
		ImGuiWindowFlags window_flags = 0;
		//window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;

		ImGui::SetNextWindowSize(ImVec2(250, 500), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Scene Entities", bOpen, window_flags))
		{
			std::vector<int> entities = ecs::ECSManager::getCurrentEntities();

			static int selection_mask = (1 << 2); // Dumb representation of what may be user-side selection state. You may carry selection state inside or outside your objects in whatever format you see fit.
			int node_clicked = -1;                // Temporary storage of what node we have clicked to process selection at the end of the loop. May be a pointer to your own node type, etc.
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3); // Increase spacing to differentiate leaves from expanded contents.
			for (int i = 0; i < (int)entities.size(); i++)
			{
				// Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
				ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((selection_mask & (1 << i)) ? ImGuiTreeNodeFlags_Selected : 0);

				node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
				ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "Entity ID: %d", entities.at(i));
				if (ImGui::IsItemClicked())
				{
					node_clicked = i;
					EventManager::getEventDispatcher()->trigger<EditorManager::SelectEntity>(entities.at(i));
				}
			}
			if (node_clicked != -1)
			{
				// Update selection state. Process outside of tree loop to avoid visual inconsistencies during the clicking-frame.
				if (ImGui::GetIO().KeyCtrl)
					selection_mask ^= (1 << node_clicked);          // CTRL+click to toggle
				else //if (!(selection_mask & (1 << node_clicked))) // Depending on selection behavior you want, this commented bit preserve selection when clicking on item that is part of the selection
					selection_mask = (1 << node_clicked);           // Click to single-select
			}
			ImGui::PopStyleVar();

			if (ImGui::Button("Create"))
			{
				EventManager::getEventDispatcher()->trigger<EditorManager::CreateEntity>();
			}

			ImGui::End();
		}
	}
};