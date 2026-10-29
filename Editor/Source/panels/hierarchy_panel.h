#pragma once

#include <core.h>
#include <editor_manager.h>
#include <imgui.h>

#include <layout/editor_panel.h>

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
  DockSlot Slot() const override { return DockSlot::kLeft; }

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
    m_filter.Draw("##hierarchy_filter", -1.0f);
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
    uint32_t to_destroy = INVALID_ENTITY_ID;

    // Counted up front so the toolbar can report it without a second pass.
    uint32_t visible_count = 0;

    rse::ECSManager::GetView<BasicComponent>().each([&](auto entity, BasicComponent& basic) {
      const uint32_t entity_id = static_cast<uint32_t>(entity);

      if (!m_filter.PassFilter(basic.m_name.c_str())) {
        return;
      }

      ++visible_count;

      ImGui::PushID(static_cast<int>(entity_id));

      ImGuiTreeNodeFlags flags =
          ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;

      if (entity_id == selected) {
        flags |= ImGuiTreeNodeFlags_Selected;
      }

      // A disabled entity stays in the list so it can be selected and turned
      // back on, but it is greyed out and marked: it is not in the scene the
      // viewport is drawing or the game is running.
      const bool enabled = rse::ECSManager::IsEntityEnabled(entity_id);

      if (!enabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
      }

      // The id is shown next to the name because it is the only way to tell
      // two same-named entities apart while debugging.
      if (enabled) {
        ImGui::TreeNodeEx("##entity", flags, "%s  (%u)", basic.m_name.c_str(), entity_id);
      } else {
        ImGui::TreeNodeEx("##entity", flags, "%s  (%u)  (disabled)", basic.m_name.c_str(), entity_id);
      }

      if (!enabled) {
        ImGui::PopStyleColor();
      }

      if (ImGui::IsItemClicked()) {
        clicked = entity_id;
      }

      // Double-click frames it, the same thing F does in the viewport.
      if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        EditorManager::RequestFrameSelection();
      }

      if (ImGui::BeginPopupContextItem("##entity_context")) {
        if (ImGui::MenuItem("Delete", "Del")) {
          to_destroy = entity_id;
        }
        ImGui::EndPopup();
      }

      ImGui::PopID();
    });

    // Delete key works on the selection, unless a text field wants the input.
    const ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !io.WantTextInput &&
        ImGui::IsKeyPressed(ImGuiKey_Delete)) {
      to_destroy = selected;
    }

    // Enter a context menu target into the selection so the inspector follows
    // what is about to be acted on.
    if (clicked != INVALID_ENTITY_ID) {
      EventManager::GetEventDispatcher()->trigger<EditorManager::SelectEntity>(clicked);
    }

    if (to_destroy != INVALID_ENTITY_ID) {
      EventManager::GetEventDispatcher()->trigger<EditorManager::DestroyEntity>(to_destroy);
    }

    if (visible_count == 0) {
      ImGui::TextDisabled("%s", m_filter.IsActive() ? "No entity matches the filter." : "Scene is empty.");
    }

    ImGui::EndChild();
  }

  ImGuiTextFilter m_filter;
};
