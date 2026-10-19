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
  const char* id() const override { return "Inspector"; }
  DockSlot slot() const override { return DockSlot::Right; }

  void draw() override {
    if (!ImGui::Begin(id(), &visible)) {
      ImGui::End();
      return;
    }

    if (EditorManager::getSelectedEntity() == INVALID_ENTITY_ID) {
      ImGui::TextDisabled("Nothing selected.");
      ImGui::End();
      return;
    }

    drawEntityHeader();
    drawBasic();
    drawTransform();
    drawSprite();
    drawAudio();
    drawRigidbody();

    if (EditorManager::showComponentWidget<BasicComponent>()) {
      ImGui::Separator();
      drawAddComponentMenu();
    }

    ImGui::End();
  }

 private:
  void drawEntityHeader() {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Entity");
    ImGui::SameLine();
    ImGui::TextDisabled("(%u)", EditorManager::getSelectedEntity());

    if (!EditorManager::showComponentWidget<BasicComponent>()) {
      ImGui::Separator();
      return;
    }

    BasicComponent& basic = EditorManager::getComponent<BasicComponent>();

    char name[128] = "";
    basic.m_Name.copy(name, IM_ARRAYSIZE(name) - 1);

    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputText("##entityName", name, IM_ARRAYSIZE(name))) {
      basic.m_Name = name;
    }

    ImGui::Separator();
  }

  void drawBasic() {
    if (!EditorManager::showComponentWidget<BasicComponent>()) {
      return;
    }

    if (ImGui::CollapsingHeader("Basic", ImGuiTreeNodeFlags_DefaultOpen)) {
      BasicComponent& basic = EditorManager::getComponent<BasicComponent>();

      bool isStatic = basic.m_IsStatic;
      if (ImGui::Checkbox("Static", &isStatic)) {
        basic.m_IsStatic = isStatic;
      }
    }
  }

  void drawTransform() {
    if (!EditorManager::showComponentWidget<TransformComponent>()) {
      return;
    }

    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
      TransformComponent& transform = EditorManager::getComponent<TransformComponent>();

      ImGui::InputFloat3("Position", glm::value_ptr(transform.position), "%.2f");
      ImGui::InputFloat3("Rotation", glm::value_ptr(transform.rotation), "%.2f");
      ImGui::InputFloat3("Scale", glm::value_ptr(transform.scale), "%.2f");
    }
  }

  void drawSprite() {
    if (!EditorManager::showComponentWidget<SpriteComponent>()) {
      return;
    }

    if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
      SpriteComponent& component = EditorManager::getComponent<SpriteComponent>();
      rse::Sprite* sprite = component.m_Sprite.get();

      if (sprite == nullptr) {
        ImGui::TextDisabled("No sprite assigned.");
      } else {
        // The tint is packed into an unsigned int, so it has to be
        // unpacked for the widget and re-packed when it changes.
        const unsigned int packed = sprite->getColor();

        float rgba[4] = {
            static_cast<float>((packed >> 0) & 0xFF) / 255.0f,
            static_cast<float>((packed >> 8) & 0xFF) / 255.0f,
            static_cast<float>((packed >> 16) & 0xFF) / 255.0f,
            static_cast<float>((packed >> 24) & 0xFF) / 255.0f,
        };

        if (ImGui::ColorEdit4("Color", rgba)) {
          sprite->setColor((static_cast<unsigned int>(rgba[0] * 255.0f) << 0) |
                           (static_cast<unsigned int>(rgba[1] * 255.0f) << 8) |
                           (static_cast<unsigned int>(rgba[2] * 255.0f) << 16) |
                           (static_cast<unsigned int>(rgba[3] * 255.0f) << 24));
        }

        const std::string textureName = sprite->getTextureName();
        ImGui::LabelText("Texture", "%s", textureName.empty() ? "(none)" : textureName.c_str());
      }

      drawRemoveComponentButton(Components::Sprite, "Remove Sprite");
    }
  }

  void drawAudio() {
    if (!EditorManager::showComponentWidget<AudioComponent>()) {
      return;
    }

    if (ImGui::CollapsingHeader("Audio", ImGuiTreeNodeFlags_DefaultOpen)) {
      AudioComponent& component = EditorManager::getComponent<AudioComponent>();
      rse::Sound* sound = component.m_Sound.get();

      if (sound == nullptr) {
        ImGui::TextDisabled("No sound assigned.");
      } else {
        const std::string fileName = sound->getFileName();
        ImGui::LabelText("File", "%s", fileName.empty() ? "(none)" : fileName.c_str());

        ImGui::Checkbox("Mute", sound->getMute());
        ImGui::Checkbox("Play on play", sound->getPlayTrigger());
        ImGui::Checkbox("Loop", sound->getLoop());
      }

      drawRemoveComponentButton(Components::Audio, "Remove Audio");
    }
  }

  void drawRigidbody() {
    if (!EditorManager::showComponentWidget<RigidBodyComponent>()) {
      return;
    }

    if (ImGui::CollapsingHeader("Rigidbody", ImGuiTreeNodeFlags_DefaultOpen)) {
      RigidBodyComponent& component = EditorManager::getComponent<RigidBodyComponent>();
      Rigidbody* rigidbody = component.m_Rigidbody.get();

      if (rigidbody == nullptr) {
        ImGui::TextDisabled("No rigidbody assigned.");
      } else {
        ImGui::InputFloat("Density", rigidbody->getDensity());
        ImGui::InputFloat("Friction", rigidbody->getFriction());

        // Combo wants an int, so convert instead of aliasing the enum
        // pointer as an int pointer.
        static const char* items[] = {"b2_staticBody", "b2_kinematicBody", "b2_dynamicBody"};
        b2BodyType* bodyType = rigidbody->getBodyType();
        int type = static_cast<int>(*bodyType);

        if (ImGui::Combo("Body Type", &type, items, IM_ARRAYSIZE(items))) {
          *bodyType = static_cast<b2BodyType>(type);
        }
      }

      drawRemoveComponentButton(Components::Rigidbody, "Remove Rigidbody");
    }
  }

  void drawRemoveComponentButton(Components component, const char* label) {
    ImGui::Spacing();
    if (ImGui::SmallButton(label)) {
      EventManager::getEventDispatcher()->trigger<EditorManager::RemoveComponent>(component);
    }
  }

  void drawAddComponentMenu() {
    if (ImGui::Button("Add Component")) {
      ImGui::OpenPopup("add_component");
    }

    if (ImGui::BeginPopup("add_component")) {
      if (ImGui::MenuItem("Audio")) {
        EventManager::getEventDispatcher()->trigger<EditorManager::AddComponent>(Components::Audio);
      }
      if (ImGui::MenuItem("Ridigbody")) {
        EventManager::getEventDispatcher()->trigger<EditorManager::AddComponent>(Components::Rigidbody);
      }
      if (ImGui::MenuItem("Sprite")) {
        EventManager::getEventDispatcher()->trigger<EditorManager::AddComponent>(Components::Sprite);
      }
      ImGui::EndPopup();
    }
  }
};
