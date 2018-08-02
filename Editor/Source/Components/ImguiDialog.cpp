#include "ImguiDialog.h"

#include <imgui.h>
#include <Core.h>

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
			EventManager::Dispatch("CreateProject");
		}
		ImGui::SameLine();

		if (ImGui::Button("Open"))
		{
		}


		if (ImGui::TreeNode("Trees"))
		{
			if (ImGui::TreeNode("Basic trees"))
			{
				for (int i = 0; i < 5; i++)
					if (ImGui::TreeNode((void*)(intptr_t)i, "Child %d", i))
					{
						ImGui::Text("blah blah");
						ImGui::SameLine();
						if (ImGui::SmallButton("button")) {};
						ImGui::TreePop();
					}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Advanced, with Selectable nodes"))
			{
				ImGui::Text("Hello!");

				static int selection_mask = (1 << 2); // Dumb representation of what may be user-side selection state. You may carry selection state inside or outside your objects in whatever format you see fit.
				int node_clicked = -1;                // Temporary storage of what node we have clicked to process selection at the end of the loop. May be a pointer to your own node type, etc.
				ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3); // Increase spacing to differentiate leaves from expanded contents.
				for (int i = 0; i < 6; i++)
				{
					// Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
					ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((selection_mask & (1 << i)) ? ImGuiTreeNodeFlags_Selected : 0);
					if (i < 3)
					{
						// Node
						bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "Selectable Node %d", i);
						if (ImGui::IsItemClicked())
							node_clicked = i;
						if (node_open)
						{
							ImGui::Text("Blah blah\nBlah Blah");
							ImGui::TreePop();
						}
					}
					else
					{
						// Leaf: The only reason we have a TreeNode at all is to allow selection of the leaf. Otherwise we can use BulletText() or TreeAdvanceToLabelPos()+Text().
						node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
						ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "Selectable Leaf %d", i);
						if (ImGui::IsItemClicked())
							node_clicked = i;
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

				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
	ImGui::End();
	}
}
