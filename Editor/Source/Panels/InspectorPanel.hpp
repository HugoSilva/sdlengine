#pragma once

#include <imgui.h>
#include <string>

#include <glm/gtc/type_ptr.hpp>

#include "../EditorManager.h"
#include "../Layout/EditorPanel.hpp"
#include "Core.h"

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
// The name fields are read-only now: the components hold a shared Texture /
// Sound handle, not a name, so there is nothing an edited string could be
// written back to.
// ---------------------------------------------------------------------------
class InspectorPanel : public EditorPanel {
 public:
  const char* Id() const override { return "Inspector"; }
  DockSlot Slot() const override { return DockSlot::kRight; }

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
    }
  }

  void DrawTransform() {
    if (!EditorManager::ShowComponentWidget<TransformComponent>()) {
      return;
    }

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
      TransformComponent& transform = EditorManager::GetComponent<TransformComponent>();

      ImGui::InputFloat3("Position", glm::value_ptr(transform.position), "%.2f");
      ImGui::InputFloat3("Rotation", glm::value_ptr(transform.rotation), "%.2f");
      ImGui::InputFloat3("Scale", glm::value_ptr(transform.scale), "%.2f");
    }
  }

  void DrawSprite() {
    if (!EditorManager::ShowComponentWidget<SpriteComponent>()) {
      return;
    }

    if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
      SpriteComponent& component = EditorManager::GetComponent<SpriteComponent>();
      rse::Sprite* sprite = component.m_sprite.get();

      if (sprite == nullptr) {
        ImGui::TextDisabled("No sprite assigned.");
      } else {
        // The tint is packed into an unsigned int, so it has to be
        // unpacked for the widget and re-packed when it changes.
        const unsigned int packed = sprite->GetColor();

        float rgba[4] = {
            static_cast<float>((packed >> 0) & 0xFF) / 255.0f,
            static_cast<float>((packed >> 8) & 0xFF) / 255.0f,
            static_cast<float>((packed >> 16) & 0xFF) / 255.0f,
            static_cast<float>((packed >> 24) & 0xFF) / 255.0f,
        };

        if (ImGui::ColorEdit4("Color", rgba)) {
          sprite->SetColor((static_cast<unsigned int>(rgba[0] * 255.0f) << 0) |
                           (static_cast<unsigned int>(rgba[1] * 255.0f) << 8) |
                           (static_cast<unsigned int>(rgba[2] * 255.0f) << 16) |
                           (static_cast<unsigned int>(rgba[3] * 255.0f) << 24));
        }

        const std::string texture_name = sprite->GetTextureName();
        ImGui::LabelText("Texture", "%s", texture_name.empty() ? "(none)" : texture_name.c_str());
      }

      DrawRemoveComponentButton(Components::kSprite, "Remove Sprite");
    }
  }

  void DrawAudio() {
    if (!EditorManager::ShowComponentWidget<AudioComponent>()) {
      return;
    }

    if (ImGui::CollapsingHeader("Audio", ImGuiTreeNodeFlags_DefaultOpen)) {
      AudioComponent& component = EditorManager::GetComponent<AudioComponent>();
      rse::Sound* sound = component.m_sound.get();

      if (sound == nullptr) {
        ImGui::TextDisabled("No sound assigned.");
      } else {
        const std::string file_name = sound->GetFileName();
        ImGui::LabelText("File", "%s", file_name.empty() ? "(none)" : file_name.c_str());

        ImGui::Checkbox("Mute", sound->GetMute());
        ImGui::Checkbox("Play on play", sound->GetPlayTrigger());
        ImGui::Checkbox("Loop", sound->GetLoop());
      }

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
        ImGui::InputFloat("Friction", rigidbody->GetFriction());

        // Combo wants an int, so convert instead of aliasing the enum
        // pointer as an int pointer.
        static const char* items[] = {"b2_staticBody", "b2_kinematicBody", "b2_dynamicBody"};
        b2BodyType* body_type = rigidbody->GetBodyType();
        int type = static_cast<int>(*body_type);

        if (ImGui::Combo("Body Type", &type, items, IM_ARRAYSIZE(items))) {
          *body_type = static_cast<b2BodyType>(type);
        }
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
};
