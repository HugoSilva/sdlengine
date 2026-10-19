#pragma once

#include <core.h>

#include <layout/editor_layout.h>

// Bridges the ECS system list to the editor UI: it is the one system that runs
// in the UI phase, and all it does is hand control to EditorLayout.
class RenderSystem : public BaseSystem {
 public:
  RenderSystem(EditorLayout* layout) : BaseSystem(), m_layout(layout) {}

  void RenderUi() override { m_layout->Draw(); }

 private:
  EditorLayout* m_layout;
};
