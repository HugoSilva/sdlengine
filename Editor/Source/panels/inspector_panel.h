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

#include <components/material_shader.h>
#include <core/physics_material_manager.h>
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
// a name. The sprite's texture and material are editable, but not by typing a
// name into the component: each field shows the current asset, clicking its
// name reveals the file in the Content Browser, and the arrow opens a searchable
// picker. A picked texture is written back as a borrowed Texture* from
// TextureManager; a picked material is written back as an asset reference in
// SpriteAppearanceComponent.
// ---------------------------------------------------------------------------
class InspectorPanel : public EditorPanel {
 public:
  const char* Id() const override { return "Inspector"; }
  DockSlot Slot() const override { return DockSlot::kRight; }

  void ProjectChanged() override {
    // A different project is a different set of assets. The next popup rebuilds
    // each catalog from the new registry and directory.
    m_sprite_catalog.clear();
    m_sprite_edit_entity = INVALID_ENTITY_ID;
    m_sprite_query.clear();
    m_material_catalog.clear();
    m_material_edit_entity = INVALID_ENTITY_ID;
    m_material_query.clear();
    m_physics_material_catalog.clear();
    m_physics_material_edit_entity = INVALID_ENTITY_ID;
    m_physics_material_query.clear();
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

  void DrawBasic() {
    if (!EditorManager::ShowComponentWidget<BasicComponent>()) {
      return;
    }

    if (ImGui::CollapsingHeader("Basic", ImGuiTreeNodeFlags_DefaultOpen)) {
      const uint32_t entity = EditorManager::GetSelectedEntity();
      BasicComponent& basic = EditorManager::GetComponent<BasicComponent>();

      // The name used to live in the inspector's entity header. It belongs
      // with the rest of the entity's own fields, and the Sprite section's
      // label/value rows are the layout the other components follow.
      BeginLabeledRow("basic_name", "Name");

      char name[128] = "";
      basic.m_name.copy(name, IM_ARRAYSIZE(name) - 1);
      ImGui::SetNextItemWidth(-1.0f);

      if (ImGui::InputText("##name", name, IM_ARRAYSIZE(name))) {
        basic.m_name = name;
      }
      MarkDirtyIfEdited();
      EndLabeledRow();

      bool enabled = rse::ECSManager::IsEntityEnabled(entity);
      BeginLabeledRow("basic_enabled", "Enabled");
      if (ImGui::Checkbox("##enabled", &enabled)) {
        rse::ECSManager::SetEntityEnabled(entity, enabled);
      }
      MarkDirtyIfEdited();
      EndLabeledRow();

      bool is_static = basic.m_is_static;
      BeginLabeledRow("basic_static", "Static");
      if (ImGui::Checkbox("##static", &is_static)) {
        basic.m_is_static = is_static;
      }
      MarkDirtyIfEdited();
      EndLabeledRow();
    }
  }

  void DrawTransform() {
    if (!EditorManager::ShowComponentWidget<TransformComponent>()) {
      return;
    }

    if (DrawComponentHeader("transform", "Transform", Components::kTransform, false)) {
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

    if (DrawComponentHeaderWithEnabled<SpriteComponent>("sprite", "Sprite", Components::kSprite, true)) {
      SpriteComponent& component = EditorManager::GetComponent<SpriteComponent>();

      if (component.m_sprite == nullptr) {
        // A component loaded from an older file can have no sprite at all.
        // Creating one here is the same default the Add Component menu uses.
        ImGui::TextDisabled("No sprite assigned.");
        if (ImGui::SmallButton("Create Sprite")) {
          component.m_sprite.reset(EditorManager::CreateDefaultSprite());
          EnsureAppearance(EditorManager::GetSelectedEntity());
          SceneManager::MarkDirty();
        }
      } else {
        rse::Sprite* sprite = component.m_sprite.get();
        DrawSpriteTexture(sprite);
        DrawSpriteColor(sprite);
        DrawSpriteFlip(sprite);
        DrawSpriteMaterial(sprite);
      }
    }
  }

  void DrawSpriteColor(rse::Sprite* sprite) {
    // The tint is packed into an unsigned int, so it has to be unpacked for
    // the widget and re-packed when it changes. NoInputs leaves the swatch and
    // the picker; the numeric fields would only duplicate what the picker says.
    const unsigned int packed = sprite->GetColor();

    float rgba[4] = {
        static_cast<float>((packed >> 0) & 0xFF) / 255.0f,
        static_cast<float>((packed >> 8) & 0xFF) / 255.0f,
        static_cast<float>((packed >> 16) & 0xFF) / 255.0f,
        static_cast<float>((packed >> 24) & 0xFF) / 255.0f,
    };

    BeginLabeledRow("sprite_color", "Color");
    if (ImGui::ColorEdit4(
            "##color", rgba,
            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf)) {
      sprite->SetColor(
          (static_cast<unsigned int>(rgba[0] * 255.0f) << 0) | (static_cast<unsigned int>(rgba[1] * 255.0f) << 8) |
          (static_cast<unsigned int>(rgba[2] * 255.0f) << 16) | (static_cast<unsigned int>(rgba[3] * 255.0f) << 24));
    }
    MarkDirtyIfEdited();
    EndLabeledRow();
  }

  void DrawSpriteFlip(rse::Sprite* sprite) {
    (void)sprite;

    const uint32_t entity = EditorManager::GetSelectedEntity();
    SpriteAppearanceComponent* appearance = FindAppearance(entity);
    bool flip_x = appearance != nullptr && appearance->flip_x;
    bool flip_y = appearance != nullptr && appearance->flip_y;

    BeginLabeledRow("sprite_flip", "Flip");

    if (ImGui::Checkbox("X", &flip_x)) {
      EnsureAppearance(entity).flip_x = flip_x;
      SceneManager::MarkDirty();
    }

    ImGui::SameLine();
    if (ImGui::Checkbox("Y", &flip_y)) {
      EnsureAppearance(entity).flip_y = flip_y;
      SceneManager::MarkDirty();
    }

    EndLabeledRow();
  }

  // The sprite field looks like a disabled text input but is two affordances:
  // clicking the name reveals the texture in the Content Browser, and the arrow
  // opens a searchable popup. It never accepts typed text: a sprite holds a
  // borrowed Texture*, not a name, so a typed name would have nowhere to go.
  void DrawSpriteTexture(rse::Sprite* sprite) {
    const uint32_t entity = EditorManager::GetSelectedEntity();
    const std::string current_name = sprite->GetTextureName();

    if (m_sprite_edit_entity != entity) {
      m_sprite_edit_entity = entity;
      m_sprite_query.clear();
      m_sprite_catalog.clear();
    }

    BeginLabeledRow("sprite", "Sprite");

    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "%s", current_name.empty() ? "" : current_name.c_str());

    const float button_width = ImGui::GetFrameHeight();
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetNextItemWidth(std::max(1.0f, ImGui::GetContentRegionAvail().x - button_width - spacing));

    ImGui::InputTextWithHint("##sprite", "(none)", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_ReadOnly);
    const bool clicked = ImGui::IsItemClicked();
    const ImVec2 field_min = ImGui::GetItemRectMin();
    const ImVec2 field_max = ImGui::GetItemRectMax();

    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(kTextureDragDropPayload)) {
        if (payload->DataSize == static_cast<int>(sizeof(TextureDragPayload))) {
          const TextureDragPayload* dropped = static_cast<const TextureDragPayload*>(payload->Data);
          AssignDroppedTexture(sprite, std::filesystem::path{dropped->path});
        }
      }
      ImGui::EndDragDropTarget();
    }

    bool open_picker = false;

    ImGui::SameLine();
    if (ImGui::ArrowButton("##sprite_list", ImGuiDir_Down)) {
      m_sprite_query.clear();
      open_picker = true;
    }

    EndLabeledRow();

    if (clicked) {
      RevealTexture(sprite);
    }

    // The popup has to be opened outside the row's PushID: OpenPopup and
    // BeginPopup match by ID, and the table row pushed an ID of its own.
    if (open_picker) {
      OpenSpritePicker(sprite);
    }

    // Position before BeginPopup: that is where ImGui consumes the next-window
    // data. Doing it inside would only move the popup on its second frame.
    if (ImGui::IsPopupOpen(kSpritePickerPopup)) {
      PositionAssetPickerPopup(field_min, field_max);
    }

    if (ImGui::BeginPopup(kSpritePickerPopup)) {
      DrawSpritePickerPopup(sprite);
      ImGui::EndPopup();
    }
  }

  // Place the list below the field, or above it when the inspector is close to
  // the bottom of the screen. The size constraint keeps a large catalog inside
  // the viewport instead of growing past the bottom edge.
  static void PositionAssetPickerPopup(const ImVec2& field_min, const ImVec2& field_max) {
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
    const bool align_right = field_min.x + 280.0f > work_max.x;

    ImGui::SetNextWindowSizeConstraints(ImVec2(280.0f, 0.0f), ImVec2(FLT_MAX, max_height));
    ImGui::SetNextWindowPos(
        ImVec2(align_right ? field_max.x : field_min.x, open_below ? field_max.y + spacing : field_min.y - spacing),
        ImGuiCond_Always, ImVec2(align_right ? 1.0f : 0.0f, open_below ? 0.0f : 1.0f));
  }

  void OpenSpritePicker(rse::Sprite* sprite) {
    // Rebuild on the frame the popup opens, not on every keystroke while it is
    // open: an asset imported since the last time would otherwise be missing.
    if (!ImGui::IsPopupOpen(kSpritePickerPopup)) {
      RefreshSpriteCatalog();

      // Start on the texture the sprite already has, so Enter without typing
      // is a no-op rather than a jump to the first alphabetically. A typed
      // query starts at its first match instead.
      m_sprite_highlight = 0;

      if (m_sprite_query.empty()) {
        const std::string current_id = sprite->GetTextureAssetId();
        const std::string current_name = sprite->GetTextureName();

        for (std::size_t index = 0; index < m_sprite_catalog.size(); ++index) {
          const TextureChoice& choice = m_sprite_catalog[index];
          const bool id_match = !current_id.empty() && choice.reference == current_id;
          const bool name_match = current_id.empty() && !current_name.empty() && choice.name == current_name;

          if (id_match || name_match) {
            m_sprite_highlight = static_cast<int>(index);
            break;
          }
        }
      }

      m_focus_sprite_search = true;
    }

    ImGui::OpenPopup(kSpritePickerPopup);
  }

  void RefreshSpriteCatalog() {
    const rse::Project& project = Core::GetProject();
    m_sprite_catalog = BuildTextureCatalog(project.Directory(), project.asset_root);
  }

  void DrawSpritePickerPopup(rse::Sprite* sprite) {
    // The search field takes focus as soon as the popup appears, so opening the
    // arrow and typing is one gesture. AutoSelectAll means a query left over
    // from a previous open is replaced by the first key press.
    if (m_focus_sprite_search) {
      ImGui::SetKeyboardFocusHere();
      m_focus_sprite_search = false;
    }

    char search[256];
    std::snprintf(search, sizeof(search), "%s", m_sprite_query.c_str());
    ImGui::SetNextItemWidth(-1.0f);

    if (ImGui::InputTextWithHint("##sprite_search", "Search sprites...", search, IM_ARRAYSIZE(search),
                                 ImGuiInputTextFlags_AutoSelectAll)) {
      m_sprite_query = search;
      m_sprite_highlight = 0;
    }

    std::vector<const TextureChoice*> matches;

    for (const TextureChoice& choice : m_sprite_catalog) {
      if (MatchesTextureQuery(choice, m_sprite_query)) {
        matches.push_back(&choice);
      }
    }

    const int count = static_cast<int>(matches.size());

    if (m_sprite_highlight < 0 || m_sprite_highlight >= count) {
      m_sprite_highlight = count > 0 ? count - 1 : 0;
    }

    // Arrow keys move the row Enter will assign. They are read before the rows
    // are drawn so the highlight is correct on this frame, not the next one.
    if (count > 0) {
      if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        m_sprite_highlight = (m_sprite_highlight + 1) % count;
      } else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        m_sprite_highlight = (m_sprite_highlight + count - 1) % count;
      }
    }

    ImGui::Separator();

    if (ImGui::Selectable("None##sprite_none", sprite->GetTextureName().empty())) {
      sprite->SetTexture(nullptr);
      m_sprite_query.clear();
      SceneManager::MarkDirty();
      ImGui::CloseCurrentPopup();
    }

    for (int index = 0; index < count; ++index) {
      const TextureChoice& choice = *matches[static_cast<std::size_t>(index)];
      const bool is_current = !choice.reference.empty() && choice.reference == sprite->GetTextureAssetId();

      std::string label = choice.name;

      if (is_current) {
        label += "  (current)";
      }

      ImGui::PushID(choice.project_path.c_str());

      if (ImGui::Selectable(label.c_str(), index == m_sprite_highlight)) {
        AssignTextureChoice(sprite, choice);
        ImGui::CloseCurrentPopup();
      }

      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", choice.project_path.c_str());
      }

      ImGui::PopID();
    }

    if (count > 0 && (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))) {
      AssignTextureChoice(sprite, *matches[static_cast<std::size_t>(m_sprite_highlight)]);
      ImGui::CloseCurrentPopup();
    }

    if (count == 0) {
      ImGui::TextDisabled(m_sprite_query.empty() ? "No sprites in this project." : "No matching sprites.");
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
    m_sprite_query.clear();
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
    m_sprite_query.clear();
    SceneManager::MarkDirty();
  }

  // A two-column table keeps the label and the widget on one baseline. The
  // label column is fixed and left-aligned; the value column stretches, so the
  // widget fills the rest of the inspector. PushID makes each row's table id
  // unique.
  void BeginLabeledRow(const char* id, const char* label) {
    ImGui::PushID(id);
    ImGui::BeginTable("##row", 2, ImGuiTableFlags_SizingStretchProp);
    ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, kLabelColumnWidth);
    ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::TableNextColumn();
  }

  void EndLabeledRow() {
    ImGui::EndTable();
    ImGui::PopID();
  }

  static SpriteAppearanceComponent* FindAppearance(uint32_t entity) {
    if (entity == INVALID_ENTITY_ID || !rse::ECSManager::HasComponent<SpriteAppearanceComponent>(entity)) {
      return nullptr;
    }

    return &rse::ECSManager::GetComponent<SpriteAppearanceComponent>(entity);
  }

  static SpriteAppearanceComponent& EnsureAppearance(uint32_t entity) {
    if (!rse::ECSManager::HasComponent<SpriteAppearanceComponent>(entity)) {
      rse::ECSManager::AddComponent<SpriteAppearanceComponent>(entity);
    }

    return rse::ECSManager::GetComponent<SpriteAppearanceComponent>(entity);
  }

  static bool ChoiceMatchesReference(const TextureChoice& choice, const std::string& reference) {
    if (reference.empty()) {
      return false;
    }

    if (!choice.reference.empty() && choice.reference == reference) {
      return true;
    }

    return Lowercase(choice.name) == Lowercase(reference) || Lowercase(choice.project_path) == Lowercase(reference) ||
           Lowercase(choice.asset_root_path) == Lowercase(reference);
  }

  // Clicking the texture name is a navigation request, not an edit. A file that
  // has been deleted or an unregistered path still tries the project and asset
  // roots so the browser can show the folder it lives in.
  void RevealTexture(rse::Sprite* sprite) {
    std::string path;
    const std::string asset_id = sprite->GetTextureAssetId();

    if (!asset_id.empty()) {
      if (const rse::AssetRegistry::Asset* asset = rse::AssetRegistry::Find(asset_id)) {
        path = Core::ResolveProjectPath(asset->path);
      }
    }

    if (path.empty() && !sprite->GetTextureName().empty()) {
      path = Core::ResolveResource(sprite->GetTextureName());
    }

    if (path.empty() || !std::filesystem::exists(path)) {
      Logger::Warning("The sprite's texture is not on disk" + (path.empty() ? std::string{} : ": " + path));
      return;
    }

    EditorManager::RequestRevealAsset(path);
  }

  void DrawSpriteMaterial(rse::Sprite* sprite) {
    (void)sprite;

    const uint32_t entity = EditorManager::GetSelectedEntity();
    SpriteAppearanceComponent* appearance = FindAppearance(entity);
    const std::string reference = appearance == nullptr ? std::string{} : appearance->material;

    std::string current_name;
    std::string current_path;

    if (!reference.empty()) {
      if (rse::MaterialShader* material = rse::MaterialManager::GetOrLoad(reference)) {
        current_name = material->GetName();
        current_path = material->GetAssetPath();
      }

      if (current_name.empty()) {
        current_name = reference;
      }
    }

    if (m_material_edit_entity != entity) {
      m_material_edit_entity = entity;
      m_material_query.clear();
      m_material_catalog.clear();
    }

    BeginLabeledRow("sprite_material", "Material");

    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "%s", current_name.empty() ? "" : current_name.c_str());

    const float button_width = ImGui::GetFrameHeight();
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetNextItemWidth(std::max(1.0f, ImGui::GetContentRegionAvail().x - button_width - spacing));

    ImGui::InputTextWithHint("##material", "(none)", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_ReadOnly);
    const bool clicked = ImGui::IsItemClicked();
    const ImVec2 field_min = ImGui::GetItemRectMin();
    const ImVec2 field_max = ImGui::GetItemRectMax();

    bool open_picker = false;

    ImGui::SameLine();
    if (ImGui::ArrowButton("##material_list", ImGuiDir_Down)) {
      m_material_query.clear();
      open_picker = true;
    }

    EndLabeledRow();

    if (clicked) {
      RevealMaterial(reference, current_path);
    }

    // Match the row's PushID scope: the popup is opened and begun here, outside
    // the table, or the two IDs do not name the same popup.
    if (open_picker) {
      OpenMaterialPicker(reference);
    }

    if (ImGui::IsPopupOpen(kMaterialPickerPopup)) {
      PositionAssetPickerPopup(field_min, field_max);
    }

    if (ImGui::BeginPopup(kMaterialPickerPopup)) {
      DrawMaterialPickerPopup(entity, reference);
      ImGui::EndPopup();
    }
  }

  void OpenMaterialPicker(const std::string& reference) {
    if (!ImGui::IsPopupOpen(kMaterialPickerPopup)) {
      RefreshMaterialCatalog();
      m_material_highlight = 0;

      if (m_material_query.empty() && !reference.empty()) {
        for (std::size_t index = 0; index < m_material_catalog.size(); ++index) {
          if (ChoiceMatchesReference(m_material_catalog[index], reference)) {
            m_material_highlight = static_cast<int>(index);
            break;
          }
        }
      }

      m_focus_material_search = true;
    }

    ImGui::OpenPopup(kMaterialPickerPopup);
  }

  void RefreshMaterialCatalog() {
    const rse::Project& project = Core::GetProject();
    m_material_catalog = BuildMaterialCatalog(project.Directory(), project.asset_root);
  }

  void DrawMaterialPickerPopup(uint32_t entity, const std::string& reference) {
    if (m_focus_material_search) {
      ImGui::SetKeyboardFocusHere();
      m_focus_material_search = false;
    }

    char search[256];
    std::snprintf(search, sizeof(search), "%s", m_material_query.c_str());
    ImGui::SetNextItemWidth(-1.0f);

    if (ImGui::InputTextWithHint("##material_search", "Search materials...", search, IM_ARRAYSIZE(search),
                                 ImGuiInputTextFlags_AutoSelectAll)) {
      m_material_query = search;
      m_material_highlight = 0;
    }

    std::vector<const TextureChoice*> matches;

    for (const TextureChoice& choice : m_material_catalog) {
      if (MatchesTextureQuery(choice, m_material_query)) {
        matches.push_back(&choice);
      }
    }

    const int count = static_cast<int>(matches.size());

    if (m_material_highlight < 0 || m_material_highlight >= count) {
      m_material_highlight = count > 0 ? count - 1 : 0;
    }

    if (count > 0) {
      if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        m_material_highlight = (m_material_highlight + 1) % count;
      } else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        m_material_highlight = (m_material_highlight + count - 1) % count;
      }
    }

    ImGui::Separator();

    if (ImGui::Selectable("None##material_none", reference.empty())) {
      AssignMaterialReference(entity, {});
      m_material_query.clear();
      ImGui::CloseCurrentPopup();
    }

    for (int index = 0; index < count; ++index) {
      const TextureChoice& choice = *matches[static_cast<std::size_t>(index)];
      const bool is_current = ChoiceMatchesReference(choice, reference);

      std::string label = choice.name;

      if (is_current) {
        label += "  (current)";
      }

      ImGui::PushID(choice.project_path.c_str());

      if (ImGui::Selectable(label.c_str(), index == m_material_highlight)) {
        AssignMaterialChoice(entity, choice);
        ImGui::CloseCurrentPopup();
      }

      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", choice.project_path.c_str());
      }

      ImGui::PopID();
    }

    if (count > 0 && (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))) {
      AssignMaterialChoice(entity, *matches[static_cast<std::size_t>(m_material_highlight)]);
      ImGui::CloseCurrentPopup();
    }

    if (count == 0) {
      ImGui::TextDisabled(m_material_query.empty() ? "No materials in this project." : "No matching materials.");
      ImGui::TextDisabled("Add a .material file and import it with Assets > Import Asset.");
    }
  }

  static void AssignMaterialReference(uint32_t entity, const std::string& reference) {
    EnsureAppearance(entity).material = reference;
    SceneManager::MarkDirty();
  }

  void AssignMaterialChoice(uint32_t entity, const TextureChoice& choice) {
    std::string reference = choice.reference;

    if (reference.empty()) {
      const rse::Project& project = Core::GetProject();

      if (!project.IsAttached()) {
        Logger::Error("Open a project before assigning a material.");
        return;
      }

      const std::filesystem::path file = project.Directory() / choice.project_path;

      ImportAssetAt(file);
      reference = RegisteredAssetIdForFile(file, project.Directory(), "material");

      if (reference.empty()) {
        reference = choice.project_path;
      }
    }

    if (reference.empty()) {
      Logger::Error("Could not resolve the material " + choice.project_path);
      return;
    }

    AssignMaterialReference(entity, reference);
    m_material_query.clear();
  }

  void RevealMaterial(const std::string& reference, const std::string& known_path) {
    if (reference.empty()) {
      return;
    }

    std::string path = known_path;

    if (path.empty()) {
      if (rse::MaterialShader* material = rse::MaterialManager::GetOrLoad(reference)) {
        path = material->GetAssetPath();
      }
    }

    if (path.empty()) {
      path = Core::ResolveProjectPath(reference);
    }

    if (path.empty() || !std::filesystem::exists(path)) {
      Logger::Warning("The sprite's material is not on disk" + (path.empty() ? std::string{} : ": " + path));
      return;
    }

    EditorManager::RequestRevealAsset(path);
  }

  void DrawAudio() {
    if (!EditorManager::ShowComponentWidget<AudioComponent>()) {
      return;
    }

    if (DrawComponentHeaderWithEnabled<AudioComponent>("audio", "Audio", Components::kAudio, true)) {
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
    }
  }

  void DrawRigidbody() {
    if (!EditorManager::ShowComponentWidget<RigidBodyComponent>()) {
      return;
    }

    if (DrawComponentHeaderWithEnabled<RigidBodyComponent>("rigidbody", "Rigidbody", Components::kRigidbody, true)) {
      RigidBodyComponent& component = EditorManager::GetComponent<RigidBodyComponent>();
      Rigidbody* rigidbody = component.m_rigidbody.get();

      if (rigidbody == nullptr) {
        ImGui::TextDisabled("No rigidbody assigned.");
      } else {
        BeginLabeledRow("rigidbody_body_type", "Body Type");

        // Combo wants an int, so convert instead of aliasing the enum
        // pointer as an int pointer.
        static const char* items[] = {"Static", "Kinematic", "Dynamic"};
        b2BodyType* body_type = rigidbody->GetBodyType();
        int type = static_cast<int>(*body_type);
        ImGui::SetNextItemWidth(-1.0f);

        if (ImGui::Combo("##body_type", &type, items, IM_ARRAYSIZE(items))) {
          *body_type = static_cast<b2BodyType>(type);
          SceneManager::MarkDirty();
        }

        EndLabeledRow();

        DrawRigidbodyMaterial(rigidbody);
      }
    }
  }

  // The material field mirrors the sprite's: clicking the name reveals the
  // asset, and the arrow opens a searchable picker. The body stores an asset
  // reference, not a PhysicsMaterial value, so a later edit to the asset is
  // picked up the next time the body is created.
  void DrawRigidbodyMaterial(Rigidbody* rigidbody) {
    const uint32_t entity = EditorManager::GetSelectedEntity();
    const std::string reference = rigidbody->GetMaterial();

    std::string current_name;
    std::string current_path;

    if (!reference.empty()) {
      if (const rse::PhysicsMaterialAsset* material = rse::PhysicsMaterialManager::GetOrLoad(reference)) {
        current_name = material->name;
        current_path = material->path;
      }

      if (current_name.empty()) {
        current_name = reference;
      }
    }

    if (m_physics_material_edit_entity != entity) {
      m_physics_material_edit_entity = entity;
      m_physics_material_query.clear();
      m_physics_material_catalog.clear();
    }

    BeginLabeledRow("rigidbody_material", "Material");

    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "%s", current_name.empty() ? "" : current_name.c_str());

    const float button_width = ImGui::GetFrameHeight();
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetNextItemWidth(std::max(1.0f, ImGui::GetContentRegionAvail().x - button_width - spacing));

    ImGui::InputTextWithHint("##physics_material", "(none)", buffer, IM_ARRAYSIZE(buffer),
                             ImGuiInputTextFlags_ReadOnly);
    const bool clicked = ImGui::IsItemClicked();
    const ImVec2 field_min = ImGui::GetItemRectMin();
    const ImVec2 field_max = ImGui::GetItemRectMax();

    bool open_picker = false;

    ImGui::SameLine();
    if (ImGui::ArrowButton("##physics_material_list", ImGuiDir_Down)) {
      m_physics_material_query.clear();
      open_picker = true;
    }

    EndLabeledRow();

    if (clicked) {
      RevealPhysicsMaterial(reference, current_path);
    }

    // Match the row's PushID scope: the popup is opened and begun here, outside
    // the table, or the two IDs do not name the same popup.
    if (open_picker) {
      OpenPhysicsMaterialPicker(reference);
    }

    if (ImGui::IsPopupOpen(kPhysicsMaterialPickerPopup)) {
      PositionAssetPickerPopup(field_min, field_max);
    }

    if (ImGui::BeginPopup(kPhysicsMaterialPickerPopup)) {
      DrawPhysicsMaterialPickerPopup(entity, reference);
      ImGui::EndPopup();
    }
  }

  void OpenPhysicsMaterialPicker(const std::string& reference) {
    if (!ImGui::IsPopupOpen(kPhysicsMaterialPickerPopup)) {
      RefreshPhysicsMaterialCatalog();
      m_physics_material_highlight = 0;

      if (m_physics_material_query.empty() && !reference.empty()) {
        for (std::size_t index = 0; index < m_physics_material_catalog.size(); ++index) {
          if (ChoiceMatchesReference(m_physics_material_catalog[index], reference)) {
            m_physics_material_highlight = static_cast<int>(index);
            break;
          }
        }
      }

      m_focus_physics_material_search = true;
    }

    ImGui::OpenPopup(kPhysicsMaterialPickerPopup);
  }

  void RefreshPhysicsMaterialCatalog() {
    const rse::Project& project = Core::GetProject();
    m_physics_material_catalog = BuildPhysicsMaterialCatalog(project.Directory(), project.asset_root);
  }

  void DrawPhysicsMaterialPickerPopup(uint32_t entity, const std::string& reference) {
    if (m_focus_physics_material_search) {
      ImGui::SetKeyboardFocusHere();
      m_focus_physics_material_search = false;
    }

    char search[256];
    std::snprintf(search, sizeof(search), "%s", m_physics_material_query.c_str());
    ImGui::SetNextItemWidth(-1.0f);

    if (ImGui::InputTextWithHint("##physics_material_search", "Search physics materials...", search,
                                 IM_ARRAYSIZE(search), ImGuiInputTextFlags_AutoSelectAll)) {
      m_physics_material_query = search;
      m_physics_material_highlight = 0;
    }

    std::vector<const TextureChoice*> matches;

    for (const TextureChoice& choice : m_physics_material_catalog) {
      if (MatchesTextureQuery(choice, m_physics_material_query)) {
        matches.push_back(&choice);
      }
    }

    const int count = static_cast<int>(matches.size());

    if (m_physics_material_highlight < 0 || m_physics_material_highlight >= count) {
      m_physics_material_highlight = count > 0 ? count - 1 : 0;
    }

    if (count > 0) {
      if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        m_physics_material_highlight = (m_physics_material_highlight + 1) % count;
      } else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        m_physics_material_highlight = (m_physics_material_highlight + count - 1) % count;
      }
    }

    ImGui::Separator();

    if (ImGui::Selectable("None##physics_material_none", reference.empty())) {
      AssignPhysicsMaterialReference(entity, {});
      m_physics_material_query.clear();
      ImGui::CloseCurrentPopup();
    }

    for (int index = 0; index < count; ++index) {
      const TextureChoice& choice = *matches[static_cast<std::size_t>(index)];
      const bool is_current = ChoiceMatchesReference(choice, reference);

      std::string label = choice.name;

      if (is_current) {
        label += "  (current)";
      }

      ImGui::PushID(choice.project_path.c_str());

      if (ImGui::Selectable(label.c_str(), index == m_physics_material_highlight)) {
        AssignPhysicsMaterialChoice(entity, choice);
        ImGui::CloseCurrentPopup();
      }

      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", choice.project_path.c_str());
      }

      ImGui::PopID();
    }

    if (count > 0 && (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))) {
      AssignPhysicsMaterialChoice(entity, *matches[static_cast<std::size_t>(m_physics_material_highlight)]);
      ImGui::CloseCurrentPopup();
    }

    if (count == 0) {
      ImGui::TextDisabled(m_physics_material_query.empty() ? "No physics materials in this project."
                                                           : "No matching physics materials.");
      ImGui::TextDisabled("Add a .physicsmaterial file and import it with Assets > Import Asset.");
    }
  }

  static void AssignPhysicsMaterialReference(uint32_t entity, const std::string& reference) {
    if (!rse::ECSManager::HasComponent<RigidBodyComponent>(entity)) {
      return;
    }

    RigidBodyComponent& component = rse::ECSManager::GetComponent<RigidBodyComponent>(entity);

    if (component.m_rigidbody == nullptr) {
      return;
    }

    component.m_rigidbody->SetMaterial(reference);
    SceneManager::MarkDirty();
  }

  void AssignPhysicsMaterialChoice(uint32_t entity, const TextureChoice& choice) {
    std::string reference = choice.reference;

    if (reference.empty()) {
      const rse::Project& project = Core::GetProject();

      if (!project.IsAttached()) {
        Logger::Error("Open a project before assigning a physics material.");
        return;
      }

      const std::filesystem::path file = project.Directory() / choice.project_path;

      ImportAssetAt(file);
      reference = RegisteredAssetIdForFile(file, project.Directory(), "physics_material");

      if (reference.empty()) {
        reference = choice.project_path;
      }
    }

    if (reference.empty()) {
      Logger::Error("Could not resolve the physics material " + choice.project_path);
      return;
    }

    AssignPhysicsMaterialReference(entity, reference);
    m_physics_material_query.clear();
  }

  void RevealPhysicsMaterial(const std::string& reference, const std::string& known_path) {
    if (reference.empty()) {
      return;
    }

    std::string path = known_path;

    if (path.empty()) {
      if (const rse::PhysicsMaterialAsset* material = rse::PhysicsMaterialManager::GetOrLoad(reference)) {
        path = material->path;
      }
    }

    if (path.empty()) {
      path = Core::ResolveProjectPath(reference);
    }

    if (path.empty() || !std::filesystem::exists(path)) {
      Logger::Warning("The rigidbody's physics material is not on disk" + (path.empty() ? std::string{} : ": " + path));
      return;
    }

    EditorManager::RequestRevealAsset(path);
  }

  // A component header with the collapse arrow, its name and the right-side
  // options button. The enabled variants add the checkbox before the name; a
  // required component uses the plain form and only offers Reset.
  bool DrawComponentHeader(const char* id, const char* name, Components component, bool can_remove) {
    ImGui::PushID(id);
    ImGui::SetNextItemAllowOverlap();

    const bool open = ImGui::TreeNodeEx("##header",
                                        ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen |
                                            ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_SpanAvailWidth,
                                        "");
    ImGui::SameLine();
    DrawComponentHeaderName(name, true);
    const bool removed = DrawComponentOptions(component, can_remove);
    ImGui::PopID();
    return open && !removed;
  }

  template <typename Component>
  bool DrawComponentHeaderWithEnabled(const char* id, const char* name, Components component, bool can_remove) {
    const uint32_t entity = EditorManager::GetSelectedEntity();

    ImGui::PushID(id);
    ImGui::SetNextItemAllowOverlap();

    const bool open = ImGui::TreeNodeEx("##header",
                                        ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen |
                                            ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_SpanAvailWidth,
                                        "");
    ImGui::SameLine();

    bool enabled = rse::ECSManager::IsComponentEnabled<Component>(entity);
    if (ImGui::Checkbox("##enabled", &enabled)) {
      rse::ECSManager::SetComponentEnabled<Component>(entity, enabled);
      SceneManager::MarkDirty();
    }

    ImGui::SameLine();
    DrawComponentHeaderName(name, enabled);
    const bool removed = DrawComponentOptions(component, can_remove);
    ImGui::PopID();
    return open && !removed;
  }

  static void DrawComponentHeaderName(const char* name, bool enabled) {
    if (!enabled) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    }

    ImGui::TextUnformatted(name);

    if (!enabled) {
      ImGui::PopStyleColor();
    }
  }

  bool DrawComponentOptions(Components component, bool can_remove) {
    bool removed = false;
    const float button_width = ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x * 2.0f;
    const float button_x = ImGui::GetWindowContentRegionMax().x - button_width;

    if (button_x > ImGui::GetCursorPosX()) {
      ImGui::SameLine(button_x);
    } else {
      ImGui::SameLine();
    }

    if (ImGui::SmallButton("...")) {
      ImGui::OpenPopup("##options");
    }

    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("More options");
    }

    if (ImGui::BeginPopup("##options")) {
      if (ImGui::MenuItem("Reset Component")) {
        EventManager::GetEventDispatcher()->trigger<EditorManager::ResetComponent>(component);
      }

      if (can_remove && ImGui::MenuItem("Remove Component")) {
        EventManager::GetEventDispatcher()->trigger<EditorManager::RemoveComponent>(component);
        removed = true;
      }

      ImGui::EndPopup();
    }

    return removed;
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

  static constexpr const char* kSpritePickerPopup = "sprite_picker";
  static constexpr const char* kMaterialPickerPopup = "material_picker";
  static constexpr const char* kPhysicsMaterialPickerPopup = "physics_material_picker";
  static constexpr float kLabelColumnWidth{66.0f};

  // Each picker keeps its own query, catalog, highlighted row and "focus the
  // search on the next draw" flag. The catalogs are rebuilt every time a popup
  // opens, so imports made elsewhere show up.
  std::string m_sprite_query;
  std::vector<TextureChoice> m_sprite_catalog;
  int m_sprite_highlight{0};
  bool m_focus_sprite_search{false};
  uint32_t m_sprite_edit_entity{INVALID_ENTITY_ID};

  std::string m_material_query;
  std::vector<TextureChoice> m_material_catalog;
  int m_material_highlight{0};
  bool m_focus_material_search{false};
  uint32_t m_material_edit_entity{INVALID_ENTITY_ID};

  std::string m_physics_material_query;
  std::vector<TextureChoice> m_physics_material_catalog;
  int m_physics_material_highlight{0};
  bool m_focus_physics_material_search{false};
  uint32_t m_physics_material_edit_entity{INVALID_ENTITY_ID};
};
