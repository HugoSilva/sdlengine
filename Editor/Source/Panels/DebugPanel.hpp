#pragma once

#include <Core.h>
#include <imgui.h>

#include "../Layout/EditorPanel.hpp"

// ---------------------------------------------------------------------------
// Home for the engine's debug gizmos.
//
// OpenGLRenderer used to submit its clear-colour ImGui widget itself, which
// landed in ImGui's implicit "Debug" window: a floating window on top of the
// dock space with no panel to put it in. The renderer now just exposes the
// colour and this panel draws it.
// ---------------------------------------------------------------------------
class DebugPanel : public EditorPanel {
 public:
  const char* Id() const override { return "Debug"; }
  DockSlot Slot() const override { return DockSlot::Left; }

  void Draw() override {
    if (!ImGui::Begin(Id(), &visible)) {
      ImGui::End();
      return;
    }

    rse::Renderer2D* renderer = Core::GetRenderer();
    float* clearColor = renderer != nullptr ? renderer->GetClearColor() : nullptr;

    if (clearColor != nullptr) {
      ImGui::ColorEdit4("Clear Color", clearColor);
    } else {
      ImGui::TextDisabled("The active renderer exposes no clear colour.");
    }

    ImGui::End();
  }
};
