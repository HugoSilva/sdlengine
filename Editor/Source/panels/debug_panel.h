#pragma once

#include <core.h>
#include <imgui.h>

#include <layout/editor_panel.h>

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
  DockSlot Slot() const override { return DockSlot::kLeft; }

  void Draw() override {
    if (!ImGui::Begin(Id(), &visible)) {
      ImGui::End();
      return;
    }

    rse::Renderer2D* renderer = Core::GetRenderer();
    float* clear_color = renderer != nullptr ? renderer->GetClearColor() : nullptr;

    if (clear_color != nullptr) {
      ImGui::ColorEdit4("Clear Color", clear_color);
    } else {
      ImGui::TextDisabled("The active renderer exposes no clear colour.");
    }

    ImGui::End();
  }
};
