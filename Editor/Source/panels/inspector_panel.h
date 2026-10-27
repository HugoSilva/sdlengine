#pragma once

#include <algorithm>
#include <core.h>
#include <cstdio>
#include <editor_manager.h>
#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h>
#include <project_commands.h>
#include <string>
#include <texture_catalog.h>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

#include <layout/editor_panel.h>
#include <utils/logger.h>

// ---------------------------------------------------------------------------
// Component inspector for the selected entity.
//
// The old version had two editing bugs worth calling out, both fixed here:
//
//   * the colour picker edited a local ImVec4 that was thrown away, so the
//     sprite never changed;
//   * the texture and sound fields copied a name into a 128 byte buffer with
//     strcpy_s and never read it back, so typing did nothing (and a long name
//     was a buffer overrun).
//
// The sound field is still read-only: the component holds a Sound handle, not
// a name. The sprite's texture is editable now, but not by typing a name into
// the component: the field is a search over the project's textures, and the
// result is written back as a borrowed Texture* from TextureManager.
// ---------------------------------------------------------------------------
class InspectorPanel : public EditorPanel {
 public:
  const char* Id() const override { return "Inspector"; }
  DockSlot Slot() const override { return DockSlot::kRight; }

  void ProjectChanged() override {
    // A different project is a different set of textures. The next popup
    // rebuilds the catalog from the new registry and directory.
    m_texture_catalog.clear();
    m_texture_edit_entity = INVALID_ENTITY_ID;
    m_texture_query.clear();
  }

  void Draw() override {
    if (!ImGui::Begin(Id(), &visible)) {
      ImGui::End();
      return;
    }

    if (EditorManager::GetSelectedEntity() == INVALID_ENTITY_ID) {
      ImGui::TextDisabled("Nothing selected.");
      ImGui::End();
      return;
    }

    DrawEntityHeader();
    DrawBasic();
    DrawTransform();
    DrawSprite();
    DrawAudio();
    DrawRigidbody();

    if (EditorManager::ShowComponentWidget<BasicComponent>()) {
      ImGui::Separator();
      DrawAddComponentMenu();
    }

    ImGui::End();
  }

 private:
  // Widget edits write straight into the open scene, so the title bar has to
  // start saying there is work to lose. Called right after the widget that may
  // have edited something.
  static void MarkDirtyIfEdited() {
    if (ImGui::IsItemEdited()) {
      SceneManager::MarkDirty();
    }
  }

  void DrawEntityHeader() {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Entity");
    ImGui::SameLine();
    ImGui::TextDisabled("(%u)", EditorManager::GetSelectedEntity());

    if (!EditorManager::ShowComponentWidget<BasicComponent>()) {
      ImGui::Separator();
      return;
    }

    BasicComponent& basic = EditorManager::GetComponent<BasicComponent>();

    char name[128] = "";
    basic.m_name.copy(name, IM_ARRAYSIZE(name) - 1);

    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputText("##entityName", name, IM_ARRAYSIZE(name))) {
      basic.m_name = name;
    }
    MarkDirtyIfEdited();

    ImGui::Separator();
  }

  void DrawBasic() {
    if (!EditorManager::ShowComponentWidget<BasicComponent>()) {
      return;
    }

    if (ImGui::CollapsingHeader("Basic", ImGuiTreeNodeFlags_DefaultOpen)) {
      BasicComponent& basic = EditorManager::GetComponent<BasicComponent>();

      bool is_static = basic.m_is_static;
      if (ImGui::Checkbox("Static", &is_static)) {
        basic.m_is_static = is_static;
      }
      MarkDirtyIfEdited();
    }
  }

  void DrawTransform() {
    if (!EditorManager::ShowComponentWidget<TransformComponent>()) {
      return;
    }

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
      TransformComponent& transform = EditorManager::GetComponent<TransformComponent>();

      ImGui::InputFloat3("Position", glm::value_ptr(transform.position), "%.2f");
      MarkDirtyIfEdited();
      ImGui::InputFloat3("Rotation", glm::value_ptr(transform.rotation), "%.2f");
      MarkDirtyIfEdited();
      ImGui::InputFloat3("Scale", glm::value_ptr(transform.scale), "%.2f");
      MarkDirtyIfEdited();
    }
  }

  void DrawSprite() {
    if (!EditorManager::ShowComponentWidget<SpriteComponent>()) {
      return;
    }

    if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
      SpriteComponent& component = EditorManager::GetComponent<SpriteComponent>();

      if (component.m_sprite == nullptr) {
        // A component loaded from an older file can have no sprite at all.
        // Creating one here is the same default the Add Component menu uses.
        ImGui::TextDisabled("No sprite assigned.");
        if (ImGui::SmallButton("Create Sprite")) {
          component.m_sprite.reset(EditorManager::CreateDefaultSprite());
          SceneManager::MarkDirty();
        }
      } else {
        rse::Sprite* sprite = component.m_sprite.get();
        DrawSpriteColor(sprite);
        DrawSpriteTexture(sprite);
      }

      DrawRemoveComponentButton(Components::kSprite, "Remove Sprite");
    }
  }

  void DrawSpriteColor(rse::Sprite* sprite) {
    // The tint is packed into an unsigned int, so it has to be unpacked for
    // the widget and re-packed when it changes.
    const unsigned int packed = sprite->GetColor();

    float rgba[4] = {
        static_cast<float>((packed >> 0) & 0xFF) / 255.0f,
        static_cast<float>((packed >> 8) & 0xFF) / 255.0f,
        static_cast<float>((packed >> 16) & 0xFF) / 255.0f,
        static_cast<float>((packed >> 24) & 0xFF) / 255.0f,
    };

    if (ImGui::ColorEdit4("Color", rgba)) {
      sprite->SetColor(
          (static_cast<unsigned int>(rgba[0] * 255.0f) << 0) | (static_cast<unsigned int>(rgba[1] * 255.0f) << 8) |
          (static_cast<unsigned int>(rgba[2] * 255.0f) << 16) | (static_cast<unsigned int>(rgba[3] * 255.0f) << 24));
    }
    MarkDirtyIfEdited();
  }

  // The texture field is a search box, not a name field: the sprite holds a
  // borrowed Texture*, so the typed text only filters the catalog until a row
  // is picked. The popup opens on activation and on every edit, which is what
  // makes "start typing" show every texture that matches. It is positioned
  // outside the field and opened without focus-on-appearing, so the InputText
  // keeps the keyboard while the list is visible.
  void DrawSpriteTexture(rse::Sprite* sprite) {
    const uint32_t entity = EditorManager::GetSelectedEntity();
    const std::string current_name = sprite->GetTextureName();

    if (m_texture_edit_entity != entity) {
      m_texture_edit_entity = entity;
      m_texture_query = current_name;
      m_texture_catalog.clear();
    }

    const bool popup_open = ImGui::IsPopupOpen(kTexturePickerPopup);

    // When the field is not being edited, it follows the sprite. A reload or
    // a different selection therefore cannot leave stale search text behind.
    if (!m_texture_field_active && !popup_open && m_texture_query != current_name) {
      m_texture_query = current_name;
    }

    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "%s", m_texture_query.c_str());

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Texture");
    ImGui::SameLine();

    const float button_width = ImGui::GetFrameHeight();
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetNextItemWidth(std::max(1.0f, ImGui::GetContentRegionAvail().x - button_width - spacing));

    const bool edited = ImGui::InputTextWithHint("##texture", "(none) - type to search", buffer, IM_ARRAYSIZE(buffer));
    const bool activated = ImGui::IsItemActivated();
    const ImVec2 field_min = ImGui::GetItemRectMin();
    const ImVec2 field_max = ImGui::GetItemRectMax();
    m_texture_field_active = ImGui::IsItemActive();

    if (edited) {
      m_texture_query = buffer;
      m_texture_highlight = 0;
      OpenTexturePicker(sprite);
    } else if (activated) {
      // Clicking the field is the "browse everything" gesture; typing is what
      // narrows it. The current name comes back if nothing is picked.
      m_texture_query.clear();
      OpenTexturePicker(sprite);
    }

    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kTextureDragDropPayload)) {
        if (payload->DataSize == static_cast<int>(sizeof(TextureDragPayload))) {
          const TextureDragPayload* dropped = static_cast<const TextureDragPayload*>(payload->Data);
          AssignDroppedTexture(sprite, std::filesystem::path{dropped->path});
        }
      }
      ImGui::EndDragDropTarget();
    }

    ImGui::SameLine();
    if (ImGui::ArrowButton("##texture_list", ImGuiDir_Down)) {
      // The arrow is the "show everything" affordance; typing narrows it.
      m_texture_query.clear();
      OpenTexturePicker(sprite);
    }

    if (ImGui::IsPopupOpen(kTexturePickerPopup)) {
      PositionTexturePickerPopup(field_min, field_max);
    }

    if (ImGui::BeginPopup(kTexturePickerPopup, ImGuiWindowFlags_NoFocusOnAppearing)) {
      // NoFocusOnAppearing keeps the InputText active, but it also leaves the
      // popup behind the inspector in the hit-test order. Bring it to the
      // display front so its rows stay clickable without taking keyboard focus.
      ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
      DrawTexturePickerPopup(sprite);
      ImGui::EndPopup();
    }
  }

  // Place the list below the field, or above it when the inspector is close to
  // the bottom of the screen. The size constraint keeps a large catalog inside
  // the viewport instead of growing past the bottom edge.
  static void PositionTexturePickerPopup(const ImVec2& field_min, const ImVec2& field_max) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 work_min = viewport->WorkPos;
    const ImVec2 work_max(work_min.x + viewport->WorkSize.x, work_min.y + viewport->WorkSize.y);
    const float spacing = ImGui::GetStyle().ItemSpacing.y;

    const float below = std::max(0.0f, work_max.y - (field_max.y + spacing));
    const float above = std::max(0.0f, field_min.y - spacing - work_min.y);
    const bool open_below = below >= 120.0f || below >= above;
    const float max_height = std::max(1.0f, std::min(320.0f, open_below ? below : above));

    // In the default layout the inspector sits on the right, so align the
    // popup's right edge with the field to keep it on screen.
    const bool align_right = field_min.x + 200.0f > work_max.x;

    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(FLT_MAX, max_height));
    ImGui::SetNextWindowPos(
        ImVec2(align_right ? field_max.x : field_min.x, open_below ? field_max.y + spacing : field_min.y - spacing),
        ImGuiCond_Always, ImVec2(align_right ? 1.0f : 0.0f, open_below ? 0.0f : 1.0f));
  }

  void OpenTexturePicker(rse::Sprite* sprite) {
    // Rebuild on the frame the popup opens, not on every keystroke while it is
    // open: an asset imported since the last time would otherwise be missing.
    if (!ImGui::IsPopupOpen(kTexturePickerPopup)) {
      RefreshTextureCatalog();

      // Start on the texture the sprite already has, so Enter without typing
      // is a no-op rather than a jump to the first alphabetically. A typed
      // query starts at its first match instead.
      m_texture_highlight = 0;

      if (m_texture_query.empty()) {
        const std::string current_id = sprite->GetTextureAssetId();
        const std::string current_name = sprite->GetTextureName();

        for (std::size_t index = 0; index < m_texture_catalog.size(); ++index) {
          const TextureChoice& choice = m_texture_catalog[index];
          const bool id_match = !current_id.empty() && choice.reference == current_id;
          const bool name_match = current_id.empty() && !current_name.empty() && choice.name == current_name;

          if (id_match || name_match) {
            m_texture_highlight = static_cast<int>(index);
            break;
          }
        }
      }
    }

    ImGui::OpenPopup(kTexturePickerPopup);
  }

  void RefreshTextureCatalog() {
    const rse::Project& project = Core::GetProject();
    m_texture_catalog = BuildTextureCatalog(project.Directory(), project.asset_root);
  }

  void DrawTexturePickerPopup(rse::Sprite* sprite) {
    std::vector<const TextureChoice*> matches;

    for (const TextureChoice& choice : m_texture_catalog) {
      if (MatchesTextureQuery(choice, m_texture_query)) {
        matches.push_back(&choice);
      }
    }

    const int count = static_cast<int>(matches.size());

    if (m_texture_highlight < 0 || m_texture_highlight >= count) {
      m_texture_highlight = count > 0 ? count - 1 : 0;
    }

    // Arrow keys move the row Enter will assign. They are read before the rows
    // are drawn so the highlight is correct on this frame, not the next one.
    if (count > 0) {
      if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        m_texture_highlight = (m_texture_highlight + 1) % count;
      } else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        m_texture_highlight = (m_texture_highlight + count - 1) % count;
      }
    }

    if (ImGui::Selectable("None##none", sprite->GetTextureName().empty())) {
      sprite->SetTexture(nullptr);
      m_texture_query.clear();
      SceneManager::MarkDirty();
      ImGui::CloseCurrentPopup();
    }

    ImGui::Separator();

    for (int index = 0; index < count; ++index) {
      const TextureChoice& choice = *matches[static_cast<std::size_t>(index)];
      const bool is_current = !choice.reference.empty() && choice.reference == sprite->GetTextureAssetId();

      std::string label = choice.name;

      if (is_current) {
        label += "  (current)";
      }

      ImGui::PushID(choice.project_path.c_str());

      if (ImGui::Selectable(label.c_str(), index == m_texture_highlight)) {
        AssignTextureChoice(sprite, choice);
        ImGui::CloseCurrentPopup();
      }

      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", choice.project_path.c_str());
      }

      ImGui::PopID();
    }

    if (count > 0 && (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))) {
      AssignTextureChoice(sprite, *matches[static_cast<std::size_t>(m_texture_highlight)]);
      ImGui::CloseCurrentPopup();
    }

    if (count == 0) {
      ImGui::TextDisabled(m_texture_query.empty() ? "No textures in this project." : "No matching textures.");
      ImGui::TextDisabled("Drag an image from the Content Browser, or import one with Assets > Import Asset.");
    }
  }

  void AssignTextureChoice(rse::Sprite* sprite, const TextureChoice& choice) {
    std::string reference = choice.reference;

    if (reference.empty()) {
      const rse::Project& project = Core::GetProject();

      if (!project.IsAttached()) {
        Logger::Error("Open a project before assigning a texture.");
        return;
      }

      const std::filesystem::path file = project.Directory() / choice.project_path;

      // Choosing an image the registry does not know yet is an import: the
      // sprite then stores an id, exactly as it would after Assets > Import.
      ImportAssetAt(file);
      reference = RegisteredTextureIdForFile(file, project.Directory());

      // If the import could not write the registry, a file under the asset
      // root still works as the legacy path the loader understands.
      if (reference.empty()) {
        reference = choice.asset_root_path;
      }
    }

    if (reference.empty()) {
      Logger::Error("Could not resolve the texture " + choice.project_path);
      return;
    }

    sprite->SetTexture(rse::TextureManager::GetOrLoad(reference));
    m_texture_query = sprite->GetTextureName();
    SceneManager::MarkDirty();
  }

  void AssignDroppedTexture(rse::Sprite* sprite, const std::filesystem::path& file) {
    const rse::Project& project = Core::GetProject();

    if (!project.IsAttached()) {
      Logger::Warning("Open a project before assigning a texture.");
      return;
    }

    std::error_code absolute_error;
    const std::filesystem::path absolute = std::filesystem::absolute(file, absolute_error);
    const std::filesystem::path resolved = absolute_error ? file : absolute;

    if (!IsImageFile(resolved)) {
      Logger::Warning("Only image files can be dropped on a sprite texture field: " + resolved.string());
      return;
    }

    std::string reference = RegisteredTextureIdForFile(resolved, project.Directory());

    if (reference.empty()) {
      ImportAssetAt(resolved);
      reference = RegisteredTextureIdForFile(resolved, project.Directory());
    }

    if (reference.empty()) {
      reference = AssetRootRelative(resolved, project.Directory(), project.asset_root);
    }

    if (reference.empty()) {
      Logger::Error("A texture has to live inside the project: " + resolved.string());
      return;
    }

    sprite->SetTexture(rse::TextureManager::GetOrLoad(reference));
    m_texture_query = sprite->GetTextureName();
    SceneManager::MarkDirty();
  }

  void DrawAudio() {
    if (!EditorManager::ShowComponentWidget<AudioComponent>()) {
      return;
    }

    if (ImGui::CollapsingHeader("Audio", ImGuiTreeNodeFlags_DefaultOpen)) {
      AudioComponent& component = EditorManager::GetComponent<AudioComponent>();

      if (component.m_sound == nullptr) {
        ImGui::TextDisabled("No sound assigned.");
      } else {
        const std::string file_name = component.m_sound->GetFileName();
        ImGui::LabelText("File", "%s", file_name.empty() ? "(none)" : file_name.c_str());
      }

      ImGui::SliderInt("Volume", &component.volume, 0, 128);
      MarkDirtyIfEdited();
      ImGui::Checkbox("Mute", &component.mute);
      MarkDirtyIfEdited();
      ImGui::Checkbox("Loop", &component.loop);
      MarkDirtyIfEdited();
      ImGui::Checkbox("Autoplay", &component.autoplay);
      MarkDirtyIfEdited();

      DrawRemoveComponentButton(Components::kAudio, "Remove Audio");
    }
  }

  void DrawRigidbody() {
    if (!EditorManager::ShowComponentWidget<RigidBodyComponent>()) {
      return;
    }

    if (ImGui::CollapsingHeader("Rigidbody", ImGuiTreeNodeFlags_DefaultOpen)) {
      RigidBodyComponent& component = EditorManager::GetComponent<RigidBodyComponent>();
      Rigidbody* rigidbody = component.m_rigidbody.get();

      if (rigidbody == nullptr) {
        ImGui::TextDisabled("No rigidbody assigned.");
      } else {
        ImGui::InputFloat("Density", rigidbody->GetDensity());
        MarkDirtyIfEdited();
        ImGui::InputFloat("Friction", rigidbody->GetFriction());
        MarkDirtyIfEdited();

        // Combo wants an int, so convert instead of aliasing the enum
        // pointer as an int pointer.
        static const char* items[] = {"b2_staticBody", "b2_kinematicBody", "b2_dynamicBody"};
        b2BodyType* body_type = rigidbody->GetBodyType();
        int type = static_cast<int>(*body_type);

        if (ImGui::Combo("Body Type", &type, items, IM_ARRAYSIZE(items))) {
          *body_type = static_cast<b2BodyType>(type);
        }
        MarkDirtyIfEdited();
      }

      DrawRemoveComponentButton(Components::kRigidbody, "Remove Rigidbody");
    }
  }

  void DrawRemoveComponentButton(Components component, const char* label) {
    ImGui::Spacing();
    if (ImGui::SmallButton(label)) {
      EventManager::GetEventDispatcher()->trigger<EditorManager::RemoveComponent>(component);
    }
  }

  void DrawAddComponentMenu() {
    if (ImGui::Button("Add Component")) {
      ImGui::OpenPopup("add_component");
    }

    if (ImGui::BeginPopup("add_component")) {
      if (ImGui::MenuItem("Audio")) {
        EventManager::GetEventDispatcher()->trigger<EditorManager::AddComponent>(Components::kAudio);
      }
      if (ImGui::MenuItem("Ridigbody")) {
        EventManager::GetEventDispatcher()->trigger<EditorManager::AddComponent>(Components::kRigidbody);
      }
      if (ImGui::MenuItem("Sprite")) {
        EventManager::GetEventDispatcher()->trigger<EditorManager::AddComponent>(Components::kSprite);
      }
      ImGui::EndPopup();
    }
  }

  static constexpr const char* kTexturePickerPopup = "texture_picker";

  // The texture field's state. The query is the text being typed (or the
  // current texture's name when the field is idle); the catalog is rebuilt
  // every time the popup opens, so imports made elsewhere show up. The
  // highlight is the filtered row Enter assigns.
  std::string m_texture_query;
  std::vector<TextureChoice> m_texture_catalog;
  int m_texture_highlight{0};
  bool m_texture_field_active{false};
  uint32_t m_texture_edit_entity{INVALID_ENTITY_ID};
};
