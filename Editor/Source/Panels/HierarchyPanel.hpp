#pragma once

#include <Core.h>
#include <imgui.h>

#include "../EditorManager.h"
#include "../Layout/EditorPanel.hpp"

// ---------------------------------------------------------------------------
// Entity list for the current scene.
//
// Selection lives in EditorManager, not in this panel: the old version kept a
// local `int selection_mask` and tested `1 << entity`, which stopped working
// past entity id 31 and disagreed with what the inspector showed. Reading the
// shared selection is both correct and what makes the two panels agree.
// ---------------------------------------------------------------------------
class HierarchyPanel : public EditorPanel {
 public:
  const char* Id() const override { return "Hierarchy"; }
  DockSlot Slot() const override { return DockSlot::Left; }

  void Draw() override {
    if (!ImGui::Begin(Id(), &visible)) {
      ImGui::End();
      return;
    }

    DrawToolbar();
    ImGui::Separator();
    DrawEntityList();

    ImGui::End();
  }

 private:
  void DrawToolbar() {
    if (ImGui::Button("Create")) {
      EventManager::GetEventDispatcher()->trigger<EditorManager::CreateEntity>();
    }

    ImGui::SameLine();
    m_Filter.Draw("##hierarchy_filter", -1.0f);
  }

  void DrawEntityList() {
    // A child region keeps the list scrolling instead of the window, so the
    // toolbar stays put.
    if (!ImGui::BeginChild("##hierarchy_entities", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None,
                           ImGuiWindowFlags_HorizontalScrollbar)) {
      ImGui::EndChild();
      return;
    }

    const uint32_t selected = EditorManager::GetSelectedEntity();

    uint32_t clicked = INVALID_ENTITY_ID;
    uint32_t toDestroy = INVALID_ENTITY_ID;

    // Counted up front so the toolbar can report it without a second pass.
    uint32_t visibleCount = 0;

    rse::ECSManager::GetView<BasicComponent>().each([&](auto entity, BasicComponent& basic) {
      const uint32_t entityId = static_cast<uint32_t>(entity);

      if (!m_Filter.PassFilter(basic.m_Name.c_str())) {
        return;
      }

      ++visibleCount;

      ImGui::PushID(static_cast<int>(entityId));

      ImGuiTreeNodeFlags flags =
          ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;

      if (entityId == selected) {
        flags |= ImGuiTreeNodeFlags_Selected;
      }

      // The id is shown next to the name because it is the only way to tell
      // two same-named entities apart while debugging.
      ImGui::TreeNodeEx("##entity", flags, "%s  (%u)", basic.m_Name.c_str(), entityId);

      if (ImGui::IsItemClicked()) {
        clicked = entityId;
      }

      // Double-click frames it, the same thing F does in the viewport.
      if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        EditorManager::RequestFrameSelection();
      }

      if (ImGui::BeginPopupContextItem("##entity_context")) {
        if (ImGui::MenuItem("Delete", "Del")) {
          toDestroy = entityId;
        }
        ImGui::EndPopup();
      }

      ImGui::PopID();
    });

    // Delete key works on the selection, unless a text field wants the input.
    const ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !io.WantTextInput &&
        ImGui::IsKeyPressed(ImGuiKey_Delete)) {
      toDestroy = selected;
    }

    // Enter a context menu target into the selection so the inspector follows
    // what is about to be acted on.
    if (clicked != INVALID_ENTITY_ID) {
      EventManager::GetEventDispatcher()->trigger<EditorManager::SelectEntity>(clicked);
    }

    if (toDestroy != INVALID_ENTITY_ID) {
      EventManager::GetEventDispatcher()->trigger<EditorManager::DestroyEntity>(toDestroy);
    }

    if (visibleCount == 0) {
      ImGui::TextDisabled("%s", m_Filter.IsActive() ? "No entity matches the filter." : "Scene is empty.");
    }

    ImGui::EndChild();
  }

  ImGuiTextFilter m_Filter;
};
